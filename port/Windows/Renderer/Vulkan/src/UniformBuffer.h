#pragma once
#include "GSVector.h"
#include "VulkanIncludes.h"
#include "renderer.h"

namespace PS2 {
	struct alignas(32) VSConstantBuffer
	{
		GSVector4 VertexScale;
		GSVector4 VertexOffset;
		GSVector4 Texture_Scale_Offset;

		VSConstantBuffer()
		{
			VertexScale = GSVector4::zero();
			VertexOffset = GSVector4::zero();
			Texture_Scale_Offset = GSVector4::zero();
		}
	};

	struct alignas(32) PSConstantBuffer
	{
		GSVector4 FogColor_AREF;
		GSVector4 HalfTexel;
		GSVector4 WH;
		GSVector4 MinMax;
		GSVector4 MinF_TA;
		GSVector4i MskFix;
		GSVector4i ChannelShuffle;
		GSVector4i FbMask;

		GSVector4 TC_OffsetHack;
		GSVector4 Af;

		PSConstantBuffer()
		{
			FogColor_AREF = GSVector4::zero();
			HalfTexel = GSVector4::zero();
			WH = GSVector4::zero();
			MinMax = GSVector4::zero();
			MinF_TA = GSVector4::zero();
			MskFix = GSVector4i::zero();
			ChannelShuffle = GSVector4i::zero();
			FbMask = GSVector4i::zero();
			Af = GSVector4::zero();
		}
	};

	template<typename VertexType, typename IndexType>
	class VertexBufferPS2 {
		VkBuffer vertexBuffer;
		VkDeviceMemory vertexBufferMemory;
		VkBuffer indexBuffer;
		VkDeviceMemory indexBufferMemory;

		int vertexHead;
		int vertexMax;
		int indexHead;
		int indexMax;

	public:
		VertexBufferPS2(int vertexCount, int indexCount)
			: vertexHead(0)
			, vertexMax(vertexCount)
			, indexHead(0)
			, indexMax(indexCount)
		{
			VkDeviceSize bufferSize = sizeof(VertexType) * vertexCount;
			CreateBuffer(bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vertexBuffer, vertexBufferMemory);

			bufferSize = sizeof(IndexType) * indexCount;
			CreateBuffer(bufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, indexBuffer, indexBufferMemory);
		}

		const VkBuffer& GetVertexBuffer() {
			return vertexBuffer;
		}

		const VkBuffer& GetIndexBuffer() {
			return indexBuffer;
		}

		VkDeviceSize MapVertices(VertexType* buff, int vertexCount) {
			assert(vertexHead + vertexCount < vertexMax);
			VkDeviceSize bufferSize = sizeof(VertexType) * vertexCount;
			const VkDeviceSize bufferOffset = vertexHead * sizeof(VertexType);
			void* data;
			vkMapMemory(GetDevice(), vertexBufferMemory, bufferOffset, bufferSize, 0, &data);
			memcpy(data, buff, (size_t)bufferSize);
			vkUnmapMemory(GetDevice(), vertexBufferMemory);

			vertexHead += vertexCount;
			return bufferOffset;
		}

		VkDeviceSize MapIndices(IndexType* buff, int indexCount) {
			assert(indexHead + indexCount < indexMax);
			VkDeviceSize bufferSize = sizeof(IndexType) * indexCount;
			const VkDeviceSize bufferOffset = indexHead * sizeof(IndexType);
			void* data;
			vkMapMemory(GetDevice(), indexBufferMemory, bufferOffset, bufferSize, 0, &data);
			memcpy(data, buff, (size_t)bufferSize);
			vkUnmapMemory(GetDevice(), indexBufferMemory);

			indexHead += indexCount;
			return bufferOffset;
		}

		void Reset() {
			vertexHead = 0;
			indexHead = 0;
		}
	};

	VertexBufferPS2<Renderer::GSVertex, uint16_t>& GetBuffer();
	const VkBuffer& GetVertexBuffer();
	const VkBuffer& GetIndexBuffer();

	template<typename VertexType>
	VkDeviceSize MapVertices(VertexType* buff, int vertexCount) {
		return GetBuffer().MapVertices(buff, vertexCount);
	}

	template<typename IndexType>
	VkDeviceSize MapIndices(IndexType* buff, int indexCount) {
		return GetBuffer().MapIndices(buff, indexCount);
	}

	void ResetBuffer();
	void CreateUniformBuffers();
	void UpdateUniformBuffers();

	const VkBuffer& GetVertexConstantUniformBuffer(const int index);
	const VkBuffer& GetPixelConstantUniformBuffer(const int index);

	VSConstantBuffer& GetVertexConstantBufferData();
	PSConstantBuffer& GetPixelConstantBufferData();
}