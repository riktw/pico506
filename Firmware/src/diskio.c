// Copyright (c) Kuba Szczodrzyński 2025-9-11.

#include "pico506.h"

#include <diskio.h>

static sd_t *sd = NULL;

void disk_set_sd(sd_t *new_sd) {
	sd = new_sd;
}

DSTATUS disk_status(BYTE pdrv) {
	if (pdrv != 0 || sd == NULL)
		return STA_NOINIT;
	return sd->init_ok ? 0 : STA_NOINIT;
}

DSTATUS disk_initialize(BYTE pdrv) {
	if (pdrv != 0 || sd == NULL)
		return STA_NOINIT;

	sd_err_t err = sd_init(sd);
	if (err)
		LT_E("Disk init error: %u", err);

	return sd->init_ok ? 0 : STA_NOINIT;
}

DRESULT disk_read(BYTE pdrv, BYTE *buff, LBA_t sector, UINT count) {
	if (pdrv != 0 || sd == NULL)
		return RES_NOTRDY;

	switch (sd_read(sd, buff, sector, count)) {
		case SD_ERR_OK:
			return RES_OK;
		case SD_ERR_OUT_OF_RANGE:
			return RES_PARERR;
		case SD_ERR_NOT_INITIALIZED:
			return RES_NOTRDY;
		default:
			return RES_ERROR;
	}
}

#if FF_FS_READONLY == 0
DRESULT disk_write(BYTE pdrv, const BYTE *buff, LBA_t sector, UINT count) {
	if (pdrv != 0 || sd == NULL)
		return RES_NOTRDY;

	switch (sd_write(sd, buff, sector, count)) {
		case SD_ERR_OK:
			return RES_OK;
		case SD_ERR_OUT_OF_RANGE:
			return RES_PARERR;
		case SD_ERR_NOT_INITIALIZED:
			return RES_NOTRDY;
		default:
			return RES_ERROR;
	}
}
#endif

DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void *buff) {
	if (pdrv != 0 || sd == NULL)
		return RES_NOTRDY;
	// used for CTRL_SYNC only
	return RES_OK;
}
