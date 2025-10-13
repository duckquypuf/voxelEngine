cmake_minimum_required(VERSION 4.1.2)
project(minecraftCpp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

include_directories(
    include
    imgui
    imgui/backends
)

# ImGui source files
set(IMGUI_SOURCES
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/backends/imgui_impl_glfw.cpp
    imgui/backends/imgui_impl_opengl3.cpp
)

add_executable(minecraftCpp
    src/main.cpp
    src/glad.c
    ${IMGUI_SOURCES}
)

target_link_libraries(minecraftCpp
    glfw
    OpenGL::GL
)

add_definitions(-Wno-deprecated-declarations)
