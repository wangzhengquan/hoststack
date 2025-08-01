# configure a header file to pass some of the CMake settings
# to the source code
configure_file(hoststack_config.h.in hoststack_config.h)



file (GLOB _SOURCES_ "common/*.cpp" "util/*.cpp" "cli/*.cpp" "pty/*.cpp")

add_library (libhoststack STATIC ${_SOURCES_})
target_include_directories(libhoststack PUBLIC  ${_INCLUDES_}  ${EXTRA_INCLUDES} )

add_executable(hoststack  hoststack.cpp )
target_include_directories(hoststack PUBLIC  ${_INCLUDES_} ${EXTRA_INCLUDES} )
target_link_libraries(hoststack PUBLIC libhoststack ${EXTRA_LIBS} )
# generate md5
add_custom_command(
  OUTPUT ${PROJECT_BINARY_DIR}/bin/hoststack.md5
  COMMAND md5sum  ${PROJECT_BINARY_DIR}/bin/hoststack >  ${PROJECT_BINARY_DIR}/bin/hoststack.md5
  DEPENDS ${PROJECT_BINARY_DIR}/bin/hoststack
  COMMENT "Generate hoststack.md5"
  VERBATIM
  )
add_custom_target("genmd5" ALL DEPENDS ${PROJECT_BINARY_DIR}/bin/hoststack.md5)

#hoststack_supervisor
add_executable(hoststack_supervisor hoststack_supervisor.cpp )
target_include_directories(hoststack_supervisor PUBLIC ${_INCLUDES_} ${EXTRA_INCLUDES} )
target_link_libraries(hoststack_supervisor PUBLIC libhoststack ${EXTRA_LIBS} ) 

# install rules
# include(GNUInstallDirs)
install(TARGETS hoststack hoststack_supervisor  DESTINATION bin)



