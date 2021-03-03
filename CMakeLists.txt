cmake_minimum_required(VERSION 3.5)

# set the project name and version
project(KUCKER VERSION 1.0)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# control where the static and shared libraries are built so that on windows
# we don't need to tinker with the path to run the executable
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")


option(BUILD_DOC "Build doc" OFF)


list(APPEND EXTRA_INCLUDES 
						${PROJECT_SOURCE_DIR}/include
						${PROJECT_SOURCE_DIR}/include/usgcommon
						${PROJECT_SOURCE_DIR}/include/uuid

						)

list(APPEND EXTRA_LIBS 
						${PROJECT_SOURCE_DIR}/lib/libusgcommon.a 
						${PROJECT_SOURCE_DIR}/lib/libuuid.a 
						${PROJECT_SOURCE_DIR}/lib/libjsoncpp.a
						pthread 
						rt
						)

# build api doc
if (BUILD_DOC)
    # build the docs
    add_subdirectory(${PROJECT_SOURCE_DIR}/doc)
else()
	add_subdirectory(${PROJECT_SOURCE_DIR}/src)
	 
endif()
