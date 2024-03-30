project(Port)

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
add_subdirectory("ext/tracy") 

add_subdirectory("pcsx2/VU")

add_subdirectory("Windows/Renderer")

set(SOURCES 
	"Windows/WindowsFunctions.cpp"
	"src/ObjectNaming.cpp"
	"include/port.h"
)

add_library(Port ${SOURCES})

target_include_directories(Port PUBLIC "include")

target_link_libraries(Port PUBLIC Renderer pss Tracy::TracyClient)

add_subdirectory(DebugMenu)