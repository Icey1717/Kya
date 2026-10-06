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
	struct TextureBinding
	{
		VkDescriptorPool pool = VK_NULL_HANDLE;
		VkDescriptorSet set = VK_NULL_HANDLE;
		VkSampler sampler = VK_NULL_HANDLE;
	};

	// All material pipelines use set 1, binding 0 with this layout.
	Renderer::LayoutStageMap GetTextureLayoutBindings();
	VkDescriptorSetLayout GetTextureLayout();
	void CleanupSamplerCache();

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

		void CreateResources(const bool bPalette);
		void DestroyImageResources();
		void AssignUploadBuffer(TextureUpload::UploadBufferPtr&& buffer);
		void UploadDataFromBuffer();
		void UploadMipChain(const TextureUpload::UploadBuffer* replacementBase = nullptr);
		void UploadData(int bufferSize, uint8_t* readBuffer);
		void Resize(uint32_t newWidth, uint32_t newHeight, int bufferSize, uint8_t* pixels);

		VkDescriptorSet GetOrCreateTextureBinding(const SamplerDescription& sampler);
		void RefreshDescriptors();

		const TextureUpload::UploadBuffer& GetUploadBuffer() const { return *pUploadBuffer; }

		// Immutable sampler variants keep already queued descriptor sets valid.
		std::map<uint32_t, TextureBinding> textureBindings;
	};

	VkSampler GetSampler(const SamplerDescription& sampler);
}
