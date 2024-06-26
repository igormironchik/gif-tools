# SPDX-FileCopyrightText: 2018-2024 Igor Mironchik <igor.mironchik@gmail.com>
# SPDX-License-Identifier: GPL-3.0-or-later

project( gif-editor )

set( CMAKE_AUTOMOC ON )
set( CMAKE_AUTORCC ON )
set( CMAKE_AUTOUIC ON )

find_package(Qt6Core REQUIRED)
find_package(Qt6Widgets REQUIRED)
find_package(Qt6Gui REQUIRED)

set( SRC main.cpp
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
	tape.hpp
	view.hpp )

qt6_add_resources( SRC resources.qrc )

if( WIN32 )
	list( APPEND SRC gif-editor.rc )
endif()

include_directories( ${ImageMagick_INCLUDE_DIRS}
	${CMAKE_CURRENT_SOURCE_DIR}/../shared
	${CMAKE_CURRENT_SOURCE_DIR}/../../3rdparty/qgiflib/src
	${CMAKE_CURRENT_SOURCE_DIR}/../../3rdparty/qgiflib/3rdparty/giflib )

add_executable( gif-editor WIN32 ${SRC} )

target_link_libraries( gif-editor qgiflib gif-widgets Qt6::Widgets Qt6::Gui Qt6::Core )
