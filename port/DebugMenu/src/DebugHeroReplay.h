#pragma once

namespace Debug {
	namespace HeroReplay {
		// Called once per frame from DebugMenu::BuildImguiCommands.
		// Records the current pPadD state when recording, advances the replay
		// frame counter when replaying.
		void Update();

		void DrawContents();
		void ShowMenu(bool* bOpen);
	}
}
