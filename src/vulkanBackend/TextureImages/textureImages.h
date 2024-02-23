#include "../gpuBuffers.h"

typedef struct VkTextureImageObject{
    VkDevice logicalDevice;
    VkPhysicalDevice physicalDevice;
    VkImage* textureImage; 
    VkDeviceMemory* textureImageMemory;
    VkCommandPool commandPool;
    VkQueue commandQueue;
}VkTextureImageObject;

int transitionImageLayout(VkTextureImageObject ti, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, VkCommandBuffer* targetCommandBuffer);
int createTextureImage(VkTextureImageObject ti, unsigned char* textureBuffer, uint32_t textureWidth, uint32_t textureHeight, int textureChannels, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usageFlags, VkMemoryPropertyFlags properties);
int createTextureImageFromFile(VkTextureImageObject ti, char* filePath);
VkImageView createImageView(VkDevice logicalDevice, VkImage image, VkFormat format) ;