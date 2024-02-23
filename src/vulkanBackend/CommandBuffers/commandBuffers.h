#include <vulkan/vulkan.h>
VkCommandBuffer beginSingleUseCommands(VkDevice logicalDevice, VkCommandPool commandPool);
void endSingleUseCommands(VkDevice logicalDevice, VkCommandPool commandPool, VkCommandBuffer commandBuffer, VkQueue graphicsQueue);