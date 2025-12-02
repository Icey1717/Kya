#include "input.h"

namespace Input {
	namespace KeyboardController {
		constexpr uint numButtons = 0x18;
	}

	namespace Mouse {
		constexpr uint numButtons = 0x6;
	}

	static void PollKeyboard(EDDEV_PORT* pController)
	{
		pController->pPadD[ROUTE_START].bPressed = gInputFunctions.keyPressed(ROUTE_START);
		pController->pPadD[ROUTE_START].bReleased = gInputFunctions.keyReleased(ROUTE_START);

		pController->pPadD[ROUTE_SELECT].bPressed = gInputFunctions.keyPressed(ROUTE_SELECT);
		pController->pPadD[ROUTE_SELECT].bReleased = gInputFunctions.keyReleased(ROUTE_SELECT);

		pController->pPadD[ROUTE_L1].bPressed = gInputFunctions.keyPressed(ROUTE_L1);
		pController->pPadD[ROUTE_L1].clickValue = gInputFunctions.keyAnalog(ROUTE_L1);
		pController->pPadD[ROUTE_L1].bReleased = gInputFunctions.keyReleased(ROUTE_L1);

		pController->pPadD[ROUTE_L2].bPressed = gInputFunctions.keyPressed(ROUTE_L2);
		pController->pPadD[ROUTE_L2].clickValue = gInputFunctions.keyAnalog(ROUTE_L2);
		pController->pPadD[ROUTE_L2].bReleased = gInputFunctions.keyReleased(ROUTE_L2);

		pController->pPadD[ROUTE_R2].bPressed = gInputFunctions.keyPressed(ROUTE_R2);
		pController->pPadD[ROUTE_R2].clickValue = gInputFunctions.keyAnalog(ROUTE_R2);
		pController->pPadD[ROUTE_R2].bReleased = gInputFunctions.keyReleased(ROUTE_R2);

		pController->pPadD[ROUTE_R1].bPressed = gInputFunctions.keyPressed(ROUTE_R1);
		pController->pPadD[ROUTE_R1].clickValue = gInputFunctions.keyAnalog(ROUTE_R1);
		pController->pPadD[ROUTE_R1].bReleased = gInputFunctions.keyReleased(ROUTE_R1);

		pController->pPadD[ROUTE_CROSS].bPressed = gInputFunctions.keyPressed(ROUTE_CROSS);
		pController->pPadD[ROUTE_SQUARE].bPressed = gInputFunctions.keyPressed(ROUTE_SQUARE);
		pController->pPadD[ROUTE_CIRCLE].bPressed = gInputFunctions.keyPressed(ROUTE_CIRCLE);
		pController->pPadD[ROUTE_TRIANGLE].bPressed = gInputFunctions.keyPressed(ROUTE_TRIANGLE);

		pController->pPadD[ROUTE_CROSS].clickValue = gInputFunctions.keyAnalog(ROUTE_CROSS);
		pController->pPadD[ROUTE_SQUARE].clickValue = gInputFunctions.keyAnalog(ROUTE_SQUARE);
		pController->pPadD[ROUTE_CIRCLE].clickValue = gInputFunctions.keyAnalog(ROUTE_CIRCLE);
		pController->pPadD[ROUTE_TRIANGLE].clickValue = gInputFunctions.keyAnalog(ROUTE_TRIANGLE);

		pController->pPadD[ROUTE_CROSS].bReleased = gInputFunctions.keyReleased(ROUTE_CROSS);
		pController->pPadD[ROUTE_SQUARE].bReleased = gInputFunctions.keyReleased(ROUTE_SQUARE);
		pController->pPadD[ROUTE_CIRCLE].bReleased = gInputFunctions.keyReleased(ROUTE_CIRCLE);
		pController->pPadD[ROUTE_TRIANGLE].bReleased = gInputFunctions.keyReleased(ROUTE_TRIANGLE);

		pController->pPadD[ROUTE_DOWN].bPressed = gInputFunctions.keyPressed(ROUTE_DOWN);
		pController->pPadD[ROUTE_UP].bPressed = gInputFunctions.keyPressed(ROUTE_UP);

		pController->pPadD[ROUTE_L_ANALOG_UP].bPressed = gInputFunctions.keyPressed(ROUTE_L_ANALOG_UP);
		pController->pPadD[ROUTE_L_ANALOG_DOWN].bPressed = gInputFunctions.keyPressed(ROUTE_L_ANALOG_DOWN);

		pController->pPadD[ROUTE_L_ANALOG_LEFT].bPressed = gInputFunctions.keyPressed(ROUTE_L_ANALOG_LEFT);
		pController->pPadD[ROUTE_L_ANALOG_RIGHT].bPressed = gInputFunctions.keyPressed(ROUTE_L_ANALOG_RIGHT);

		pController->pPadD[ROUTE_UP].analogValue = gInputFunctions.keyAnalog(ROUTE_UP);
		pController->pPadD[ROUTE_DOWN].analogValue = gInputFunctions.keyAnalog(ROUTE_DOWN);

		pController->pPadD[ROUTE_L_ANALOG_UP].analogValue = gInputFunctions.keyAnalog(ROUTE_L_ANALOG_UP);
		pController->pPadD[ROUTE_L_ANALOG_DOWN].analogValue = gInputFunctions.keyAnalog(ROUTE_L_ANALOG_DOWN);

		pController->pPadD[ROUTE_L_ANALOG_LEFT].analogValue = gInputFunctions.keyAnalog(ROUTE_L_ANALOG_LEFT);
		pController->pPadD[ROUTE_L_ANALOG_RIGHT].analogValue = gInputFunctions.keyAnalog(ROUTE_L_ANALOG_RIGHT);
	}

