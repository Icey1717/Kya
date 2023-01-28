#pragma once

#include "VulkanIncludes.h"

VkImageView CreateImageView(VkImage image, VkFormat format);

class VulkanImage {
public:
	VulkanImage(char* splashFile, int width, int height);
	~VulkanImage();

	const VkImage& GetTextureImage() const { return textureImage; }
	const VkDeviceMemory& GetTextureImageMemory() const { return textureImageMemory; }
	const VkImageView& GetTextureImageView() const { return textureImageView; }
	const VkSampler& GetTextureSampler() const { return textureSampler; }

	void UpdateImage(char* pixelData);

	int GetWidth() { return texWidth; }
	int GetHeight() { return texHeight; }

private:
	void CreateTextureImage(char* splashFile);
	void CreateTextureImageView();
	void CreateTextureSampler();
	void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
	void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
	void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

	VkImage textureImage;
	VkDeviceMemory textureImageMemory;
	VkImageView textureImageView;
	VkSampler textureSampler;

	int texWidth;
	int texHeight;
};