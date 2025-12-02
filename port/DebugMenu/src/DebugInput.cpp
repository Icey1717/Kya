#include "DebugInput.h"
#include <imgui.h>
#include <cstdint>
#include "InputManager.h"
#include "input_functions.h"

void Debug::Input::ShowMenu(bool* bOpen)
{
	if (!ImGui::Begin("Gamepad Debug", bOpen))
	{
		ImGui::End();
		return;
	}

	// Check if we have a valid controller port
	if (edDevRoot.nbPorts == 0 || !edDevRoot.aPorts[0].pPadD)
	{
		ImGui::Text("No controller detected");
		ImGui::End();
		return;
	}

	EDDEV_PORT* pController = &edDevRoot.aPorts[0];
	Pad_d* pPadD = pController->pPadD;

	ImGui::Text("Gamepad Input States");
	ImGui::Separator();

	// Button states table
	if (ImGui::BeginTable("ButtonStates", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
	{
		ImGui::TableSetupColumn("Button");
		ImGui::TableSetupColumn("Pressed");
		ImGui::TableSetupColumn("Released");
		ImGui::TableSetupColumn("Analog");
		ImGui::TableHeadersRow();

		// Face buttons
		const struct { const char* name; uint32_t route; } buttons[] = {
			{ "Cross (X)", ROUTE_CROSS },
			{ "Circle (O)", ROUTE_CIRCLE },
			{ "Square", ROUTE_SQUARE },
			{ "Triangle", ROUTE_TRIANGLE },
			{ "L1", ROUTE_L1 },
			{ "R1", ROUTE_R1 },
			{ "L2", ROUTE_L2 },
			{ "R2", ROUTE_R2 },
			{ "Select", ROUTE_SELECT },
			{ "Start", ROUTE_START },
			{ "D-Pad Up", ROUTE_UP },
			{ "D-Pad Down", ROUTE_DOWN },
			{ "L-Analog Up", ROUTE_L_ANALOG_UP },
			{ "L-Analog Down", ROUTE_L_ANALOG_DOWN },
			{ "L-Analog Left", ROUTE_L_ANALOG_LEFT },
			{ "L-Analog Right", ROUTE_L_ANALOG_RIGHT },
		};

		for (const auto& button : buttons)
		{
			ImGui::TableNextRow();

			ImGui::TableSetColumnIndex(0);
			ImGui::Text("%s", button.name);

			ImGui::TableSetColumnIndex(1);
			bool pressed = pPadD[button.route].bPressed;
			ImGui::TextColored(pressed ? ImVec4(0.0f, 1.0f, 0.0f, 1.0f) : ImVec4(0.5f, 0.5f, 0.5f, 1.0f),
				pressed ? "YES" : "NO");

			ImGui::TableSetColumnIndex(2);
			bool released = pPadD[button.route].bReleased;
			ImGui::TextColored(released ? ImVec4(1.0f, 0.5f, 0.0f, 1.0f) : ImVec4(0.5f, 0.5f, 0.5f, 1.0f),
				released ? "YES" : "NO");

			ImGui::TableSetColumnIndex(3);
			float analog = pPadD[button.route].analogValue;
			ImGui::ProgressBar(analog, ImVec2(-1.0f, 0.0f));
			ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
			ImGui::Text("%.2f", analog);
		}

		ImGui::EndTable();
	}

	ImGui::Separator();

	// Analog stick visualization
	ImGui::Text("Left Analog Stick");
	ImGui::Spacing();

	float leftRight = pPadD[ROUTE_L_ANALOG_RIGHT].analogValue;
	float leftLeft = pPadD[ROUTE_L_ANALOG_LEFT].analogValue;
	float leftDown = pPadD[ROUTE_L_ANALOG_DOWN].analogValue;
	float leftUp = pPadD[ROUTE_L_ANALOG_UP].analogValue;

	float leftX = leftRight - leftLeft;
	float leftY = leftDown - leftUp;

	ImGui::Text("X-Axis: %.2f", leftX);
	ImGui::ProgressBar((leftX + 1.0f) * 0.5f, ImVec2(-1.0f, 0.0f));

	ImGui::Text("Y-Axis: %.2f", leftY);
	ImGui::ProgressBar((leftY + 1.0f) * 0.5f, ImVec2(-1.0f, 0.0f));

	ImGui::Spacing();
	ImGui::Text("Raw values:");
	ImGui::BulletText("Up: %.2f, Down: %.2f", leftUp, leftDown);
	ImGui::BulletText("Left: %.2f, Right: %.2f", leftLeft, leftRight);

	ImGui::End();
}