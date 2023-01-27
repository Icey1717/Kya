#pragma once
#include "VulkanIncludes.h"

VkCommandBuffer BeginSingleTimeCommands();
void EndSingleTimeCommands(VkCommandBuffer commandBuffer);