#ifndef _PROFILING_H
#define _PROFILING_H

#ifdef  PLATFORM_WIN

#ifdef ENABLE_PROFILING
#include <tracy/Tracy.hpp>

#define ZONE_SCOPED ZoneScoped
#define ZONE_SCOPED_NAME(...) ZoneScopedN(__VA_ARGS__)

#define FRAME_MARK FrameMark
#else
#define ZONE_SCOPED
#define ZONE_SCOPED_NAME(...)

#define FRAME_MARK
#endif
#endif


#endif // _PROFILING_H