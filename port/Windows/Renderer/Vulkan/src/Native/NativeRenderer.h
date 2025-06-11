#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

#include "Pipeline.h"
#include "FrameBuffer.h"
#include "VulkanIncludes.h"

namespace Renderer
{
	struct SimpleTexture;
	struct SimpleMesh;

	namespace Native
	{
		constexpr int gWidth = 0x200;
		constexpr int gHeight = 0x200;

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

		using PipelineMap = std::unordered_map<size_t, Renderer::Pipeline>;

		void CreatePipeline(const PipelineCreateInfo<PipelineKey>& createInfo, const VkRenderPass& renderPass, Renderer::Pipeline& pipeline, const char* name);

		// Initializes and updates descriptor sets for the native pipeline.
		void InitializeDescriptorsSets(SimpleTexture* pTexture);

		FrameBufferBase& GetFrameBuffer();

		void Setup();
		void Render(const VkFramebuffer& framebuffer, const VkExtent2D& extent);

		void BindTexture(SimpleTexture* pTexture);
		void RenderMesh(SimpleMesh* pMesh, const uint32_t renderFlags);

		const VkSampler& GetSampler();
		const VkImageView& GetColorImageView();

		double GetRenderTime();
		double GetRenderWaitTime();
		double GetRenderThreadTime();
	}
}