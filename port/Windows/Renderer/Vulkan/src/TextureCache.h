#pragma once

#include <stdint.h>
#include <vector>
#include <optional>
#include "GSState.h"
#include "Pipeline.h"
#include "renderer.h"
#include "UniformBuffer.h"

namespace PS2 {
	struct GSTexDescriptor {
		GSTexDescriptor();

		const VkDescriptorSet& GetSet(int index) const {
			return descriptorSets[index];
		}

		VkDescriptorPool descriptorPool;
		std::vector<VkDescriptorSet> descriptorSets;
		Renderer::LayoutBindingMap layoutBindingMap;

		// These need to be refactored!
		UniformBuffer<PS2::VSConstantBuffer> vertexConstBuffer;
		UniformBuffer<PS2::PSConstantBuffer> pixelConstBuffer;
	};

	struct PSSamplerSelector
	{
		union
		{
			struct
			{
				uint32_t tau : 1;
				uint32_t tav : 1;
				uint32_t ltf : 1;
			};

			uint32_t key;
		};

		operator uint32_t() { return key & 0x7; }

		PSSamplerSelector() : key(0) {}
	};

	struct GSSimpleTexture {
		VkImage image = VK_NULL_HANDLE;
		VkDeviceMemory imageMemory = VK_NULL_HANDLE;
		VkImageView imageView = VK_NULL_HANDLE;

		Renderer::ImageData imageData;

		uint32_t width;
		uint32_t height;

		PSSamplerSelector samplerSelector;

		inline void UpdateSamplerSelector(const PSSamplerSelector& selector) { samplerSelector = selector; }

		void CreateResources(const bool bPalette);
		void UploadData(int bufferSize, uint8_t* readBuffer);

		GSTexDescriptor& AddDescriptorSets(const Renderer::Pipeline& pipeline);
		GSTexDescriptor& GetDescriptorSets(const Renderer::Pipeline& pipeline);

		void UpdateDescriptorSets(const Renderer::Pipeline& pipeline, const Renderer::DescriptorWriteList& writeList);
		void UpdateDescriptorSets(const VkDescriptorSet& descriptorSet, const Renderer::LayoutBindingMap& layoutBindingMap, const Renderer::DescriptorWriteList& writeList);

		std::unordered_map<const Renderer::Pipeline*, GSTexDescriptor> descriptorMap;
	};

	VkSampler& GetSampler(const PSSamplerSelector& selector, bool bPalette = false);
}