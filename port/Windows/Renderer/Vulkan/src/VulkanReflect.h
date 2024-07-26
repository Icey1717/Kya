#pragma once

#include "VulkanIncludes.h"
#include <vector>
#include <string>

struct DescriptorSetLayoutData {
	uint32_t setNumber;
	std::vector<VkDescriptorSetLayoutBinding> bindings;
};

struct ReflectData {
	std::vector<DescriptorSetLayoutData> setLayouts;
	std::vector<VkPushConstantRange> pushConstants;

	std::string entryPointname;

	inline std::vector<DescriptorSetLayoutData>& GetLayouts() { return setLayouts; }
	inline const std::vector<DescriptorSetLayoutData>& GetLayouts() const { return setLayouts; }
	inline std::vector<VkVertexInputAttributeDescription>& GetAttributes() { return attributeDescriptions; }
	inline std::string& GetEntryPointName() { return entryPointname; }

	inline void MarkUniformBufferDynamic(const uint32_t setNumber, const uint32_t bindingNumber) {
		for (auto& set : setLayouts) {
			if (set.setNumber == setNumber) {
				for (auto& binding : set.bindings) {
					if (binding.binding == bindingNumber) {
						binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
						return;
					}
				}
			}
		}
	}

	VkVertexInputBindingDescription bindingDescription;

private:
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
};

ReflectData CreateReflectionData(const std::vector<char>& shaderCode);