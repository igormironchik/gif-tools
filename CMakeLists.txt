# SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.19)

project(gif-tools)

set(GIF_TOOLS_VERSION_NUMBER "1.0.2")
set(HomeDir "@HomeDir@")
string(TIMESTAMP GIF_CURRENT_DATE "%Y-%m-%d")

list(PREPEND CMAKE_PREFIX_PATH ${CMAKE_CURRENT_BINARY_DIR})
list(PREPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_BINARY_DIR})

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.xml.in ${CMAKE_CURRENT_SOURCE_DIR}/installer/config/config.xml @ONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/package.xml.in ${CMAKE_CURRENT_SOURCE_DIR}/installer/packages/mironchik.igor.gif/meta/package.xml @ONLY)

if(UNIX AND NOT APPLE)
    list(PREPEND CMAKE_INSTALL_RPATH "${QT6_INSTALL_PREFIX}/lib:\$ORIGIN/../lib")
else()
    list(PREPEND CMAKE_INSTALL_RPATH "${QT6_INSTALL_PREFIX}/lib")
    list(PREPEND CMAKE_INSTALL_RPATH "@executable_path/../lib")
    list(PREPEND CMAKE_INSTALL_RPATH "@loader_path/../lib")
endif()

set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set(CMAKE_MACOSX_RPATH TRUE)

link_directories(${CMAKE_BINARY_DIR}/lib)

install(FILES "gif-editor.desktop" DESTINATION "${CMAKE_INSTALL_PREFIX}/share/applications/")
install(FILES "gif-recorder.desktop" DESTINATION "${CMAKE_INSTALL_PREFIX}/share/applications/")

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release"
		CACHE STRING "Choose the type of build."
		FORCE)
endif(NOT CMAKE_BUILD_TYPE)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/qgiflib)

set(BUILD_QGIFLIB_TESTS OFF CACHE INTERNAL "" FORCE)
add_subdirectory(3rdparty/qgiflib)

add_subdirectory(src)
