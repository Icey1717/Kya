#include "edDevInit.h"
#include "edDev.h"

#ifdef PLATFORM_WIN
#include "port/input.h"
#endif

#ifdef PLATFORM_PS2
#include <eekernel.h>
#endif // PLATFORM_PS2


DevRoot edDevRoot;

static EDDEV_PORT EDDEV_PORT_ARRAY_00497700[8];
static Pad_d Pad_d_ARRAY_004978c0[0x100];
static EDDEV_ROUTE EDDEV_ROUTE_ARRAY_0049acc0[0x100];

void _edDevScanPorts(uint flags)
{
	edDevRoot.field_0x3c = 0;

	if ((flags & 0xcaca0001) != 0xcaca0001) {
		IMPLEMENTATION_GUARD_LOG(
		_edDevScanPsxPorts();)
	}

	return;
}

void _edDevInitRoot(void)
{
	uint uVar1;
	int iVar2;
	undefined* puVar3;
	EDDEV_PORT* pDVar4;
	undefined* puVar4;
	Pad_d* pPVar5;
	undefined* puVar5;
	undefined* puVar6;
	undefined* puVar7;
	undefined* puVar8;
	undefined* puVar9;
	undefined* puVar10;

	edDevRoot.aPorts = EDDEV_PORT_ARRAY_00497700;
	//puVar3 = &DAT_0049b6c0;
	edDevRoot.nbPorts = 0;
	edDevRoot.nbPadD = 0;
	edDevRoot.aPadD = Pad_d_ARRAY_004978c0;
	iVar2 = 0;
	//edDevRoot.field_0x8 = 0;
	//edDevRoot.field_0xc = 0;
	edDevRoot.aRoutes = EDDEV_ROUTE_ARRAY_0049acc0;
	//edDevRoot.field_0x20 = 0;
	//edDevRoot.field_0x38 = &PTR_DAT_0049b4c0;
	//uVar1 = 0;
	//do {
	//	uVar1 = uVar1 + 8;
	//	puVar10 = puVar3 + 0x40;
	//	puVar9 = puVar3 + 0x80;
	//	puVar8 = puVar3 + 0xc0;
	//	puVar7 = puVar3 + 0x100;
	//	puVar6 = puVar3 + 0x140;
	//	puVar5 = puVar3 + 0x180;
	//	puVar4 = puVar3 + 0x1c0;
	//	*(undefined**)((int)edDevRoot.field_0x38 + iVar2) = puVar3;
	//	puVar3 = puVar3 + 0x200;
	//	*(undefined**)((int)edDevRoot.field_0x38 + iVar2 + 8) = puVar10;
	//	*(undefined**)((int)edDevRoot.field_0x38 + iVar2 + 0x10) = puVar9;
	//	*(undefined**)((int)edDevRoot.field_0x38 + iVar2 + 0x18) = puVar8;
	//	*(undefined**)((int)edDevRoot.field_0x38 + iVar2 + 0x20) = puVar7;
	//	*(undefined**)((int)edDevRoot.field_0x38 + iVar2 + 0x28) = puVar6;
	//	*(undefined**)((int)edDevRoot.field_0x38 + iVar2 + 0x30) = puVar5;
	//	*(undefined**)((int)edDevRoot.field_0x38 + iVar2 + 0x38) = puVar4;
	//	iVar2 = iVar2 + 0x40;
	//} while (uVar1 < 0x40);
	uVar1 = 0;
	pDVar4 = edDevRoot.aPorts;
	do {
		pDVar4->controllerId = 0;
		uVar1 = uVar1 + 1;
		pDVar4->maxControlId = 0;
		pDVar4->flags = 0x20000000;
		pDVar4->pEventFunc = (ControllerEventFuncPtr)0x0;
		pDVar4->pDisconnectedHandler = (ControllerConnectedFuncPtr)0x0;
		pDVar4->pConnectedHandler = (ControllerConnectedFuncPtr)0x0;
		//pDVar4->field_0x18 = 10;
		//pDVar4->field_0x1c = 0;
		//pDVar4->field_0x20 = 10;
		//pDVar4->field_0x24 = 0;
		//pDVar4->field_0x28 = 0;
		pDVar4->field_0x2c = 0;
		pDVar4->pPad180 = (Pad_180*)0x0;
		pDVar4->pPadD = (Pad_d*)0x0;
		pDVar4 = pDVar4 + 1;
	} while (uVar1 < 8);

	uVar1 = 0;
	pPVar5 = edDevRoot.aPadD;
	do {
		pPVar5->type = 0;
		uVar1 = uVar1 + 1;
		pPVar5->field_0x4 = 0;
		pPVar5->analogValue = 0.0;
		pPVar5->field_0xc = 0;
		pPVar5->clickValue = 0.0;
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
	} while (uVar1 < 0x100);

	// #hack
	edDevRoot.nbMaxPorts = 0x8;
	edDevRoot.nbMaxRoutes = 0x100;
	edDevRoot.nbMaxPadD = 0x100;
	return;
}

void edDevInit(uint flags)
{
	/* < edDev Version : 1.000 >\n */
	edDebugPrintf("< edDev Version : 1.000 >\n");
	_edDevInitRoot();

	IMPLEMENTATION_GUARD_LOG(
		_edDevInitLibs(););
	_edDevScanPorts(flags);
	return;
}

static int g_ControllerCount_004497dc = 0;
static Pad_180 Pad_180_ARRAY_00496b00[8];

