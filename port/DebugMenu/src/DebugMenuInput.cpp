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
	{ ROUTE_L2,				ImGuiKey_Q },			// 0x00
	{ ROUTE_R2,				ImGuiKey_W },			// 0x01
	{ ROUTE_L1,				ImGuiKey_C },			// 0x02
	{ ROUTE_R1,				ImGuiKey_R },			// 0x03
	{ ROUTE_TRIANGLE,		ImGuiKey_Comma },		// 0x04
	{ ROUTE_CIRCLE,			ImGuiKey_E },			// 0x05
	{ ROUTE_CROSS,			ImGuiKey_A },			// 0x06
	{ ROUTE_SQUARE,			ImGuiKey_O },			// 0x07
	{ ROUTE_SELECT,			ImGuiKey_Backspace },	// 0x08
	{ ROUTE_UP,				ImGuiKey_UpArrow },		// 0x09
	{ ROUTE_LEFT,			ImGuiKey_LeftArrow },	// 0x0a
	{ ROUTE_DOWN,			ImGuiKey_DownArrow },	// 0x0b
	{ ROUTE_RIGHT,			ImGuiKey_RightArrow },	// 0x0c
	{ ROUTE_L_ANALOG_UP,	ImGuiKey_G },			// 0x0d
	{ ROUTE_L_ANALOG_DOWN,	ImGuiKey_H },			// 0x0e
	{ ROUTE_L_ANALOG_LEFT,	ImGuiKey_D },			// 0x0f
	{ ROUTE_L_ANALOG_RIGHT,	ImGuiKey_T },			// 0x10
	{ ROUTE_R_ANALOG_DOWN,	ImGuiKey_K },			// 0x11
	{ ROUTE_R_ANALOG_UP,	ImGuiKey_I },			// 0x12
	{ ROUTE_R_ANALOG_LEFT,	ImGuiKey_J },			// 0x13
	{ ROUTE_R_ANALOG_RIGHT,	ImGuiKey_L },			// 0x14
	{ ROUTE_L3,				ImGuiKey_F },			// 0x15
	{ ROUTE_R3,				ImGuiKey_S },			// 0x16
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
		Debug::Camera::gMouseDeltaX = 0.0f;
	}
	else if (routeId == MOUSE_INPUT_DY) {
		delta = Debug::Camera::gMouseDeltaY;
		Debug::Camera::gMouseDeltaY = 0.0f;
	}
	else if (routeId == MOUSE_INPUT_WHEEL) {
		delta = ImGui::GetIO().MouseWheel;
	}

	return delta;
}
