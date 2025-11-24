cmake_minimum_required(VERSION 4.1.2)
project(minecraft)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

include_directories(include src)

file(GLOB IMGUI_SOURCES
    include/imgui/*.cpp
)

add_executable(minecraft
    src/main.cpp
    src/glad.c
    src/voxelData.cpp
    src/chunk.cpp
    ${IMGUI_SOURCES}
)

target_link_libraries(minecraft
    glfw
    OpenGL::GL
)

add_definitions(-Wno-deprecated-declarations)
