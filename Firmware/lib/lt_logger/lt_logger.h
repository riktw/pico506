/* Copyright (c) Kuba Szczodrzyński 2022-04-28. */

#pragma once

#include <stdint.h>

#include "lt_config.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#if LT_LOGGER_CALLER
#define LT_LOG(level, caller, line, ...) lt_log(level, caller, line, __VA_ARGS__)
#define LT_LOGM(level, module, caller, line, ...)                  \
	do {                                                           \
		if (LT_DEBUG_##module) {                                   \
			lt_log(level, caller, line, #module ": " __VA_ARGS__); \
		}                                                          \
	} while (0)
void lt_log(const uint8_t level, const char *caller, const unsigned short line, const char *format, ...)
	__attribute__((format(printf, 4, 5)));
#else
#define LT_LOG(level, caller, line, ...) lt_log(level, __VA_ARGS__)
#define LT_LOGM(level, module, caller, line, ...)    \
	do {                                             \
		if (LT_DEBUG_##module) {                     \
			lt_log(level, #module ": " __VA_ARGS__); \
		}                                            \
	} while (0)
void lt_log(const uint8_t level, const char *format, ...) __attribute__((format(printf, 2, 3)));
#endif

#if LT_LEVEL_TRACE >= LT_LOGLEVEL
#define LT_T(...)		   LT_LOG(LT_LEVEL_TRACE, __FUNCTION__, __LINE__, __VA_ARGS__)
#define LT_V(...)		   LT_LOG(LT_LEVEL_TRACE, __FUNCTION__, __LINE__, __VA_ARGS__)
#define LT_TM(module, ...) LT_LOGM(LT_LEVEL_TRACE, module, __FUNCTION__, __LINE__, __VA_ARGS__)
#define LT_VM(module, ...) LT_LOGM(LT_LEVEL_TRACE, module, __FUNCTION__, __LINE__, __VA_ARGS__)
#else
#define LT_T(...)
#define LT_V(...)
#define LT_TM(...)
#define LT_VM(...)
#endif

#if LT_LEVEL_DEBUG >= LT_LOGLEVEL
#define LT_D(...)		   LT_LOG(LT_LEVEL_DEBUG, __FUNCTION__, __LINE__, __VA_ARGS__)
#define LT_DM(module, ...) LT_LOGM(LT_LEVEL_DEBUG, module, __FUNCTION__, __LINE__, __VA_ARGS__)
#else
#define LT_D(...)
#define LT_DM(...)
#endif

#if LT_LEVEL_INFO >= LT_LOGLEVEL
#define LT_I(...)		   LT_LOG(LT_LEVEL_INFO, __FUNCTION__, __LINE__, __VA_ARGS__)
#define LT_IM(module, ...) LT_LOGM(LT_LEVEL_INFO, module, __FUNCTION__, __LINE__, __VA_ARGS__)
#else
#define LT_I(...)
#define LT_IM(...)
#endif

#if LT_LEVEL_WARN >= LT_LOGLEVEL
#define LT_W(...)		   LT_LOG(LT_LEVEL_WARN, __FUNCTION__, __LINE__, __VA_ARGS__)
#define LT_WM(module, ...) LT_LOGM(LT_LEVEL_WARN, module, __FUNCTION__, __LINE__, __VA_ARGS__)
#else
#define LT_W(...)
#define LT_WM(...)
#endif

#if LT_LEVEL_ERROR >= LT_LOGLEVEL
#define LT_E(...)		   LT_LOG(LT_LEVEL_ERROR, __FUNCTION__, __LINE__, __VA_ARGS__)
#define LT_EM(module, ...) LT_LOGM(LT_LEVEL_ERROR, module, __FUNCTION__, __LINE__, __VA_ARGS__)
#else
#define LT_E(...)
#define LT_EM(...)
#endif

#if LT_LEVEL_FATAL >= LT_LOGLEVEL
#define LT_F(...)		   LT_LOG(LT_LEVEL_FATAL, __FUNCTION__, __LINE__, __VA_ARGS__)
#define LT_FM(module, ...) LT_LOGM(LT_LEVEL_FATAL, module, __FUNCTION__, __LINE__, __VA_ARGS__)
#else
#define LT_F(...)
#define LT_FM(...)
#endif

#ifdef __cplusplus
} // extern "C"
#endif
