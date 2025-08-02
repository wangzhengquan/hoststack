
add_executable(test_tty  test_tty.cpp )
target_include_directories(test_tty PUBLIC  ${_INCLUDES_} ${EXTRA_INCLUDES} )
target_link_libraries(test_tty PUBLIC  ${EXTRA_LIBS} ) 


add_executable(test_uuid  test_uuid.cpp )
target_include_directories(test_uuid PUBLIC  ${_INCLUDES_} ${EXTRA_INCLUDES} )
target_link_libraries(test_uuid PUBLIC  ${EXTRA_LIBS} ) 

add_executable(test  test.cpp )
target_include_directories(test_tty PUBLIC  ${_INCLUDES_} ${EXTRA_INCLUDES} )
target_link_libraries(test_tty PUBLIC  ${EXTRA_LIBS} ) 