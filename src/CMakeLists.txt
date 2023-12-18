project(src)

add_library(uriscv-lib STATIC
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
  gdb/gdb.cc
)

set_property(SOURCE support/liburiscv/liburiscv.S PROPERTY LANGUAGE C)
set(URISCV_DATA_DIR ${CMAKE_INSTALL_FULL_DATADIR}/uriscv)
target_compile_definitions(uriscv-lib PRIVATE -DPACKAGE_DATA_DIR="${URISCV_DATA_DIR}")
target_include_directories(uriscv-lib
	PUBLIC ${PROJECT_SOURCE_DIR}/include ${SIGCPP_INCLUDE_DIRS}
    )
target_compile_options(uriscv-lib PUBLIC ${SIGCPP_CFLAGS_OTHER})
target_link_libraries(uriscv-lib ${SIGCPP_LIBRARIES} ${Boost_LIBRARIES})

add_executable(uriscv-elf2uriscv uriscv/elf2uriscv.cc)
target_include_directories(uriscv-elf2uriscv
        PUBLIC ${PROJECT_SOURCE_DIR}/include
    )
target_link_libraries(uriscv-elf2uriscv ${LIBELF})

install(TARGETS uriscv-elf2uriscv RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

add_executable(uriscv-objdump uriscv/objdump.cc uriscv/disassemble.cc uriscv/symbol_table.cc uriscv/utility.cc)
target_include_directories(uriscv-objdump
        PUBLIC ${PROJECT_SOURCE_DIR}/include
    )
target_link_libraries(uriscv-objdump)

add_executable(uriscv-mkdev uriscv/mkdev.cc)
target_include_directories(uriscv-mkdev
        PUBLIC ${PROJECT_SOURCE_DIR}/include
    )
add_executable(uriscv-mkconfig uriscv/mkconfig.cc)
target_include_directories(uriscv-mkconfig
        PUBLIC ${PROJECT_SOURCE_DIR}/include
    )
target_link_libraries(uriscv-mkconfig PUBLIC uriscv-lib ${SIGCPP_LIBRARIES} ${Boost_LIBRARIES})

install(TARGETS uriscv-mkdev RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(TARGETS uriscv-mkconfig RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

add_subdirectory(frontends)
add_subdirectory(support)

set(URISCV_INCLUDE_DIR ${CMAKE_INSTALL_INCLUDEDIR}/uriscv)
set(LIBURISCV_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include/uriscv)

install(FILES ${LIBURISCV_INCLUDE_DIR}/arch.h ${LIBURISCV_INCLUDE_DIR}/aout.h ${LIBURISCV_INCLUDE_DIR}/regdef.h ${LIBURISCV_INCLUDE_DIR}/bios.h ${LIBURISCV_INCLUDE_DIR}/cpu.h ${LIBURISCV_INCLUDE_DIR}/const.h ${CMAKE_INSTALL_INCLUDEDIR}/uriscv/csr.h
	DESTINATION ${URISCV_INCLUDE_DIR})
