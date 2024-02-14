#include "vulkanBackend.h"
#include <stdio.h>
#include <string.h>

#define windowWidth 800
#define windowHeight 600

VkInstance instance;
VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
VkDevice logicalDevice = VK_NULL_HANDLE;
VkQueue graphicsQueue;
VkQueue presentQueue;
VkSurfaceKHR windowSurface;
VkSwapchainKHR swapChain;
uint32_t swapChainImageCount;
VkImage* swapChainImages;
VkFormat swapChainImageFormat;
VkExtent2D swapChainExtent;
VkImageView* swapChainImageViews;

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice targetDevice)
{
    QueueFamilyIndices indicies = {1};
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(targetDevice, &queueFamilyCount, NULL);

    VkQueueFamilyProperties queueFamilies[queueFamilyCount];
    vkGetPhysicalDeviceQueueFamilyProperties(targetDevice, &queueFamilyCount, queueFamilies);
    uint32_t result = 0;
    for (int i = 0; i < queueFamilyCount; i++)
    {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            result = 1;
            indicies.graphicsFamily = i;
            break;
        }
    }

    indicies.valid &= result;

    result = 0;
    for (int i = 0; i < queueFamilyCount; i++)
    {
        vkGetPhysicalDeviceSurfaceSupportKHR(targetDevice, i, windowSurface, &result);
        if (result)
        {
            indicies.presentFamily = i;
            break;
        }
    }

    indicies.valid &= result;

    return indicies;
}

SwapChainDetails findSwapChainDetails(VkPhysicalDevice targetDevice)
{
    SwapChainDetails details = {};

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(targetDevice, windowSurface, &details.capabilities);


    vkGetPhysicalDeviceSurfaceFormatsKHR(targetDevice, windowSurface, &details.formatSize, NULL);

    if (details.formatSize != 0) {
        details.formats = malloc(sizeof(VkSurfaceFormatKHR) * details.formatSize);
        vkGetPhysicalDeviceSurfaceFormatsKHR(targetDevice, windowSurface, &details.formatSize, details.formats);
    }

    for (int i = 0; i < details.formatSize; i++)
    {
        printf("Format: %u and colorSpace: %u\n", details.formats[i].format, details.formats[i].colorSpace);
    }
    
    
    vkGetPhysicalDeviceSurfacePresentModesKHR(targetDevice, windowSurface, &details.presentSize, NULL);

    if (details.presentSize != 0) {
        details.presentModes = malloc(sizeof(VkPresentModeKHR) * details.presentSize);
        vkGetPhysicalDeviceSurfacePresentModesKHR(targetDevice, windowSurface, &details.presentSize, details.presentModes);
    }

    return details;
}

void freeSwapChainDetails(SwapChainDetails details)
{
    free(details.formats);
    free(details.presentModes);
}

int checkValidationLayerAvailabillity(const char* const* requiredValidationLayers, char validationLayerCount)
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

int checkExtensionAvailabillity(VkPhysicalDevice targetDevice)
{
    printf("Starting check for available device extentions!\n");
    uint32_t requiredExtentionCount = 1;
    char* requiredExtentions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(targetDevice, NULL, &extensionCount, NULL);
    VkExtensionProperties extensions[extensionCount];
    vkEnumerateDeviceExtensionProperties(targetDevice, NULL, &extensionCount, extensions);

    printf("Device supports %u extentions!\nExtentions are: \n", extensionCount);
    for (int i = 0; i < extensionCount; i++)
    {
        printf("%s\n", extensions[i].extensionName);
    }

    for (int i = 0; i < requiredExtentionCount; i++)
    {
        char result = 0;
        for (int k = 0; k < extensionCount; k++)
        {
            if (!strcmp(requiredExtentions[i], extensions[k].extensionName))
            {
                result = 1;
                break;
            }

        }

        if (!result)
        {
            printf("Device is lacking required extention: %s\n", requiredExtentions[i]);
            return 0;
        }
    }

    printf("Device extentions check OK!\n");
    return 1;
}

