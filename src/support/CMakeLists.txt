# SPDX-FileCopyrightText: 2020 Mattia Biondi
#
# SPDX-License-Identifier: GPL-3.0-or-later

set(URISCV_LIB_DIR ${CMAKE_INSTALL_LIBDIR}/uriscv)

add_subdirectory(bios)
add_subdirectory(crt)
add_subdirectory(ldscripts)
add_subdirectory(liburiscv)

