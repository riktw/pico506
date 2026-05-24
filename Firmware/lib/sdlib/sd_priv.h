// Copyright (c) Kuba Szczodrzyński 2025-9-11.

#pragma once

#include "sd.h"
#include "sd_defs.h"

#include <lt_logger.h>

#define SD_DEBUG_INIT 0

#if SD_DEBUG_INIT
#define DEBUG_INIT(...) \
	do {                \
		__VA_ARGS__;    \
	} while (0)
#else
#define DEBUG_INIT(...)
#endif

#define FOR_TIMEOUT(ms) \
	for (absolute_time_t start = get_absolute_time(); absolute_time_diff_us(start, get_absolute_time()) < (ms) * 1000;)
