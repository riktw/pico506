// Copyright (c) Kuba Szczodrzyński 2025-9-9.

#include "sd_priv.h"

static void sd_spi_power_on(sd_t *sd);
static void sd_spi_command(sd_t *sd, uint8_t cmd, uint32_t arg, uint8_t crc);
static sd_err_t sd_spi_wait_busy(sd_t *sd, uint8_t *resp, uint32_t timeout);
static sd_err_t sd_spi_read_r1(sd_t *sd, uint8_t *resp);
static sd_err_t sd_spi_read_rx(sd_t *sd, uint8_t *resp, uint32_t len);
static sd_err_t sd_spi_read_data(sd_t *sd, uint8_t *data, uint32_t len);
static sd_err_t sd_spi_write_data(sd_t *sd, const uint8_t *data, uint32_t len, uint8_t token);
static sd_err_t sd_spi_write_stop(sd_t *sd);

void sd_spi_init(sd_t *sd, uint32_t freq) {
	sd->power_on   = sd_spi_power_on;
	sd->command	   = sd_spi_command;
	sd->wait_busy  = sd_spi_wait_busy;
	sd->read_r1	   = sd_spi_read_r1;
	sd->read_rx	   = sd_spi_read_rx;
	sd->read_data  = sd_spi_read_data;
	sd->write_data = sd_spi_write_data;
	sd->write_stop = sd_spi_write_stop;
	sd->spi.inst   = spi0;

	spi_init(sd->spi.inst, freq);
	spi_set_format(sd->spi.inst, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
	gpio_set_function(sd->spi.sck_pin, GPIO_FUNC_SPI);
	gpio_set_function(sd->spi.tx_pin, GPIO_FUNC_SPI);
	gpio_set_function(sd->spi.rx_pin, GPIO_FUNC_SPI);
	gpio_set_function(sd->spi.cs_pin, GPIO_FUNC_SPI);
	gpio_set_pulls(sd->spi.rx_pin, true, false);

	sd->spi.use_dma	  = false;
	sd->spi.tx_dma	  = dma_claim_unused_channel(true);
	sd->spi.rx_dma	  = dma_claim_unused_channel(true);
	sd->spi.tx_config = dma_channel_get_default_config(sd->spi.tx_dma);
	sd->spi.rx_config = dma_channel_get_default_config(sd->spi.rx_dma);

	channel_config_set_transfer_data_size(&sd->spi.tx_config, DMA_SIZE_8);
	channel_config_set_transfer_data_size(&sd->spi.rx_config, DMA_SIZE_8);
	channel_config_set_dreq(&sd->spi.tx_config, spi_get_dreq(sd->spi.inst, true));
	channel_config_set_dreq(&sd->spi.rx_config, spi_get_dreq(sd->spi.inst, false));
	channel_config_set_sniff_enable(&sd->spi.tx_config, true);
	channel_config_set_sniff_enable(&sd->spi.rx_config, true);
	channel_config_set_write_increment(&sd->spi.tx_config, false);
	channel_config_set_read_increment(&sd->spi.rx_config, false);
}

static void sd_spi_power_on(sd_t *sd) {
	uint8_t resp[10];
	gpio_set_function(sd->spi.cs_pin, GPIO_FUNC_SIO);
	gpio_set_dir(sd->spi.cs_pin, true);
	gpio_put(sd->spi.cs_pin, 1);
	sleep_ms(10);
	spi_read_blocking(sd->spi.inst, 0xFF, resp, 10);
	gpio_set_function(sd->spi.cs_pin, GPIO_FUNC_SPI);
}

static void sd_spi_command(sd_t *sd, uint8_t cmd, uint32_t arg, uint8_t crc) {
	uint8_t buf[] = {
		0xFF,
		0x40 | cmd,
		arg >> 24,
		arg >> 16,
		arg >> 8,
		arg >> 0,
		crc | 0x01,
	};
	spi_write_blocking(sd->spi.inst, buf, sizeof(buf));
}

static sd_err_t sd_spi_wait_busy(sd_t *sd, uint8_t *resp, uint32_t timeout) {
	FOR_TIMEOUT(timeout) {
		spi_read_blocking(sd->spi.inst, 0xFF, resp, 1);
		if (resp[0] == 0xFF)
			return SD_ERR_OK;
	}
	return SD_ERR_BUSY_TIMEOUT;
}

static sd_err_t sd_spi_read_r1(sd_t *sd, uint8_t *resp) {
	for (uint32_t i = 0; i < 8; i++) {
		spi_read_blocking(sd->spi.inst, 0xFF, resp, 1);
		if (resp[0] != 0xFF) {
			if (resp[0] > 1)
				return SD_ERR_BAD_R1;
			return SD_ERR_OK;
		}
	}
	return SD_ERR_CMD_TIMEOUT;
}

static sd_err_t sd_spi_read_rx(sd_t *sd, uint8_t *resp, uint32_t len) {
	sd_err_t err;
	if ((err = sd_spi_read_r1(sd, resp)))
		return err;
	spi_read_blocking(sd->spi.inst, 0xFF, &resp[1], len - 1);
	return SD_ERR_OK;
}

static sd_err_t sd_spi_read_data(sd_t *sd, uint8_t *data, uint32_t len) {
	FOR_TIMEOUT(100) {
		// wait for a start token
		uint8_t resp;
		spi_read_blocking(sd->spi.inst, 0xFF, &resp, 1);
		if (resp == 0xFF)
			continue;
		if (resp == CTRL_TOKEN_START)
			goto token_ok;
		return SD_ERR_READ_BAD_TOKEN;
	}
	return SD_ERR_READ_TIMEOUT;

token_ok:
	// read block data and CRC
	if (!sd->spi.use_dma) {
		spi_read_blocking(sd->spi.inst, 0xFF, data, len);
		// in non-DMA mode, just ignore the CRC
		uint8_t crc[2];
		spi_read_blocking(sd->spi.inst, 0xFF, crc, 2);
	} else {
		channel_config_set_read_increment(&sd->spi.tx_config, false);
		channel_config_set_write_increment(&sd->spi.rx_config, true);
		static uint8_t fill = 0xFF;
		dma_channel_configure(sd->spi.tx_dma, &sd->spi.tx_config, &spi0_hw->dr, &fill, len, false);
		dma_channel_configure(sd->spi.rx_dma, &sd->spi.rx_config, data, &spi0_hw->dr, len, false);
		// switch the DMA sniffer to RX and reset it
		dma_sniffer_enable(sd->spi.rx_dma, DMA_SNIFF_CTRL_CALC_VALUE_CRC16, false);
		dma_sniffer_set_data_accumulator(0);
		// start TX and RX channels and wait for them to finish
		dma_start_channel_mask((1 << sd->spi.tx_dma) | (1 << sd->spi.rx_dma));
		dma_channel_wait_for_finish_blocking(sd->spi.tx_dma);
		dma_channel_wait_for_finish_blocking(sd->spi.rx_dma);
		// in DMA mode the CRC16 can be calculated in hardware (for free)
		uint8_t crc[2];
		spi_read_blocking(sd->spi.inst, 0xFF, crc, 2);
		uint32_t crc_value = (crc[0] << 8) | (crc[1] << 0);
		if (crc_value != dma_sniffer_get_data_accumulator())
			return SD_ERR_READ_BAD_CRC;
	}
	return SD_ERR_OK;
}

static sd_err_t sd_spi_write_data(sd_t *sd, const uint8_t *data, uint32_t len, uint8_t token) {
	// write a start token
	spi_write_blocking(sd->spi.inst, &token, 1);
	// write block data and CRC
	if (!sd->spi.use_dma) {
		spi_write_blocking(sd->spi.inst, data, len);
		// in non-DMA mode, write a dummy CRC
		uint16_t crc = 0xFFFF;
		spi_write_blocking(sd->spi.inst, (void *)&crc, 2);
	} else {
		channel_config_set_read_increment(&sd->spi.tx_config, true);
		channel_config_set_write_increment(&sd->spi.rx_config, false);
		static uint8_t dummy;
		dma_channel_configure(sd->spi.tx_dma, &sd->spi.tx_config, &spi0_hw->dr, data, len, false);
		dma_channel_configure(sd->spi.rx_dma, &sd->spi.rx_config, &dummy, &spi0_hw->dr, len, false);
		// switch the DMA sniffer to TX and reset it
		dma_sniffer_enable(sd->spi.tx_dma, DMA_SNIFF_CTRL_CALC_VALUE_CRC16, false);
		dma_sniffer_set_data_accumulator(0);
		// start TX and RX channels and wait for them to finish
		dma_start_channel_mask((1 << sd->spi.tx_dma) | (1 << sd->spi.rx_dma));
		dma_channel_wait_for_finish_blocking(sd->spi.tx_dma);
		dma_channel_wait_for_finish_blocking(sd->spi.rx_dma);
		// in DMA mode the CRC16 can be calculated in hardware (for free)
		uint32_t crc = dma_sniffer_get_data_accumulator();
		crc			 = (crc >> 8) | (crc << 8);
		spi_write_blocking(sd->spi.inst, (void *)&crc, 2);
	}

	FOR_TIMEOUT(250) {
		// wait for a data accepted token
		uint8_t resp;
		spi_read_blocking(sd->spi.inst, 0xFF, &resp, 1);
		if (resp == 0xFF)
			continue;
		resp = resp & 0x1F;
		if (resp == CTRL_DATA_ACCEPTED)
			return SD_ERR_OK;
		if (resp == CTRL_DATA_CRC_ERROR)
			return SD_ERR_WRITE_BAD_CRC;
		if (resp == CTRL_DATA_WRITE_ERROR)
			return SD_ERR_WRITE_ERROR;
		return SD_ERR_WRITE_BAD_TOKEN;
	}
	return SD_ERR_WRITE_TIMEOUT;
}

static sd_err_t sd_spi_write_stop(sd_t *sd) {
	uint8_t resp;
	// write a stop token
	spi_read_blocking(sd->spi.inst, CTRL_TOKEN_STOP, &resp, 1);
	// read one dummy byte
	spi_read_blocking(sd->spi.inst, 0xFF, &resp, 1);
	return SD_ERR_OK;
}
