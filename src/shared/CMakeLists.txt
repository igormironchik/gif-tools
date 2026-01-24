# SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
# SPDX-License-Identifier: GPL-3.0-or-later

project(gif-widgets)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6Core REQUIRED)
find_package(Qt6Widgets REQUIRED)

set(SRC license_dialog.hpp
	license_dialog.cpp
	license_dialog.ui
	utils.hpp
    utils.cpp)

qt6_add_resources(SRC qt.qrc)
qt6_add_resources(SRC icon.qrc)

install(FILES "icon/icon_16x16.png" DESTINATION "${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/16x16/apps/" RENAME "gif-tools.png")
install(FILES "icon/icon_22x22.png" DESTINATION "${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/22x22/apps/" RENAME "gif-tools.png")
install(FILES "icon/icon_24x24.png" DESTINATION "${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/24x24/apps/" RENAME "gif-tools.png")
install(FILES "icon/icon_32x32.png" DESTINATION "${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/32x32/apps/" RENAME "gif-tools.png")
install(FILES "icon/icon_48x48.png" DESTINATION "${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/48x48/apps/" RENAME "gif-tools.png")
install(FILES "icon/icon_64x64.png" DESTINATION "${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/64x64/apps/" RENAME "gif-tools.png")
install(FILES "icon/icon_128x128.png" DESTINATION "${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/128x128/apps/" RENAME "gif-tools.png")
install(FILES "icon/icon_256x256.png" DESTINATION "${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/256x256/apps/" RENAME "gif-tools.png")
    
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(gif-widgets STATIC ${SRC})

target_link_libraries(gif-widgets Qt6::Widgets Qt6::Core)
