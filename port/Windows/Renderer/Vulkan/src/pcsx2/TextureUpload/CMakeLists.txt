project(TextureUpload)

set (CMAKE_CXX_STANDARD 20)

add_subdirectory("ext/xbyak")

set (SOURCES
	"src/common/Pcsx2Types.h"
	"src/common/Pcsx2Defs.h"
	"src/common/VectorIntrin.h"
	"src/GSBlock.cpp"
	"src/GSBlock.h"
	"src/GSTables.cpp"
	"src/GSTables.h"
	"src/GSRegs.h"
	"src/GSExtra.h"
	"src/GSLocalMemoryMultiISA.cpp"
	"src/GSAlignedClass.h"
	"src/GSClut.cpp"
	"src/GSClut.h"
	"src/GSLocalMemory.cpp"
	"src/GSLocalMemory.h"
	"src/GS.h"
	"src/MultiISA.cpp"
	"src/MultiISA.h"
	"src/GSXXH.h"
	"src/TextureUpload.cpp"
	"src/TextureUpload.h"
	"src/GSVector.h"
	"src/GSVector4.h"
	"src/GSVector4i.h"
	"src/GSVector8.h"
	"src/GSVector8i.h"
)

add_library(TextureUpload ${SOURCES})

target_link_libraries(TextureUpload xbyak)

if(MSVC)
	target_compile_options(TextureUpload PRIVATE "/arch:SSE4.1")
elseif(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	target_compile_options(TextureUpload PUBLIC "-msse4.1")
endif()

target_compile_definitions(TextureUpload PUBLIC "NOMINMAX")

target_precompile_headers(TextureUpload PRIVATE "src/common/Pcsx2Types.h")

target_include_directories(TextureUpload PRIVATE "ext/xxHash")

target_include_directories(TextureUpload PUBLIC "src")
target_include_directories(TextureUpload PUBLIC "include")