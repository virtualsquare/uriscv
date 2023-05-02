set(CRT_FILES crtso crti)

set(CRT_CFLAGS -fno-pic -ffreestanding -c -g -static -march=rv32imfd -mabi=ilp32d )
set(CRT_CPPFLAGS -I${PROJECT_SOURCE_DIR}/include)

foreach(FILE ${CRT_FILES})
	add_custom_target(${FILE}.o ALL
		COMMAND ${XCGCC} ${CRT_CPPFLAGS} ${CRT_CFLAGS} -o
			${CMAKE_CURRENT_BINARY_DIR}/${FILE}.o ${FILE}.S
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

	set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES
		${FILE}.o)

	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${FILE}.o
		DESTINATION ${URISCV_LIB_DIR})

	install(FILES ${FILE}.S
		DESTINATION ${URISCV_DATA_DIR})
endforeach()

