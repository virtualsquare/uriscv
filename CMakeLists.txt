cmake_minimum_required(VERSION 3.5)

project(uriscv VERSION 3.0.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-D_FORTIFY_SOURCE=2 -O2 -Wall -pedantic)

include(GNUInstallDirs)

include(TestBigEndian)
TEST_BIG_ENDIAN(WORDS_BIGENDIAN)
configure_file(config.h.in config.h)



set(XT_PREFIXES ${RISCV_TOOL_PREFIX})
if(NOT XT_PREFIXES)
      find_program(GCC_TEST riscv32-unknown-linux-gnu-gcc)
      if (GCC_TEST)
        set(XT_PREFIXES riscv32-unknown-linux-gnu- 
          ${XT_PREFIXES})
      else ()
        set(XT_PREFIXES riscv64-unknown-elf- 
          ${XT_PREFIXES})
      endif()
endif()

foreach(PREF ${XT_PREFIXES})
        find_program(XCGCC ${PREF}gcc)
        if(XCGCC)
                set(XT_PREFIX ${PREF})
                break()
        endif()
endforeach()

if(NOT XCGCC)
  message(FATAL_ERROR "RISCV toolchain (gcc) not found.")
endif()

find_library(LIBELF elf)
if(NOT LIBELF)
        message(FATAL_ERROR "libelf not found.")
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(SIGCPP REQUIRED sigc++-2.0)

if (NOT SIGCPP_LIBRARIES)
        message(FATAL_ERROR "sigc++ not found.")
endif()

find_package(Boost 1.34 COMPONENTS program_options REQUIRED)
find_package(Qt6 COMPONENTS Widgets REQUIRED)

add_subdirectory(src)
add_subdirectory(app)