	static void PollGamepad(EDDEV_PORT* pController)
	{
		if (gInputFunctions.controllerAnyPressed()) {
			pController->pPadD[ROUTE_START].bPressed = gInputFunctions.controllerPressed(ROUTE_START);
			pController->pPadD[ROUTE_START].bReleased = gInputFunctions.controllerReleased(ROUTE_START);

			pController->pPadD[ROUTE_SELECT].bPressed = gInputFunctions.controllerPressed(ROUTE_SELECT);
			pController->pPadD[ROUTE_SELECT].bReleased = gInputFunctions.controllerReleased(ROUTE_SELECT);

			pController->pPadD[ROUTE_L1].bPressed = gInputFunctions.controllerPressed(ROUTE_L1);
			pController->pPadD[ROUTE_L1].clickValue = gInputFunctions.controllerAnalog(ROUTE_L1);
			pController->pPadD[ROUTE_L1].bReleased = gInputFunctions.controllerReleased(ROUTE_L1);

			pController->pPadD[ROUTE_L2].bPressed = gInputFunctions.controllerPressed(ROUTE_L2);
			pController->pPadD[ROUTE_L2].clickValue = gInputFunctions.controllerAnalog(ROUTE_L2);
			pController->pPadD[ROUTE_L2].bReleased = gInputFunctions.controllerReleased(ROUTE_L2);

			pController->pPadD[ROUTE_R2].bPressed = gInputFunctions.controllerPressed(ROUTE_R2);
			pController->pPadD[ROUTE_R2].clickValue = gInputFunctions.controllerAnalog(ROUTE_R2);
			pController->pPadD[ROUTE_R2].bReleased = gInputFunctions.controllerReleased(ROUTE_R2);

			pController->pPadD[ROUTE_R1].bPressed = gInputFunctions.controllerPressed(ROUTE_R1);
			pController->pPadD[ROUTE_R1].clickValue = gInputFunctions.controllerAnalog(ROUTE_R1);
			pController->pPadD[ROUTE_R1].bReleased = gInputFunctions.controllerReleased(ROUTE_R1);

			pController->pPadD[ROUTE_CROSS].bPressed = gInputFunctions.controllerPressed(ROUTE_CROSS);
			pController->pPadD[ROUTE_SQUARE].bPressed = gInputFunctions.controllerPressed(ROUTE_SQUARE);
			pController->pPadD[ROUTE_CIRCLE].bPressed = gInputFunctions.controllerPressed(ROUTE_CIRCLE);
			pController->pPadD[ROUTE_TRIANGLE].bPressed = gInputFunctions.controllerPressed(ROUTE_TRIANGLE);

			pController->pPadD[ROUTE_CROSS].clickValue = gInputFunctions.controllerAnalog(ROUTE_CROSS);
			pController->pPadD[ROUTE_SQUARE].clickValue = gInputFunctions.controllerAnalog(ROUTE_SQUARE);
			pController->pPadD[ROUTE_CIRCLE].clickValue = gInputFunctions.controllerAnalog(ROUTE_CIRCLE);
			pController->pPadD[ROUTE_TRIANGLE].clickValue = gInputFunctions.controllerAnalog(ROUTE_TRIANGLE);

			pController->pPadD[ROUTE_CROSS].bReleased = gInputFunctions.controllerReleased(ROUTE_CROSS);
			pController->pPadD[ROUTE_SQUARE].bReleased = gInputFunctions.controllerReleased(ROUTE_SQUARE);
			pController->pPadD[ROUTE_CIRCLE].bReleased = gInputFunctions.controllerReleased(ROUTE_CIRCLE);
			pController->pPadD[ROUTE_TRIANGLE].bReleased = gInputFunctions.controllerReleased(ROUTE_TRIANGLE);

			pController->pPadD[ROUTE_DOWN].bPressed = gInputFunctions.controllerPressed(ROUTE_DOWN);
			pController->pPadD[ROUTE_UP].bPressed = gInputFunctions.controllerPressed(ROUTE_UP);
			pController->pPadD[ROUTE_DOWN].analogValue = gInputFunctions.controllerAnalog(ROUTE_DOWN);
			pController->pPadD[ROUTE_UP].analogValue = gInputFunctions.controllerAnalog(ROUTE_UP);
			pController->pPadD[ROUTE_DOWN].bReleased = gInputFunctions.controllerReleased(ROUTE_DOWN);
			pController->pPadD[ROUTE_UP].bReleased = gInputFunctions.controllerReleased(ROUTE_UP);


			pController->pPadD[ROUTE_L_ANALOG_UP].bPressed = gInputFunctions.controllerPressed(ROUTE_L_ANALOG_UP);
			pController->pPadD[ROUTE_L_ANALOG_DOWN].bPressed = gInputFunctions.controllerPressed(ROUTE_L_ANALOG_DOWN);
			pController->pPadD[ROUTE_L_ANALOG_LEFT].bPressed = gInputFunctions.controllerPressed(ROUTE_L_ANALOG_LEFT);
			pController->pPadD[ROUTE_L_ANALOG_RIGHT].bPressed = gInputFunctions.controllerPressed(ROUTE_L_ANALOG_RIGHT);

			pController->pPadD[ROUTE_L_ANALOG_UP].analogValue = gInputFunctions.controllerAnalog(ROUTE_L_ANALOG_UP);
			pController->pPadD[ROUTE_L_ANALOG_DOWN].analogValue = gInputFunctions.controllerAnalog(ROUTE_L_ANALOG_DOWN);
			pController->pPadD[ROUTE_L_ANALOG_LEFT].analogValue = gInputFunctions.controllerAnalog(ROUTE_L_ANALOG_LEFT);
			pController->pPadD[ROUTE_L_ANALOG_RIGHT].analogValue = gInputFunctions.controllerAnalog(ROUTE_L_ANALOG_RIGHT);

			pController->pPadD[ROUTE_L_ANALOG_UP].bReleased = gInputFunctions.controllerReleased(ROUTE_L_ANALOG_UP);
			pController->pPadD[ROUTE_L_ANALOG_DOWN].bReleased = gInputFunctions.controllerReleased(ROUTE_L_ANALOG_DOWN);
			pController->pPadD[ROUTE_L_ANALOG_LEFT].bReleased = gInputFunctions.controllerReleased(ROUTE_L_ANALOG_LEFT);
			pController->pPadD[ROUTE_L_ANALOG_RIGHT].bReleased = gInputFunctions.controllerReleased(ROUTE_L_ANALOG_RIGHT);
		}
	}
}

