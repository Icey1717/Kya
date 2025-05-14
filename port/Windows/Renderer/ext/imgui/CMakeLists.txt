project(imgui)

set(SOURCES
    imgui/imgui.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
)

add_library(imgui ${SOURCES})

target_include_directories(imgui PUBLIC ${CMAKE_CURRENT_LIST_DIR}/imgui)
