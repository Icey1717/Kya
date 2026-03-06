#pragma once

#include "Pipeline.h"
#include "VulkanIncludes.h"
#include "glm/mat4x4.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"

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

			void AddLine(const glm::vec3& start, const glm::vec3& end, const glm::vec4& color);
			void AddLine(float sx, float sy, float sz, float ex, float ey, float ez, float r, float g, float b, float a);
			bool& GetCollisionLinesEnabled();

			// mat4x4 is in edF32MATRIX4 raw[] layout: rowX, rowY, rowZ, rowT (each 4 floats)
			void AddOBB(const float* mat4x4, float hw, float hh, float hd, float r, float g, float b, float a);
			// Line variant gated on the actor OBB toggle (not the collision ray toggle)
			void AddActorLine(float sx, float sy, float sz, float ex, float ey, float ez, float r, float g, float b, float a);
			bool& GetActorObbEnabled();
			uint32_t GetLineCount();

			void CreatePipeline(const VkRenderPass& renderPass, Renderer::Pipeline& pipeline, const char* name);
			void Record(const VkCommandBuffer& cmd, const Renderer::Pipeline& pipeline);
		}
	}
}