uint Input::_edDevKeyboard(uint eventId, EDDEV_PORT* pController, void* param_3)
{
	Pad_180* pPad180;
	int button;
	bool bVar2;
	int iVar3;
	uint uVar4;
	Pad_d* pPVar5;
	uint status;
	uint uVar7;
	uint uVar8;
	int* piVar9;
	float fVar10;
	byte local_4;
	undefined local_3;
	undefined local_1;

	// hacky index
	static int pressIndex = 0;

	pPad180 = pController->pPad180;
	status = 0;
	switch (eventId) {
	case EVENT_GET_NUM_BUTTONS:
		status = KeyboardController::numButtons;
	break;
	case EVENT_INIT_CONTROLLER:
		status = 0;
		pController->flags = 0x108;
		pController->pDisconnectedHandler = (ControllerConnectedFuncPtr)0x0;
		pController->pConnectedHandler = (ControllerConnectedFuncPtr)0x0;
		pController->field_0x2c = 0;

		pPVar5 = pController->pPadD;
		do {
			if ((status < 8) || (0xb < status)) {
				pPVar5->type = 3;
			}
			else {
				pPVar5->type = 1;
			}
			pPVar5->field_0x4 = 0;
			status = status + 1;
			pPVar5->analogValue = 0.0f;
			pPVar5->field_0xc = 0;
			pPVar5->clickValue = 0.0f;
			pPVar5->field_0x14 = 0;
			pPVar5->bPressed = false;
			pPVar5->bReleased = false;
			pPVar5->field_0x18 = 0.6666667f;
			pPVar5->field_0x1c = 0.3333333f;
			pPVar5->rateA = 0.01f;
			pPVar5->rateB = 0.01f;
			pPVar5->field_0x28 = 0.0f;
			pPVar5->field_0x2c = 0.0f;
			pPVar5->field_0x30 = 1.0f;
			pPVar5 = pPVar5 + 1;
		} while (status < KeyboardController::numButtons);

		status = 0;
	break;
	case EVENT_READY_TO_CONNECT:
		if (edDevRoot.field_0x3c != 0) {
			pPad180->field_0x8 = 0;
		}

		// Ready to connect state?
		pPad180->state = 1;
	break;
	case EVENT_UPDATE_CONNECTION_STATE:
	{
		const int pad180State = pPad180->state;
		if ((pad180State == 0) && (pPad180->prevState != 0)) {
			pController->flags = pController->flags | 0x40000000;
			if (pController->pDisconnectedHandler != (ControllerConnectedFuncPtr)0x0) {
				pController->pDisconnectedHandler(pController->controllerId);
			}
			pPad180->prevState = 0;
		}
		else {
			if ((pPad180->prevState == 0) && (pad180State != 0)) {
				if (pad180State == 1) {
					pController->flags = pController->flags & 0xbfffffff;
					if (pController->pConnectedHandler != (ControllerConnectedFuncPtr)0x0) {
						pController->pConnectedHandler(pController->controllerId);
					}
					pPad180->prevState = pPad180->state;
				}
			}
			else {
				if ((pad180State == 1) && (0x61 < (uint)pPad180->field_0x8)) {
					// Check state?
				}
			}
		}
	}
	break;
	case EVENT_SET_DATA:
		IMPLEMENTATION_GUARD(
			*(undefined*)&pPad180->field_0x24 = *(undefined*)param_3;
		*(undefined*)&pPad180->field_0x25 = *(undefined*)(param_3 + 1);
		)
			pController->field_0x2c = 1;
	break;
	case EVENT_SEND_DATA:
		IMPLEMENTATION_GUARD(
			local_1 = 3;
		local_4 = *(char*)&pPad180->field_0x24 != '\0' | *(char*)&pPad180->field_0x25 << 1;
		local_3 = (undefined)((int)(uint) * (byte*)&pPad180->field_0x25 >> 7);
		FUN_0035af78(pPad180->socketNumber, 1, (undefined8*)&local_1, 2, (undefined8*)&local_4);
		)
			pController->field_0x2c = 0;
	break;
	case EVENT_POLL_INPUT:
	{
		PollKeyboard(pController);
		PollGamepad(pController);
	}
	break;
	case EVENT_GET_STATUS:
		status = 0x10ff;
		if (((pController->flags & 0x40000000) == 0) && (status = 2, pPad180->port == 0)) {
			status = 1;
		}
	}

	return status;
}

