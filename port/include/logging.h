#pragma once

#ifdef PLATFORM_WIN
#define ENABLE_MY_LOG
#else
#define uintptr_t int
//#define ENABLE_MY_LOG
#endif

#ifdef ENABLE_MY_LOG
#if defined(PLATFORM_WIN)
#define scePrintf(format, ...) Log::GetInstance().AddLog(LogLevel::Info, "PS2", format, ##__VA_ARGS__)
#define MY_LOG(format, ...) Log::GetInstance().AddLog(LogLevel::Info, "General", format, ##__VA_ARGS__)

#define FLUSH_LOG(...) Log::GetInstance().ForceFlush()

#define MY_LOG_CATEGORY(category, level, format, ...) Log::GetInstance().AddLog(level, category, format, ##__VA_ARGS__)

#else
#include <eekernel.h>
#define MY_LOG(...) scePrintf(##__VA_ARGS__); scePrintf("\n")
#define MY_LOG_CATEGORY(category, level, format, ...) scePrintf(format, ##__VA_ARGS__); scePrintf("\n")
#endif

#include <stdio.h>

#include <stdlib.h>

#else
#define MY_LOG(...)
#define MY_LOG_CATEGORY(...)
#define FLUSH_LOG(...)

#ifdef PLATFORM_WIN
#define scePrintf(...)
#endif
#endif