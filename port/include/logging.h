#pragma once

#ifdef PLATFORM_WIN
#define ENABLE_MY_LOG
#else
#define uintptr_t int
//#define ENABLE_MY_LOG
#endif

#ifdef ENABLE_MY_LOG
#if defined(PLATFORM_WIN)
#include "log_categories.h"
#define scePrintf(format, ...) MY_LOG_NAMED_CATEGORY(PS2, LogLevel::Info, format, ##__VA_ARGS__)
#define MY_LOG(format, ...) MY_LOG_NAMED_CATEGORY(General, LogLevel::Info, format, ##__VA_ARGS__)

#define FLUSH_LOG(...) Log::GetInstance().ForceFlush()

#define MY_LOG_CATEGORY(category, level, format, ...) do { \
	auto& kyaLogInstance = Log::GetInstance(); \
	auto& kyaLogEntry = kyaLogInstance.GetCategory(category); \
	if (kyaLogEntry.bEnabled) { \
		kyaLogInstance.AddLog(level, kyaLogEntry, format, ##__VA_ARGS__); \
	} \
} while (false)

#else
#include <eekernel.h>
#define MY_LOG(...) scePrintf(##__VA_ARGS__); scePrintf("\n")
#define MY_LOG_CATEGORY(category, level, format, ...) scePrintf(format, ##__VA_ARGS__); scePrintf("\n")
#define MY_LOG_NAMED_CATEGORY(category, level, format, ...) MY_LOG_CATEGORY(#category, level, format, ##__VA_ARGS__)
#endif

#include <stdio.h>

#include <stdlib.h>

#else
#define MY_LOG(...)
#define MY_LOG_CATEGORY(...)
#define MY_LOG_NAMED_CATEGORY(...)
#define FLUSH_LOG(...)

#ifdef PLATFORM_WIN
#define scePrintf(...)
#endif
#endif
