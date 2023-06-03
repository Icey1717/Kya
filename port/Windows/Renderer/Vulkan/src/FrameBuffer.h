#pragma once

#include "VulkanIncludes.h"

struct Vector2i { int x; int y; };
struct Vector2f { float x; float y; };

Vector2i GetRTSize();
Vector2f GetRTScale();

struct FrameBuffer {
	VkRenderPass renderPass;
	VkImage colorImage;
	VkImageView colorImageView;
	VkImage depthImage;
	VkImageView depthImageView;
	VkFramebuffer framebuffer;
	VkDeviceMemory imageMemory;
	VkDeviceMemory depthImageMemory;
	int FBP;

	static FrameBuffer Create(Vector2i size, int FBP);

	static FrameBuffer& Get(int FBP);
};