project(app)

add_executable(uriscv-cli main.cc)

target_link_libraries(uriscv-cli PRIVATE uriscv ${SIGCPP_LIBRARIES})

install(TARGETS uriscv-cli RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