#ifdef MOUSE_SUPPORT_EXTENSION_ENABLED
int edDevInitPortMouse(EDDEV_PORT* pController)
{
	int result = 0;

	pController->pEventFunc = Input::_edDevMouse;

	uint uVar2 = pController->pEventFunc(0x90000000, pController, (void*)0x0);
	pController->maxControlId = uVar2;

	if (edDevRoot.nbMaxPadD < edDevRoot.nbPadD + pController->maxControlId) {
		result = -5;
	}
	else {
		pController->pPadD = edDevRoot.aPadD + edDevRoot.nbPadD;
		uVar2 = pController->pEventFunc(0x90000001, pController, (void*)0x0);

		if (uVar2 == 0) {
			pController->flags = pController->flags | 0x40000000;
			pController->controllerId = edDevRoot.nbPorts;
			g_ControllerCount_004497dc = g_ControllerCount_004497dc + 1;
			edDevRoot.nbPorts = edDevRoot.nbPorts + 1;
			edDevRoot.nbPadD = edDevRoot.nbPadD + pController->maxControlId;
		}
	}

	return result;
}
#endif

int edDevInitPort(uint port, int slot, uint type)
{
	undefined uVar1;
	uint uVar2;
	int iVar3;
	int* piVar4;
	undefined* puVar5;
	EDDEV_PORT* pController;
	int result;
	//scePad2SocketParam local_20;
	Pad_180* pPVar2;

	uVar2 = 1;

	if (edDevRoot.nbPorts < edDevRoot.nbMaxPorts) {
		result = 0;
		pController = edDevRoot.aPorts + edDevRoot.nbPorts;
		if (type == INPUT_TYPE_DUALSHOCK) {
			if (g_ControllerCount_004497dc < 8) {
				if (pController->pPad180 == (Pad_180*)0x0) {
					pController->pPad180 = Pad_180_ARRAY_00496b00 + g_ControllerCount_004497dc;
				}

				pPVar2 = pController->pPad180;
#if 0
				puVar5 = &edDevRoot.field_0x4c;
				piVar4 = &local_20.number;
				iVar3 = 10;
#endif
				pPVar2->port = port;
				pPVar2->slot = slot;
				pPVar2->field_0x8 = 0;
				pPVar2->state = 0;
#if 0
				local_20.option = edDevRoot.g_PadOption_00426860;
				do {
					iVar3 = iVar3 + -1;
					uVar1 = puVar5[1];
					*(undefined*)piVar4 = *puVar5;
					*(undefined*)((int)piVar4 + 1) = uVar1;
					puVar5 = puVar5 + 2;
					piVar4 = (int*)((int)piVar4 + 2);
				} while (0 < iVar3);
				local_20.port = port;
				local_20.slot = slot;
				iVar3 = scePad2CreateSocket(&local_20, pPVar2->padBuffer);
				pPVar2->socketNumber = iVar3;
#endif
#ifdef PLATFORM_PS2
				//pController->pEventFunc = _edDevDualShock2;
				//uVar2 = (*pController->pEventFunc)(0x90000000, pController, (InputEventData*)0x0);
				//pController->maxControlId = uVar2;
				//if (edDevRoot.nbMaxPadD < edDevRoot.nbPadD + pController->maxControlId) {
				//	result = -5;
				//}
				//else {
				//	pController->pPadD = edDevRoot.aPadD + edDevRoot.nbPadD;
				//	uVar2 = (*pController->pEventFunc)(0x90000001, pController, (InputEventData*)0x0);
				//	if (uVar2 == 0) {
				//		pController->flags = pController->flags | 0x40000000;
				//		pPVar2->state = 0;
				//		pPVar2->prevState = 0;
				//		pController->controllerId = edDevRoot.nbPorts;
				//		g_ControllerCount_004497dc = g_ControllerCount_004497dc + 1;
				//		edDevRoot.nbPorts = edDevRoot.nbPorts + 1;
				//		edDevRoot.nbPadD = edDevRoot.nbPadD + pController->maxControlId;
				//	}
				//}
#else
				pController->pEventFunc = Input::_edDevKeyboard;

				uVar2 = (*pController->pEventFunc)(0x90000000, pController, (void*)0x0);
				pController->maxControlId = uVar2;
				if (edDevRoot.nbMaxPadD < edDevRoot.nbPadD + pController->maxControlId) {
					result = -5;
				}
				else {
					pController->pPadD = edDevRoot.aPadD + edDevRoot.nbPadD;
					uVar2 = (*pController->pEventFunc)(0x90000001, pController, (void*)0x0);
					if (uVar2 == 0) {
						pController->flags = pController->flags | 0x40000000;
						pPVar2->state = 0;
						pPVar2->prevState = 0;
						pController->controllerId = edDevRoot.nbPorts;
						g_ControllerCount_004497dc = g_ControllerCount_004497dc + 1;
						edDevRoot.nbPorts = edDevRoot.nbPorts + 1;
						edDevRoot.nbPadD = edDevRoot.nbPadD + pController->maxControlId;
					}
				}
#endif
			}
			else {
				result = -3;
			}
		}
		else {
			result = 0;

#ifdef MOUSE_SUPPORT_EXTENSION_ENABLED
			if (type == INPUT_TYPE_MOUSE) {
				result = edDevInitPortMouse(pController);
			}
#endif
		}
	}
	else {
		result = -3;
	}

	return result;
}
