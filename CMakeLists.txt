cmake_minimum_required (VERSION 3.12 FATAL_ERROR)

project (fuse3-p7zip
	VERSION 0.9.0
	LANGUAGES CXX
)

configure_file (
	${CMAKE_CURRENT_LIST_DIR}/cmake/version.hpp.in
	${CMAKE_CURRENT_LIST_DIR}/include/version.hpp
	@ONLY
)

set (CMAKE_EXPORT_COMPILE_COMMANDS ON)
set (CMAKE_INSTALL_PREFIX /usr)
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

find_package        (FUSE3 REQUIRED)
include_directories (${FUSE3_INCLUDE_DIR})
include_directories (${CMAKE_CURRENT_LIST_DIR}/3rdparty/p7zip)
include_directories (${CMAKE_CURRENT_LIST_DIR}/3rdparty/p7zip/CPP)
include_directories (${CMAKE_CURRENT_LIST_DIR}/3rdparty/p7zip/CPP/include_windows)
include_directories (${CMAKE_CURRENT_LIST_DIR}/3rdparty/p7zip/CPP/myWindows)

add_executable      (${PROJECT_NAME}
	src/main.cpp
	src/library.cpp
	src/string.cpp
	src/fuse3.cpp
	src/exception.cpp

	src/7zip/Library.cpp
	src/7zip/Format.cpp
	src/7zip/Method.cpp
	src/7zip/Archive.cpp
	src/7zip/ReadStream.cpp
	src/7zip/WriteStream.cpp
	src/7zip/ArchiveExtractor.cpp
	src/7zip/com.cpp

	3rdparty/p7zip/CPP/Common/MyWindows.cpp
	3rdparty/p7zip/CPP/Windows/PropVariant.cpp
)

# set_target_properties      (${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)

target_compile_features    (${PROJECT_NAME} PUBLIC
	cxx_std_20
)
target_compile_definitions (${PROJECT_NAME} PUBLIC
	_FILE_OFFSET_BITS=64
)
target_compile_options     (${PROJECT_NAME} PUBLIC
	-Wall
	-fmax-errors=20
	#--pedantic
	#-Wextra
)

target_include_directories (${PROJECT_NAME} PRIVATE
	src/include
)
target_include_directories (${PROJECT_NAME} PUBLIC
	include
)
target_link_libraries      (${PROJECT_NAME} PUBLIC
	dl
	${FUSE3_LIBRARIES}
)

# man: https://cmake.org/cmake/help/v3.12/command/install.html
install(TARGETS ${PROJECT_NAME}
	CONFIGURATIONS Debug
	ARCHIVE DESTINATION ~/.local/lib/static
	LIBRARY DESTINATION ~/.local/lib
	RUNTIME DESTINATION ~/.local/bin
)
install(TARGETS ${PROJECT_NAME}
	CONFIGURATIONS Release
	ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/static
	LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
	RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
)
