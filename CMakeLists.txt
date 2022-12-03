cmake_minimum_required(VERSION 3.5)

project(uriscv VERSION 3.0.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-D_FORTIFY_SOURCE=2 -O2 -Wall -pedantic)

include(GNUInstallDirs)

include(TestBigEndian)
TEST_BIG_ENDIAN(WORDS_BIGENDIAN)
configure_file(config.h.in config.h)

include(FindPkgConfig)
pkg_check_modules(SIGCPP REQUIRED sigc++-2.0)

find_package(Boost 1.34 REQUIRED)

add_subdirectory(src)
add_subdirectory(app)



