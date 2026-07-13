# SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
# SPDX-License-Identifier: GPL-3.0-or-later

project(gif-recorder)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui Concurrent)
find_package(KF${KF_MAJOR_VERSION} ${KF_MIN_VERSION} COMPONENTS
    IconThemes ColorScheme Config
)

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
    ${CMAKE_CURRENT_SOURCE_DIR}/../../3rdparty/qgiflib/3rdparty/giflib
    ${CMAKE_CURRENT_SOURCE_DIR}/../../3rdparty/QHotkey)

add_executable(gif-recorder WIN32 ${SRC})

if(KF${KF_MAJOR_VERSION}IconThemes_FOUND AND KF${KF_MAJOR_VERSION}ColorScheme_FOUND
    AND KF${KF_MAJOR_VERSION}Config_FOUND)
    list(APPEND ADDITIONAL_TARGETS KF${KF_MAJOR_VERSION}::ConfigCore
        KF${KF_MAJOR_VERSION}::IconThemes KF${KF_MAJOR_VERSION}::ColorScheme)
    target_compile_definitions(gif-recorder PRIVATE MD_BREEZE)
endif()

if(WIN32)
    set(WIN_LIBS "Dwmapi")
endif()

target_link_libraries(gif-recorder qgiflib gif-widgets qhotkey ${ADDITIONAL_TARGETS}
    Qt6::Concurrent Qt6::Widgets Qt6::Gui Qt6::Core ${WIN_LIBS})

if(UNIX)
    target_link_libraries(gif-recorder Xtst Xfixes X11)
endif()

install(TARGETS gif-recorder)
