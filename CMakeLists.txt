cmake_minimum_required(VERSION 3.3)
project(kv)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp server.cpp server.hpp KV_Manager.cpp KV_Manager.hpp String_Utilities.hpp Epoll_Manager.cpp Epoll_Manager.hpp Connection.cpp Connection.hpp)
add_executable(server ${SOURCE_FILES})
target_link_libraries(server pthread)