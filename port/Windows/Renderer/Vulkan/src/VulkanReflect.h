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
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

	std::string entryPointname;

	inline std::vector<DescriptorSetLayoutData>& GetLayouts() { return setLayouts; }
	inline std::vector<VkVertexInputAttributeDescription>& GetAttributes() { return attributeDescriptions; }

	VkVertexInputBindingDescription bindingDescription;
};

ReflectData reflectDescriptorSetLayout(const std::vector<char>& shaderCode);