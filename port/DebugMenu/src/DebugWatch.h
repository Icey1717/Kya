#pragma once

namespace Debug::Watch {
	// IDs identify live sources, never addresses inside game objects.
	void DrawReadout(const char* id);
	bool DrawFloatEditor(const char* id, const char* label, float* value);
	void PinButton(const char* id);
	void Update();
	void ShowMenu(bool* pOpen);
}
