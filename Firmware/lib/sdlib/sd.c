// Copyright (c) Kuba Szczodrzyński 2025-9-9.

#include "sd_priv.h"

sd_err_t sd_init(sd_t *sd) {
	sd_err_t err = SD_ERR_OK;
	uint8_t resp[16];

	// send power up sequence
	sd->init_ok = false;
	sd->power_on(sd);

	FOR_TIMEOUT(100) {
		sleep_ms(1);
		// send CMD0_GO_IDLE_STATE and read R1
		sd->command(sd, CMD0_GO_IDLE_STATE, 0, 0x94);
		if ((err = sd->read_r1(sd, resp)))
			continue;
		DEBUG_INIT(sd_print_r1(resp, false));
		// break if already IDLE
		if (resp[0] == R1_IN_IDLE)
			break;
		// otherwise indicate error and try again
		err = SD_ERR_INIT_IDLE_FAILED;
	}
	if (err)
		return err;
	sleep_ms(1);

	// send CMD8_SEND_IF_COND and read R7
	sd->command(sd, CMD8_SEND_IF_COND, (0x1 << 8) | (0xAA << 0), 0x86);
	if ((err = sd->read_rx(sd, resp, 5)))
		return err;
	DEBUG_INIT(sd_print_r7(resp));
	// return if not IDLE
	if (resp[0] != R1_IN_IDLE)
		return SD_ERR_INIT_IDLE_FAILED;
	// return if echo is bad
	if (resp[4] != 0xAA)
		return SD_ERR_INIT_BAD_ECHO;

	FOR_TIMEOUT(1000) {
		sleep_ms(1);
		// send CMD55_APP_CMD and read R1
		sd->command(sd, CMD55_APP_CMD, 0, 0);
		if ((err = sd->read_r1(sd, resp)))
			continue;
		DEBUG_INIT(sd_print_r1(resp, false));
		// send ACMD41_SD_SEND_OP_COND and read R1
		sd->command(sd, ACMD41_SD_SEND_OP_COND, (1 << 30), 0);
		if ((err = sd->read_r1(sd, resp)))
			continue;
		DEBUG_INIT(sd_print_r1(resp, false));
		// break if already READY
		if (resp[0] == R1_READY)
			break;
		// otherwise indicate error and try again
		err = SD_ERR_INIT_READY_FAILED;
	}
	if (err)
		return err;
	sleep_ms(1);

	// wait for the line to be idle
	if ((err = sd->wait_busy(sd, resp, 100)))
		return err;

	// send CMD58_READ_OCR and read R3
	sd->command(sd, CMD58_READ_OCR, 0, 0);
	if ((err = sd->read_rx(sd, resp, 5)))
		return err;
	DEBUG_INIT(sd_print_r3(resp));
	// return if power up status is BUSY
	if ((resp[1] & R3_POWER_UP) != R3_POWER_UP)
		return SD_ERR_INIT_POWER_UP_BUSY;

	// save CCS bit
	sd->ccs = (resp[1] & R3_CCS) == R3_CCS;

	// read CSD register
	sd->command(sd, CMD9_SEND_CSD, 0, 0);
	if ((err = sd->read_r1(sd, resp)))
		return err;
	if ((err = sd->read_data(sd, resp, 16)))
		return err;
	memcpy(sd->csd, resp, sizeof(sd->csd));

	// read CID register
	sd->command(sd, CMD10_SEND_CID, 0, 0);
	if ((err = sd->read_r1(sd, resp)))
		return err;
	if ((err = sd->read_data(sd, resp, 16)))
		return err;
	memcpy(sd->cid, resp, sizeof(sd->cid));

	// get product name from CID
	memcpy(sd->name, &sd->cid[3], 5);
	sd->name[5] = '\0';

	// get capacity info from CSD
	if ((sd->csd[0] & 0xC0) == 0x00) {
		// CSD Version 1.0
		uint32_t c_size		 = ((sd->csd[6] & 0b11) << 10) | (sd->csd[7] << 2) | (sd->csd[8] >> 6);
		uint32_t c_size_mult = ((sd->csd[9] & 0b11) << 1) | (sd->csd[10] >> 7);
		uint32_t read_bl_len = sd->csd[5] & 0b1111;
		uint32_t mult		 = 1 << (c_size_mult + 2);
		uint32_t block_nr	 = (c_size + 1) * mult;
		uint32_t block_len	 = 1 << (read_bl_len);
		sd->sectors			 = block_nr * block_len / 512;
	} else if ((sd->csd[0] & 0xC0) == 0x40) {
		// CSD Version 2.0
		uint32_t c_size = ((sd->csd[7] & 0b111111) << 16) | (sd->csd[8] << 8) | (sd->csd[9] << 0);
		sd->sectors		= (c_size + 1) * 1024;
	}

	// set block length on non-SDHC cards
	if (!sd->ccs) {
		sd->command(sd, CMD16_SET_BLOCKLEN, 512, 0);
		if ((err = sd->read_r1(sd, resp)))
			return err;
	}

	sd->init_ok = true;
	return SD_ERR_OK;
}

