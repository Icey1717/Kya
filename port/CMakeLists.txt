project(Port)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory("Pool")
add_subdirectory("Log")
add_subdirectory("Job")
add_subdirectory("Hash")
add_subdirectory("Archive")

add_subdirectory("ext/pss/pss")

#Tracy profiler
option(TRACY_ENABLE "" ON)
option(TRACY_ON_DEMAND "" OFF)
if(ENABLE_PERFORMANCE_PROFILING OR ENABLE_MEMORY_PROFILING)
	# Root CMakeLists sets TRACY_CALLSTACK=10 via add_compile_definitions; suppress Tracy's
	# own boolean define here to avoid a redefinition warning (-DTRACY_CALLSTACK=1 vs =10).
	set(TRACY_CALLSTACK OFF CACHE BOOL "" FORCE)
else()
	option(TRACY_CALLSTACK "" ON)
endif()
add_subdirectory("ext/tracy") 

add_subdirectory("ext/readerwriterqueue")

add_subdirectory("pcsx2/VU")

add_subdirectory("Windows/Renderer")
add_subdirectory("KyaTexture")
add_subdirectory("KyaMesh")
add_subdirectory("Sprite")

set(SOURCES 
	"Windows/windows_impl.cpp"
	"Windows/Input/gamepad.cpp"
	"src/object_naming.cpp"
	"src/input_functions.cpp"
	"include/port.h"
	"include/gamepad.h"
)

add_library(Port ${SOURCES})

target_include_directories(Port PUBLIC "include")

target_link_libraries(Port PUBLIC Renderer pss Tracy::TracyClient Texture Mesh Sprite)

add_subdirectory(DebugMenu)