# SPDX-FileCopyrightText: 2020 Mattia Biondi
# SPDX-FileCopyrightText: 2023 Gianmaria Rovelli
# SPDX-FileCopyrightText: 2024 Luca Bassi
#
# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.22)

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
      find_program(GCC_TEST_32 riscv32-unknown-linux-gnu-gcc)
      find_program(GCC_TEST_64 riscv64-unknown-elf-gcc)
      if (GCC_TEST_32)
        set(XT_PREFIXES riscv32-unknown-linux-gnu- 
          ${XT_PREFIXES})
      elseif (GCC_TEST_64)
        set(XT_PREFIXES riscv64-unknown-elf- 
          ${XT_PREFIXES})
      else ()
        set(XT_PREFIXES riscv64-elf-
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

set(CPACK_DEBIAN_PACKAGE_DEPENDS "libqt6widgets6, libqt6svg6, libsigc++-2.0-0v5")
set(CPACK_DEBIAN_PACKAGE_RECOMMENDS "gcc-riscv64-unknown-elf")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "VirtualSquare")
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
cmake_host_system_information(RESULT DISTRO QUERY DISTRIB_INFO)
set(CPACK_DEBIAN_PACKAGE_RELEASE ${DISTRO_ID})
set(CPACK_RPM_PACKAGE_SUGGESTS "gcc-riscv64-linux-gnu")
set(CPACK_RPM_SPEC_MORE_DEFINE "%define __spec_install_post /bin/true")
set(CPACK_RPM_FILE_NAME RPM-DEFAULT)
set(CPACK_RPM_PACKAGE_RELEASE_DIST ON)
include(CPack)
