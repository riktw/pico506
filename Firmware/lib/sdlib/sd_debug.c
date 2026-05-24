// Copyright (c) Kuba Szczodrzyński 2025-9-9.

#include "sd_priv.h"

#define ENUM_STR(name) \
	case name:         \
		return #name

const char *sd_error_str(sd_err_t err) {
	switch (err) {
		ENUM_STR(SD_ERR_OK);
		ENUM_STR(SD_ERR_CMD_TIMEOUT);
		ENUM_STR(SD_ERR_BUSY_TIMEOUT);
		ENUM_STR(SD_ERR_BAD_R1);
		ENUM_STR(SD_ERR_INIT_IDLE_FAILED);
		ENUM_STR(SD_ERR_INIT_READY_FAILED);
		ENUM_STR(SD_ERR_INIT_BAD_ECHO);
		ENUM_STR(SD_ERR_INIT_POWER_UP_BUSY);
		ENUM_STR(SD_ERR_READ_TIMEOUT);
		ENUM_STR(SD_ERR_READ_BAD_TOKEN);
		ENUM_STR(SD_ERR_READ_BAD_CRC);
		ENUM_STR(SD_ERR_WRITE_TIMEOUT);
		ENUM_STR(SD_ERR_WRITE_BAD_TOKEN);
		ENUM_STR(SD_ERR_WRITE_BAD_CRC);
		ENUM_STR(SD_ERR_WRITE_ERROR);
		ENUM_STR(SD_ERR_NOT_INITIALIZED);
		ENUM_STR(SD_ERR_OUT_OF_RANGE);
	}
}

void sd_print_r1(const uint8_t *resp, bool no_bytes) {
	if (!no_bytes)
		LT_I("R1 = %02x", resp[0]);
	if (resp[0] & R1_INVALID)
		LT_E("     \\ Invalid R1");
	if (resp[0] == 0)
		LT_I("     \\ Card Ready");
	if (resp[0] & R1_PARAM_ERROR)
		LT_W("     \\ Parameter Error");
	if (resp[0] & R1_ADDR_ERROR)
		LT_W("     \\ Address Error");
	if (resp[0] & R1_ERASE_SEQ_ERROR)
		LT_W("     \\ Erase Sequence Error");
	if (resp[0] & R1_CRC_ERROR)
		LT_W("     \\ CRC Error");
	if (resp[0] & R1_ILLEGAL_CMD)
		LT_W("     \\ Illegal Command");
	if (resp[0] & R1_ERASE_RESET)
		LT_W("     \\ Erase Reset Error");
	if (resp[0] & R1_IN_IDLE)
		LT_I("     \\ In Idle State");
}

void sd_print_r3(const uint8_t *resp) {
	LT_I("R3 = %02x %02x %02x %02x %02x", resp[0], resp[1], resp[2], resp[3], resp[4]);
	if (resp[0] > 1) {
		sd_print_r1(resp, true);
		return;
	}
	if (resp[3] & 0x80)
		LT_I("     |  |  |  \\ VDD Window: 2.7-2.8 V");
	if (resp[2] & 0x01)
		LT_I("     |  |  \\ VDD Window: 2.8-2.9 V");
	if (resp[2] & 0x02)
		LT_I("     |  |  \\ VDD Window: 2.9-3.0 V");
	if (resp[2] & 0x04)
		LT_I("     |  |  \\ VDD Window: 3.0-3.1 V");
	if (resp[2] & 0x08)
		LT_I("     |  |  \\ VDD Window: 3.1-3.2 V");
	if (resp[2] & 0x10)
		LT_I("     |  |  \\ VDD Window: 3.2-3.3 V");
	if (resp[2] & 0x20)
		LT_I("     |  |  \\ VDD Window: 3.3-3.4 V");
	if (resp[2] & 0x40)
		LT_I("     |  |  \\ VDD Window: 3.4-3.5 V");
	if (resp[2] & 0x80)
		LT_I("     |  |  \\ VDD Window: 3.5-3.6 V");
	if (resp[1] & 0x80) {
		LT_I("     |  \\ Power Up: Ready, CCS: %d", resp[1] & 0x40 ? 1 : 0);
	} else {
		LT_W("     |  \\ Power Up: Busy");
	}
	sd_print_r1(resp, true);
}

void sd_print_r7(const uint8_t *resp) {
	LT_I("R7 = %02x %02x %02x %02x %02x", resp[0], resp[1], resp[2], resp[3], resp[4]);
	if (resp[0] > 1) {
		sd_print_r1(resp, true);
		return;
	}
	LT_I("     |  |  |  |  \\ Echo: %02x", resp[4]);
	switch (resp[3] & 0x1F) {
		case 0x01:
			LT_I("     |  |  |  \\ Voltage: 2.7-3.6 V");
			break;
		case 0x02:
			LT_I("     |  |  |  \\ Voltage: Low Voltage");
			break;
		case 0x04:
			LT_W("     |  |  |  \\ Voltage: Reserved 1");
			break;
		case 0x08:
			LT_W("     |  |  |  \\ Voltage: Reserved 2");
			break;
		default:
			LT_W("     |  |  |  \\ Voltage: Unknown");
			break;
	}
	LT_I("     |  \\ Command Version: %x", (resp[1] >> 4) & 0xF0);
	sd_print_r1(resp, true);
}
