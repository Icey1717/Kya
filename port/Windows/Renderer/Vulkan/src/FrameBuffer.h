#pragma once

#include "VulkanIncludes.h"
#include <unordered_map>
#include "Pipeline.h"

struct Vector2i { int x; int y; };
struct Vector2f { float x; float y; };

Vector2i GetRTSize();
Vector2f GetRTScale();

namespace Renderer {
	struct FrameBufferBase {
		void SetupBase(Vector2i size, const VkRenderPass& renderPass, bool bDepthAttachment);

		VkImage colorImage;
		VkImageView colorImageView;

		VkImage depthImage;
		VkImageView depthImageView;

		VkFramebuffer framebuffer;
		VkDeviceMemory imageMemory;
		VkDeviceMemory depthImageMemory;

		VkSampler sampler;
	};
}

namespace PS2 {
	struct FrameBuffer : public Renderer::FrameBufferBase {
		VkRenderPass clearPass;
		VkRenderPass renderPass;
		VkRenderPass finalPass;
	
		int FBP;

		Renderer::Pipeline finalPipeline;

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
}