#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

typedef struct QueueFamilyIndices {
    char valid;
    uint32_t graphicsFamily; 
} QueueFamilyIndices;

int initVulkan();
int initWindow(GLFWwindow** window);
int cleanupWindow(GLFWwindow** window);