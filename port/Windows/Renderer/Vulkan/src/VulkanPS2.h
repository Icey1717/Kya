#pragma once

#include "VulkanIncludes.h"

struct HardwareState {
	int FBP;
	bool bActivePass;
	VkRect2D scissor;
};

HardwareState& GetHardwareState();

namespace PS2 {
	void Setup();
	void BeginFrame();
	void Cleanup();
}
