#pragma once

#include "imgui.h"
#include <filesystem>
#include <vector>

namespace Debug::SaveLoad
{
	// BMP bytes from the most recently submitted game framebuffer (no debug UI).
	std::vector<char> CaptureScreenshot();
	void DrawScreenshot(const std::filesystem::path& path);
	void ClearScreenshots();
}
