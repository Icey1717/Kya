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
		GIFReg::GSTex value;
		void* pBitmap;
		void* pPalette;

		static GSTexKey CreateFromTEX(const GIFReg::GSTex& TEX, void* pInBitmap, void* pInPalette) {
			return { TEX, pInBitmap, pInPalette };
		}

		bool operator==(const GSTexKey& other) const {
			// #Hack this should probably key properly, using the pointers here is bad.
			return value == other.value && pBitmap == other.pBitmap && pPalette == other.pPalette;
		}
	};

	struct GSTexValueCreateInfo {
		GSTexValueCreateInfo(GSTexKey& inKey)
			: key(inKey)
		{}

		GSTexValueCreateInfo(const GSTexKey& inKey)
			: key(inKey)
		{}

		const GSTexKey& key;
	};

	struct GSTexDescriptor {
		const VkDescriptorSet& GetSet(int index) const {
			return descriptorSets[index];
		}

		VkDescriptorPool descriptorPool;
		std::vector<VkDescriptorSet> descriptorSets;
	};

	struct GSTexImage {
		GSTexImage(const Renderer::ImageData& inImageData);

		VkImage image;
		VkDeviceMemory imageMemory;
		VkImageView imageView;

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		VkSampler sampler;

		Renderer::ImageData imageData;

		std::unordered_map<const Renderer::Pipeline*, GSTexDescriptor> descriptorMap;

		uint32_t width;
		uint32_t height;

		// Buffer emulating the PS2 vram memory.
		std::vector<uint8_t> writeBuffer;

		// Buffer of 32bit colour pixels.
		std::vector<uint8_t> readBuffer;

		void UploadData(int bufferSize, std::vector<uint8_t>& readBuffer);

		void CreateResources(const bool bTextureFiltering);
		void Cleanup();

		const GSTexDescriptor& AddDescriptorSets(const Renderer::Pipeline& pipeline);
		const GSTexDescriptor& GetDescriptorSets(const Renderer::Pipeline& pipeline);

		void CreateDescriptorSets(const Renderer::LayoutVector& descriptorSetLayouts, const Renderer::LayoutBindingMap& descriptorSetLayoutBindingsMap);
		void CreateDescriptorPool(const Renderer::LayoutBindingMap& descriptorSetLayoutBindingsMap);
	};

	struct TextureDebug {
		// Debug values
		std::vector<uint32_t> paletteIndexes;
	};

	struct GSTexValue {
	public:
		GSTexValue(const GSTexValueCreateInfo& createInfo);

		// For Debug textures.
		GSTexValue(const Renderer::TextureData& inTextureData, uint32_t CBP);

		void Cleanup();
		void UploadImage();
		void CreateResources();

		GSTexImage image;
		GSTexImage paletteImage;

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
		GSTexEntry& Create(const GIFReg::GSTex& TEX);
		GSTexEntry& Lookup(const GIFReg::GSTex& TEX);
		const std::vector<GSTexEntry>& GetEntries() { return texcache; }
	};

	inline GSTexKey CreateKeyFromTEX(const GIFReg::GSTex& TEX) {
		return { TEX.TBP0, TEX.TBW, TEX.PSM, TEX.TW, (uint32_t)TEX.TH };
	}

	TextureCache& GetTextureCache();
}