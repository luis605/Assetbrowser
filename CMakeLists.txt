cmake_minimum_required(VERSION 3.29)
project(RayGui)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG "master"
        GIT_PROGRESS TRUE
        GIT_SHALLOW 1
)

FetchContent_MakeAvailable(raylib)

FetchContent_Declare(
        rlimgui
        GIT_REPOSITORY https://github.com/raylib-extras/rlImGui.git
        GIT_TAG "main"
        GIT_PROGRESS TRUE
        GIT_SHALLOW 1
)

FetchContent_MakeAvailable(rlimgui)

FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG "docking"
        GIT_PROGRESS TRUE
        GIT_SHALLOW 1
)

FetchContent_MakeAvailable(imgui)

file(GLOB PROJECT_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp" "${CMAKE_CURRENT_LIST_DIR}/src/*.h"
        "${imgui_SOURCE_DIR}/*.cpp" "${imgui_SOURCE_DIR}/*.h"
        "${imgui_SOURCE_DIR}/misc/cpp/*.cpp" "${imgui_SOURCE_DIR}/misc/cpp/*.h"
        "${rlimgui_SOURCE_DIR}/*.cpp" "${rlimgui_SOURCE_DIR}/*.h"
)

set(PROJECT_INCLUDE
        "${CMAKE_CURRENT_LIST_DIR}/src/"
        "${imgui_SOURCE_DIR}"
        "${imgui_SOURCE_DIR}/backends/"
        "${imgui_SOURCE_DIR}/misc/cpp"
        "${rlimgui_SOURCE_DIR}/"
)

add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_INCLUDE})
target_link_libraries(${PROJECT_NAME} PRIVATE raylib)

# Setting ASSETS_PATH
target_compile_definitions(${PROJECT_NAME} PUBLIC ASSETS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/assets/") # Set the asset path macro to the absolute path on the dev machine
#target_compile_definitions(${PROJECT_NAME} PUBLIC ASSETS_PATH="./assets") # Set the asset path macro in release mode to a relative path that assumes the assets folder is in the same directory as the game executable