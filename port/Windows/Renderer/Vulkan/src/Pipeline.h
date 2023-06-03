#pragma once

#include "VulkanIncludes.h"
#include <unordered_map>
#include <string>

namespace PS2 {
	using LayoutBindingMap = std::unordered_map<int, std::vector<VkDescriptorSetLayoutBinding>>;
	using LayoutVector = std::vector<VkDescriptorSetLayout>;

	struct PipelineKey {
		std::string hash;
		VkPrimitiveTopology topology;
	};

	struct PipelineKeyHash {
		size_t operator()(const PipelineKey& key) const {
			size_t hash = 0;
			// combine the hash of the string and the primitive topology
			std::hash<std::string> stringHasher;
			hash ^= stringHasher(key.hash) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
			hash ^= std::hash<int>()(key.topology) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
			return hash;
		}
	};

	struct Pipeline {
		VkPipeline pipeline;
		VkPipelineLayout layout;
		VkDescriptorPool descriptorPool;
		std::vector<VkDescriptorSet> descriptorSets;
		LayoutVector descriptorSetLayouts;
		// Set - Descriptor Binding
		LayoutBindingMap descriptorSetLayoutBindings;
	};

	using PipelineMap = std::unordered_map<PipelineKey, Pipeline, PipelineKeyHash>;

	inline bool operator==(const PipelineKey& lhs, const PipelineKey& rhs) {
		return lhs.hash == rhs.hash && lhs.topology == rhs.topology;
	}

	void CreateDefaultRenderPass();
	void CreateGraphicsPipelines();
	PipelineMap& GetPipelines();
}