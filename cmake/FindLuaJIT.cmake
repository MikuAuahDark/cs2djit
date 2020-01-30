# Sets the following variables:
#
# LUAJIT_FOUND
# LUAJIT_INCLUDE_DIR
# LUAJIT_LIBRARY

if (DEFINED ENV{LUAJIT_DIR})
	set(LUAJIT_DIR "${LUAJIT_DIR} $ENV{LUAJIT_DIR}")
endif()
string(STRIP "${LUAJIT_DIR}" LUAJIT_DIR)
message(STATUS "LuaJIT DIR: ${LUAJIT_DIR}")

find_path(LUAJIT_INCLUDE_DIR
	NAMES luajit.h lua.h
	PATH_SUFFIXES include/luajit-2.1 include/luajit2.1 include/luajit-2.0 include/luajit2.0 include src
	PATHS /usr/local /usr ${LUAJIT_DIR}
)
find_path(LUAJIT_INCLUDE_DIR
	NAMES luajit.h lua.h
	HINTS ${LUAJIT_DIR}
	NO_CMAKE_FIND_ROOT_PATH
)

find_library(LUAJIT_LIBRARY
	NAMES luajit5.1.dll luajit5.1 luajit-5.1 lua51
	PATH_SUFFIXES lib
	PATHS /usr/local /usr ${LUAJIT_DIR}
)
find_library(LUAJIT_LIBRARY
	NAMES luajit5.1.dll luajit5.1 luajit-5.1 lua51
	HINTS ${LUAJIT_DIR}
	NO_CMAKE_FIND_ROOT_PATH
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LuaJIT DEFAULT_MSG LUAJIT_LIBRARY LUAJIT_INCLUDE_DIR)

mark_as_advanced(LUAJIT_INCLUDE_DIR LUAJIT_LIBRARY)
