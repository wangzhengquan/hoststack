cmake_minimum_required(VERSION 3.5)

# set the project name and version
project(hoststack VERSION 2.0)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# control where the static and shared libraries are built so that on windows
# we don't need to tinker with the path to run the executable
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")


option(BUILD_DOC "Build doc" OFF)

include_directories ("${PROJECT_SOURCE_DIR}/include")

# list(APPEND EXTRA_INCLUDES ${PROJECT_SOURCE_DIR}/include )

list(APPEND EXTRA_LIBS 
						${PROJECT_SOURCE_DIR}/lib/libjsoncpp.a
						pthread 
						rt
						)



list(APPEND _INCLUDES_ 
    ${PROJECT_BINARY_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cli
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pty
    ${CMAKE_CURRENT_SOURCE_DIR}/src/util
    ${CMAKE_CURRENT_SOURCE_DIR}/src/common
)

message(STATUS "_INCLUDES_ value: ${_INCLUDES_}")

# build api doc
if (BUILD_DOC)
    # build the docs
    add_subdirectory(${PROJECT_SOURCE_DIR}/doc)
else()
	add_subdirectory(${PROJECT_SOURCE_DIR}/src)
	add_subdirectory(${PROJECT_SOURCE_DIR}/etc)
	add_subdirectory(${PROJECT_SOURCE_DIR}/test) 
endif()


# setup installer
include(InstallRequiredSystemLibraries)
set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/License.txt")
set(CPACK_PACKAGE_VERSION_MAJOR "${hoststack_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${hoststack_VERSION_MINOR}")
include(CPack)

configure_file(install.sh.in install.sh @ONLY)

