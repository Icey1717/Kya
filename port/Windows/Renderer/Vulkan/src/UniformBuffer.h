#pragma once
#include "GSVector.h"
#include "VulkanIncludes.h"
#include "renderer.h"
#include "VulkanBuffer.h"

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
	class VertexBuffer {
		VkBuffer vertexBuffer;
		VkDeviceMemory vertexBufferMemory;
		VkBuffer indexBuffer;
		VkDeviceMemory indexBufferMemory;

		int vertexHead;
		int vertexMax;
		int indexHead;
		int indexMax;

	public:
		VertexBuffer(const int vertexCount, const int indexCount)
			: vertexHead(0)
			, vertexMax(vertexCount)
			, indexHead(0)
			, indexMax(indexCount)
		{
			if (vertexCount > 0) {
				VkDeviceSize bufferSize = sizeof(VertexType) * vertexCount;
				CreateBuffer(bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vertexBuffer, vertexBufferMemory);
			}

			if (indexCount > 0) {
				VkDeviceSize bufferSize = sizeof(IndexType) * indexCount;
				CreateBuffer(bufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, indexBuffer, indexBufferMemory);
			}
		}

		const VkBuffer& GetVertexBuffer() const {
			return vertexBuffer;
		}

		const VkBuffer& GetIndexBuffer() const {
			return indexBuffer;
		}

		int GetVertexBufferMax() const {
			return vertexMax;
		}

		int GetIndexBufferMax() const {
			return indexMax;
		}

		VkDeviceSize MapVertices(const VertexType* buff, int vertexCount) {
			if (vertexCount < 1) {
				return 0;
			}

			assert(vertexHead + vertexCount <= vertexMax);
			VkDeviceSize bufferSize = sizeof(VertexType) * vertexCount;
			const VkDeviceSize bufferOffset = vertexHead * sizeof(VertexType);
			void* data;
			vkMapMemory(GetDevice(), vertexBufferMemory, bufferOffset, bufferSize, 0, &data);
			memcpy(data, buff, (size_t)bufferSize);
			vkUnmapMemory(GetDevice(), vertexBufferMemory);

			vertexHead += vertexCount;
			return bufferOffset;
		}

		VkDeviceSize MapIndices(const IndexType* buff, int indexCount) {
			if (indexCount < 1) {
				return 0;
			}

			assert(indexHead + indexCount <= indexMax);
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

	template<typename VertexType, typename IndexType>
	class FrameVertexBuffers {
	public:
		void Init(const int vertexCount, const int indexCount) {
			for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
				buffers.emplace_back(vertexCount, indexCount);
			}

			drawBufferData.Init(vertexCount, indexCount);
		}

		VertexBuffer<VertexType, IndexType>& GetBuffer() {
			return buffers[GetCurrentFrame()];
		}

		const VkBuffer& GetVertexBuffer() {
			return buffers[GetCurrentFrame()].GetVertexBuffer();
		}

		const VkBuffer& GetIndexBuffer() {
			return buffers[GetCurrentFrame()].GetIndexBuffer();
		}

		int GetVertexBufferMax() const {
			return drawBufferData.vertex.maxcount;
		}

		int GetIndexBufferMax() const {
			return drawBufferData.index.maxcount;
		}

		DrawBufferData<VertexType, IndexType>& GetDrawBufferData() {
			return drawBufferData;
		}

		VkDeviceSize MapVertexData() {
			return GetBuffer().MapVertices(drawBufferData.vertex.buff, drawBufferData.vertex.tail);
		}

		VkDeviceSize MapIndexData() {
			return GetBuffer().MapIndices(drawBufferData.index.buff, drawBufferData.index.tail);
		}

		void MapAndBindVertexData(const VkCommandBuffer& cmd) {
			const VkDeviceSize vertexOffset = MapVertexData();
			VkBuffer vertexBuffers[] = { GetVertexBuffer() };
			VkDeviceSize offsets[] = { vertexOffset };
			vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers, offsets);
		}

		void MapAndBindIndexData(const VkCommandBuffer& cmd) {
			const VkDeviceSize indexOffset = MapIndexData();
			vkCmdBindIndexBuffer(cmd, GetIndexBuffer(), indexOffset, VK_INDEX_TYPE_UINT16);
		}

		void MapAndBindData(const VkCommandBuffer& cmd) {
			MapAndBindVertexData(cmd);
			MapAndBindIndexData(cmd);
		}

		void MapData() {
			MapVertexData();
			MapIndexData();
		}

		void BindBuffers(const VkCommandBuffer& cmd) {
			VkBuffer vertexBuffers[] = { GetVertexBuffer() };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers, offsets);

			vkCmdBindIndexBuffer(cmd, GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT16);
		}

		void Reset() {
			buffers[GetCurrentFrame()].Reset();
		}

		void MergeData(const DrawBufferData<VertexType, IndexType>& newDrawBufferData) {
			const auto drawBufferTail = drawBufferData.GetIndexTail();
			const auto newDrawBufferTail = newDrawBufferData.GetIndexTail();

			const auto vertexTail = drawBufferData.GetVertexTail();
			const auto newVertexTail = newDrawBufferData.GetVertexTail();

			assert(drawBufferTail + newDrawBufferTail <= drawBufferData.index.maxcount);
			assert(vertexTail + newVertexTail <= drawBufferData.vertex.maxcount);

			IndexType* __restrict pIndexDst = drawBufferData.index.buff + drawBufferTail;
			const IndexType* __restrict pIndexSrc = newDrawBufferData.index.buff;

			VertexType* __restrict pVertexDst = drawBufferData.vertex.buff + vertexTail;
			const VertexType* __restrict pVertexSrc = newDrawBufferData.vertex.buff;

			memcpy(pIndexDst, pIndexSrc, newDrawBufferTail * sizeof(IndexType));
			memcpy(pVertexDst, pVertexSrc, newVertexTail * sizeof(VertexType));

			drawBufferData.index.tail += newDrawBufferTail;
			drawBufferData.vertex.tail += newVertexTail;
		}

	private:
		std::vector<VertexBuffer<VertexType, IndexType>> buffers;
		DrawBufferData<VertexType, IndexType> drawBufferData;
	};

	UniformBuffer<PS2::VSConstantBuffer>& GetVertexUniformBuffer();
	UniformBuffer<PS2::PSConstantBuffer>& GetPixelUniformBuffer();
}