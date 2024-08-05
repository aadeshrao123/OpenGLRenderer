cmake_minimum_required(VERSION 3.10)
project(OpenGLRenderer)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)

# Include directories
include_directories(
    external/glfw/include
    ${CMAKE_SOURCE_DIR}/external/glew/include
    ${CMAKE_SOURCE_DIR}/external/tinygltf
    ${CMAKE_SOURCE_DIR}/external/glm
)

# Add GLFW submodule
add_subdirectory(external/glfw)

# Add GLEW submodule
option(BUILD_UTILS "Build the glewinfo and visualinfo programs" OFF)
option(ONLY_LIBS "Build only the GLEW library" ON)
add_subdirectory(external/glew/build/cmake)

# Add tinygltf submodule
add_subdirectory(external/tinygltf)

# Find OpenGL
find_package(OpenGL REQUIRED)

# Source files
set(SOURCES src/main.cpp)

# Add executable
add_executable(OpenGLRenderer ${SOURCES})

# Link libraries
target_link_libraries(OpenGLRenderer
    ${OPENGL_LIBRARIES}
    glew_s
    glfw
    tinygltf
)
