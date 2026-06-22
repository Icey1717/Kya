#pragma once

#include "VulkanIncludes.h"
#include <array>

class VulkanBuffer
{
public:
	VulkanBuffer() = default;
	VulkanBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
	~VulkanBuffer();

	VulkanBuffer(const VulkanBuffer&) = delete;
	VulkanBuffer& operator=(const VulkanBuffer&) = delete;

	VulkanBuffer(VulkanBuffer&& other) noexcept;
	VulkanBuffer& operator=(VulkanBuffer&& other) noexcept;

	void Create(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
	void Reset();

	const VkBuffer& Get() const { return buffer; }
	const VkDeviceMemory& Memory() const { return memory; }

private:
	VkDevice device = VK_NULL_HANDLE;
	VkAllocationCallbacks* allocator = nullptr;
	VkBuffer buffer = VK_NULL_HANDLE;
	VkDeviceMemory memory = VK_NULL_HANDLE;
};

void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
void CopyBufferImmediate(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
void CopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

// Utility to round up to next power of two
inline size_t NextPowerOfTwo(size_t v)
{
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;

	if constexpr (sizeof(size_t) == 8)
	{
		v |= v >> 32;
	}

	v++;
	return v;
}

template<typename BufferType>
struct DynamicUniformBuffer
{
	DynamicUniformBuffer() = default;

	void Init(int instanceCount, uint32_t range = 1)
	{
		this->range = range;
		this->instanceCount = instanceCount;

		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(GetPhysicalDevice(), &properties);

		const size_t minUboAlignment = properties.limits.minUniformBufferOffsetAlignment;
		size_t baseAlignment = std::max(sizeof(BufferType), minUboAlignment);

		dynamicAlignment = NextPowerOfTwo(baseAlignment);

		size = dynamicAlignment * instanceCount;

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			buffers[i].Create(size,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		}

		bufferData = reinterpret_cast<BufferType*>(_aligned_malloc(size, dynamicAlignment));
		assert(bufferData != nullptr);
	}

	~DynamicUniformBuffer() {
		_aligned_free(bufferData);
	}

	DynamicUniformBuffer(const DynamicUniformBuffer&) = delete;
	DynamicUniformBuffer& operator=(const DynamicUniformBuffer&) = delete;

	inline const VkBuffer& GetBuffer(const int index) { return buffers[index].Get(); }

	void DestroyResources() {
		for (auto& buffer : buffers) {
			buffer.Reset();
		}
	}

	inline BufferType* GetBufferData() { return bufferData; }

	inline BufferType* GetInstancePtr(int index) const {
		assert((index + (range - 1)) < instanceCount);
		return reinterpret_cast<BufferType*>(
			reinterpret_cast<char*>(bufferData) + index * dynamicAlignment);
	}

	inline void SetInstanceData(int index, const BufferType& data) {
		assert((index + (range - 1)) < instanceCount);
		*GetInstancePtr(index) = data;
	}

	inline uint32_t GetDynamicAlignment() const { return static_cast<uint32_t>(dynamicAlignment); }

	// For handing off to dynamicOffsets in vkCmdBindDescriptorSets.
	uint32_t GetOffsetForIndex(const int index) const
	{
		assert((index + (range - 1)) < instanceCount);
		return index * GetDynamicAlignment();
	}

	inline VkDeviceSize GetSize() const { return size; }

	inline void Map(const int index) {
		void* data;
		vkMapMemory(GetDevice(), buffers[index].Memory(), 0, size, 0, &data);
		memcpy(data, bufferData, size);
		vkUnmapMemory(GetDevice(), buffers[index].Memory());
	}

	inline VkDescriptorBufferInfo GetDescBufferInfo(const int index, const int range = 0) const {
		VkDescriptorBufferInfo descBufferInfo{};
		descBufferInfo.buffer = buffers[index].Get();
		descBufferInfo.offset = 0;
		descBufferInfo.range = range == 0 ? dynamicAlignment : range;
		return descBufferInfo;
	}

private:
	BufferType* bufferData = nullptr;
	VkDeviceSize size = 0;
	VkDeviceSize dynamicAlignment = 0;
	std::array<VulkanBuffer, MAX_FRAMES_IN_FLIGHT> buffers;
	uint32_t range = 0;
	int instanceCount = 0;
};

// A densely-packed, per-frame GPU storage buffer (SSBO).
// Unlike DynamicUniformBuffer, there is no alignment padding between elements —
// data is stored as a plain array. The buffer is re-uploaded in full each frame
// via Map(). Shaders access elements by an absolute index supplied via push constants.
template<typename BufferType>
struct StorageBuffer
{
	StorageBuffer() = default;

	void Init(int capacity)
	{
		this->capacity = capacity;
		const VkDeviceSize bufferSize = sizeof(BufferType) * capacity;

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			buffers[i].Create(bufferSize,
				VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		}

		bufferData = new BufferType[capacity];
	}

	inline void SetInstanceData(int index, const BufferType& data) {
		assert(index >= 0 && index < capacity);
		bufferData[index] = data;
	}

	inline BufferType* GetInstancePtr(int index) const {
		assert(index >= 0 && index < capacity);
		return &bufferData[index];
	}

	inline int GetCapacity() const { return capacity; }

	inline void Map(const int frameIndex) {
		const VkDeviceSize bufferSize = sizeof(BufferType) * capacity;
		void* data;
		vkMapMemory(GetDevice(), buffers[frameIndex].Memory(), 0, bufferSize, 0, &data);
		memcpy(data, bufferData, bufferSize);
		vkUnmapMemory(GetDevice(), buffers[frameIndex].Memory());
	}

	inline VkDescriptorBufferInfo GetDescBufferInfo(const int frameIndex) const {
		VkDescriptorBufferInfo descBufferInfo{};
		descBufferInfo.buffer = buffers[frameIndex].Get();
		descBufferInfo.offset = 0;
		descBufferInfo.range = sizeof(BufferType) * capacity;
		return descBufferInfo;
	}

	~StorageBuffer() {
		delete[] bufferData;
	}

	StorageBuffer(const StorageBuffer&) = delete;
	StorageBuffer& operator=(const StorageBuffer&) = delete;

	void DestroyResources() {
		for (auto& buffer : buffers) {
			buffer.Reset();
		}
	}

private:
	BufferType* bufferData = nullptr;
	int capacity = 0;
	std::array<VulkanBuffer, MAX_FRAMES_IN_FLIGHT> buffers;
};

template<typename BufferType>
struct UniformBuffer {
	UniformBuffer() = default;

	void Init() {
		const VkDeviceSize dataSize = sizeof(BufferType);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			buffers[i].Create(dataSize,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		}
	}

	UniformBuffer(const UniformBuffer&) = delete;
	UniformBuffer& operator=(const UniformBuffer&) = delete;

	void DestroyResources() {
		for (auto& buffer : buffers) {
			buffer.Reset();
		}
	}

	inline const VkBuffer& GetBuffer(const int index) { return buffers[index].Get(); }

	inline BufferType& GetBufferData() { return bufferData; }

	inline void Map(const int index) {
		void* data;
		vkMapMemory(GetDevice(), buffers[index].Memory(), 0, sizeof(BufferType), 0, &data);
		memcpy(data, &bufferData, sizeof(BufferType));
		vkUnmapMemory(GetDevice(), buffers[index].Memory());
	}

	inline VkDescriptorBufferInfo GetDescBufferInfo(const int index) const {
		VkDescriptorBufferInfo descBufferInfo{};
		descBufferInfo.buffer = buffers[index].Get();
		descBufferInfo.offset = 0;
		descBufferInfo.range = sizeof(BufferType);
		return descBufferInfo;
	}

private:
	BufferType bufferData;
	std::array<VulkanBuffer, MAX_FRAMES_IN_FLIGHT> buffers;
};
