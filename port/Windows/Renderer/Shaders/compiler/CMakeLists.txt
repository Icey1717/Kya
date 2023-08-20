set(TargetName "ShaderCompiler")
project(${TargetName})


add_executable(${TargetName} "src/ShaderCompiler.cpp")

# Define the output folder for the compiled C# project
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

target_link_libraries(${TargetName} PRIVATE Hash Archive)

cmake_path(NATIVE_PATH PS2_SHADER_SRC_DIR PS2_SHADER_SRC_DIR)
string(REPLACE "\\" "\\\\" PS2_SHADER_SRC_DIR ${PS2_SHADER_SRC_DIR})

cmake_path(NATIVE_PATH DXC_COMPILER DXC_COMPILER)
string(REPLACE "\\" "\\\\" DXC_COMPILER ${DXC_COMPILER})

cmake_path(NATIVE_PATH SHADER_OUTPUT_FOLDER SHADER_OUTPUT_FOLDER)
string(REPLACE "\\" "\\\\" SHADER_OUTPUT_FOLDER ${SHADER_OUTPUT_FOLDER})

target_compile_definitions(${TargetName} PRIVATE
	SHADER_OUTPUT_FOLDER="${SHADER_OUTPUT_FOLDER}"
	DXC_COMPILER="${DXC_COMPILER}"
	PS2_SHADER_SRC_DIR="${PS2_SHADER_SRC_DIR}"
)
