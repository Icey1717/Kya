#include "input.h"

namespace Input {
	static InputFunctions gInputFunctions;

	namespace KeyboardController {
		constexpr uint numButtons = 0x18;
	}

	namespace Mouse {
		constexpr uint numButtons = 0x6;
	}
}

void Input::SetInputFunctions(InputFunctions inputFunctions)
{
	gInputFunctions = inputFunctions;
}

uint Input::_edDevKeyboard(uint eventID, EDDEV_PORT* pController, void* param_3)
{
	Pad_180* pPVar1;
	int button;
	bool bVar2;
	int iVar3;
	uint uVar4;
	Pad_d* pPVar5;
	uint uVar6;
	uint uVar7;
	uint uVar8;
	int* piVar9;
	float fVar10;
	byte local_4;
	undefined local_3;
	undefined local_1;

	// hacky index
	static int pressIndex = 0;

	pPVar1 = pController->pPad180;
	uVar6 = 0;
	switch (eventID) {
	case 0x90000000:
		uVar6 = KeyboardController::numButtons;
	break;
	case 0x90000001:
		uVar6 = 0;
		pController->flags = 0x108;
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
		} while (uVar6 < KeyboardController::numButtons);
		uVar6 = 0;
	break;
	case 0x90000002:
		if (edDevRoot.field_0x3c != 0) {
			pPVar1->field_0x8 = 0;
		}

		// Ready to connect state?
		pPVar1->state = 1;
	break;
	case 0x90000003:
		iVar3 = pPVar1->state;
		if ((iVar3 == 0) && (pPVar1->prevState != 0)) {
			pController->flags = pController->flags | 0x40000000;
			if (pController->pDisconnectedHandler != (ControllerConnectedFuncPtr)0x0) {
				pController->pDisconnectedHandler(pController->controllerId);
			}
			pPVar1->prevState = 0;
		}
		else {
			if ((pPVar1->prevState == 0) && (iVar3 != 0)) {
				if (iVar3 == 1) {
					pController->flags = pController->flags & 0xbfffffff;
					if (pController->pConnectedHandler != (ControllerConnectedFuncPtr)0x0) {
						pController->pConnectedHandler(pController->controllerId);
					}
					pPVar1->prevState = pPVar1->state;
				}
			}
			else {
				if ((iVar3 == 1) && (0x61 < (uint)pPVar1->field_0x8)) {
					// Check state?
				}
			}
		}
	break;
	case 0x90000004:
		IMPLEMENTATION_GUARD(
			*(undefined*)&pPVar1->field_0x24 = *(undefined*)param_3;
		*(undefined*)&pPVar1->field_0x25 = *(undefined*)(param_3 + 1);
		)
			pController->field_0x2c = 1;
	break;
	case 0x90000005:
		IMPLEMENTATION_GUARD(
			local_1 = 3;
		local_4 = *(char*)&pPVar1->field_0x24 != '\0' | *(char*)&pPVar1->field_0x25 << 1;
		local_3 = (undefined)((int)(uint) * (byte*)&pPVar1->field_0x25 >> 7);
		FUN_0035af78(pPVar1->socketNumber, 1, (undefined8*)&local_1, 2, (undefined8*)&local_4);
		)
			pController->field_0x2c = 0;
	break;
	case 0x90000006:
	{
		pController->pPadD[ROUTE_START].bReleased = gInputFunctions.keyReleased(ROUTE_START);
		pController->pPadD[ROUTE_SELECT].bPressed = gInputFunctions.keyPressed(ROUTE_SELECT);

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
	break;
	case 0x90000007:
		IMPLEMENTATION_GUARD(
			uVar6 = 0x10ff;
		if (((pController->flags & 0x40000000) == 0) && (uVar6 = 2, pPVar1->port == 0)) {
			uVar6 = 1;
		})
	}
	return uVar6;
}

#pragma clang optimize off
uint Input::_edDevMouse(uint eventID, EDDEV_PORT* pController, void* param_3)
{
	uint uVar6;
	Pad_d* pPVar5;

	uVar6 = 0;

	switch (eventID) {
	case 0x90000000:
		uVar6 = Mouse::numButtons;
		break;
	case 0x90000001:
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
	case 0x90000002:
	case 0x90000003:
		// Do nothing for now.
		break;
	case 0x90000006:
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
