/* Copyright (c) Kuba Szczodrzyński 2022-04-28. */

#pragma once

// Loglevels
#define LT_LEVEL_VERBOSE LT_LEVEL_TRACE
#define LT_LEVEL_TRACE	 0
#define LT_LEVEL_DEBUG	 1
#define LT_LEVEL_INFO	 2
#define LT_LEVEL_WARN	 3
#define LT_LEVEL_ERROR	 4
#define LT_LEVEL_FATAL	 5

// Logger format options
#define LT_LOGGER_TIMESTAMP 1
#define LT_LOGGER_CALLER	0
#define LT_LOGGER_COLOR		1
#define LT_LOGLEVEL			LT_LEVEL_INFO
