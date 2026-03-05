#pragma once

#include "Pipeline.h"
#include "VulkanIncludes.h"
#include "glm/mat4x4.hpp"

namespace Renderer
{
	namespace Native
	{
		namespace DebugShapes
		{
			void Setup();
			void ResetFrame();

			void SetInitialViewProjection(const glm::mat4& viewMatrix, const glm::mat4& projMatrix);
			void SetLastModelMatrix(const glm::mat4& modelMatrix);

			void CreatePipeline(const VkRenderPass& renderPass, Renderer::Pipeline& pipeline, const char* name);
			void Record(const VkCommandBuffer& cmd, const Renderer::Pipeline& pipeline);
		}
	}
}
