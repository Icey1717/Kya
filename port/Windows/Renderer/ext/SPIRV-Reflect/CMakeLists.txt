project(spirv-reflect-static)

set(SOURCES
    SPIRV-Reflect/spirv_reflect.cpp
    SPIRV-Reflect/common/output_stream.cpp
)

add_library(spirv-reflect-static ${SOURCES})

target_include_directories(spirv-reflect-static PUBLIC ${CMAKE_CURRENT_LIST_DIR}/SPIRV-Reflect)
target_include_directories(spirv-reflect-static PUBLIC ${CMAKE_CURRENT_LIST_DIR}/SPIRV-Reflect/common)
target_include_directories(spirv-reflect-static PUBLIC ${CMAKE_CURRENT_LIST_DIR}/SPIRV-Reflect/include/spirv/unified1)



