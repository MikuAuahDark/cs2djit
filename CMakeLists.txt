# Copyright (c) 2020 Miku AuahDark
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.

cmake_minimum_required(VERSION 3.13)

###############
# Some checks #
###############

# Prevent in-tree build.
if (${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_BINARY_DIR})
	message(FATAL_ERROR "Prevented in-tree build!")
endif()

# Check source/header files
if (NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/luawrap/cs2dluafunc.h")
	message(FATAL_ERROR "CS2D Lua Wrapper is missing. Clone the submodule correctly!")
endif()

# TODO: Remove
if (APPLE)
	message(FATAL_ERROR "Patches welcome for macOS!")
endif()

###########
# Project #
###########

project(cs2djit LANGUAGES C)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Check bitness
if (NOT CMAKE_SIZEOF_VOID_P EQUAL 4 AND NOT APPLE)
	message(FATAL_ERROR "Only support 32-bit target. Maybe add CLFAGS=-m32 or use 32-bit toolchain?")
endif()

find_package(LuaJIT REQUIRED)
message(STATUS "LuaJIT include: ${LUAJIT_INCLUDE_DIR}")
message(STATUS "LuaJIT lib: ${LUAJIT_LIBRARY}")

if (WIN32)
	message(FATAL_ERROR "TODO")
elseif (APPLE)
	# TODO
	message(FATAL_ERROR "Patches welcome for macOS!")
else ()
	# For Linux, use shared object LD_PRELOAD method
	add_library(cs2djit SHARED
		src/cs2djitbase.c
		src/cs2djitlinux.c
		src/cs2djitmem.c
	)
	target_link_options(cs2djit PRIVATE -Wl,--no-undefined)
	target_include_directories(cs2djit PRIVATE ${LUAJIT_INCLUDE_DIR})
	target_link_libraries(cs2djit ${LUAJIT_LIBRARY})
	install(TARGETS cs2djit DESTINATION bin)
	return()
endif()

message(FATAL_ERROR "Unknown target OS")
