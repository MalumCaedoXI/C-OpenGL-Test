#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <limits.h>

typedef struct QueueFamilyIndices {
    char valid;
    uint32_t graphicsFamily; 
    uint32_t presentFamily;
} QueueFamilyIndices;

typedef struct SwapChainDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    uint32_t formatSize;
    VkSurfaceFormatKHR* formats;
    uint32_t presentSize;
    VkPresentModeKHR* presentModes;
}SwapChainDetails;

int initVulkan();
int initWindow(GLFWwindow** window);
int cleanupWindow(GLFWwindow** window);