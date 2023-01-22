project(app)

add_executable(uriscv main.cc)

target_link_libraries(uriscv PRIVATE src ${SIGCPP_LIBRARIES})

install(TARGETS uriscv RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