int createInstance()
{
    char validationLayerCount = 1;
    char validationEnabled = 0;

    #ifdef DEBUG
    validationEnabled = 1;
    #endif

    const char const* requiredValidationLayers[] = {"VK_LAYER_KHRONOS_validation"};
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

VkSurfaceFormatKHR chooseSwapSurfaceFormat(VkSurfaceFormatKHR* formats, uint32_t size)
{
    for (uint32_t i = 0; i < size; i++)
    {   
        if (formats[i].format == VK_FORMAT_B8G8R8A8_SRGB && formats[i].colorSpace  == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) 
        {
            return formats[i];
        }

    }
    return formats[0];
}

VkPresentModeKHR chooseSwapPresentMode(VkPresentModeKHR* presentModes, uint32_t size)
{
    for (uint32_t i = 0; i < size; i++)
    {
        if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) 
        {
            return presentModes[i];
        }

    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D chooseSwapExtent(VkSurfaceCapabilitiesKHR capabilities, GLFWwindow* window)
{
    if (capabilities.currentExtent.width != UINT32_MAX)
    {
        return capabilities.currentExtent;
    }
    else
    {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        VkExtent2D extent = {width, height};
        extent.width = extent.width > capabilities.maxImageExtent.width ? capabilities.maxImageExtent.width : extent.width;//TODO: Find a better way to clamp.
        extent.width = extent.width < capabilities.minImageExtent.width ? capabilities.minImageExtent.width : extent.width;
        extent.height = extent.height > capabilities.maxImageExtent.height ? capabilities.maxImageExtent.height : extent.height;
        extent.height = extent.height < capabilities.minImageExtent.height ? capabilities.minImageExtent.height : extent.height;

        return extent;
       

    }

}



int physicalDeviceSuitable(VkPhysicalDevice targetDevice)
{
    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(targetDevice, &features);
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(targetDevice, &properties);
    SwapChainDetails details = findSwapChainDetails(targetDevice);
    freeSwapChainDetails(details); //Only really care about the sizes here


    if((properties.deviceType== VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU || properties.deviceType== VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) && features.geometryShader && 
    findQueueFamilies(targetDevice).valid && checkExtensionAvailabillity(targetDevice) && details.formatSize && details.presentSize)
    {
        printf("Found suitable physical device: %s!\n", properties.deviceName);
        printf("Device has %u surface formats and %u presentation formats!\n", details.formatSize, details.presentSize);
        return 1;
    }
    return 0;
}

int pickPhysicalDevice()
{
    printf("Starting to check for physical devices!\n");
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, NULL);

    if (deviceCount == 0)
    {
        printf("No physical devices were found during vulkan init!\n");
        return 0;
    }

    VkPhysicalDevice devices[deviceCount] ;
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices);

    for (int i = 0; i < deviceCount; i++)
    {
        if (physicalDeviceSuitable(devices[i]))
        {
            physicalDevice = devices[i];
            break;
        }
    }

    if (physicalDevice == VK_NULL_HANDLE)
    {
        printf("No physical device was found to be suitable during vulkan init!\n");
        return 0;
    }
    
    return 1;
}


int createLogicalDevice()
{
    printf("Starting creation of logical device!\n");
    QueueFamilyIndices indecies = findQueueFamilies(physicalDevice);
    char queueInfoCount = 2;
    VkDeviceQueueCreateInfo queueInfos[queueInfoCount];
    uint32_t targetIndecies[] = {indecies.graphicsFamily, indecies.presentFamily};
    float queuePriority = 1.0f;

    for (int i = 0; i < queueInfoCount; i++)
    {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = targetIndecies[i];
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueInfos[i] = queueCreateInfo;
    }
    VkPhysicalDeviceFeatures deviceFeatures ={};

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.pQueueCreateInfos = queueInfos;
    createInfo.queueCreateInfoCount = queueInfoCount;

    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledLayerCount = 0;

    createInfo.enabledExtensionCount = 1;
    const char const* requiredExtentions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    createInfo.ppEnabledExtensionNames = requiredExtentions;

    char validationLayerCount = 1;
    char validationEnabled = 0;

    #ifdef DEBUG
    validationEnabled = 1;
    #endif

    const char const* requiredValidationLayers[] = {"VK_LAYER_KHRONOS_validation"};
    if (validationEnabled)
    {
        createInfo.enabledLayerCount = validationLayerCount;
        createInfo.ppEnabledLayerNames = requiredValidationLayers;
    }

    if (vkCreateDevice(physicalDevice, &createInfo, NULL, &logicalDevice) != VK_SUCCESS) 
    {
        printf("Logical device creation failed!\n");
        return 0;
    }

    printf("Logical device creation OK!\n");
    vkGetDeviceQueue(logicalDevice, indecies.graphicsFamily, 0, &graphicsQueue);
    vkGetDeviceQueue(logicalDevice, indecies.graphicsFamily, 0, &presentQueue);
    return 1;
}