#pragma clang optimize off
uint Input::_edDevMouse(uint eventID, EDDEV_PORT* pController, void* param_3)
{
	uint uVar6;
	Pad_d* pPVar5;

	uVar6 = 0;

	switch (eventID) {
	case EVENT_GET_NUM_BUTTONS:
		uVar6 = Mouse::numButtons;
		break;
	case EVENT_INIT_CONTROLLER:
	{
		uVar6 = 0;
		pController->flags = 0x210;
		pController->pDisconnectedHandler = (ControllerConnectedFuncPtr)0x0;
		pController->pConnectedHandler = (ControllerConnectedFuncPtr)0x0;
		pController->field_0x2c = 0;

		pPVar5 = pController->pPadD;
		do {
			if ((uVar6 < 8) || (0xb < uVar6)) {
				pPVar5->type = 3;
			}
			else {
				pPVar5->type = 1;
			}
			pPVar5->field_0x4 = 0;
			uVar6 = uVar6 + 1;
			pPVar5->analogValue = 0.0f;
			pPVar5->field_0xc = 0;
			pPVar5->clickValue = 0.0f;
			pPVar5->field_0x14 = 0;
			pPVar5->bPressed = false;
			pPVar5->bReleased = false;
			pPVar5->field_0x18 = 0.6666667f;
			pPVar5->field_0x1c = 0.3333333f;
			pPVar5->rateA = 0.01f;
			pPVar5->rateB = 0.01f;
			pPVar5->field_0x28 = 0.0f;
			pPVar5->field_0x2c = 0.0f;
			pPVar5->field_0x30 = 1.0f;
			pPVar5 = pPVar5 + 1;
		} while (uVar6 < Mouse::numButtons);

		uVar6 = 0;
		}
		break;
	case EVENT_READY_TO_CONNECT:
	case EVENT_UPDATE_CONNECTION_STATE:
		// Do nothing for now.
		break;
	case EVENT_POLL_INPUT:
	{
		pController->pPadD[MOUSE_INPUT_BUTTON1].clickValue = gInputFunctions.mousePressed(MOUSE_INPUT_BUTTON1);
		pController->pPadD[MOUSE_INPUT_BUTTON2].clickValue = gInputFunctions.mousePressed(MOUSE_INPUT_BUTTON2);
		pController->pPadD[MOUSE_INPUT_BUTTON3].clickValue = gInputFunctions.mousePressed(MOUSE_INPUT_BUTTON3);

		pController->pPadD[MOUSE_INPUT_DX].analogValue = gInputFunctions.mouseAnalog(MOUSE_INPUT_DX);
		pController->pPadD[MOUSE_INPUT_DY].analogValue = gInputFunctions.mouseAnalog(MOUSE_INPUT_DY);
		pController->pPadD[MOUSE_INPUT_WHEEL].analogValue = gInputFunctions.mouseAnalog(MOUSE_INPUT_WHEEL);
	}
	break;
	default:
		IMPLEMENTATION_GUARD();
		break;
	}

	return uVar6;
}
#pragma clang optimize on
