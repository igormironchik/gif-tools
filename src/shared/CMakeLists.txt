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
    
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(gif-widgets STATIC ${SRC})

target_link_libraries(gif-widgets Qt6::Widgets Qt6::Core)
