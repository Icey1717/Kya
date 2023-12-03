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

	std::string entryPointname;

	inline std::vector<DescriptorSetLayoutData>& GetLayouts() { return setLayouts; }
	inline const std::vector<DescriptorSetLayoutData>& GetLayouts() const { return setLayouts; }
	inline std::vector<VkVertexInputAttributeDescription>& GetAttributes() { return attributeDescriptions; }

	VkVertexInputBindingDescription bindingDescription;

private:
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
};

ReflectData reflectDescriptorSetLayout(const std::vector<char>& shaderCode);