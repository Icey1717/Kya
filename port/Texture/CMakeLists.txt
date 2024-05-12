set(TargetName "Texture")
project(${TargetName})

set(SOURCES 
	"src/Texture.cpp"
	"src/Texture.h"
)

add_library(${TargetName} ${SOURCES})

target_link_libraries(${TargetName} PRIVATE Log Kya Renderer)

target_include_directories(${TargetName} PUBLIC "src")