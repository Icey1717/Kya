#pragma once

#include "VulkanIncludes.h"
#include "VulkanReflect.h"
#include <unordered_map>
#include <string>
#include <optional>
#include "pcsx2/Selectors.h"
#include "VulkanShader.h"

namespace PipelineDebug
{
	struct ConfigData
	{
		std::string topology;
		size_t hash;
	};
}

namespace PS2 {

	struct PipelineKey
	{
		Shader::ShaderDefinitions shaderDefinitions;
		VkPrimitiveTopology topology;
		int stride;
		PipelineSelector pipelineSelector;
		PipelineDebug::ConfigData debugData;

		std::string ToString() const;
	};

	struct PipelineKeyHash
	{
		size_t operator()(const PipelineKey& key) const {
			size_t hash = 0;
			// combine the hash of the string and the primitive topology
			std::hash<std::string> stringHasher;
			hash ^= stringHasher(key.shaderDefinitions.gsDef) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
			hash ^= stringHasher(key.shaderDefinitions.psDef) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
			hash ^= stringHasher(key.shaderDefinitions.vsDef) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
			hash ^= std::hash<int>()(key.topology) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
			hash ^= std::hash<int>()(key.stride) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
			hash ^= PipelineSelectorHash()(key.pipelineSelector) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
			return hash;
		}
	};
}

namespace Renderer {
	enum class EBindingStage
	{
		Vertex,
		Geometry,
		Fragment,
		None,
	};

	using LayoutStageMap = std::unordered_map<EBindingStage, std::vector<VkDescriptorSetLayoutBinding>>;
	using LayoutBindingMap = std::unordered_map<int, LayoutStageMap>;
	using LayoutVector = std::vector<VkDescriptorSetLayout>;

	std::vector<VkDescriptorSetLayoutBinding> CollectDescriptorSets(const LayoutStageMap& stageMap);
	void CreateDescriptorPool(const LayoutBindingMap& descriptorSetLayoutBindingsMap, VkDescriptorPool& descriptorPool);

	struct Pipeline
	{
		VkPipeline pipeline;
		VkPipelineLayout layout;
		VkDescriptorPool descriptorPool;
		std::vector<VkDescriptorSet> descriptorSets;
		LayoutVector descriptorSetLayouts;
		// Set - Descriptor Binding
		LayoutBindingMap descriptorSetLayoutBindings;

		PS2::PipelineKey key;

		void AddBindings(const EBindingStage bindingStage, const ReflectData& reflectData);
		void CreateDescriptorSetLayouts();
		void CreateLayout();

		void CreateDescriptorPool();
		void CreateDescriptorSets();
	};

	struct DescriptorWrite
	{
		int dstBinding = 0;
		Renderer::EBindingStage stage = Renderer::EBindingStage::None;
		const VkDescriptorBufferInfo* pBufferInfo = nullptr;
		const VkDescriptorImageInfo* pImageInfo = nullptr;
		std::optional<VkDescriptorType> descriptorType;
	};

	struct DescriptorWriteList
	{
	private:
		std::optional<VkWriteDescriptorSet> AddWrite(Renderer::EBindingStage stage, VkDescriptorType type, const VkDescriptorSet& dstSet, int dstBinding) const;

	public:
		void EmplaceWrite(const DescriptorWrite& write)
		{
			writes.push_back(write);
		}

		std::vector<VkWriteDescriptorSet> CreateWriteDescriptorSetList(const VkDescriptorSet& dstSet, const Renderer::LayoutBindingMap& layoutBindingMap) const;
		std::vector<DescriptorWrite> writes;
	};
}

namespace PS2 {
	using PipelineMap = std::unordered_map<PipelineKey, Renderer::Pipeline, PipelineKeyHash>;

	inline bool operator==(const PipelineKey& lhs, const PipelineKey& rhs) {
		PipelineKeyHash hasher;
		return hasher(lhs) == hasher(rhs);
	}

	void CreateDefaultRenderPass();
	PipelineMap& GetPipelines();

	const Renderer::Pipeline& GetPipeline(const PS2::PipelineKey& key);
}