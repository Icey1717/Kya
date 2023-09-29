set(TargetName "VU")

set(SOURCES 
	"include/PCSX2_VU.h"

	"src/Pcsx2Defs.h"
	"src/Pcsx2Types.h"
	"src/Common.cpp"
	"src/Common.h"
	"src/MTVU.cpp"
	"src/MTVU.h"
	"src/VU.h"
	"src/VUflags.cpp"
	"src/VUflags.h"
	"src/VUmicro.cpp"
	"src/VUmicro.h"
	"src/VUmicroMem.cpp"
	"src/VUops.cpp"
	"src/VUops.h"
	"src/VU0.cpp"
	"src/VU0micro.cpp"
	"src/VU0microInterp.cpp"
	"src/VU1micro.cpp"
	"src/VU1microInterp.cpp"
	"src/PCSX2_VU.cpp"
	"src/DisVU1Micro.cpp"
	"src/DisVUmicro.h"
	"src/DisVUops.h"
)

add_library(${TargetName} ${SOURCES})

target_link_libraries(${TargetName} PRIVATE Log)
target_include_directories(${TargetName} PUBLIC "include")