int createSurface(GLFWwindow* window)
{
    
}

int createSwapChain(GLFWwindow* window)
{
    printf("Starting to create Swapchain!\n");
    SwapChainDetails details = findSwapChainDetails(physicalDevice);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(details.formats, details.formatSize);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(details.presentModes, details.presentSize);
    VkExtent2D extent = chooseSwapExtent(details.capabilities, window);

    uint32_t imageCount = details.capabilities.minImageCount + 1;

    if (details.capabilities.maxImageCount > 0 && imageCount > details.capabilities.maxImageCount) 
    {
    imageCount = details.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = windowSurface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indecies = findQueueFamilies(physicalDevice);
    uint32_t queueFamilyIndices[] = {indecies.graphicsFamily, indecies.presentFamily};

    if (indecies.graphicsFamily != indecies.presentFamily)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0; 
        createInfo.pQueueFamilyIndices = NULL;
    }

    createInfo.preTransform = details.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(logicalDevice, &createInfo, NULL, &swapChain) != VK_SUCCESS)
    {
        freeSwapChainDetails(details);
        printf("SwapChain Creation failed!\n");
        return 0;
    }
    freeSwapChainDetails(details);
    printf("SwapChain creation OK!\n");
    vkGetSwapchainImagesKHR(logicalDevice, swapChain, &swapChainImageCount, NULL);
    swapChainImages = malloc(sizeof(VkImage) * swapChainImageCount);//TODO: This should probably never need to be freed, but still feels like a good idea to do so. Figure out how to conditionally free this if the code has reached here in cleanup.
    vkGetSwapchainImagesKHR(logicalDevice, swapChain, &swapChainImageCount, swapChainImages);
    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;
    return 1;
}

int createSwapChainImageViews()
{
    printf("Starting creation of Swap Chain Image Views!\n");
    swapChainImageViews = malloc(sizeof(VkImageView) * swapChainImageCount);
    for (int i = 0; i < swapChainImageCount; i++)
    {
        VkImageViewCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = swapChainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = swapChainImageFormat;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;
        if (vkCreateImageView(logicalDevice, &createInfo, NULL, &swapChainImageViews[i]) != VK_SUCCESS)
        {
            printf("Swap Chain Image View Creation failed on loop: %i\n", i);
            return 0;
        }
    }

    printf("Swap Chain Image View Creation OK!\n");
    return 1;
}

int initVulkan(GLFWwindow* window)
{
    if (!createInstance())
    {
        return 0;
    }

    if (!glfwCreateWindowSurface(instance, window, NULL, &windowSurface) == VK_SUCCESS)
    {
        printf("Creating window surface failed!\n");
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
        printf("%s\n", extensions[i].extensionName);
    }
    if (!pickPhysicalDevice())
    {
        return 0;
    }
    if (!createLogicalDevice())
    {
        return 0;   
    }
    if (!createSwapChain(window))
    {
        return 0;
    }
    if (!(createSwapChainImageViews()))
    {
        return 0;
    }

    

    return 1;
}

int initWindow(GLFWwindow** window)
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    *window = glfwCreateWindow(windowWidth, windowHeight, "Vulkan", NULL, NULL);
}

int cleanupWindow(GLFWwindow** window)
{

    for (int i = 0; i < swapChainImageCount; i++)
    {
        vkDestroyImageView(logicalDevice, swapChainImageViews[i], NULL);
    }

    vkDestroySwapchainKHR(logicalDevice, swapChain, NULL);
    vkDestroyDevice(logicalDevice, NULL);
    vkDestroyInstance(instance, NULL);
    glfwDestroyWindow(*window);

    glfwTerminate();
}


    
