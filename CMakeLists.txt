cmake_minimum_required(VERSION 3.20)

project(AQCanvas
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(Lua REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(WAYLAND REQUIRED wayland-client)

add_executable(AQCanvas main.cpp)

target_include_directories(AQCanvas PRIVATE 
    ${LUA_INCLUDE_DIR}
    ${WAYLAND_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/protocols
)

target_link_libraries(AQCanvas PRIVATE 
    ${LUA_LIBRARIES}
    ${WAYLAND_LIBRARIES}
)