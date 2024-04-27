#include "_edDevDualShock2.h"
#include "../edDev.h"

uint _edDevDualShock2(uint eventID, EDDEV_PORT* pController, void* param_3)
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

	pPVar1 = pController->pPad180;
	uVar6 = 0;
	switch (eventID) {
	case 0x90000000:
		uVar6 = 0x18;
		break;
	case 0x90000001:
		uVar6 = 0;
		pController->flags = 0x108;
#if 0
		pController->pControllerDisconnectedHandler = (ControllerEventFunc*)0x0;
		pController->pControllerReconnectedHandler = (ControllerEventFunc*)0x0;
#endif
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
		} while (uVar6 < 0x18);
		uVar6 = 0;
		break;
	case 0x90000002:
		if (edDevRoot.field_0x3c != 0) {
			pPVar1->field_0x8 = 0;
		}

		IMPLEMENTATION_GUARD_LOG(
		_edDevAttachPsxPort(pController);)
		break;
	case 0x90000003:
		IMPLEMENTATION_GUARD(
		iVar3 = pPVar1->state;
		if ((iVar3 == 0) && (pPVar1->prevState != 0)) {
			pController->flags = pController->flags | 0x40000000;
			if (pController->pControllerDisconnectedHandler != (ControllerEventFunc*)0x0) {
				(*pController->pControllerDisconnectedHandler)(pController->controllerId);
			}
			pPVar1->prevState = 0;
		}
		else {
			if ((pPVar1->prevState == 0) && (iVar3 != 0)) {
				if (iVar3 == 1) {
					pController->flags = pController->flags & 0xbfffffff;
					if (pController->pControllerReconnectedHandler != (ControllerEventFunc*)0x0) {
						(*pController->pControllerReconnectedHandler)(pController->controllerId);
					}
					pPVar1->prevState = pPVar1->state;
				}
			}
			else {
				if ((iVar3 == 1) && (0x61 < (uint)pPVar1->field_0x8)) {
					iVar3 = scePad2Read(pPVar1->socketNumber, pPVar1->pad2ButtonData);
					*(int*)&pPVar1->field_0x20 = iVar3;
				}
			}
		})
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
		IMPLEMENTATION_GUARD(
		bVar2 = pPVar1->state == 1;
		if (bVar2) {
			bVar2 = 0x61 < (uint)pPVar1->field_0x8;
		}
		uVar4 = 0;
		if (bVar2) {
			uVar4 = ~(uint)CONCAT11(pPVar1->pad2ButtonData[2], pPVar1->pad2ButtonData[3]) & 0xffff;
		}
		uVar8 = 1;
		uVar7 = 0;
		piVar9 = &DAT_004267a0;
		*(uint*)&pPVar1->field_0x14 = pPVar1->field_0x10;
		pPVar1->field_0x10 = uVar4;
		pPVar5 = pController->pPadD;
		do {
			if ((pController->field_0x24 == 0) && (pController->field_0x20 == 0)) {
				pPVar5->field_0xc = 0;
			}
			else {
				pPVar5->field_0xc = pPVar5->clickValue;
			}
			iVar3 = *piVar9;
			pPVar5->field_0x4 = pPVar5->analogValue;
			button = *(int*)(&DAT_00426760 + iVar3 * 4);
			uVar4 = scePad2GetButtonInfo(pPVar1->socketNumber, pPVar1->pad2ButtonData, iVar3);
			if (uVar4 == 0) {
				pPVar5->clickValue = 0.0;
			}
			else {
				pPVar5->clickValue = 1.0;
			}
			if (button == 0) {
				FUN_0039a960((int)pPVar5, &pPVar5->rateA);
			}
			else {
				if (bVar2) {
					uVar4 = scePad2GetButtonInfo(pPVar1->socketNumber, pPVar1->pad2ButtonData, button);
					pPVar5->analogValue = (float)uVar4 / 255.0;
				}
				else {
					pPVar5->analogValue = 0.0;
				}
				FUN_0039a870((int)pPVar5, &pPVar5->field_0x28);
			}
			FUN_0039a760((int)pPVar5);
			FUN_0039a7d0((int)pPVar5);
			FUN_0039a820((int)pPVar5);
			uVar7 = uVar7 + 1;
			pPVar5 = pPVar5 + 1;
			piVar9 = piVar9 + 1;
			uVar8 = uVar8 << 1;
		} while (uVar7 < 0x10);
		piVar9 = &DAT_004267a0 + uVar7;
		do {
			fVar10 = 0.5;
			if (bVar2) {
				uVar4 = scePad2GetButtonInfo(pPVar1->socketNumber, pPVar1->pad2ButtonData, *piVar9);
				fVar10 = (float)uVar4 / 255.0;
			}
			if (fVar10 < 0.5) {
				pPVar5->field_0xc = pPVar5->clickValue;
				pPVar5->field_0x4 = pPVar5->analogValue;
				pPVar5->analogValue = (0.5 - fVar10) * 2.0;
				FUN_0039a870((int)pPVar5, &pPVar5->field_0x28);
				if ((pController->field_0x24 == 0) && (pController->field_0x20 == 0)) {
					pPVar5->field_0xc = 0;
				}
				else {
					pPVar5->field_0xc = pPVar5->clickValue;
				}
				FUN_0039a8f0((int)pPVar5, &pPVar5->field_0x18);
				FUN_0039a760((int)pPVar5);
				FUN_0039a7d0((int)pPVar5);
				FUN_0039a820((int)pPVar5);
				if (pPVar5->clickValue == 1.0) {
					pPVar1->field_0x10 = pPVar1->field_0x10 | uVar8;
				}
				pPVar5[1].field_0xc = pPVar5[1].clickValue;
				pPVar5[1].field_0x4 = pPVar5[1].analogValue;
				pPVar5[1].analogValue = 0.0;
				FUN_0039a870((int)(pPVar5 + 1), &pPVar5[1].field_0x28);
				if ((pController->field_0x24 == 0) && (pController->field_0x20 == 0)) {
					pPVar5[1].field_0xc = 0;
				}
				else {
					pPVar5[1].field_0xc = pPVar5[1].clickValue;
				}
				FUN_0039a8f0((int)(pPVar5 + 1), &pPVar5[1].field_0x18);
				FUN_0039a760((int)(pPVar5 + 1));
				FUN_0039a7d0((int)(pPVar5 + 1));
				FUN_0039a820((int)(pPVar5 + 1));
				if (pPVar5[1].clickValue == 1.0) {
					pPVar1->field_0x10 = pPVar1->field_0x10 | uVar8 << 1;
				}
			}
			else {
				pPVar5->field_0xc = pPVar5->clickValue;
				pPVar5->field_0x4 = pPVar5->analogValue;
				pPVar5->analogValue = 0.0;
				FUN_0039a870((int)pPVar5, &pPVar5->field_0x28);
				if ((pController->field_0x24 == 0) && (pController->field_0x20 == 0)) {
					pPVar5->field_0xc = 0;
				}
				else {
					pPVar5->field_0xc = pPVar5->clickValue;
				}
				FUN_0039a8f0((int)pPVar5, &pPVar5->field_0x18);
				FUN_0039a760((int)pPVar5);
				FUN_0039a7d0((int)pPVar5);
				FUN_0039a820((int)pPVar5);
				if (pPVar5->clickValue == 1.0) {
					pPVar1->field_0x10 = pPVar1->field_0x10 | uVar8;
				}
				pPVar5[1].field_0xc = pPVar5[1].clickValue;
				pPVar5[1].field_0x4 = pPVar5[1].analogValue;
				pPVar5[1].analogValue = (fVar10 - 0.5) * 2.0;
				FUN_0039a870((int)(pPVar5 + 1), &pPVar5[1].field_0x28);
				if ((pController->field_0x24 == 0) && (pController->field_0x20 == 0)) {
					pPVar5[1].field_0xc = 0;
				}
				else {
					pPVar5[1].field_0xc = pPVar5[1].clickValue;
				}
				FUN_0039a8f0((int)(pPVar5 + 1), &pPVar5[1].field_0x18);
				FUN_0039a760((int)(pPVar5 + 1));
				FUN_0039a7d0((int)(pPVar5 + 1));
				FUN_0039a820((int)(pPVar5 + 1));
				if (pPVar5[1].clickValue == 1.0) {
					pPVar1->field_0x10 = pPVar1->field_0x10 | uVar8 << 1;
				}
			}
			pPVar5 = pPVar5 + 2;
			uVar8 = uVar8 << 2;
			uVar7 = uVar7 + 1;
			piVar9 = piVar9 + 1;
		} while (uVar7 < 0x14);
		if (((*(uint*)&pPVar1->field_0x14 & pPVar1->field_0x10) == 0) || (pController->field_0x1c == 0)) {
			pController->field_0x24 = 0;
			pController->field_0x20 = pController->field_0x18;
		}
		else {
			if (pController->field_0x20 == 0) {
				pController->field_0x24 = pController->field_0x24 + 1;
				if ((uint)pController->field_0x1c < (uint)pController->field_0x24) {
					pController->field_0x24 = 0;
				}
			}
			else {
				pController->field_0x20 = pController->field_0x20 + -1;
			}
		})
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