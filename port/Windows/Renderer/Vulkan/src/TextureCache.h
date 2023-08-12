#pragma once

#include <stdint.h>
#include <vector>
#include <optional>
#include "GSState.h"
#include "Pipeline.h"
#include "renderer.h"

namespace Renderer {
	struct TextureData;

	const TextureData& GetImageData();
}

namespace PS2 {
	struct GSTexKey {
		Renderer::GSTex value;
		void* pBitmap;
		void* pPalette;

		static GSTexKey CreateFromTEX(const Renderer::GSTex& TEX, void* pInBitmap, void* pInPalette) {
			return { TEX, pInBitmap, pInPalette };
		}

		bool operator==(const GSTexKey& other) const {
			// #Hack this should probably key properly, using the pointers here is bad.
			return value == other.value && pBitmap == other.pBitmap && pPalette == other.pPalette;
		}
	};

	struct GSTexValueCreateInfo {
		GSTexValueCreateInfo(GSTexKey& inKey, Renderer::LayoutVector& inDescriptorSetLayouts, Renderer::LayoutBindingMap& inDescriptorSetLayoutBindings)
			: key(inKey)
			, descriptorSetLayouts(inDescriptorSetLayouts)
			, descriptorSetLayoutBindings(inDescriptorSetLayoutBindings)
		{}

		GSTexValueCreateInfo(const GSTexKey& inKey, const Renderer::LayoutVector& inDescriptorSetLayouts, const Renderer::LayoutBindingMap& inDescriptorSetLayoutBindings)
			: key(inKey)
			, descriptorSetLayouts(inDescriptorSetLayouts)
			, descriptorSetLayoutBindings(inDescriptorSetLayoutBindings)
		{}

		const GSTexKey& key;
		const Renderer::LayoutVector& descriptorSetLayouts;
		const Renderer::LayoutBindingMap& descriptorSetLayoutBindings;
	};

	struct GSTexImage {
		GSTexImage(const Renderer::ImageData& inImageData)
		: imageData(inImageData)
		{

		}

		VkImage image;
		VkDeviceMemory imageMemory;
		VkImageView imageView;

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		VkSampler sampler;

		Renderer::ImageData imageData;

		uint32_t width;
		uint32_t height;

		VkDescriptorPool descriptorPool;
		std::vector<VkDescriptorSet> descriptorSets;

		void UploadData(int bufferSize, std::vector<uint8_t>& readBuffer);

		void CreateResources(const bool bTextureFiltering);
		void Cleanup();

		void CreateDescriptorSets(const Renderer::LayoutVector& descriptorSetLayouts);
		void CreateDescriptorPool(const Renderer::LayoutBindingMap& descriptorSetLayoutBindingsMap);
	};

	struct TextureDebug {
		// Debug values
		std::vector<uint32_t> paletteIndexes;
		std::vector<uint32_t> convertedPalette;
	};

	struct GSTexValue {
	public:
		GSTexValue(const GSTexValueCreateInfo& createInfo);

		// For Debug textures.
		GSTexValue(const Renderer::TextureData& inTextureData);

		void AllocateBuffers();
		void Cleanup();
		void UploadImage();
		void CreateResources();

		GSTexImage image;
		GSTexImage paletteImage;

		// Buffer emulating the PS2 vram memory.
		std::vector<uint8_t> writeBuffer;

		// Buffer of 32bit colour pixels.
		std::vector<uint8_t> readBuffer;

		TextureDebug debugData;
	};

	struct GSTexEntry {
		GSTexKey key;
		GSTexValue value;

		GSTexEntry(const GSTexValueCreateInfo& createInfo)
			: key(createInfo.key)
			, value(GSTexValue(createInfo)) {

		}

		bool operator==(const GSTexKey& other) const {
			return key == other;
		}
	};

	class TextureCache
	{
		std::vector<GSTexEntry> texcache;

	public:
		GSTexEntry& Create(const Renderer::GSTex& TEX, const Renderer::LayoutVector& descriptorSetLayouts, const Renderer::LayoutBindingMap& descriptorSetLayoutBindings);
		GSTexEntry& Lookup(const Renderer::GSTex& TEX, const Renderer::LayoutVector& descriptorSetLayouts, const Renderer::LayoutBindingMap& descriptorSetLayoutBindings);
		const std::vector<GSTexEntry>& GetEntries() { return texcache; }
	};

	inline GSTexKey CreateKeyFromTEX(const Renderer::GSTex& TEX) {
		return { TEX.TBP0, TEX.TBW, TEX.PSM, TEX.TW, (uint32_t)TEX.TH };
	}

	TextureCache& GetTextureCache();
}