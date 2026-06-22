#include "VulkanBuffer.h"
#include "VulkanCommands.h"
#include "VulkanRenderer.h"
#include <stdexcept>
#include <utility>

VulkanBuffer::VulkanBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
{
	Create(size, usage, properties);
}

VulkanBuffer::~VulkanBuffer()
{
	Reset();
}

VulkanBuffer::VulkanBuffer(VulkanBuffer&& other) noexcept
	: device(std::exchange(other.device, VK_NULL_HANDLE))
	, allocator(std::exchange(other.allocator, nullptr))
	, buffer(std::exchange(other.buffer, VK_NULL_HANDLE))
	, memory(std::exchange(other.memory, VK_NULL_HANDLE))
{
}

VulkanBuffer& VulkanBuffer::operator=(VulkanBuffer&& other) noexcept
{
	if (this != &other) {
		Reset();
		device = std::exchange(other.device, VK_NULL_HANDLE);
		allocator = std::exchange(other.allocator, nullptr);
		buffer = std::exchange(other.buffer, VK_NULL_HANDLE);
		memory = std::exchange(other.memory, VK_NULL_HANDLE);
	}

	return *this;
}

void VulkanBuffer::Create(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
{
	Reset();

	device = GetDevice();
	allocator = GetAllocator();
	CreateBuffer(size, usage, properties, buffer, memory);
}

void VulkanBuffer::Reset()
{
	if (buffer != VK_NULL_HANDLE) {
		vkDestroyBuffer(device, buffer, allocator);
		buffer = VK_NULL_HANDLE;
	}

	if (memory != VK_NULL_HANDLE) {
		vkFreeMemory(device, memory, allocator);
		memory = VK_NULL_HANDLE;
	}

	device = VK_NULL_HANDLE;
	allocator = nullptr;
}

void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(GetDevice(), &bufferInfo, GetAllocator(), &buffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to create buffer!");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(GetDevice(), buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(GetDevice(), &allocInfo, GetAllocator(), &bufferMemory) != VK_SUCCESS) {
		vkDestroyBuffer(GetDevice(), buffer, GetAllocator());
		buffer = VK_NULL_HANDLE;
		throw std::runtime_error("failed to allocate buffer memory!");
	}

	if (vkBindBufferMemory(GetDevice(), buffer, bufferMemory, 0) != VK_SUCCESS) {
		vkFreeMemory(GetDevice(), bufferMemory, GetAllocator());
		vkDestroyBuffer(GetDevice(), buffer, GetAllocator());
		bufferMemory = VK_NULL_HANDLE;
		buffer = VK_NULL_HANDLE;
		throw std::runtime_error("failed to bind buffer memory!");
	}
}

void CopyBufferImmediate(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
	VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

	VkBufferCopy copyRegion{};
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	EndSingleTimeCommands(commandBuffer);
}

void CopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
	VkBufferCopy copyRegion{};
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
}
