#pragma once

namespace Debug
{
	namespace Cinematic
	{
		void ShowMenu(bool* bOpen);

		// Jumps every playing cinematic to the end of its timeline, matching the
		// "Skip Cutscenes" debug setting. Returns the number skipped.
		int SkipActiveCutscenes();
	}
}