#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

int initVulkan();
int initWindow(GLFWwindow** window);
int cleanupWindow(GLFWwindow** window);