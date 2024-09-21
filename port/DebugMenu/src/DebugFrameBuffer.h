#pragma once

#include <imgui.h>

namespace Debug {
	namespace FrameBuffer {
		void ShowMenu(bool* bOpen);
		void ShowFramebuffers(bool* bOpen);
		void ShowNativeFrameBuffer(bool* bOpen);
		void ShowGame();

		ImVec2 GetGameWindowPosition();
		ImVec2 GetGameWindowSize();
	}
}