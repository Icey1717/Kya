set(TargetName "Mesh")
project(${TargetName})

set(SOURCES 
	"src/Mesh.cpp"
	"src/Mesh.h"
)

add_library(${TargetName} ${SOURCES})

target_link_libraries(${TargetName} PRIVATE Log Kya Renderer)

target_include_directories(${TargetName} PUBLIC "src")