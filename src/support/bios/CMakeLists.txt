include(GNUInstallDirs)
set(ROM_TYPES coreboot exec)
set(CMAKE_VERBOSE_MAKEFILE ON)

foreach(TYPE ${ROM_TYPES})
        # install(FILES ${TYPE}.S
        #         DESTINATION ${URISCV_DATA_DIR})
endforeach()

set(BIOS_CFLAGS -fno-pic -ffreestanding -static -g -march=rv32imfd -mabi=ilp32d )
set(BIOS_CPPFLAGS -I${PROJECT_SOURCE_DIR}/include)

foreach(TYPE ${ROM_TYPES})
  message(STATUS ${BIOS_CPPFLAGS})
        add_custom_target(${TYPE}.eb ALL
                COMMAND ${XCGCC} -c -B ${BIOS_CFLAGS} ${BIOS_CPPFLAGS} -o
                        ${CMAKE_CURRENT_BINARY_DIR}/${TYPE}.eb ${TYPE}.S
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

        add_custom_target(${TYPE}.el ALL
                COMMAND ${XCGCC} -c -L ${BIOS_CFLAGS} ${BIOS_CPPFLAGS} -o
                        ${CMAKE_CURRENT_BINARY_DIR}/${TYPE}.el ${TYPE}.S
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

        add_custom_target(${TYPE}.eb.rom.uriscv ALL
                COMMAND uriscv-elf2uriscv -v -b ${TYPE}.eb
                WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
        add_dependencies(${TYPE}.eb.rom.uriscv ${TYPE}.eb uriscv-elf2uriscv)

        add_custom_target(${TYPE}.el.rom.uriscv ALL
                COMMAND uriscv-elf2uriscv -v -b ${TYPE}.el
                WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
        add_dependencies(${TYPE}.el.rom.uriscv ${TYPE}.el uriscv-elf2uriscv)

        set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES
                ${TYPE}.eb
                ${TYPE}.el
                ${TYPE}.eb.rom.uriscv
                ${TYPE}.el.rom.uriscv)
endforeach()

if(${WORDS_BIGENDIAN})
        set(ENDIAN eb)
else()
        set(ENDIAN el)
endif()

foreach(TYPE ${ROM_TYPES})
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${TYPE}.${ENDIAN}.rom.uriscv DESTINATION ${URISCV_DATA_DIR} RENAME ${TYPE}.rom.uriscv)
endforeach()

