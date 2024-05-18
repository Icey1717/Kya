#pragma once

#include <stdint.h>
#include <vector>
#include <optional>
#include "GSState.h"
#include "Pipeline.h"
#include "renderer.h"
#include "UniformBuffer.h"

namespace PS2 {
	struct GSTexKey {
		GIFReg::GSTex value;
		uint32_t CBP;
		void* pBitmap;
		void* pPalette;

		static GSTexKey CreateFromTEX(const GIFReg::GSTex& TEX, const uint32_t inCBP, void* pInBitmap, void* pInPalette) {
			return { TEX, inCBP, pInBitmap, pInPalette };
		}

		bool operator==(const GSTexKey& other) const {
			// #Hack this should probably key properly, using the pointers here is bad.
			return value == other.value && CBP == other.CBP && pBitmap == other.pBitmap && pPalette == other.pPalette;
		}
	};

	struct GSTexValueCreateInfo {
		GSTexValueCreateInfo(GSTexKey& inKey, Renderer::TextureData& inTextureData)
			: key(inKey)
			, textureData(inTextureData)
		{}

		GSTexValueCreateInfo(const GSTexKey& inKey, Renderer::TextureData& inTextureData)
			: key(inKey)
			, textureData(inTextureData)
		{}

		const GSTexKey& key;
		Renderer::TextureData textureData;
	};

	struct GSTexDescriptor {
		GSTexDescriptor();

		const VkDescriptorSet& GetSet(int index) const {
			return descriptorSets[index];
		}

		void UpdateSet(int index) const;

		VkDescriptorPool descriptorPool;
		std::vector<VkDescriptorSet> descriptorSets;
		Renderer::LayoutBindingMap layoutBindingMap;

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

		void CreateResources(const bool bPalette);
		void UploadData(int bufferSize, uint8_t* readBuffer);

		GSTexDescriptor& AddDescriptorSets(const Renderer::Pipeline& pipeline);
		GSTexDescriptor& GetDescriptorSets(const Renderer::Pipeline& pipeline);

		std::unordered_map<const Renderer::Pipeline*, GSTexDescriptor> descriptorMap;
	};

	VkSampler& GetSampler(const PSSamplerSelector& selector, bool bPalette = false);

	struct GSTexImage {
		GSTexImage(const Renderer::ImageData& inImageData);

		VkImage image = VK_NULL_HANDLE;
		VkDeviceMemory imageMemory = VK_NULL_HANDLE;
		VkImageView imageView = VK_NULL_HANDLE;

		VkBuffer stagingBuffer = VK_NULL_HANDLE;
		VkDeviceMemory stagingBufferMemory = VK_NULL_HANDLE;

		VkSampler sampler = VK_NULL_HANDLE;
		PSSamplerSelector samplerSelector;

		Renderer::ImageData imageData;

		std::unordered_map<const Renderer::Pipeline*, GSTexDescriptor> descriptorMap;

		uint32_t width;
		uint32_t height;

		// Buffer emulating the PS2 vram memory.
		std::vector<uint8_t> writeBuffer;

		// Buffer of 32bit colour pixels.
		std::vector<uint8_t> readBuffer;

		void UploadData(int bufferSize, std::vector<uint8_t>& readBuffer);

		void CreateResources(const bool bPalette);
		void Cleanup();

		GSTexDescriptor& AddDescriptorSets(const Renderer::Pipeline& pipeline);
		GSTexDescriptor& GetDescriptorSets(const Renderer::Pipeline& pipeline);

		void CreateSampler(bool bPalette = false);
		void UpdateSampler();
		void UpdateSampler(PSSamplerSelector selector);
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

		void* pMaterial;

		TextureDebug debugData;
	};

	struct GSTexEntry {
		GSTexKey key;
		GSTexValue value;

		int cleanCounter = 0;

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
		GSTexEntry& Create(const GIFReg::GSTex& TEX, Renderer::TextureData& textureData, const uint32_t CBP);
		GSTexEntry& Lookup(const GIFReg::GSTex& TEX, Renderer::TextureData& textureData, uint32_t CBP);
		const std::vector<GSTexEntry>& GetEntries() const { return texcache; }
		std::vector<GSTexEntry>& GetEntries() { return texcache; }

		// Removes all texture cache entries that belong to that ed_g2d_material.
		void RemoveByMaterial(void* pMaterial);
	};

	inline GSTexKey CreateKeyFromTEX(const GIFReg::GSTex& TEX) {
		return { TEX.TBP0, TEX.TBW, TEX.PSM, TEX.TW, (uint32_t)TEX.TH };
	}

	TextureCache& GetTextureCache();
}