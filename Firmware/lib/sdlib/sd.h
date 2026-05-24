// Copyright (c) Kuba Szczodrzyński 2025-9-9.

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <hardware/clocks.h>
#include <hardware/dma.h>
#include <hardware/spi.h>
#include <pico/stdlib.h>
#include <pico/time.h>

typedef struct sd_t sd_t;

typedef enum {
	SD_ERR_OK = 0,
	SD_ERR_CMD_TIMEOUT,
	SD_ERR_BUSY_TIMEOUT,
	SD_ERR_BAD_R1,
	SD_ERR_INIT_IDLE_FAILED,
	SD_ERR_INIT_READY_FAILED,
	SD_ERR_INIT_BAD_ECHO,
	SD_ERR_INIT_POWER_UP_BUSY,
	SD_ERR_READ_TIMEOUT,
	SD_ERR_READ_BAD_TOKEN,
	SD_ERR_READ_BAD_CRC,
	SD_ERR_WRITE_TIMEOUT,
	SD_ERR_WRITE_BAD_TOKEN,
	SD_ERR_WRITE_BAD_CRC,
	SD_ERR_WRITE_ERROR,
	SD_ERR_NOT_INITIALIZED,
	SD_ERR_OUT_OF_RANGE,
} sd_err_t;

typedef void (*sd_power_on_t)(sd_t *sd);
typedef void (*sd_command_t)(sd_t *sd, uint8_t cmd, uint32_t arg, uint8_t crc);
typedef sd_err_t (*sd_wait_busy_t)(sd_t *sd, uint8_t *resp, uint32_t timeout);
typedef sd_err_t (*sd_read_r1_t)(sd_t *sd, uint8_t *resp);
typedef sd_err_t (*sd_read_rx_t)(sd_t *sd, uint8_t *resp, uint32_t len);
typedef sd_err_t (*sd_read_data_t)(sd_t *sd, uint8_t *data, uint32_t len);
typedef sd_err_t (*sd_write_data_t)(sd_t *sd, const uint8_t *data, uint32_t len, uint8_t token);
typedef sd_err_t (*sd_write_stop_t)(sd_t *sd);
typedef bool (*sd_interrupt_t)(void *param);

typedef struct sd_t {
	bool init_ok;
	bool ccs;
	uint8_t csd[16];
	uint8_t cid[16];

	char name[6];
	uint32_t sectors;

	sd_power_on_t power_on;
	sd_command_t command;
	sd_wait_busy_t wait_busy;
	sd_read_r1_t read_r1;
	sd_read_rx_t read_rx;
	sd_read_data_t read_data;
	sd_write_data_t write_data;
	sd_write_stop_t write_stop;

	sd_interrupt_t interrupt_check;
	void *interrupt_param;
	volatile bool interrupted;

	struct {
		spi_inst_t *inst;
		uint sck_pin;
		uint tx_pin;
		uint rx_pin;
		uint cs_pin;
		bool use_dma;
		uint tx_dma;
		uint rx_dma;
		dma_channel_config tx_config;
		dma_channel_config rx_config;
	} spi;
} sd_t;

// sd.c
sd_err_t sd_init(sd_t *sd);
sd_err_t sd_read(sd_t *sd, uint8_t *data, uint32_t start, uint32_t count);
sd_err_t sd_write(sd_t *sd, const uint8_t *data, uint32_t start, uint32_t count);

// sd_spi.c
void sd_spi_init(sd_t *sd, uint32_t freq);

// sd_debug.c
const char *sd_error_str(sd_err_t err);
void sd_print_r1(const uint8_t *resp, bool no_bytes);
void sd_print_r3(const uint8_t *resp);
void sd_print_r7(const uint8_t *resp);
