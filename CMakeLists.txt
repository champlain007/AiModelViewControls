cmake_minimum_required(VERSION 3.25)
project(AiModelViewControls)

# Enable standard tooling support (clangd, IDEs, code navigators)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Universally generic C++26 standard configuration
set(CMAKE_CXX_STANDARD 20) # Base CMake parsing level
set(CMAKE_CXX_STANDARD_REQUIRED OFF)

# Force the compiler to use C++26 (or draft C++2c) universally
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++2c") # Many compilers use 2c for 26 draft
elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++latest")
endif()

include(FetchContent)

# 1. JSON library
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(json)

# 2. HTTP Library (cpp-httplib)
FetchContent_Declare(
    cpp-httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG v0.15.3
)
FetchContent_MakeAvailable(cpp-httplib)

# 3. FTXUI (Terminal UI)
FetchContent_Declare(
    ftxui
    GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI.git
    GIT_TAG v5.0.0
)
FetchContent_MakeAvailable(ftxui)

# 4. IXWebSocket
FetchContent_Declare(
    ixwebsocket
    GIT_REPOSITORY https://github.com/machinezone/IXWebSocket.git
    GIT_TAG v11.4.4
)
FetchContent_MakeAvailable(ixwebsocket)

# Include directories
include_directories(include)

# Executable
add_executable(AiModelViewControls 
    src/main.cpp
    src/Model.cpp
    src/connectors/IConnectorStrategy.cpp
    src/connectors/ConnectorStrategies.cpp
    src/Ledger.cpp
    src/Controller.cpp
    src/ViewTUI.cpp
    src/Crypto.cpp src/AgenticEngine.cpp src/RouterEngine.cpp
    src/Security.cpp src/Orchestrator.cpp src/McpServer.cpp src/WebSocketSandbox.cpp
    src/cli/CliClient.cpp
    src/docker/DockerFacade.cpp
    src/SandboxManager.cpp
    src/XWayland.cpp
)

# Add CLI Engine sub-packages
add_subdirectory(AgenticMVCserverCLI)
add_subdirectory(AgenticMVCclientCLI)

if(TARGET cpp-httplib)
    target_link_libraries(AiModelViewControls PRIVATE cpp-httplib)
else()
    target_include_directories(AiModelViewControls PRIVATE ${cpp-httplib_SOURCE_DIR})
endif()

find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)
find_package(Threads REQUIRED)

target_link_libraries(AiModelViewControls
    PRIVATE
    nlohmann_json::nlohmann_json
    ftxui::screen
    ftxui::dom
    ftxui::component
    ixwebsocket::ixwebsocket
    OpenSSL::SSL
    OpenSSL::Crypto
    ZLIB::ZLIB
    Threads::Threads
)

# Copy web files to build directory
add_custom_command(TARGET AiModelViewControls POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/www
        $<TARGET_FILE_DIR:AiModelViewControls>/www
)

# --- Standard Code Viewing & Navigation Tooling ---

# 1. ctags support (universal function/class viewing)
find_program(CTAGS_EXECUTABLE ctags)
if(CTAGS_EXECUTABLE)
    add_custom_target(tags
        COMMAND ${CTAGS_EXECUTABLE} -R --c++-kinds=+p --fields=+iaS --extra=+q ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/include
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Generating ctags for viewing classes and methods..."
    )
endif()

# 2. cscope support (call graphs, symbols)
find_program(CSCOPE_EXECUTABLE cscope)
if(CSCOPE_EXECUTABLE)
    add_custom_target(cscope
        COMMAND find ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/include -name "*.cpp" -o -name "*.hpp" > cscope.files
        COMMAND ${CSCOPE_EXECUTABLE} -b -q -k
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Generating cscope database..."
    )
endif()

# 3. Doxygen support (HTML/PDF class documentation generation)
find_package(Doxygen)
if(DOXYGEN_FOUND)
    add_custom_target(docs
        COMMAND ${DOXYGEN_EXECUTABLE} "${CMAKE_SOURCE_DIR}/Doxyfile"
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Generating Doxygen API documentation for codebase..."
    )
endif()
