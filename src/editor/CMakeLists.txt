# SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
# SPDX-License-Identifier: GPL-3.0-or-later

project(gif-editor)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui Concurrent StateMachine Network)
find_package(KF${KF_MAJOR_VERSION} ${KF_MIN_VERSION} COMPONENTS
    IconThemes ColorScheme Config
)

set(SRC main.cpp
	about.cpp
	busyindicator.cpp
	crop.cpp
	frame.cpp
	frameontape.cpp
	mainwindow.cpp
	tape.cpp
	view.cpp
	about.hpp
	busyindicator.hpp
	crop.hpp
	frame.hpp
	frameontape.hpp
	mainwindow.hpp
    mainwindow_private.hpp
    mainwindow_private.cpp
	tape.hpp
    view.hpp
    rectangle.hpp
    rectangle.cpp
    text.hpp
    text.cpp
    texteditor.hpp
    texteditor.cpp
    settings.hpp
    settings.cpp
    settings.ui
    drawrect.hpp
    drawrect.cpp
    drawarrow.hpp
    drawarrow.cpp
    delay.hpp
    delay.cpp
    delay.ui
    tips.hpp
    tips.cpp
    constants.hpp
    constants.cpp
    uistates.hpp
    uistates.cpp)

qt6_add_resources(SRC resources.qrc)

if(WIN32)
    list(APPEND SRC gif-editor.rc)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../shared
	${CMAKE_CURRENT_SOURCE_DIR}/../../3rdparty/qgiflib/src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../3rdparty/qgiflib/3rdparty/giflib
    ${CMAKE_CURRENT_SOURCE_DIR}/../../3rdparty/github-release/src)

add_executable(gif-editor WIN32 ${SRC})

if(KF${KF_MAJOR_VERSION}IconThemes_FOUND AND KF${KF_MAJOR_VERSION}ColorScheme_FOUND
    AND KF${KF_MAJOR_VERSION}Config_FOUND)
    list(APPEND ADDITIONAL_TARGETS KF${KF_MAJOR_VERSION}::ConfigCore
        KF${KF_MAJOR_VERSION}::IconThemes KF${KF_MAJOR_VERSION}::ColorScheme)
    target_compile_definitions(gif-editor PRIVATE MD_BREEZE)
endif()

target_link_libraries(gif-editor qgiflib gif-widgets github-release ${ADDITIONAL_TARGETS}
    Qt6::Concurrent Qt6::StateMachine Qt6::Network
    Qt6::Widgets Qt6::Gui Qt6::Core)

install(TARGETS gif-editor)
