cmake_minimum_required (VERSION 3.12 FATAL_ERROR)

project (fuse3-p7zip
	VERSION 0.9.1
	DESCRIPTION "fuse3 file system that uses the p7zip library to mount archives"
	LANGUAGES CXX
)

configure_file (
	"${CMAKE_CURRENT_LIST_DIR}/cmake/version.hpp.in"
	"${CMAKE_CURRENT_LIST_DIR}/include/version.hpp"
	@ONLY
)

set (CMAKE_EXPORT_COMPILE_COMMANDS  ON)
set (CMAKE_INSTALL_PREFIX           "/usr")
set (CMAKE_MODULE_PATH              "${CMAKE_MODULE_PATH}" "${CMAKE_SOURCE_DIR}/cmake")

set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

find_package               (FUSE3 REQUIRED)

## Add clang-tidy if available
#option(CLANG_TIDY_FIX "Perform fixes for Clang-Tidy" OFF)
#find_program(
#	CLANG_TIDY_EXE
#	NAMES "clang-tidy"
#	DOC "Path to clang-tidy executable"
#)
#
#if(CLANG_TIDY_EXE)
#	if(CLANG_TIDY_FIX)
#		set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE}" "-fix")
#	else()
#		set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE}" "-checks=-*,readability-*")
#	endif()
#endif()

add_executable             ("${PROJECT_NAME}"
	src/main.cpp
	src/exception.cpp
	src/fuse3.cpp
	src/library.cpp
	src/logger.cpp
	src/string.cpp
	src/7zip/Archive.cpp
	src/7zip/ArchiveExtractor.cpp
	src/7zip/Format.cpp
	src/7zip/Library.cpp
	src/7zip/Method.cpp
	src/7zip/ReadStream.cpp
	src/7zip/WriteStream.cpp
	src/7zip/com.cpp
	3rdparty/p7zip/CPP/Common/MyWindows.cpp
	3rdparty/p7zip/CPP/Windows/PropVariant.cpp
)

set_target_properties      ("${PROJECT_NAME}" PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
# set_target_properties      ("${PROJECT_NAME}" PROPERTIES LINKER_LANGUAGE CXX)

target_compile_features    ("${PROJECT_NAME}" PUBLIC
	cxx_std_20
)
target_compile_definitions ("${PROJECT_NAME}" PRIVATE
	_FILE_OFFSET_BITS=64
)
target_compile_options     ("${PROJECT_NAME}" PRIVATE
	-Wall
	#-fmax-errors=20
	#--pedantic
	#-Wextra
)
target_include_directories ("${PROJECT_NAME}" PRIVATE
	src/include
	"${FUSE3_INCLUDE_DIR}"
	"${CMAKE_CURRENT_LIST_DIR}/3rdparty/p7zip"
	"${CMAKE_CURRENT_LIST_DIR}/3rdparty/p7zip/CPP"
	"${CMAKE_CURRENT_LIST_DIR}/3rdparty/p7zip/CPP/include_windows"
	"${CMAKE_CURRENT_LIST_DIR}/3rdparty/p7zip/CPP/myWindows"
)
target_include_directories ("${PROJECT_NAME}" PRIVATE
	include
)
target_link_libraries      ("${PROJECT_NAME}" PRIVATE
	"${CMAKE_DL_LIBS}"
	"${FUSE3_LIBRARIES}"
)

# man: https://cmake.org/cmake/help/v3.12/command/install.html
install(TARGETS "${PROJECT_NAME}"
	CONFIGURATIONS Debug
	ARCHIVE DESTINATION "~/.local/lib/static"
	LIBRARY DESTINATION "~/.local/lib"
	RUNTIME DESTINATION "~/.local/bin"
)
install(TARGETS "${PROJECT_NAME}"
	CONFIGURATIONS Release
	ARCHIVE DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/static"
	LIBRARY DESTINATION "${CMAKE_INSTALL_PREFIX}/lib"
	RUNTIME DESTINATION "${CMAKE_INSTALL_PREFIX}/bin"
)

set(CPACK_PACKAGE_VENDOR              "Andrew Grechkin")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_DESCRIPTION}")
set(CPACK_PACKAGE_VERSION_MAJOR       "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR       "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH       "${PROJECT_VERSION_PATCH}")
set(CPACK_RESOURCE_FILE_LICENSE       "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README        "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_GENERATOR                   "TGZ")
include(CPack)
