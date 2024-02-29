#include "textureImages.h"
#define STB_IMAGE_IMPLEMENTATION
#include "../../../external/stb/stb_image.h"


void copyBufferToImage(VkTextureImageObject ti, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
    VkCommandBuffer commandBuffer = beginSingleUseCommands(ti.logicalDevice, ti.commandPool);

    VkBufferImageCopy region = {};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    VkOffset3D offset = {0, 0, 0};
    region.imageOffset = offset;
    VkExtent3D extent = {width, height, 1};
    region.imageExtent = extent;

    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    endSingleUseCommands(ti.logicalDevice, ti.commandPool, commandBuffer, ti.commandQueue);
}

int transitionImageLayout(VkTextureImageObject ti, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, VkCommandBuffer* targetCommandBuffer)
{

    VkCommandBuffer commandBuffer;
    unsigned char issueCommandBuffer = 0;
    if (targetCommandBuffer == NULL)
    {
        issueCommandBuffer = 1;
        commandBuffer = beginSingleUseCommands(ti.logicalDevice, ti.commandPool);
    }
    else
    {
        commandBuffer = *targetCommandBuffer;
    }

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) 
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) 
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } 
    else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT ;

        sourceStage =  VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    } 
    else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) 
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;

        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    }
    else 
    {
        printf("Image format transformation layout is unsupported!\n");
        if (issueCommandBuffer)
        {
            endSingleUseCommands(ti.logicalDevice, ti.commandPool, commandBuffer, ti.commandQueue);
        }
        return 0;

    }

    vkCmdPipelineBarrier(
    commandBuffer,
    sourceStage, destinationStage,
    0,
    0, NULL,
    0, NULL,
    1, &barrier
    );

    if (issueCommandBuffer)
    {
        endSingleUseCommands(ti.logicalDevice, ti.commandPool, commandBuffer, ti.commandQueue);
    }

    return 1;
}

int createTextureImage(VkTextureImageObject ti, uint32_t textureWidth, uint32_t textureHeight, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usageFlags, VkMemoryPropertyFlags properties)
{
    

    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = textureWidth;
    imageInfo.extent.height = textureHeight;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usageFlags;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.flags = 0;

    if (vkCreateImage(ti.logicalDevice, &imageInfo, NULL, ti.textureImage) != VK_SUCCESS) 
    {
        printf("Image creation failed!\n");
        return 0;
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(ti.logicalDevice, *(ti.textureImage), &memRequirements);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(ti.physicalDevice, memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(ti.logicalDevice, &allocInfo, NULL, ti.textureImageMemory) != VK_SUCCESS) 
    {
        printf("Image Buffer Memory Allocation failed!\n");
    }

    vkBindImageMemory(ti.logicalDevice, *(ti.textureImage), *(ti.textureImageMemory), 0);

    
}

int createTextureImageFromBuffer(VkTextureImageObject ti, unsigned char* textureBuffer, uint32_t textureWidth, uint32_t textureHeight, int textureChannels, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usageFlags, VkMemoryPropertyFlags properties)
{
    VkDeviceSize imageSize = textureWidth * textureHeight * 4;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(ti.logicalDevice, ti.physicalDevice, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
    &stagingBuffer, &stagingBufferMemory);

    void* data;
    vkMapMemory(ti.logicalDevice, stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, textureBuffer, (size_t)imageSize);
    vkUnmapMemory(ti.logicalDevice, stagingBufferMemory);

    createTextureImage(ti, textureWidth, textureHeight, format, tiling, usageFlags, properties);

    transitionImageLayout(ti ,*(ti.textureImage), VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, NULL);
    copyBufferToImage(ti, stagingBuffer, *(ti.textureImage), textureWidth, textureHeight);
    transitionImageLayout(ti, *(ti.textureImage), VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, NULL);


    vkDestroyBuffer(ti.logicalDevice, stagingBuffer, NULL);
    vkFreeMemory(ti.logicalDevice, stagingBufferMemory, NULL);
    return 1;

}

int createTextureImageFromFile(VkTextureImageObject ti, char* filePath)
{
    int textureWidth, textureHeight, textureChannels;
    stbi_uc* pixels = stbi_load(filePath, &textureWidth, &textureHeight, &textureChannels, STBI_rgb_alpha);

    if (!pixels)
    {
        printf("Finding texture failed!\n");
        return 0;
    }
    int result = createTextureImageFromBuffer(ti, pixels, textureWidth, textureHeight, textureChannels, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, 
    VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    stbi_image_free(pixels);

    return result;
}

VkImageView createImageView(VkDevice logicalDevice, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) 
{
    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    if (vkCreateImageView(logicalDevice, &viewInfo, NULL, &imageView) != VK_SUCCESS) 
    {
        printf("Failed to create texture image view!\n");
    }

    return imageView;
}
