// Implementations of the keyboard/mouse input functions declared in DebugMenu.h.
// Translates PS2 route IDs into ImGui key/mouse queries.
#include "DebugMenu.h"
#include "DebugCamera.h"
#include "InputManager.h"

#include <imgui.h>
#include <unordered_map>
#include <cassert>

#include "input_functions.h"

static std::unordered_map<uint32_t, ImGuiKey> gKeyMap = {
	{ ROUTE_START,			ImGuiKey_Enter },
	{ ROUTE_L2,				ImGuiKey_Q },		// 0x0
	{ ROUTE_R2,				ImGuiKey_W },		// 0x1
	{ ROUTE_L1,				ImGuiKey_C },		// 0x2
	{ ROUTE_R1,				ImGuiKey_R },		// 0x3
	{ ROUTE_TRIANGLE,		ImGuiKey_Comma },	// 0x4
	{ ROUTE_CIRCLE,			ImGuiKey_E },		// 0x5
	{ ROUTE_CROSS,			ImGuiKey_A },		// 0x6
	{ ROUTE_SQUARE,			ImGuiKey_O },		// 0x7
	{ ROUTE_SELECT,			ImGuiKey_Backspace },
	{ ROUTE_UP,				ImGuiKey_UpArrow },
	{ ROUTE_LEFT,			ImGuiKey_LeftArrow },
	{ ROUTE_DOWN,			ImGuiKey_DownArrow },
	{ ROUTE_RIGHT,			ImGuiKey_RightArrow },
	{ ROUTE_L_ANALOG_UP,	ImGuiKey_G },
	{ ROUTE_L_ANALOG_DOWN,	ImGuiKey_H },
	{ ROUTE_L_ANALOG_LEFT,	ImGuiKey_D },
	{ ROUTE_L_ANALOG_RIGHT,	ImGuiKey_T },
	{ ROUTE_R_ANALOG_DOWN,	ImGuiKey_K },
	{ ROUTE_R_ANALOG_UP,	ImGuiKey_I },
	{ ROUTE_R_ANALOG_LEFT,	ImGuiKey_J },
	{ ROUTE_R_ANALOG_RIGHT,	ImGuiKey_L },
};

void DebugMenu::AddKeyboardMouseSupport()
{
	Input::gInputFunctions.keyPressed  = GetKeyPressed;
	Input::gInputFunctions.keyReleased = GetKeyReleased;
	Input::gInputFunctions.keyAnalog   = GetKeyAnalog;

	Input::gInputFunctions.mousePressed  = GetMousePressed;
	Input::gInputFunctions.mouseReleased = GetMouseReleased;
	Input::gInputFunctions.mouseAnalog   = GetMouseAnalog;
}

bool DebugMenu::GetKeyPressed(uint32_t routeId)
{
	if (ImGui::GetIO().WantCaptureKeyboard) {
		return false;
	}

	assert(gKeyMap.find(routeId) != gKeyMap.end());
	return ImGui::IsKeyPressed(gKeyMap[routeId]);
}

bool DebugMenu::GetKeyReleased(uint32_t routeId)
{
	if (ImGui::GetIO().WantCaptureKeyboard) {
		return false;
	}

	assert(gKeyMap.find(routeId) != gKeyMap.end());
	return ImGui::IsKeyReleased(gKeyMap[routeId]);
}

float DebugMenu::GetKeyAnalog(uint32_t routeId)
{
	if (ImGui::GetIO().WantCaptureKeyboard) {
		return 0.0f;
	}

	assert(gKeyMap.find(routeId) != gKeyMap.end());
	return ImGui::IsKeyDown(gKeyMap[routeId]) ? 1.0f : 0.0f;
}

bool DebugMenu::GetMousePressed(uint32_t routeId)
{
	if (routeId == MOUSE_INPUT_BUTTON1) {
		return ImGui::IsMouseDown(ImGuiMouseButton_Left);
	}
	else if (routeId == MOUSE_INPUT_BUTTON2) {
		return ImGui::IsMouseDown(ImGuiMouseButton_Middle);
	}
	else if (routeId == MOUSE_INPUT_BUTTON3) {
		return ImGui::IsMouseDown(ImGuiMouseButton_Right);
	}

	return false;
}

bool DebugMenu::GetMouseReleased(uint32_t routeId)
{
	return false;
}

namespace Debug::Camera {
	extern float gMouseDeltaX;
	extern float gMouseDeltaY;
}

float DebugMenu::GetMouseAnalog(uint32_t routeId)
{
	float delta = 0.0f;

	if (routeId == MOUSE_INPUT_DX) {
		delta = Debug::Camera::gMouseDeltaX;
	}
	else if (routeId == MOUSE_INPUT_DY) {
		delta = Debug::Camera::gMouseDeltaY;
	}
	else if (routeId == MOUSE_INPUT_WHEEL) {
		delta = ImGui::GetIO().MouseWheel;
	}

	return delta;
}
