#pragma once

#include "VulkanIncludes.h"
#include "VulkanImage.h"

#include <vector>

void CreateDescriptorSetLayout();
const VkDescriptorSetLayout& GetDescriptorSetLayout();

class VulkanColorImage {
public:
	VulkanColorImage(char* splashFile, int width, int height);
	~VulkanColorImage();

	const VkDescriptorSet& GetDescriptorSet(int index) { return descriptorSets[index]; }

private:
	void CreateDescriptorSets();
	void CreateDescriptorPool();

	VkDescriptorPool descriptorPool;
	std::vector<VkDescriptorSet> descriptorSets;

	VulkanImage image;
};