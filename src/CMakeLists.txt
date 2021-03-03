# configure a header file to pass some of the CMake settings
# to the source code
configure_file(kucker_config.h.in kucker_config.h)


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

add_executable(backer  kucker.cpp ${_SOURCES_})
target_include_directories(backer PUBLIC  ${_INCLUDES_} ${EXTRA_INCLUDES} )
target_link_libraries(backer PUBLIC  ${EXTRA_LIBS} )
# generate md5
add_custom_command(
  OUTPUT ${PROJECT_BINARY_DIR}/bin/backer.md5
  COMMAND md5sum  ${PROJECT_BINARY_DIR}/bin/backer >  ${PROJECT_BINARY_DIR}/bin/backer.md5
  DEPENDS ${PROJECT_BINARY_DIR}/bin/backer
  COMMENT "Generate backer.md5"
  VERBATIM
  )
add_custom_target("genmd5" ALL DEPENDS ${PROJECT_BINARY_DIR}/bin/backer.md5)

#backer_daemon
add_executable(backer_daemon  kucker_daemon.cpp ${_SOURCES_})
target_include_directories(backer_daemon PUBLIC  ${_INCLUDES_} ${EXTRA_INCLUDES} )
target_link_libraries(backer_daemon PUBLIC  ${EXTRA_LIBS} ) 

# install rules
install(TARGETS backer backer_daemon  DESTINATION bin)

