#pragma once
#include <stdint.h>

namespace DebugMenu {
	void Setup();
	void Render(const struct VkFramebuffer& framebuffer, const struct VkExtent2D& extent);
}