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
VkShaderModule vertexShader;
VkShaderModule fragmentShader;
VkRenderPass renderPass;
VkPipelineLayout pipelineLayout;
VkPipeline graphicsPipeline;
VkFramebuffer* swapChainFrameBuffers;
VkCommandPool commandPool;
VkCommandBuffer commandBuffer;
VkSemaphore imageAvailableSemaphore;
VkSemaphore renderFinishedSemaphore;
VkFence inFlightFence;

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
    char* requiredExtentions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME, 
    VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME};
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

    createInfo.enabledExtensionCount = 3;
    const char const* requiredExtentions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME,
    VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME};
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
ShaderBytecode readShaderFile(const char* path)
{
    
    ShaderBytecode bytecode = {};
    FILE* file = fopen(path, "rb");
    if (file == NULL)
    {
        printf("File \"%s\" does not exist!", path);
        bytecode.buffer = NULL;
        bytecode.fileSize = 0;
        return bytecode;
    }
    fseek(file, 0, SEEK_END);
    bytecode.fileSize = ftell(file);

    bytecode.buffer = malloc(sizeof(char) * bytecode.fileSize);
    fseek(file, 0, SEEK_SET);
    fread(bytecode.buffer, sizeof(char), bytecode.fileSize, file);

    return bytecode;
}

VkShaderModule createShaderModule(ShaderBytecode bytecode)
{

    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = bytecode.fileSize;

    printf("Spir-v codesize is: %zd\n", createInfo.codeSize);
    createInfo.pCode = (uint32_t*)bytecode.buffer;
 
    VkShaderModule shaderModule;
    if (vkCreateShaderModule(logicalDevice, &createInfo, NULL, &shaderModule) != VK_SUCCESS) 
    {
        printf("Creation of shader module failed!\n");
    }

    
    return shaderModule;
}

int createGraphicsPipeline()
{
   printf("Starting creation of Graphics Pipeline! Finally!\n");
   ShaderBytecode vertShaderBytecode = readShaderFile("baseVertexShader.spv");
   ShaderBytecode fragShaderBytecode = readShaderFile("baseFragmentShader.spv");

    vertexShader = createShaderModule(vertShaderBytecode);
    fragmentShader = createShaderModule(fragShaderBytecode);

    free(vertShaderBytecode.buffer);
    free(fragShaderBytecode.buffer);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertexShader;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragmentShader;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};
    VkDynamicState dynamicStates[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

    VkPipelineDynamicStateCreateInfo dynamicState = {};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = 2;
    dynamicState.pDynamicStates = dynamicStates;

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.pVertexBindingDescriptions = NULL;
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    vertexInputInfo.pVertexAttributeDescriptions = NULL; 

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f; 
    rasterizer.depthBiasClamp = 0.0f; 
    rasterizer.depthBiasSlopeFactor = 0.0f; 

    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f; 
    multisampling.pSampleMask = NULL; 
    multisampling.alphaToCoverageEnable = VK_FALSE; 
    multisampling.alphaToOneEnable = VK_FALSE; 

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = NULL;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = NULL;
    if (vkCreatePipelineLayout(logicalDevice, &pipelineLayoutInfo, NULL, &pipelineLayout) != VK_SUCCESS)
     {
        printf("Graphics Pipeline Layout creation failed!\n");
        return 0;
    }

    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = NULL;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    if (vkCreateGraphicsPipelines(logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &graphicsPipeline) != VK_SUCCESS)
    {
        printf("Graphics Pipeline creation failed!\n");
        return 0;
    }

    printf("Graphics Pipeline creation OK!\n");
   return 1;
}

int createRenderPass()
{
    printf("Starting creation of Render Pass!\n");

    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = swapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;

    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;

    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(logicalDevice, &renderPassInfo, NULL, &renderPass) != VK_SUCCESS) 
    {
        printf("Render Pass creation failed!\n");
        return 0;
    }

    printf("Render Pass creation OK!\n");
    return 1;
}

int createFrameBuffers()
{
    printf("Starting FrameBuffer Creation!\n");
    swapChainFrameBuffers = malloc(sizeof(VkFramebuffer) * swapChainImageCount);

    for (uint32_t i = 0; i < swapChainImageCount; i++) 
    {
        VkImageView attachments[] = {swapChainImageViews[i]};

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = swapChainExtent.width;
        framebufferInfo.height = swapChainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(logicalDevice, &framebufferInfo, NULL, &swapChainFrameBuffers[i]) != VK_SUCCESS) 
        {
            printf("FrameBuffer Creation failed on iteration: %i!\n", i);
            return 0;
        }
    }


    printf("FrameBuffer Creation OK!\n");
    return 1;
}
int createCommandPool()
{
    printf("Starting Command Pool Creation!\n");
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;

    if (vkCreateCommandPool(logicalDevice, &poolInfo, NULL, &commandPool) != VK_SUCCESS)
    {
        printf("Command Pool Creation failed!");
        return 0;
    }
    
    printf("Command Pool Creation OK!\n");
    return 1;
}

