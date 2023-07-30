#include "UniformBuffer.h"
#include "VulkanBuffer.h"
#include "VulkanIncludes.h"
#include <vector>
#include "VulkanRenderer.h"

namespace PS2_Internal {
	PS2::VSConstantBuffer vertexConstBuffer;
	PS2::PSConstantBuffer pixelConstBuffer;
	std::vector<VkBuffer> vextexUniformBuffers;
	std::vector<VkDeviceMemory> vertexUniformBuffersMemory;
	std::vector<VkBuffer> pixelUniformBuffers;
	std::vector<VkDeviceMemory> pixelUniformBuffersMemory;

	std::vector<PS2::VertexBufferPS2<Renderer::GSVertex, uint16_t>> vertexBuffers;
}

PS2::VertexBufferPS2<Renderer::GSVertex, uint16_t>& PS2::GetBuffer()
{
	return PS2_Internal::vertexBuffers[GetCurrentFrame()];
}

const VkBuffer& PS2::GetVertexBuffer()
{
	return PS2_Internal::vertexBuffers[GetCurrentFrame()].GetVertexBuffer();
}

const VkBuffer& PS2::GetIndexBuffer()
{
	return PS2_Internal::vertexBuffers[GetCurrentFrame()].GetIndexBuffer();
}

void PS2::ResetBuffer()
{
	PS2_Internal::vertexBuffers[GetCurrentFrame()].Reset();
}

void PS2::CreateUniformBuffers() {
	const VkDeviceSize vertexBufferSize = sizeof(VSConstantBuffer);
	const VkDeviceSize pixelBufferSize = sizeof(PSConstantBuffer);

	PS2_Internal::vextexUniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
	PS2_Internal::vertexUniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);

	PS2_Internal::pixelUniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
	PS2_Internal::pixelUniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		CreateBuffer(vertexBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, PS2_Internal::vextexUniformBuffers[i], PS2_Internal::vertexUniformBuffersMemory[i]);
		CreateBuffer(pixelBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, PS2_Internal::pixelUniformBuffers[i], PS2_Internal::pixelUniformBuffersMemory[i]);
	}

	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		PS2_Internal::vertexBuffers.emplace_back(Renderer::VertexIndexBufferSize, Renderer::VertexIndexBufferSize);
	}
}

void PS2::UpdateUniformBuffers()
{
	{
		void* data;
		vkMapMemory(GetDevice(), PS2_Internal::vertexUniformBuffersMemory[GetCurrentFrame()], 0, sizeof(VSConstantBuffer), 0, &data);
		memcpy(data, &PS2_Internal::vertexConstBuffer, sizeof(VSConstantBuffer));
		vkUnmapMemory(GetDevice(), PS2_Internal::vertexUniformBuffersMemory[GetCurrentFrame()]);
	}

	{
		void* data;
		vkMapMemory(GetDevice(), PS2_Internal::pixelUniformBuffersMemory[GetCurrentFrame()], 0, sizeof(PSConstantBuffer), 0, &data);
		memcpy(data, &PS2_Internal::pixelConstBuffer, sizeof(PSConstantBuffer));
		vkUnmapMemory(GetDevice(), PS2_Internal::pixelUniformBuffersMemory[GetCurrentFrame()]);
	}
}

const VkBuffer& PS2::GetVertexConstantUniformBuffer(const int index)
{
	return PS2_Internal::vextexUniformBuffers[index];
}

const VkBuffer& PS2::GetPixelConstantUniformBuffer(const int index)
{
	return PS2_Internal::pixelUniformBuffers[index];
}

PS2::VSConstantBuffer& PS2::GetVertexConstantBufferData()
{
	return PS2_Internal::vertexConstBuffer;
}

PS2::PSConstantBuffer& PS2::GetPixelConstantBufferData()
{
	return PS2_Internal::pixelConstBuffer;
}
