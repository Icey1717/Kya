#ifndef _PROFILING_H
#define _PROFILING_H

#ifdef PLATFORM_WIN
#if defined(ENABLE_PERFORMANCE_PROFILING) || defined(ENABLE_MEMORY_PROFILING)
#include <tracy/Tracy.hpp>
#define FRAME_MARK FrameMark

#define ZONE_SCOPED ZoneScoped
#define ZONE_SCOPED_NAME(...) ZoneScopedN(__VA_ARGS__)
#define ZONE_NAME(...) ZoneName(__VA_ARGS__)
#else
#define ZONE_SCOPED
#define ZONE_SCOPED_NAME(...)
#define ZONE_NAME(...)

#define FRAME_MARK
#endif // ENABLE_PERFORMANCE_PROFILING || ENABLE_MEMORY_PROFILING

#if defined (ENABLE_MEMORY_PROFILING)
#define TRACK_ALLOC(ptr, size) TracyAlloc(ptr, size)
#define TRACK_FREE(ptr) TracyFree(ptr)
#else
#define TRACK_ALLOC(ptr, size)
#define TRACK_FREE(ptr)
#endif // ENABLE_MEMORY_PROFILING
#endif // PLATFORM_WIN

#endif // _PROFILING_H