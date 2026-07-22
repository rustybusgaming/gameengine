cmake_minimum_required(VERSION 3.16)

# Basic demo executable
add_executable(BasicDemo
    basic_demo/main.cpp
)

# Link with core engine
target_link_libraries(BasicDemo NexusCore)

# Copy Python demo script
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/basic_demo.py
    ${CMAKE_BINARY_DIR}/bin/basic_demo.py
    COPYONLY
)

# OpenGL Triangle Demo
if(NEXUS_OPENGL_ENABLED AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/opengl_triangle/CMakeLists.txt")
    add_subdirectory(opengl_triangle)
endif()

# Simple Triangle Demo (multi-API)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/simple_triangle/CMakeLists.txt")
    add_subdirectory(simple_triangle)
endif()
