set(TargetName "Gizmo")

set(SOURCES 
	"include/gizmo.h" 
	"src/gizmo.cpp" 
	"imGuIZMO.quat/imguizmo_quat/imGuIZMOquat.cpp"
)

add_library(${TargetName} ${SOURCES})

set_target_properties(${TargetName} PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(${TargetName} PUBLIC "include")
target_include_directories(${TargetName} PRIVATE "imGuIZMO.quat/imguizmo_quat")

target_compile_definitions(${TargetName} PUBLIC "IMGUIZMO_IMGUI_FOLDER=""")

target_link_libraries(${TargetName} PRIVATE imgui glm)
