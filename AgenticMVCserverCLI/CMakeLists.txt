cmake_minimum_required(VERSION 3.10)
project(AgenticMVCserverCLI CXX)

set(CMAKE_CXX_STANDARD 17)

# No external dependencies needed for this lightweight version
# unless we want to link against the main Model later.

include_directories(include)

add_executable(AgenticMVCserverCLI src/main.cpp)

# Install rules
install(TARGETS AgenticMVCserverCLI DESTINATION bin)
