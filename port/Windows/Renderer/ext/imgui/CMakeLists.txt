cmake_minimum_required(VERSION 3.5)

project(imgui)

set( SOURCES
	"imgui/imgui_demo.cpp"
	"imgui/imgui_draw.cpp"
	"imgui/imgui_tables.cpp"
	"imgui/imgui_widgets.cpp"
	"imgui/imgui.cpp"
	"imgui/imgui.h"
	"imgui/backends/imgui_impl_vulkan.cpp"
	"imgui/backends/imgui_impl_vulkan.h"
	"imgui/backends/imgui_impl_glfw.cpp"
	"imgui/backends/imgui_impl_glfw.h"
)

add_library(imgui ${SOURCES})

target_include_directories(imgui PUBLIC "${CMAKE_CURRENT_LIST_DIR}/imgui")
target_include_directories(imgui PRIVATE "${Vulkan_INCLUDE_DIRS}")

target_link_libraries(imgui ${Vulkan_LIBRARIES} glfw)
