#pragma once

#include "VulkanIncludes.h"

void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
void CopyBufferImmediate(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
void CopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);