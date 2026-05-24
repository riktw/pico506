/* Copyright (c) Kuba Szczodrzyński 2022-04-28. */

#include "lt_logger.h"

#include <stdarg.h>
#include <stdio.h>

#include <pico/time.h>

#define COLOR_FMT			 "\e[0;30m"
#define COLOR_BLACK			 0x00
#define COLOR_RED			 0x01
#define COLOR_GREEN			 0x02
#define COLOR_YELLOW		 0x03
#define COLOR_BLUE			 0x04
#define COLOR_MAGENTA		 0x05
#define COLOR_CYAN			 0x06
#define COLOR_WHITE			 0x07
#define COLOR_BRIGHT_BLACK	 0x10
#define COLOR_BRIGHT_RED	 0x11
#define COLOR_BRIGHT_GREEN	 0x12
#define COLOR_BRIGHT_YELLOW	 0x13
#define COLOR_BRIGHT_BLUE	 0x14
#define COLOR_BRIGHT_MAGENTA 0x15
#define COLOR_BRIGHT_CYAN	 0x16
#define COLOR_BRIGHT_WHITE	 0x17

static const char levels[] = {'V', 'D', 'I', 'W', 'E', 'F'};

#if LT_LOGGER_COLOR
static const uint8_t colors[] = {
	COLOR_BRIGHT_CYAN,
	COLOR_BRIGHT_BLUE,
	COLOR_BRIGHT_GREEN,
	COLOR_BRIGHT_YELLOW,
	COLOR_BRIGHT_RED,
	COLOR_BRIGHT_MAGENTA,
};
#endif

#if LT_LOGGER_CALLER
void lt_log(const uint8_t level, const char *caller, const unsigned short line, const char *format, ...) {
#else
void lt_log(const uint8_t level, const char *format, ...) {
#endif

#if LT_LOGGER_TIMESTAMP
	unsigned int millis = to_ms_since_boot(get_absolute_time());
#endif

#if LT_LOGGER_COLOR
	char c_bright = '0' + (colors[level] >> 4);
	char c_value  = '0' + (colors[level] & 0x7);
#endif

	printf(
	// format:
#if LT_LOGGER_COLOR
		"\e[%c;3%cm"
#endif
		"%c "
#if LT_LOGGER_TIMESTAMP
		"[%7u.%03u] "
#endif
#if LT_LOGGER_COLOR
		"\e[0m"
#endif
#if LT_LOGGER_CALLER
		"%s():%hu: "
#endif
		,
	// arguments:
#if LT_LOGGER_COLOR
		c_bright, // whether text is bright
		c_value,  // text color
#endif
		levels[level]
#if LT_LOGGER_TIMESTAMP
		,
		millis / 1000,
		millis % 1000
#endif
#if LT_LOGGER_CALLER
		,
		caller,
		line
#endif
	);

	va_list va_args;
	va_start(va_args, format);
	vprintf(format, va_args);
	va_end(va_args);
	putchar('\r');
	putchar('\n');
}
