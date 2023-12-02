#pragma once

#include "VulkanIncludes.h"
#include <array>

void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
void CopyBufferImmediate(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
void CopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);


template<typename BufferData>
struct UniformBuffer {
	void Init() {
		const VkDeviceSize dataSize = sizeof(BufferData);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			CreateBuffer(dataSize,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				buffers[i],
				buffersMemory[i]);
		}
	}

	inline const VkBuffer& GetBuffer(const int index) { return buffers[index]; }

	inline BufferData& GetBufferData() { return bufferData; }

	inline void Update(const int index) {
		void* data;
		vkMapMemory(GetDevice(), buffersMemory[index], 0, sizeof(BufferData), 0, &data);
		memcpy(data, &bufferData, sizeof(BufferData));
		vkUnmapMemory(GetDevice(), buffersMemory[index]);
	}

	inline VkDescriptorBufferInfo GetDescBufferInfo(const int index) {
		VkDescriptorBufferInfo descBufferInfo{};
		descBufferInfo.buffer = buffers[index];
		descBufferInfo.offset = 0;
		descBufferInfo.range = sizeof(BufferData);
		return descBufferInfo;
	}

private:
	BufferData bufferData;
	std::array<VkBuffer, MAX_FRAMES_IN_FLIGHT> buffers;
	std::array<VkDeviceMemory, MAX_FRAMES_IN_FLIGHT> buffersMemory;
};