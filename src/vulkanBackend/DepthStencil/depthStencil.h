#include <vulkan/vulkan.h>
#include <stdio.h>
#include "../TextureImages/textureImages.h"

VkFormat findDepthFormat(VkPhysicalDevice physicalDevice);
VkImageView createDepthResources(VkTextureImageObject ti, uint32_t swapChainWidth, uint32_t swapChainHeight);