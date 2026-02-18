cmake_minimum_required(VERSION 3.10)
project(TableFan3D)

set(CMAKE_C_STANDARD 11)

# Include directories
include_directories(include)

# Source files
set(SOURCES
    src/main.c
    src/tablefan.c
    src/renderer.c
    src/math_utils.c
)

# Add executable
add_executable(TableFan3D ${SOURCES})

# Link libraries (if any, e.g., OpenGL, GLFW)
find_package(OpenGL REQUIRED)
find_package(GLM REQUIRED)

target_link_libraries(TableFan3D ${OPENGL_LIBRARIES} glm)