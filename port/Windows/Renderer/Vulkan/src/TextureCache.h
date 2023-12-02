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
		const VkDescriptorSet& GetSet(int index) const {
			return descriptorSets[index];
		}

		VkDescriptorPool descriptorPool;
		std::vector<VkDescriptorSet> descriptorSets;
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

	struct GSTexImageConstantBuffer {
		UniformBuffer<PS2::VSConstantBuffer> vertexConstBuffer;
		UniformBuffer<PS2::PSConstantBuffer> pixelConstBuffer;

		void CreateUniformBuffers();
		void UpdateUniformBuffers();

		inline const VkBuffer& GetVertexConstantUniformBuffer(const int index) { return vertexConstBuffer.GetBuffer(index); }
		inline const VkBuffer& GetPixelConstantUniformBuffer(const int index) { return pixelConstBuffer.GetBuffer(index); }

		inline VSConstantBuffer& GetVertexConstantBufferData() { return vertexConstBuffer.GetBufferData(); }
		inline PSConstantBuffer& GetPixelConstantBufferData() { return pixelConstBuffer.GetBufferData(); }
	};

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

		GSTexImageConstantBuffer constantBuffer;

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

		void CreateSampler(bool bPalette = false);
		void UpdateSampler();
		void UpdateSampler(PSSamplerSelector selector);

		//void CreateDescriptorSets(const Renderer::LayoutVector& descriptorSetLayouts, const Renderer::LayoutBindingMap& descriptorSetLayoutBindingsMap);
		//void CreateDescriptorPool(const Renderer::LayoutBindingMap& descriptorSetLayoutBindingsMap);
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
		GSTexEntry& Create(const GIFReg::GSTex& TEX, Renderer::TextureData& textureData);
		GSTexEntry& Lookup(const GIFReg::GSTex& TEX, Renderer::TextureData& textureData);
		const std::vector<GSTexEntry>& GetEntries() const { return texcache; }
		std::vector<GSTexEntry>& GetEntries() { return texcache; }
	};

	inline GSTexKey CreateKeyFromTEX(const GIFReg::GSTex& TEX) {
		return { TEX.TBP0, TEX.TBW, TEX.PSM, TEX.TW, (uint32_t)TEX.TH };
	}

	TextureCache& GetTextureCache();
}