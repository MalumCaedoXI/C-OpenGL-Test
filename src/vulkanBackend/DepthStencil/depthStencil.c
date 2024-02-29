#include "depthStencil.h"



VkFormat findSupportedFormat(VkPhysicalDevice physicalDevice, VkFormat* candidates, int canidateSize, VkImageTiling tiling, VkFormatFeatureFlags features) 
{
    for (int i = 0; i < canidateSize; i++)
    {
        VkFormat format = candidates[i];
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) 
        {
            return format;
        } 
        else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) 
        {
            return format;
        }
    }

    printf("Failed to find a correct format for depth stencil! Trying default out of desperation...\n");
    return VK_FORMAT_R32G32B32_SFLOAT;
}

VkFormat findDepthFormat(VkPhysicalDevice physicalDevice) {
    VkFormat formats[] = {VK_FORMAT_D32_SFLOAT, 
                        VK_FORMAT_D32_SFLOAT_S8_UINT, 
                        VK_FORMAT_D24_UNORM_S8_UINT};
    return findSupportedFormat(physicalDevice, formats, 3, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

VkImageView createDepthResources(VkTextureImageObject ti, uint32_t swapChainWidth, uint32_t swapChainHeight)
{
    VkFormat depthFormat = findDepthFormat(ti.physicalDevice);

    createTextureImage(ti, swapChainWidth, swapChainHeight, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    return createImageView(ti.logicalDevice, *ti.textureImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
}