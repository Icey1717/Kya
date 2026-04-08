#include "DebugProjection.h"

#include "VulkanIncludes.h"

#include "DebugMenuLayout.h"
#include "Types.h"
#include "ed3D.h"
#include "MathOps.h"
#include "Native/NativeRenderer.h"

namespace Renderer
{
	namespace Native
	{
		extern glm::mat4 gInitialViewMatrix;
		extern glm::mat4 gInitialProjMatrix;
	}
}

namespace {
	bool WorldToFramebufferScreen(const edF32VECTOR4& worldPos, ImVec2& frameBufferPos)
	{
		const VkExtent2D frameBufferSize = Renderer::Native::GetFrameBufferSize();
		if (frameBufferSize.width == 0 || frameBufferSize.height == 0) {
			return false;
		}

		const glm::mat4 viewProjMatrix = Renderer::Native::gInitialProjMatrix * Renderer::Native::gInitialViewMatrix;

		// Step 1: Multiply the world position by the view-projection matrix to get clip space coordinates.
		glm::vec4 clipSpacePos = viewProjMatrix * glm::vec4(worldPos.x, worldPos.y, worldPos.z, 1.0f);

		// Because of reverse Z, we need to check if the point is behind the near clip plane.
		if (clipSpacePos.z > -g_DefaultNearClip_0044851c) {
			return false;
		}

		// Step 2: Perform perspective divide (clip space -> normalized device coordinates).
		if (clipSpacePos.w == 0.0f) {
			return false;
		}

		clipSpacePos.x /= clipSpacePos.w;
		clipSpacePos.y /= clipSpacePos.w;
		clipSpacePos.z /= clipSpacePos.w;

		clipSpacePos.y = -clipSpacePos.y; // Flip the y-axis.

		// Step 3: Map normalized device coordinates to native framebuffer pixels.
		const float frameBufferWidth = static_cast<float>(frameBufferSize.width);
		const float frameBufferHeight = static_cast<float>(frameBufferSize.height);
		frameBufferPos.x = ((clipSpacePos.x + 1.0f) * 0.5f) * frameBufferWidth;
		frameBufferPos.y = ((1.0f - clipSpacePos.y) * 0.5f) * frameBufferHeight;

		if (frameBufferPos.x < 0.0f || frameBufferPos.x > frameBufferWidth) {
			return false;
		}

		if (frameBufferPos.y < 0.0f || frameBufferPos.y > frameBufferHeight) {
			return false;
		}

		return true;
	}
}

bool Debug::Projection::WorldToScreen(const edF32VECTOR4& worldPos, ImVec2& screenPos)
{
	const ImVec2 gameViewportSize = Debug::GetGameViewportImageSize();
	if (gameViewportSize.x <= 0.0f || gameViewportSize.y <= 0.0f) {
		return false;
	}

	ImVec2 frameBufferPos;
	if (!WorldToFramebufferScreen(worldPos, frameBufferPos)) {
		return false;
	}

	const VkExtent2D frameBufferSize = Renderer::Native::GetFrameBufferSize();
	const float frameBufferWidth = static_cast<float>(frameBufferSize.width);
	const float frameBufferHeight = static_cast<float>(frameBufferSize.height);
	screenPos.x = (frameBufferPos.x / frameBufferWidth) * gameViewportSize.x;
	screenPos.y = (frameBufferPos.y / frameBufferHeight) * gameViewportSize.y;

	return true;
}

bool Debug::Projection::WorldToScreenAbsolute(const edF32VECTOR4& worldPos, ImVec2& screenPos)
{
	if (!WorldToScreen(worldPos, screenPos)) {
		return false;
	}

	const ImVec2 gameViewportPos = Debug::GetGameViewportImagePosition();
	screenPos.x += gameViewportPos.x;
	screenPos.y += gameViewportPos.y;
	return true;
}

