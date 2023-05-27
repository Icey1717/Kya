#pragma once

#include "VulkanIncludes.h"

struct HardwareState {
	int FBP;
	bool bActivePass;
};

HardwareState& GetHardwareState();

namespace PS2 {
	void Setup();
	void BeginFrame();
	void Cleanup();
}
