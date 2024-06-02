function(compile_ispc OUTPUT_VAR)
	set(options)
	set(oneValueArgs TARGET)
	set(multiValueArgs SOURCES)
	cmake_parse_arguments(ISPC "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
	set(header_dir "${CMAKE_CURRENT_BINARY_DIR}/ispc/include")
	set(ISPC_INCLUDE_DIR "${header_dir}" PARENT_SCOPE)
	
	set(output_files)
	foreach(source ${ISPC_SOURCES})
		get_filename_component(source_we ${source} NAME_WE)
		set(output_file "${CMAKE_CURRENT_BINARY_DIR}/${source_we}_ispc.o")
		set(header_file "${header_dir}/${source_we}.h")

		set(ispc_args "--arch=x86-64" "--target=sse2")

		add_custom_command(
			OUTPUT ${output_file}
			COMMAND ${CMAKE_COMMAND} -E make_directory ${header_dir}
			COMMAND ${CMAKE_ISPC_COMPILER} ${source} -g -o ${output_file} -h ${header_file} ${ispc_args}
			DEPENDS ${source}
			COMMENT "Compiling ISPC source ${source} (${CMAKE_ISPC_COMPILER} ${source} -o ${output_file} -h ${header_file} ${ispc_args})"
		)
		list(APPEND output_files ${output_file})
	endforeach()
	
	set(${OUTPUT_VAR} ${output_files} PARENT_SCOPE)
endfunction()