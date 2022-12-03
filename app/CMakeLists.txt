project(app)

add_executable(emu main.cc)

target_link_libraries(emu PRIVATE src ${SIGCPP_LIBRARIES})
