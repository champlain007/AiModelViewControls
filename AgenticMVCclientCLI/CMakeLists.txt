cmake_minimum_required(VERSION 3.10)
project(AgenticMVCclientCLI CXX)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

add_executable(AgenticMVCclientCLI src/main.cpp)

install(TARGETS AgenticMVCclientCLI DESTINATION bin)
