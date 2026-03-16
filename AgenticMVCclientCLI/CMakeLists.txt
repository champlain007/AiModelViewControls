cmake_minimum_required(VERSION 3.10)
project(AgenticMVCclientCLI CXX)

set(CMAKE_CXX_STANDARD 17)

# Include the root include directory. Sub-paths like core/, security/ 
# should be included as #include "core/..."
include_directories(include)

# Recursively find all source files in the src directory
file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(AgenticMVCclientCLI ${SOURCES})

# Link against the httplib target (defined in its own CMakeLists.txt)
# and nlohmann_json.
target_link_libraries(AgenticMVCclientCLI 
    PRIVATE 
    httplib
    nlohmann_json::nlohmann_json
)

# Ensure include paths are correct for httplib if it's not handled by the target
if(cpp-httplib_SOURCE_DIR)
    target_include_directories(AgenticMVCclientCLI PRIVATE ${cpp-httplib_SOURCE_DIR})
endif()

install(TARGETS AgenticMVCclientCLI DESTINATION bin)
