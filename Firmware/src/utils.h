// Copyright (c) Kuba Szczodrzyński 2025-9-12.

#pragma once

#include "pico506.h"

#define BUILD_BUG_ON(condition) ((void)sizeof(char[1 - 2 * !!(condition)]))

inline uint bswap(uint word) {
	return ((word & 0xFF) << 24) | ((word & 0xFF00) << 8) | ((word & 0xFF0000) >> 8) | ((word & 0xFF000000) >> 24);
}

inline uint randrange(uint start, uint stop) {
	return start + rand() % (stop - start); // NOLINT(*-msc50-cpp)
}

void hexdump(const uint8_t *buf, size_t len);
uint16_t calc_crc16(const uint8_t *data, uint len, uint16_t poly, uint16_t crc);
