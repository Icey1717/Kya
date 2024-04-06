#pragma once
#include <stdint.h>

namespace Input {
	struct InputFunctions;
}

namespace DebugMenu {
	void BuildImguiCommands();
	double GetDeltaTime();
	void Init();

	Input::InputFunctions& GetInputFunctions();

	bool GetMousePressed(uint32_t routeId);
	bool GetMouseReleased(uint32_t routeId);
	float GetMouseAnalog(uint32_t routeId);

	bool GetKeyPressed(uint32_t routeId);
	bool GetKeyReleased(uint32_t routeId);
	float GetKeyAnalog(uint32_t routeId);
}