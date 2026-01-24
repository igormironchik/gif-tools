# SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
# SPDX-License-Identifier: GPL-3.0-or-later

project(gif-recorder)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6Core REQUIRED)
find_package(Qt6Widgets REQUIRED)
find_package(Qt6Gui REQUIRED)

set(SRC main.cpp
	mainwindow.hpp
	mainwindow.cpp
	settings.hpp
	settings.cpp
	settings.ui
	event_monitor.hpp
    event_monitor.cpp
    sizedlg.hpp
    sizedlg.cpp
    sizedlg.ui)

qt6_add_resources(SRC resources.qrc)

if(WIN32)
    list(APPEND SRC gif-recorder.rc)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../shared
	${CMAKE_CURRENT_SOURCE_DIR}/../../3rdparty/qgiflib/src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../3rdparty/qgiflib/3rdparty/giflib)

add_executable(gif-recorder WIN32 ${SRC})

target_link_libraries(gif-recorder qgiflib gif-widgets Qt6::Widgets Qt6::Gui Qt6::Core)

if(UNIX)
    target_link_libraries(gif-recorder Xtst Xfixes X11)
endif()

install(TARGETS gif-recorder)
