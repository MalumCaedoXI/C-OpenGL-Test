cmake_minimum_required(VERSION 3.5)
project(C-OpenGL-Test)

add_compile_definitions(GLAD_GL_IMPLEMENTATION)

add_subdirectory(${CMAKE_SOURCE_DIR}/external/glfw)

add_executable(C-OpenGL-Test ./src/main.c)

target_include_directories(C-OpenGL-Test PRIVATE 
${CMAKE_SOURCE_DIR}/external/glfw/deps)

target_link_libraries(C-OpenGL-Test glfw)



