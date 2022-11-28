project(src)

add_library(src STATIC
  uriscv/utility.cc
  uriscv/config.cc
  uriscv/bus.cc
  uriscv/hart.cc
  uriscv/processor.cc
  support/liburiscv/liburiscv.S
  )

set_property(SOURCE support/liburiscv/liburiscv.S PROPERTY LANGUAGE C)
target_include_directories(src
        PUBLIC ${PROJECT_SOURCE_DIR}/include
    )
