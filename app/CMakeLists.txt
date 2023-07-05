project(app)

add_executable(uriscv-cli main.cc)

INCLUDE_DIRECTORIES(${SIGCPP_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})

target_link_libraries(uriscv-cli PRIVATE uriscv-lib ${SIGCPP_LIBRARIES} ${Boost_LIBRARIES} "-lpthread -ldl")

install(TARGETS uriscv-cli RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
