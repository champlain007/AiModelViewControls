cmake_minimum_required(VERSION 3.10)
project(AgenticMVCserverCLI CXX)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

add_executable(AgenticMVCserverCLI src/main.cpp)

# Link against the httplib target (defined in its own CMakeLists.txt)
# and nlohmann_json.
target_link_libraries(AgenticMVCserverCLI 
    PRIVATE 
    httplib
    nlohmann_json::nlohmann_json
)

# Ensure include paths are correct for httplib if it's not handled by the target
if(cpp-httplib_SOURCE_DIR)
    target_include_directories(AgenticMVCserverCLI PRIVATE ${cpp-httplib_SOURCE_DIR})
endif()

# Install rules
install(TARGETS AgenticMVCserverCLI DESTINATION bin)
