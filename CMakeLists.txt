cmake_minimum_required(VERSION 3.28)
project(CMakeSFMLProject LANGUAGES CXX C)

set(BINARY_NAME smw)
set(ASSETS_DIR ${PROJECT_SOURCE_DIR}/assets)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(FetchContent)
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.1.0
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM)
FetchContent_MakeAvailable(SFML)

file(GLOB_RECURSE MAIN_SOURCES CONFIGURE_DEPENDS "src/*.cpp" "src/*.c")

add_executable(${BINARY_NAME} ${MAIN_SOURCES})
target_compile_features(${BINARY_NAME} PRIVATE cxx_std_11)
target_link_libraries(${BINARY_NAME} PRIVATE SFML::Graphics SFML::Audio)
target_include_directories(${BINARY_NAME} PRIVATE
    "vendor"
    "src/core"
    "src/actors"
    "src/world"
    "src/render"
    "src/audio"
    "src/particles"
)

add_custom_target(link_target ALL COMMAND ${CMAKE_COMMAND} -E create_symlink ${ASSETS_DIR} ${CMAKE_BINARY_DIR}/bin/assets)