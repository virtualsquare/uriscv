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
        # if(WORDS_BIGENDIAN)
        #         set(XT_PREFIXES mips-linux-gnu- mips-linux-
        #                 mips-elf- ${XT_PREFIXES})
        # else()
                set(XT_PREFIXES riscv32-unknown-linux-gnu- 
                        ${XT_PREFIXES})
        # endif()
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

include(FindPkgConfig)
pkg_check_modules(SIGCPP REQUIRED sigc++-2.0)

find_package(Boost 1.34 REQUIRED)

add_subdirectory(src)
add_subdirectory(app)



