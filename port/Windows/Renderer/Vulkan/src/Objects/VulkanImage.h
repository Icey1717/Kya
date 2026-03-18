#pragma once

#include "VulkanIncludes.h"

struct OwnedImage
{
	VkImage        image  = VK_NULL_HANDLE;
	VkDeviceMemory memory = VK_NULL_HANDLE;
	VkImageView    view   = VK_NULL_HANDLE;

	void Destroy();
};

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

	static void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageAspectFlags aspectMask, VkCommandBuffer commandBuffer = VK_NULL_HANDLE);
	static void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, VkCommandBuffer commandBuffer = VK_NULL_HANDLE);
	
	static void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
	static void CreateImageView(const VkImage& image, VkFormat format, VkImageAspectFlags aspect, VkImageView& imageView);

	// Convenience factories: create image + view in one call.
	// extraUsage is OR-ed with the base usage flags for that image type.
	static OwnedImage CreateColor(uint32_t width, uint32_t height, VkImageUsageFlags extraUsage = 0);
	static OwnedImage CreateDepth(uint32_t width, uint32_t height, VkImageUsageFlags extraUsage = 0);

private:
	void CreateTextureImage(char* splashFile);
	void CreateTextureImageView();
	void CreateTextureSampler();

	VkImage textureImage;
	VkDeviceMemory textureImageMemory;
	VkImageView textureImageView;
	VkSampler textureSampler;

	int texWidth;
	int texHeight;
};