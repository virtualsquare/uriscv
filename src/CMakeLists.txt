project(src)

add_library(src STATIC
  base/json.cc
  base/trackable_mixin.cc
  uriscv/time_stamp.cc
  uriscv/utility.cc
  uriscv/config.cc
  uriscv/stoppoint.cc
  uriscv/mybus.cc
  uriscv/systembus.cc
  uriscv/hart.cc
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
  )

set_property(SOURCE support/liburiscv/liburiscv.S PROPERTY LANGUAGE C)
set(UMPS_DATA_DIR ${CMAKE_INSTALL_FULL_DATADIR}/uriscv)
target_compile_definitions(src PRIVATE -DPACKAGE_DATA_DIR="${URISCV_DATA_DIR}")
target_include_directories(src
        PUBLIC ${PROJECT_SOURCE_DIR}/include
    )
