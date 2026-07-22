cmake_minimum_required(VERSION 3.16)

project(SimpleTriangleExample)

# Find NexusCore library
find_package(NexusCore REQUIRED HINTS ${CMAKE_SOURCE_DIR}/build)

add_executable(SimpleTriangle
    SimpleTriangle.cpp
)

target_link_libraries(SimpleTriangle
    PRIVATE
        NexusCore
)

# Copy shaders to build directory
add_custom_command(TARGET SimpleTriangle POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/shaders
        $<TARGET_FILE_DIR:SimpleTriangle>/shaders
)

set_target_properties(SimpleTriangle PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/examples
)