sd_err_t sd_read(sd_t *sd, uint8_t *data, uint32_t start, uint32_t count) {
	// precondition checks
	if (!sd->init_ok)
		return SD_ERR_NOT_INITIALIZED;
	if (start + count > sd->sectors)
		return SD_ERR_OUT_OF_RANGE;
	if (count == 0)
		return SD_ERR_OK;

	sd_err_t err;
	uint8_t resp[1];
	// calculate address based on CCS bit
	uint32_t addr = sd->ccs ? (start) : (start * SD_BLOCK_LEN);

	// request to read single or multiple blocks
	if (count == 1)
		sd->command(sd, CMD17_READ_SINGLE_BLOCK, addr, 0);
	else
		sd->command(sd, CMD18_READ_MULTIPLE_BLOCK, addr, 0);
	if ((err = sd->read_r1(sd, resp)))
		return err;

	// transfer blocks one by one
	for (uint32_t i = 0; i < count; i++) {
		if ((err = sd->read_data(sd, data, SD_BLOCK_LEN)))
			break;
		data += SD_BLOCK_LEN;
		if (sd->interrupt_check && sd->interrupt_check(sd->interrupt_param)) {
			sd->interrupted = true;
			break;
		}
	}

	// if multiple blocks, stop transmission after finished/errored out
	if (count > 1) {
		sd->command(sd, CMD12_STOP_TRANSMISSION, 0, 0);
		sd->read_r1(sd, resp);
		// CMD12 responds with R1b
		if ((err = sd->wait_busy(sd, resp, 100)))
			return err;
	}
	return err;
}

sd_err_t sd_write(sd_t *sd, const uint8_t *data, uint32_t start, uint32_t count) {
	// precondition checks
	if (!sd->init_ok)
		return SD_ERR_NOT_INITIALIZED;
	if (start + count > sd->sectors)
		return SD_ERR_OUT_OF_RANGE;
	if (count == 0)
		return SD_ERR_OK;

	sd_err_t err;
	uint8_t resp[1];
	// calculate address based on CCS bit
	uint32_t addr = sd->ccs ? (start) : (start * SD_BLOCK_LEN);

	// pre-erase blocks if writing multiple
	if (count > 1) {
		sd->command(sd, CMD55_APP_CMD, 0, 0);
		if ((err = sd->read_r1(sd, resp)))
			return err;
		sd->command(sd, ACMD23_SET_WR_BLK_ERASE_COUNT, count, 0);
		if ((err = sd->read_r1(sd, resp)))
			return err;
	}

	// request to write single or multiple blocks
	uint8_t token = count == 1 ? CTRL_TOKEN_START : CTRL_TOKEN_START_MULTIPLE;
	if (count == 1)
		sd->command(sd, CMD24_WRITE_BLOCK, addr, 0);
	else
		sd->command(sd, CMD25_WRITE_MULTIPLE_BLOCK, addr, 0);
	if ((err = sd->read_r1(sd, resp)))
		return err;

	// transfer blocks one by one
	for (uint32_t i = 0; i < count; i++) {
		if ((err = sd->write_data(sd, data, SD_BLOCK_LEN, token)))
			break;
		if ((err = sd->wait_busy(sd, resp, 500)))
			return err;
		data += SD_BLOCK_LEN;
		if (sd->interrupt_check && sd->interrupt_check(sd->interrupt_param)) {
			sd->interrupted = true;
			break;
		}
	}

	// if multiple blocks, stop transmission after finished/errored out
	if (count > 1) {
		sd->write_stop(sd);
		// stop transmission token sends busy response
		if ((err = sd->wait_busy(sd, resp, 100)))
			return err;
	}
	return err;
}
