cmake_minimum_required(VERSION 3.16)

project(OpenGLTriangleDemo)

# Require OpenGL to be enabled
if(NOT NEXUS_OPENGL_ENABLED)
    message(WARNING "OpenGL not enabled - skipping OpenGL triangle demo")
    return()
endif()

# Create executable
add_executable(OpenGLTriangleDemo main.cpp)

# Link against NexusCore
target_link_libraries(OpenGLTriangleDemo PRIVATE NexusCore)

# Set output directory
set_target_properties(OpenGLTriangleDemo PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/examples
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

message(STATUS "Added OpenGL Triangle Demo")
