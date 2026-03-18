#pragma once

#include "Objects/Pipeline.h"
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

			void AddLine(const glm::vec3& start, const glm::vec3& end, const glm::vec4& color);
			void AddLine(float sx, float sy, float sz, float ex, float ey, float ez, float r, float g, float b, float a);

			// mat4x4 is in edF32MATRIX4 raw[] layout: rowX, rowY, rowZ, rowT (each 4 floats, local-to-world)
			void AddOBB(const float* mat4x4, float hw, float hh, float hd, float r, float g, float b, float a);
			// Type 3: cylinder with y ∈ [-0.5, 0.5] and xz radius = 1 in local space.
			void AddCylinder(const float* mat4x4, float r, float g, float b, float a);
			// Type 4: cone with apex at y=0.5, base at y=-0.5 with xz radius = 1 in local space.
			void AddCone(const float* mat4x4, float r, float g, float b, float a);
			// Type 1: plane+box: x,z ∈ [-0.5, 0.5], y ≥ 0 (open-top box). Drawn to y=1 with tick marks.
			void AddOpenTopBox(const float* mat4x4, float r, float g, float b, float a);
			// Type 0: half-space y ≥ 0. Drawn as a cross on the boundary plane and an arrow in +y.
			void AddHalfSpace(const float* mat4x4, float r, float g, float b, float a);
			void AddSphere(const glm::vec3& center, float radius, const glm::vec4& color);
			void AddSphere(float cx, float cy, float cz, float radius, float r, float g, float b, float a);
			// Line variant gated on the actor OBB toggle (not the collision ray toggle)
			uint32_t GetLineCount();

			void CreatePipeline(const VkRenderPass& renderPass, Renderer::Pipeline& pipeline, const char* name);
			void Record(const VkCommandBuffer& cmd, const Renderer::Pipeline& pipeline);

			// Dedicated end-of-frame debug pass that depth-tests against a saved copy of
			// the first render pass depth buffer, bypassing any subsequent depth clears.
			void SetupDedicatedPass(VkImageView colorImageView, int width, int height);
			void DestroyDedicatedPass();
			void SaveDepth(VkCommandBuffer cmd, VkImage srcDepthImage);
			void RecordDedicatedPass(VkCommandBuffer cmd);
		}
	}
}
