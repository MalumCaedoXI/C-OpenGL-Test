cmake_minimum_required(VERSION 3.5)
project(C-OpenGL-Test)
add_compile_definitions(DEBUG)


add_compile_definitions(GLAD_GL_IMPLEMENTATION)

add_subdirectory(${CMAKE_SOURCE_DIR}/external/glfw)

find_package(Vulkan REQUIRED)

add_executable(C-OpenGL-Test ./src/main.c)

target_include_directories(C-OpenGL-Test PRIVATE 
${CMAKE_SOURCE_DIR}/external/glfw/deps)

add_subdirectory(${CMAKE_SOURCE_DIR}/src/vulkanBackend)
add_subdirectory(${CMAKE_SOURCE_DIR}/external/stb)

target_link_libraries(C-OpenGL-Test glfw)
target_link_libraries(C-OpenGL-Test Vulkan::Vulkan)



