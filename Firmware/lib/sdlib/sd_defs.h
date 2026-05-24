// Copyright (c) Kuba Szczodrzyński 2025-9-10.

#pragma once

#define CMD0_GO_IDLE_STATE		   0  // R1
#define CMD6_SWITCH_FUNC		   6  // R1
#define CMD8_SEND_IF_COND		   8  // R7
#define CMD9_SEND_CSD			   9  // R1
#define CMD10_SEND_CID			   10 // R1
#define CMD12_STOP_TRANSMISSION	   12 // R1b
#define CMD13_SEND_STATUS		   13 // R1
#define CMD16_SET_BLOCKLEN		   16 // R1
#define CMD17_READ_SINGLE_BLOCK	   17 // R1
#define CMD18_READ_MULTIPLE_BLOCK  18 // R1
#define CMD24_WRITE_BLOCK		   24 // R1
#define CMD25_WRITE_MULTIPLE_BLOCK 25 // R1
#define CMD27_PROGRAM_CSD		   27 // R1
#define CMD28_SET_WRITE_PROT	   28 // R1b
#define CMD29_CLR_WRITE_PROT	   29 // R1b
#define CMD30_SEND_WRITE_PROT	   30 // R1
#define CMD32_ERASE_WR_BLK_START   32 // R1
#define CMD33_ERASE_WR_BLK_END	   33 // R1
#define CMD38_ERASE				   38 // R1b
#define CMD42_LOCK_UNLOCK		   42 // R1
#define CMD55_APP_CMD			   55 // R1
#define CMD56_GEN_CMD			   56 // R1
#define CMD58_READ_OCR			   58 // R3
#define CMD59_CRC_ON_OFF		   59 // R1

#define ACMD13_SD_STATUS			  13 // R2
#define ACMD22_SEND_NUM_WR_BLOCKS	  22 // R1
#define ACMD23_SET_WR_BLK_ERASE_COUNT 23 // R1
#define ACMD41_SD_SEND_OP_COND		  41 // R1
#define ACMD42_SET_CLR_CARD_DETECT	  42 // R1
#define ACMD51_SEND_SCR				  51 // R1

#define CTRL_TOKEN_START		  0xFE
#define CTRL_TOKEN_START_MULTIPLE 0xFC
#define CTRL_TOKEN_STOP			  0xFD
#define CTRL_DATA_ACCEPTED		  0x05
#define CTRL_DATA_CRC_ERROR		  0x0B
#define CTRL_DATA_WRITE_ERROR	  0x0D

#define R1_INVALID		   0x80
#define R1_PARAM_ERROR	   0x40
#define R1_ADDR_ERROR	   0x20
#define R1_ERASE_SEQ_ERROR 0x10
#define R1_CRC_ERROR	   0x08
#define R1_ILLEGAL_CMD	   0x04
#define R1_ERASE_RESET	   0x02
#define R1_IN_IDLE		   0x01
#define R1_READY		   0x00

#define R3_POWER_UP 0x80
#define R3_CCS		0x40

#define SD_BLOCK_LEN 512
