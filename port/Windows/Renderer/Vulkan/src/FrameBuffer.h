#pragma once

#include "VulkanIncludes.h"
#include <unordered_map>
#include "Pipeline.h"

struct Vector2i { int x; int y; };
struct Vector2f { float x; float y; };

Vector2i GetRTSize();
Vector2f GetRTScale();

struct FrameBuffer {
	VkRenderPass clearPass;
	VkRenderPass renderPass;
	VkRenderPass finalPass;
	VkImage colorImage;
	VkImageView colorImageView;
	VkImage depthImage;
	VkImageView depthImageView;
	VkFramebuffer framebuffer;
	VkDeviceMemory imageMemory;
	VkDeviceMemory depthImageMemory;
	int FBP;

	Renderer::Pipeline finalPipeline;
	VkSampler sampler;
	VkImage finalImage;
	VkDeviceMemory finalImageMemory;
	VkImageView finalImageView;
	VkFramebuffer finalFramebuffer;

	using FrameBufferMap = std::unordered_map<int, FrameBuffer>;

	void ExecuteClearPass();
	void ExecuteFinalPass();

	void CreateFinalPassPipeline();

	static FrameBuffer Create(Vector2i size, int FBP);

	static bool Exists(int FBP);
	static FrameBuffer& Get(int FBP);
	static FrameBufferMap& GetAll();
};