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
				// 32 bit key
				struct {
					uint32_t bWireframe : 1;
					uint32_t bGlsl : 1;

					uint32_t alphaA : 2;
					uint32_t alphaB : 2;
					uint32_t alphaC : 2;
					uint32_t alphaD : 2;
					uint32_t alphaFix : 4;
				} options;

				uint32_t key{};
			};
		};

		static_assert(sizeof(uint32_t) == sizeof(PipelineKey), "PipelineKey size mismatch");

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
		void PushGlobalMatrices(float* pModel, float* pView, float* pProj);
		void PushLightData(float* pDirection, float* pColor, float* pAmbient);
		void PushModelMatrix(float* pModel);
		void PushAnimMatrix(float* pAnim);

		const VkSampler& GetSampler();
		const VkImageView& GetColorImageView();

		double GetRenderTime();
		double GetRenderWaitTime();
	}
}