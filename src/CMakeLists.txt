cmake_minimum_required(VERSION 3.16)
project(raylib_game LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(RAYLIB REQUIRED raylib)
pkg_check_modules(ENET REQUIRED libenet)


file(GLOB SOURCES
    *.cpp
    *.c
)

add_executable(game ${SOURCES})

target_include_directories(game PRIVATE ${RAYLIB_INCLUDE_DIRS} ${ENET_INCLUDE_DIRS})
target_link_libraries(game PRIVATE ${RAYLIB_LIBRARIES} ${ENET_LIBRARIES})
target_link_directories(game PRIVATE ${RAYLIB_LIBRARY_DIRS} ${ENET_LIBRARY_DIRS})

# ✅ executable -> project root
set_target_properties(game PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/..
)

