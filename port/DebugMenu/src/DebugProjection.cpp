#include "DebugProjection.h"

#include "VulkanIncludes.h"

#include "DebugFrameBuffer.h"
#include "Types.h"
#include "ed3D.h"
#include "MathOps.h"

namespace Renderer
{
	namespace Native
	{
		extern glm::mat4 gInitialViewMatrix;
		extern glm::mat4 gInitialProjMatrix;
	}
}

bool Debug::Projection::WorldToScreen(const edF32VECTOR4& worldPos, ImVec2& screenPos)
{
	auto viewProjMatrix = Renderer::Native::gInitialProjMatrix * Renderer::Native::gInitialViewMatrix;

	// Step 1: Multiply the world position by the view-projection matrix to get clip space coordinates
	glm::vec4 clipSpacePos = viewProjMatrix * glm::vec4(worldPos.x, worldPos.y, worldPos.z, 1.0f);

	// Because of revers ze, we need to check if the actor is behind the near clip plane
	if (clipSpacePos.z > -g_DefaultNearClip_0044851c) {
		return false; // The actor is behind the camera, don't draw it
	}

	// Step 2: Perform perspective divide (clip space -> normalized device coordinates)
	if (clipSpacePos.w != 0.0f) {
		clipSpacePos.x /= clipSpacePos.w;
		clipSpacePos.y /= clipSpacePos.w;
		clipSpacePos.z /= clipSpacePos.w;
	}

	clipSpacePos.y = -clipSpacePos.y; // Flip the y-axis

	// Step 3: Map normalized device coordinates to screen space (from [-1,1] to [0, displaySize])
	screenPos.x = ((clipSpacePos.x + 1.0f) * 0.5f) * FrameBuffer::GetGameWindowSize().x;
	screenPos.y = ((1.0f - clipSpacePos.y) * 0.5f) * FrameBuffer::GetGameWindowSize().y;

	if (screenPos.x < 0.0f || screenPos.x > FrameBuffer::GetGameWindowSize().x) {
		return false;
	}

	if (screenPos.y < 0.0f || screenPos.y > FrameBuffer::GetGameWindowSize().y) {
		return false;
	}

	return true;
}

