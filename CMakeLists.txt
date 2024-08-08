cmake_minimum_required(VERSION 3.10)
project(OpenGLRenderer)

set(CMAKE_CXX_STANDARD 17)

# Add GLFW submodule
add_subdirectory(external/glfw)

# Add GLEW submodule
option(BUILD_UTILS "Build the glewinfo and visualinfo programs" OFF)
option(ONLY_LIBS "Build only the GLEW library" ON)
add_subdirectory(external/glew/build/cmake)

# Add tinygltf submodule
add_subdirectory(external/tinygltf)

# Add GLM
set(GLM_DIR "${CMAKE_SOURCE_DIR}/external/glm")
include_directories(${GLM_DIR})

# Add ImGui
add_subdirectory(external/imgui)

# Find OpenGL
find_package(OpenGL REQUIRED)

# Include directories
include_directories(
    ${OPENGL_INCLUDE_DIR} 
    external/glfw/include 
    ${CMAKE_SOURCE_DIR}/external/glew/include
    ${CMAKE_SOURCE_DIR}/external/tinygltf
    ${CMAKE_SOURCE_DIR}/external/imgui
    ${CMAKE_SOURCE_DIR}/external/imgui/backends
)

# Source files
set(SOURCES 
    src/main.cpp 
    src/Camera.cpp 
    src/Shader.cpp 
    src/Model.cpp 
    src/Texture.cpp
)

# Add executable
add_executable(OpenGLRenderer ${SOURCES})

# Link libraries
target_link_libraries(OpenGLRenderer
    ${OPENGL_LIBRARIES}
    glew_s
    glfw
    tinygltf
    imgui
)
