# configure a header file to pass some of the CMake settings
# to the source code
configure_file(hoststack_config.h.in hoststack_config.h)


list(APPEND _INCLUDES_ 
   ${PROJECT_BINARY_DIR}/src
   ${CMAKE_CURRENT_SOURCE_DIR}
   ${CMAKE_CURRENT_SOURCE_DIR}/cli
   ${CMAKE_CURRENT_SOURCE_DIR}/pty
)

list(APPEND _SOURCES_ 

./cli/container_start_cli.cpp
./cli/container_cli.cpp
./cli/container_run_cli.cpp
./cli/container_stop_cli.cpp
./cli/container_attach_cli.cpp
./cli/container_rm_cli.cpp
./cli/container_ls_cli.cpp
./cli/container_exec_cli.cpp
./container_fs.cpp
./container_service.cpp
./container_info.cpp
./container_dao.cpp
./path_assembler.cpp
./pty/pty_exec_util.cpp
./pty/pty_master_open.cpp
./pty/pty_fork.cpp
./pty/pty_server.cpp
./pty/tty_functions.cpp


)

add_executable(hoststack  hoststack.cpp ${_SOURCES_})
target_include_directories(hoststack PUBLIC  ${_INCLUDES_} ${EXTRA_INCLUDES} )
target_link_libraries(hoststack PUBLIC  ${EXTRA_LIBS} )
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
add_executable(hoststack_supervisor  hoststack_supervisor.cpp ${_SOURCES_})
target_include_directories(hoststack_supervisor PUBLIC  ${_INCLUDES_} ${EXTRA_INCLUDES} )
target_link_libraries(hoststack_supervisor PUBLIC  ${EXTRA_LIBS} ) 

# install rules
# include(GNUInstallDirs)
install(TARGETS hoststack hoststack_supervisor  DESTINATION bin)



