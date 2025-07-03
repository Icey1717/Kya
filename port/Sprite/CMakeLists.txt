set(TargetName "Sprite")
project(${TargetName})

set(SOURCES 
	"src/Sprite.cpp"
	"src/Sprite.h"
)

add_library(${TargetName} ${SOURCES})

target_link_libraries(${TargetName} PRIVATE Log Kya Renderer)

target_include_directories(${TargetName} PUBLIC "src")