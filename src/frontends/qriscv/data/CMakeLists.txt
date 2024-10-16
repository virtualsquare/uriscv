# SPDX-FileCopyrightText: 2020 Mattia Biondi
#
# SPDX-License-Identifier: GPL-3.0-or-later

set(ID org.virtualsquare.umps3)

add_subdirectory(icons)

install(FILES ${ID}.desktop
        DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)

install(FILES ${ID}.metainfo.xml
        DESTINATION ${CMAKE_INSTALL_DATADIR}/metainfo)
