#include "vulkanBackend.h"
#include <stdio.h>
#include <string.h>

#define width 800
#define height 600

VkInstance instance;

int checkValidationLayerAvailabillity(char** requiredValidationLayers, char validationLayerCount)
{
    printf("Checking validation layers!\n");
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, NULL);

    VkLayerProperties availableLayers[layerCount];
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers);

    for (char i = 0; i < validationLayerCount; i++)
    {
        char result = 0;
        for (char k = 0; k < layerCount; k++)
        {
            if (!strcmp(requiredValidationLayers[i], availableLayers[k].layerName))
            {
                result = 1;
                break;
            }
        }

        if (!result)
        {
            printf("Validation layer check failed!\n");
            return 0;
        }

    }
    printf("Validation layer check OK!\n");
    return 1;
}

int createInstance()
{
    char validationLayerCount = 1;
    char validationEnabled = 0;

    #ifdef DEBUG
    validationEnabled = 1;
    #endif

    char* requiredValidationLayers[validationLayerCount];
    requiredValidationLayers[0] = "VK_LAYER_KHRONOS_validation";
    if (validationEnabled && !checkValidationLayerAvailabillity(requiredValidationLayers, validationLayerCount))
    {
        printf("Missing Layer for layer validation!\n");
        return 0;
    }

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;

    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    createInfo.enabledExtensionCount = glfwExtensionCount;
    createInfo.ppEnabledExtensionNames = glfwExtensions;
    createInfo.enabledLayerCount = 0;

    if (validationEnabled)
    {
        createInfo.enabledLayerCount = validationLayerCount;
        createInfo.ppEnabledLayerNames = requiredValidationLayers;
    }

    VkResult result = vkCreateInstance(&createInfo, NULL, &instance);
    if (result == VK_SUCCESS)
    {
        printf("Succeeded in creating Vulkan Instance!\n");
        return 1;
    }

    return 0;
}

int initVulkan()
{
    if (!createInstance())
    {
        return 0;
    }

    uint32_t extensionCount;
    vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL);
    printf("Vulkan Extentions available: %u\n", extensionCount);
    VkExtensionProperties extensions[extensionCount];
    vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, extensions);
    printf("Extentions are:\n");
    for (int i = 0; i < extensionCount; i++)
    {
        printf(extensions[i].extensionName);
        printf("\n");
    }

    return 1;
}

int initWindow(GLFWwindow** window)
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    *window = glfwCreateWindow(width, height, "Vulkan", NULL, NULL);
}

int cleanupWindow(GLFWwindow** window)
{

    vkDestroyInstance(instance, NULL);
    glfwDestroyWindow(*window);

    glfwTerminate();
}


    
