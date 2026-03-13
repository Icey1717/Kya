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

	void UpdateImage(char* newImage);

	int GetWidth() { return image.GetWidth(); }
	int GetHeight() { return image.GetHeight(); }

private:
	void CreateDescriptorSets();
	void UpdateDescriptorSets();
	void CreateDescriptorPool();

	VkDescriptorPool descriptorPool;
	std::vector<VkDescriptorSet> descriptorSets;

	VulkanImage image;
};