#pragma once

#include <stdint.h>
#include <vector>
#include "GSState.h"
#include "Pipeline.h"

namespace PS2 {
	struct GSTexKey {
		uint32_t TBP;
		uint32_t TBW;
		uint32_t PSM;
		uint32_t TW;
		uint32_t TH;

		static GSTexKey CreateFromTEX(const GSState::GSTex& TEX) {
			return { TEX.TBP, TEX.TBW, TEX.PSM, TEX.TW, TEX.TH };
		}

		bool operator==(const GSTexKey& other) const {
			return TBP == other.TBP &&
				TBW == other.TBW &&
				PSM == other.PSM &&
				TW == other.TW &&
				TH == other.TH;
		}
	};

	struct GSTexValueCreateInfo {
		GSTexValueCreateInfo(GSTexKey& inKey, LayoutVector& inDescriptorSetLayouts, LayoutBindingMap& inDescriptorSetLayoutBindings)
			: key(inKey)
			, descriptorSetLayouts(inDescriptorSetLayouts)
			, descriptorSetLayoutBindings(inDescriptorSetLayoutBindings)
		{}

		GSTexValueCreateInfo(const GSTexKey& inKey, const LayoutVector& inDescriptorSetLayouts, const LayoutBindingMap& inDescriptorSetLayoutBindings)
			: key(inKey)
			, descriptorSetLayouts(inDescriptorSetLayouts)
			, descriptorSetLayoutBindings(inDescriptorSetLayoutBindings)
		{}

		const GSTexKey& key;
		const LayoutVector& descriptorSetLayouts;
		const LayoutBindingMap& descriptorSetLayoutBindings;
	};

	struct GSTexValue {
	public:
		GSTexValue(const GSTexValueCreateInfo& createInfo);
		void Cleanup();
		void UploadImage();
		void CreateResources();

		VkImage image;
		VkDeviceMemory imageMemory;
		VkImageView imageView;

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		VkSampler sampler;

		VkDescriptorPool descriptorPool;
		std::vector<VkDescriptorSet> descriptorSets;

		int width, height;

	private:
		void CreateDescriptorSets(const LayoutVector& descriptorSetLayouts);
		void CreateDescriptorPool(const LayoutBindingMap& descriptorSetLayoutBindingsMap);
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
		GSTexEntry& Create(const GSState::GSTex& TEX, const LayoutVector& descriptorSetLayouts, const LayoutBindingMap& descriptorSetLayoutBindings);
		GSTexEntry& Lookup(const GSState::GSTex& TEX, const LayoutVector& descriptorSetLayouts, const LayoutBindingMap& descriptorSetLayoutBindings);
	};

	inline GSTexKey CreateKeyFromTEX(const GSState::GSTex& TEX) {
		return { TEX.TBP, TEX.TBW, TEX.PSM, TEX.TW, TEX.TH };
	}

	TextureCache& GetTextureCache();
}