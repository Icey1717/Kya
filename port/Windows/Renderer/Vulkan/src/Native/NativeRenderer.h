#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>
#include "Objects/Pipeline.h"
#include "Objects/FrameBuffer.h"
#include "VulkanIncludes.h"

#include "renderer.h"

#include <glm/glm.hpp>

namespace Renderer
{
	struct SimpleTexture;
	struct SimpleMesh;

	namespace Native
	{
		constexpr int kDefaultWidth = 0x200;
		constexpr int kDefaultHeight = 0x200;
		extern int gWidth;
		extern int gHeight;

		constexpr uint32_t topologyTriangleList = 0;
		constexpr uint32_t topologyLineList = 1;

		struct PipelineKey {
			union {
				// 32 bit key
				struct {
					uint32_t bWireframe : 1;
					uint32_t bGlsl : 1;
					uint32_t topology : 2;
				} options;

				uint32_t key{};
			};
		};

		static_assert(sizeof(uint32_t) == sizeof(PipelineKey), "PipelineKey size mismatch");

		template<typename KeyType>
		struct PipelineCreateInfo {
			std::string vertShaderFilename;
			std::string fragShaderFilename;
			std::string geomShaderFilename;

			KeyType key;
		};

		struct GraphicsPipelineState
		{
			bool depthTestEnable = true;
			bool depthWriteEnable = true;
			bool blendEnable = false;
			VkBlendFactor srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
			VkBlendFactor dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
			VkBlendOp colorBlendOp = VK_BLEND_OP_ADD;
			VkBlendFactor srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			VkBlendFactor dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
			VkBlendOp alphaBlendOp = VK_BLEND_OP_ADD;
		};

		using PipelineMap = std::unordered_map<size_t, Renderer::Pipeline>;

		void CreatePipeline(const PipelineCreateInfo<PipelineKey>& createInfo, const VkRenderPass& renderPass, Renderer::Pipeline& pipeline, const char* name, const GraphicsPipelineState& state = {});

		// Initializes and updates descriptor sets for the native pipeline.
		void InitializeDescriptorsSets(SimpleTexture* pTexture);

		FrameBufferBase& GetFrameBuffer();

		void Render(const VkFramebuffer& framebuffer, const VkExtent2D& extent, Renderer::CommandBufferList& commandBufferList);

		void BindTexture(SimpleTexture* pTexture);
		void BindUntextured();
		void RenderMesh(SimpleMesh* pMesh, const uint32_t renderFlags);

		const VkSampler& GetSampler();
		const VkImageView& GetColorImageView();

		double GetRenderTime();
		double GetRenderWaitTime();
		double GetRenderThreadTime();

		void ResizeFrameBuffer(int width, int height);
		void ApplyPendingResizeIfNeeded();
		VkExtent2D GetFrameBufferSize();
		void Cleanup();
		const glm::mat4& GetInitialViewMatrix();
		const glm::mat4& GetInitialProjMatrix();

		// Actor preview: second-camera offscreen render.
		void SetupPreview(int width, int height);
		void SetPreviewCamera(const float* viewMatrix, const float* projMatrix);
		void ClearPreviewCamera();
		const VkSampler& GetPreviewSampler();
		const VkImageView& GetPreviewColorImageView();
	}
}
