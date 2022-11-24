project(src)

add_library(src STATIC
  uriscv/utility.cc
  uriscv/config.cc
  uriscv/bus.cc
  uriscv/processor.cc
  )

target_include_directories(src
        PUBLIC ${PROJECT_SOURCE_DIR}/include
    )