int createCommandBuffer()
{

    printf("Starting Command Buffer Creation!\n");
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(logicalDevice, &allocInfo, &commandBuffer) != VK_SUCCESS) 
    {
        printf("Command Buffer Creation failed!\n");
        return 0;
    }
    
    printf("Command Buffer Creation OK!\n");
    return 1;

}

int createSyncObjects()
{
    printf("Starting Sync Objects Creation!\n");
    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    if (vkCreateSemaphore(logicalDevice, &semaphoreInfo, NULL, &imageAvailableSemaphore) != VK_SUCCESS ||
    vkCreateSemaphore(logicalDevice, &semaphoreInfo, NULL, &renderFinishedSemaphore) != VK_SUCCESS ||
    vkCreateFence(logicalDevice, &fenceInfo, NULL, &inFlightFence) != VK_SUCCESS) 
    {
        printf("Sync Objects Creation failed!\n");
        return 0;
    }

    printf("Sync Objects Creation OK!\n");
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
    if (!(createRenderPass()))
    {
        return 0;
    }
    if(!(createGraphicsPipeline()))
    {
        return 0;
    }
    if(!(createFrameBuffers()))
    {
        return 0;
    }
    if(!(createCommandPool()))
    {
        return 0;
    }
    if(!(createCommandBuffer()))
    {
        return 0;
    }
    if(!(createSyncObjects()))
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
    vkDestroySemaphore(logicalDevice, imageAvailableSemaphore, NULL);
    vkDestroySemaphore(logicalDevice, renderFinishedSemaphore, NULL);
    vkDestroyFence(logicalDevice, inFlightFence, NULL);

    vkDestroyCommandPool(logicalDevice, commandPool, NULL);
    vkDestroyPipeline(logicalDevice, graphicsPipeline, NULL);
    vkDestroyPipelineLayout(logicalDevice, pipelineLayout, NULL);
    

    vkDestroyShaderModule(logicalDevice, vertexShader, NULL);
    vkDestroyShaderModule(logicalDevice, fragmentShader, NULL);
    vkDestroyRenderPass(logicalDevice, renderPass, NULL);

    for (int i = 0; i < swapChainImageCount; i++)
    {
        vkDestroyFramebuffer(logicalDevice, swapChainFrameBuffers[i], NULL);
        vkDestroyImageView(logicalDevice, swapChainImageViews[i], NULL);
    }

    vkDestroySwapchainKHR(logicalDevice, swapChain, NULL);
    vkDestroyDevice(logicalDevice, NULL);
    vkDestroyInstance(instance, NULL);
    glfwDestroyWindow(*window);

    glfwTerminate();
}


int recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) 
{
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;
    beginInfo.pInheritanceInfo = NULL;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        printf("Start Begin Command Buffer failed!");
        return 0;
    }

    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = swapChainFrameBuffers[imageIndex];
    VkOffset2D renderAreaOffset = {0, 0};
    renderPassInfo.renderArea.offset = renderAreaOffset;
    renderPassInfo.renderArea.extent = swapChainExtent;
    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)swapChainExtent.width;
    viewport.height = (float)swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor = {};
    VkOffset2D scissorOffset= {0, 0};
    scissor.offset = scissorOffset;
    scissor.extent = swapChainExtent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    vkCmdDraw(commandBuffer, 3, 1, 0, 0);

    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) 
    {
        printf("Render Pass failed!\n");
        return 0;
    }
    return 1;
}

int drawFrame() 
{
    vkWaitForFences(logicalDevice, 1, &inFlightFence, VK_TRUE, UINT64_MAX);
    vkResetFences(logicalDevice, 1, &inFlightFence);

    uint32_t imageIndex;
    vkAcquireNextImageKHR(logicalDevice, swapChain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
    vkResetCommandBuffer(commandBuffer, 0);
    recordCommandBuffer(commandBuffer, imageIndex);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {imageAvailableSemaphore};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    VkSemaphore signalSemaphores[] = {renderFinishedSemaphore};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFence) != VK_SUCCESS) 
    {
        printf("Render Pass failed!\n");
        return 0;
    }

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    
    VkSwapchainKHR swapChains[] = {swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = NULL;

    vkQueuePresentKHR(presentQueue, &presentInfo);

    return 1;
}

void waitIdle()
{
    vkDeviceWaitIdle(logicalDevice);
}

    
