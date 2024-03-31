#ifndef _PROFILING_H
#define _PROFILING_H

#ifdef  PLATFORM_WIN
#include <tracy/Tracy.hpp>

#define ZONE_SCOPED ZoneScoped
#define ZONE_SCOPED_NAME(...) ZoneScopedN(__VA_ARGS__)

#define FRAME_MARK FrameMark
#endif


#endif _PROFILING_H