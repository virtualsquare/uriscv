project(src)

add_library(uriscv STATIC
  base/json.cc
  base/trackable_mixin.cc
  uriscv/time_stamp.cc
  uriscv/utility.cc
  uriscv/config.cc
  uriscv/stoppoint.cc
  uriscv/disassemble.cc
  uriscv/systembus.cc
  uriscv/processor.cc
  uriscv/machine_config.cc
  uriscv/blockdev.cc
  uriscv/vde_network.cc
  uriscv/device.cc
  uriscv/event.cc
  uriscv/memspace.cc
  uriscv/mpic.cc
  uriscv/mp_controller.cc
  uriscv/machine.cc
  uriscv/symbol_table.cc
)

set_property(SOURCE support/liburiscv/liburiscv.S PROPERTY LANGUAGE C)
set(URISCV_DATA_DIR ${CMAKE_INSTALL_FULL_DATADIR}/uriscv)
target_compile_definitions(uriscv PRIVATE -DPACKAGE_DATA_DIR="${URISCV_DATA_DIR}")
target_include_directories(uriscv
        PUBLIC ${PROJECT_SOURCE_DIR}/include
    )

add_executable(uriscv-elf2uriscv uriscv/elf2uriscv.cc)
target_include_directories(uriscv-elf2uriscv
        PUBLIC ${PROJECT_SOURCE_DIR}/include
    )
target_link_libraries(uriscv-elf2uriscv ${LIBELF})

install(TARGETS uriscv-elf2uriscv RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

add_executable(testdis uriscv/testdis.cc uriscv/disassemble.cc)
target_include_directories(testdis
        PUBLIC ${PROJECT_SOURCE_DIR}/include
    )
target_link_libraries(testdis)

add_executable(uriscv-objdump uriscv/objdump.cc uriscv/disassemble.cc uriscv/symbol_table.cc uriscv/utility.cc)
target_include_directories(uriscv-objdump
        PUBLIC ${PROJECT_SOURCE_DIR}/include
    )
target_link_libraries(uriscv-objdump)

install(TARGETS testdis RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})


set(URISCV_INCLUDE_DIR ${CMAKE_INSTALL_INCLUDEDIR}/uriscv)
set(LIBURISCV_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include/uriscv)

install(FILES ${LIBURISCV_INCLUDE_DIR}/arch.h ${LIBURISCV_INCLUDE_DIR}/aout.h ${LIBURISCV_INCLUDE_DIR}/regdef.h ${LIBURISCV_INCLUDE_DIR}/bios.h ${LIBURISCV_INCLUDE_DIR}/cpu.h
	DESTINATION ${URISCV_INCLUDE_DIR})
