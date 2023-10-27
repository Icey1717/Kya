#include "UniformBuffer.h"
#include "VulkanBuffer.h"
#include "VulkanIncludes.h"
#include <vector>
#include "VulkanRenderer.h"

namespace PS2_Internal {
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

void PS2::CreateVertexUniformBuffers() {
	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		PS2_Internal::vertexBuffers.emplace_back(Renderer::VertexIndexBufferSizeGPU, Renderer::VertexIndexBufferSizeGPU);
	}
}

//const VkBuffer& PS2::GetVertexConstantUniformBuffer(const int index)
//{
//	return PS2_Internal::vextexUniformBuffers[index];
//}
//
//const VkBuffer& PS2::GetPixelConstantUniformBuffer(const int index)
//{
//	return PS2_Internal::pixelUniformBuffers[index];
//}
//
//PS2::VSConstantBuffer& PS2::GetVertexConstantBufferData()
//{
//	return PS2_Internal::vertexConstBuffer;
//}
//
//PS2::PSConstantBuffer& PS2::GetPixelConstantBufferData()
//{
//	return PS2_Internal::pixelConstBuffer;
//}
