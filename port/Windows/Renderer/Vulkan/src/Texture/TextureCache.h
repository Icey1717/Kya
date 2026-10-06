#pragma once

#include <stdint.h>
#include <vector>
#include <optional>
#include <map>
#include "GSState.h"
#include "Objects/Pipeline.h"
#include "renderer.h"
#include "Objects/UniformBuffer.h"
#include "UploadBuffer.h"
#include "VulkanPS2.h"

namespace PS2
{
	struct GSTexDescriptor
	{
		GSTexDescriptor();

		const VkDescriptorSet& GetSet(int index) const {
			return descriptorSets[index];
		}

		size_t GetSetCount() const {
			return descriptorSets.size();
		}

		VkDescriptorPool descriptorPool;
		VkSampler sampler = VK_NULL_HANDLE;
		std::vector<VkDescriptorSet> descriptorSets;
		Renderer::LayoutBindingMap layoutBindingMap;

		// These need to be refactored!
		UniformBuffer<PS2::VSConstantBuffer> vertexConstBuffer;
		UniformBuffer<PS2::PSConstantBuffer> pixelConstBuffer;
	};

	struct GSSimpleTexture
	{
		VkImage image = VK_NULL_HANDLE;
		VkDeviceMemory imageMemory = VK_NULL_HANDLE;
		VkImageView imageView = VK_NULL_HANDLE;

		Renderer::ImageData imageData;

		// The vulkan compatible pixel data that is uploaded to the GPU. 
		// Used for upscaling and reverting to original texture data.
		TextureUpload::UploadBufferPtr pUploadBuffer;
		std::vector<TextureUpload::UploadBufferPtr> mipUploadBuffers;
		uint32_t mipLevels = 1;

		uint32_t width;
		uint32_t height;

		PSSamplerSelector samplerSelector;

		inline void UpdateSamplerSelector(const PSSamplerSelector& selector) { samplerSelector = selector; }

		void CreateResources(const bool bPalette);
		void DestroyImageResources();
		void AssignUploadBuffer(TextureUpload::UploadBufferPtr&& buffer);
		void UploadDataFromBuffer();
		void UploadMipChain(const TextureUpload::UploadBuffer* replacementBase = nullptr);
		void UploadData(int bufferSize, uint8_t* readBuffer);
		void Resize(uint32_t newWidth, uint32_t newHeight, int bufferSize, uint8_t* pixels);

		GSTexDescriptor& AddDescriptorSets(const Renderer::Pipeline& pipeline, const Renderer::DescriptorWriteList& writeList, uint32_t samplerKey = 0);
		GSTexDescriptor& GetDescriptorSets(const Renderer::Pipeline& pipeline, const Renderer::DescriptorWriteList* const pWriteList = nullptr, uint32_t samplerKey = 0);

		bool HasDescriptorSets(const Renderer::Pipeline& pipeline, uint32_t samplerKey = 0) const;

		void UpdateDescriptorSets(const Renderer::Pipeline& pipeline, const Renderer::DescriptorWriteList& writeList, int frameIndex, uint32_t samplerKey = 0);
		void UpdateDescriptorSets(const VkDescriptorSet& descriptorSet, const Renderer::LayoutBindingMap& layoutBindingMap, const Renderer::DescriptorWriteList& writeList);
		void RefreshDescriptors();

		const TextureUpload::UploadBuffer& GetUploadBuffer() const { return *pUploadBuffer; }

		// Immutable sampler variants keep already queued descriptor sets valid.
		std::map<std::pair<const Renderer::Pipeline*, uint32_t>, GSTexDescriptor> descriptorMap;
	};

	VkSampler& GetSampler(const PSSamplerSelector& selector, bool bPalette = false);
}
