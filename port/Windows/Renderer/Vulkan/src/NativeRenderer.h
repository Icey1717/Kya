#pragma once

#include <cstdint>
#include <string>

#include "Pipeline.h"
#include "VulkanIncludes.h"

namespace Renderer
{
	struct SimpleTexture;
	struct SimpleMesh;

	namespace Native
	{
		struct PipelineKey {
			union {
				struct {
					uint32_t bWireframe : 1;
					uint32_t bGlsl : 1;
				} options;

				uint32_t key{};
			};
		};

		struct PipelineCreateInfo {
			std::string vertShaderFilename;
			std::string fragShaderFilename;

			PipelineKey key;
		};

		using PipelineMap = std::unordered_map<size_t, Renderer::Pipeline>;

		void CreateRenderPass(VkRenderPass& renderPass, const char* name);
		void CreatePipeline(const PipelineCreateInfo& createInfo, const VkRenderPass& renderPass, Renderer::Pipeline& pipeline, const char* name);

		void Setup();
		void Render(const VkFramebuffer& framebuffer, const VkExtent2D& extent);

		void BindTexture(SimpleTexture* pTexture);
		void AddMesh(SimpleMesh* pMesh);
		void UpdateMatrices(float* pWorld, float* pView, float* pProj, float* pScreen);

		const VkSampler& GetSampler();
		const VkImageView& GetColorImageView();
	}
}