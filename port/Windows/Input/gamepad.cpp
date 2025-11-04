#include "input_functions.h"
#include "gamepad.h"
#include <windows.h>
#include <winrt/Windows.Gaming.Input.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <algorithm>

using namespace winrt;
using namespace Windows::Gaming::Input;

namespace GamepadImpl
{
	namespace
	{
		// Helper to get the first connected gamepad
		Windows::Gaming::Input::Gamepad GetConnectedGamepad()
		{
			try
			{
				auto gamepads = Windows::Gaming::Input::Gamepad::Gamepads();
				if (gamepads.Size() > 0)
				{
					return gamepads.GetAt(0);
				}
			}
			catch (...)
			{
				// No gamepad available
			}
			return nullptr;
		}
	}

	bool GetGamepadPressed(uint32_t routeId)
	{
		auto gamepad = GetConnectedGamepad();
		if (!gamepad)
		{
			return false;
		}

		auto reading = gamepad.GetCurrentReading();

		switch (routeId)
		{
		// D-Pad
		case ROUTE_UP:
			return (reading.Buttons & GamepadButtons::DPadUp) != GamepadButtons::None;
		case ROUTE_DOWN:
			return (reading.Buttons & GamepadButtons::DPadDown) != GamepadButtons::None;

		// Shoulder buttons
		case ROUTE_L1:
			return (reading.Buttons & GamepadButtons::LeftShoulder) != GamepadButtons::None;
		case ROUTE_R1:
			return (reading.Buttons & GamepadButtons::RightShoulder) != GamepadButtons::None;

		// Face buttons
		case ROUTE_TRIANGLE:
			return (reading.Buttons & GamepadButtons::Y) != GamepadButtons::None;
		case ROUTE_CIRCLE:
			return (reading.Buttons & GamepadButtons::B) != GamepadButtons::None;
		case ROUTE_CROSS:
			return (reading.Buttons & GamepadButtons::A) != GamepadButtons::None;
		case ROUTE_SQUARE:
			return (reading.Buttons & GamepadButtons::X) != GamepadButtons::None;

		// Menu buttons
		case ROUTE_SELECT:
			return (reading.Buttons & GamepadButtons::View) != GamepadButtons::None;
		case ROUTE_START:
			return (reading.Buttons & GamepadButtons::Menu) != GamepadButtons::None;

		// Left analog stick - treat as directional
		case ROUTE_L_ANALOG_UP:
			return reading.LeftThumbstickY > 0.5f;
		case ROUTE_L_ANALOG_DOWN:
			return reading.LeftThumbstickY < -0.5f;
		case ROUTE_L_ANALOG_LEFT:
			return reading.LeftThumbstickX < -0.5f;
		case ROUTE_L_ANALOG_RIGHT:
			return reading.LeftThumbstickX > 0.5f;

		// Triggers - treat as pressed if > 0.5
		case ROUTE_L2:
			return reading.LeftTrigger > 0.5f;
		case ROUTE_R2:
			return reading.RightTrigger > 0.5f;

		default:
			return false;
		}
	}

	bool GetGamepadReleased(uint32_t routeId)
	{
		// For released state, simply return the inverse of pressed
		// For production use, maintain state between frames to detect transitions
		return !GetGamepadPressed(routeId);
	}

	float GetGamepadAnalog(uint32_t routeId)
	{
		auto gamepad = GetConnectedGamepad();
		if (!gamepad)
		{
			return 0.0f;
		}

		auto reading = gamepad.GetCurrentReading();

		switch (routeId)
		{
		// Triggers return full analog range [0.0, 1.0]
		case ROUTE_L2:
			return static_cast<float>(reading.LeftTrigger);
		case ROUTE_R2:
			return static_cast<float>(reading.RightTrigger);

		// Left analog stick return normalized values [-1.0, 1.0]
		case ROUTE_L_ANALOG_UP:
			return static_cast<float>(reading.LeftThumbstickY);
		case ROUTE_L_ANALOG_DOWN:
			// Remap to positive range
			return -static_cast<float>(reading.LeftThumbstickY);
		case ROUTE_L_ANALOG_LEFT:
			// Remap to positive range
			return -static_cast<float>(reading.LeftThumbstickX);
		case ROUTE_L_ANALOG_RIGHT:
			return static_cast<float>(reading.LeftThumbstickX);

		// Buttons return binary [0.0 or 1.0]
		case ROUTE_UP:
			return ((reading.Buttons & GamepadButtons::DPadUp) != GamepadButtons::None) ? 1.0f : 0.0f;
		case ROUTE_DOWN:
			return ((reading.Buttons & GamepadButtons::DPadDown) != GamepadButtons::None) ? 1.0f : 0.0f;

		case ROUTE_L1:
			return ((reading.Buttons & GamepadButtons::LeftShoulder) != GamepadButtons::None) ? 1.0f : 0.0f;
		case ROUTE_R1:
			return ((reading.Buttons & GamepadButtons::RightShoulder) != GamepadButtons::None) ? 1.0f : 0.0f;

		case ROUTE_TRIANGLE:
			return ((reading.Buttons & GamepadButtons::Y) != GamepadButtons::None) ? 1.0f : 0.0f;
		case ROUTE_CIRCLE:
			return ((reading.Buttons & GamepadButtons::B) != GamepadButtons::None) ? 1.0f : 0.0f;
		case ROUTE_CROSS:
			return ((reading.Buttons & GamepadButtons::A) != GamepadButtons::None) ? 1.0f : 0.0f;
		case ROUTE_SQUARE:
			return ((reading.Buttons & GamepadButtons::X) != GamepadButtons::None) ? 1.0f : 0.0f;

		case ROUTE_SELECT:
			return ((reading.Buttons & GamepadButtons::View) != GamepadButtons::None) ? 1.0f : 0.0f;
		case ROUTE_START:
			return ((reading.Buttons & GamepadButtons::Menu) != GamepadButtons::None) ? 1.0f : 0.0f;

		default:
			return 0.0f;
		}
	}

	bool IsAnyGamepadButtonPressed()
	{
		auto gamepad = GetConnectedGamepad();
		if (!gamepad)
		{
			return false;
		}
		auto reading = gamepad.GetCurrentReading();
		return reading.Buttons != GamepadButtons::None ||
			reading.LeftTrigger > 0.0f ||
			reading.RightTrigger > 0.0f ||
			reading.LeftThumbstickX != 0.0f ||
			reading.LeftThumbstickY != 0.0f ||
			reading.RightThumbstickX != 0.0f ||
			reading.RightThumbstickY != 0.0f;
	}
}

void KyaGamepad::AddGamepadSupport()
{
	Input::gInputFunctions.controllerPressed = GamepadImpl::GetGamepadPressed;
	Input::gInputFunctions.controllerReleased = GamepadImpl::GetGamepadReleased;
	Input::gInputFunctions.controllerAnalog = GamepadImpl::GetGamepadAnalog;
	Input::gInputFunctions.controllerAnyPressed = GamepadImpl::IsAnyGamepadButtonPressed;
}