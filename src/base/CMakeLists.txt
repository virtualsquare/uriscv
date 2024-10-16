# SPDX-FileCopyrightText: 2020 Mattia Biondi
#
# SPDX-License-Identifier: GPL-3.0-or-later

add_library(base STATIC json.cc trackable_mixin.cc)
target_include_directories(base PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_compile_options(base PRIVATE ${SIGCPP_CFLAGS})
