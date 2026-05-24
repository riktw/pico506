// Copyright (c) Kuba Szczodrzyński 2025-8-4.

#include "pico506.h"

void hexdump(const uint8_t *buf, size_t len) {
	size_t pos = 0;
	while (pos < len) {
		// print hex offset
		printf("%06zx ", pos);
		// calculate current line width
		uint8_t lineWidth = (len - pos) > 16 ? 16 : (len - pos);
		// print hexadecimal representation
		for (size_t i = 0; i < lineWidth; i++) {
			if (i % 8 == 0) {
				printf(" ");
			}
			printf("%02x ", buf[pos + i]);
		}
		// print ascii representation
		printf(" |");
		for (size_t i = 0; i < lineWidth; i++) {
			char c = buf[pos + i];
			putchar((c >= 0x20 && c <= 0x7f) ? c : '.');
		}
		puts("|\r");
		pos += lineWidth;
	}
}

uint16_t calc_crc16(const uint8_t *data, uint len, uint16_t poly, uint16_t crc) {
	for (int i = 0; i < len; i++) {
		crc ^= data[i] << 8;
		for (int j = 0; j < 8; j++) {
			if (crc & 0x8000)
				crc = ((crc << 1) & 0xFFFF) ^ poly;
			else
				crc = ((crc << 1) & 0xFFFF);
		}
	}
	return crc;
}
