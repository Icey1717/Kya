#include "SaveManagement.h"
#include "BootData.h"
#include "edC/edCBank.h"
#include "edMem.h"
#include "EdenLib/edText/sources/edTextResources.h"
#include "edStr.h"
#include "LevelScheduleManager.h"
#include "Settings.h"
#include "MenuMessageBox.h"
#include "kya.h"
#include <string.h>

#include "edFilePath.h"
#include "edFileFiler.h"

#ifdef PLATFORM_PS2
#include "libpc.h"
#include <stdio.h>
#include <eekernel.h>
#endif

edBANK_ENTRY_INFO gSaveManagementInfoIconSys;
edBANK_ENTRY_INFO gSaveManagementInfoFileIco;
char gSaveManagementSaveGameSize[256];

CSaveManagement gSaveManagement = {};

char gDumpedSettings[256] = {0x53, 0x47, 0x54, 0x53, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

void SaveManagementBootCheck(void)
{
	bool bVar1;
	int iVar2;
	uint uVar3;
	long lVar4;
	uint uVar5;
	char acStack512[512];

	BootData_GetResource("icon.sys", &gSaveManagementInfoIconSys);
	BootData_GetResource("KYA.ICO", &gSaveManagementInfoFileIco);

	gSaveManagement.pBigAlloc_0x34 = (SaveBigAlloc*)edMemAlloc(TO_HEAP(H_MAIN), 0x10000);
	gSaveManagement.gameSaveMaxBufferSize = CLevelScheduler::SaveGame_GetMaxBufferSize();

	if (gSaveManagement.pBigAlloc_0x34 == (SaveBigAlloc*)0x0) {
		gSaveManagement.gameSaveMaxBufferSize = 0;
	}

	gSaveManagement.pSettings_0x38 = edMemAlloc(TO_HEAP(H_MAIN), CSettings::GetMaxSaveBufferSize());
	if (gSaveManagement.pSettings_0x38 == (void*)0x0) {
		gSaveManagement.pSettings_0x38 = (void*)0x0;
	}

	gSaveManagement.settingsSize_0x40 = gSaveManagement.settingsSize_0x40;
	uVar5 = gSaveManagementInfoIconSys.size + 0x3ff;
	uVar3 = gSaveManagementInfoFileIco.size + 0x3ff;
	sprintf(gSaveManagementSaveGameSize, "%d KB", ((uVar5 >> 10) + 0x10b + (uVar3 >> 10) + (CSettings::GetMaxSaveBufferSize() + 0x41bU >> 10)) * 0x400 + 0x3ff >> 10);

	edTextResources.TextAdd("SAVESIZE", gSaveManagementSaveGameSize);
	edFilePathSplit(gSaveManagement.memCardDriveString, gSaveManagement.memCardPathEnd, (char*)0x0, (char*)0x0, "<mc>0:\\");
	edStrCopy(gSaveManagement.memCardAccessPath, "<mc>0:\\");
	gSaveManagement.field_0x8b4 = edFileGetFiler(acStack512, gSaveManagement.memCardAccessPath, 1);
	if (gVideoConfig.omode == 3) {
		edStrCopy(gSaveManagement.serialNumber, "BESLES-51473-KYA");
	}
	else {
		edStrCopy(gSaveManagement.serialNumber, "BASLUS-20440-KYA");
	}

	if ((gSaveManagement.boot_check_load()) != 0) {
		gSaveManagement.field_0x0 = 0;
//#ifdef PLATFORM_WIN
		memcpy(gSaveManagement.pSettings_0x38, gDumpedSettings, CSettings::GetMaxSaveBufferSize());
//#endif
		//iVar2 = gSaveManagement.getType();
		//if (((iVar2 == 3) || (iVar2 == 2)) && (lVar4 = gSaveManagement.load_settings(), lVar4 != 0)) {
			bVar1 = gSettings.LoadFromBuffer(gSaveManagement.pSettings_0x38);
		//	//if (bVar1 != false) {
				gSettings.SetSettingsToGlobal();
		//	//}
		//	gSaveManagement.field_0x0 = 1;
		//}
	}
	return;
}

bool CSaveManagement::boot_check_load()
{
	int deviceRoomCheckResult;
	bool bClosed;

	this->field_0x0 = 0;
	do {
		while (true) {
#ifdef PLATFORM_PS2
			scePcStop();
#endif
			this->bBootCheckLoadComplete = 1;
			deviceRoomCheckResult = 3; // test_device_has_enough_room();
			if (deviceRoomCheckResult == 3) {
				this->field_0x0 = 1;
				return true;
			}

			if (deviceRoomCheckResult != 2) break;

			bClosed = message_box(3, 6);
			if (bClosed == true) {
				return false;
			}
		}

		if (bClosed == true) {
			return false;
		}
	} while ((deviceRoomCheckResult != 0) || (bClosed = message_box(3, 0), bClosed != true));

	return false;
}

bool CSaveManagement::message_box(long operationID, long messageID)
{
	bool bVar1;
	ulong uVar2;
	ulong uVar3;
	ulong uVar4;
	ulong uVar5;
	ulong flags;

	uVar3 = 0x1210100006164e;
	flags = 0;
	uVar2 = 0x1201141a170a02;
	switch (operationID) {
	case 0:
	case 1:
		if (operationID == 1) {
			uVar5 = 0x45585649544c490c;
		}
		else {
			uVar5 = 0x414147001a0b1a18;
		}
		if (messageID == 1) {
			uVar4 = 0x5f46474c454d5941;
			flags = 5;
			uVar3 = 0x494648031106181c;
			uVar2 = 0x1201141a170a02;
		}
		else {
			if (messageID == 5) {
				uVar3 = 0;
				uVar2 = 0;
				uVar4 = 0x5744534a5d071e14;
			}
			else {
				if (messageID == 4) {
					uVar2 = 0;
					uVar4 = 0xc535b505243455d;
					uVar3 = 0x120d1e54544f4e;
				}
				else {
					if (messageID == 3) {
						uVar4 = 0x1701150d445a5d5d;
						uVar3 = 0x1210100006164e;
						uVar2 = 0x1201141a170a02;
					}
					else {
						if (messageID == 2) {
							uVar4 = 0x49574c564a480f00;
							uVar3 = 0x1210100006164e;
							uVar2 = 0x1201141a170a02;
						}
						else {
							/* CSaveManagement::message_box: Unknow code */
							uVar4 = 0;
							edDebugPrintf("CSaveManagement::message_box: Unknow code");
						}
					}
				}
			}
		}
		break;
	case 2:
		if (operationID == 2) {
			uVar5 = 0x584358481d020200;
		}
		else {
			uVar5 = 0x4c474159544c4553;
		}
		if (messageID == 5) {
			uVar3 = 0;
			uVar2 = 0;
			uVar4 = 0x59535e4d54401e0b;
		}
		else {
			if (messageID == 2) {
				uVar4 = 0x514d445840585a09;
			}
			else {
				/* CSaveManagement::message_box: Unknow code */
				uVar4 = 0;
				edDebugPrintf("CSaveManagement::message_box: Unknow code");
			}
		}
		break;
	case 3:
		uVar5 = 0x4347020c0b0f0d1a;
		if (messageID == 6) {
			uVar4 = 0x14474442585e4b46;
			flags = 1;
			uVar2 = 0;
			uVar3 = 0x4155440c0600011c;
		}
		else {
			if (messageID == 0) {
				uVar4 = 0x594d5d4557521e09;
				flags = 1;
				uVar2 = 0;
				uVar3 = 0x4155440c0600011c;
			}
			else {
				/* CSaveManagement::message_box: Unknow code */
				uVar4 = 0;
				edDebugPrintf("CSaveManagement::message_box: Unknow code");
			}
		}
		break;
	case 4:
		uVar5 = 0x525d4545475c0c11;
		if (messageID == 2) {
			uVar4 = 0xd5c4c55444c525c;
		}
		else {
			if (messageID == 5) {
				uVar3 = 0;
				uVar2 = 0;
				uVar4 = 0x594057454e4e5046;
			}
			else {
				/* CSaveManagement::message_box: Unknow code */
				uVar4 = 0;
				edDebugPrintf("CSaveManagement::message_box: Unknow code");
			}
		}
		break;
	case 5:
		uVar5 = 0x5e40464d5e010000;
		if (messageID == 2) {
			uVar4 = 0x5855515244405a5c;
		}
		else {
			if (messageID == 5) {
				uVar3 = 0;
				uVar2 = 0;
				uVar4 = 0x160f115349595b;
			}
			else {
				/* CSaveManagement::message_box: Unknow code */
				uVar4 = 0;
				edDebugPrintf("CSaveManagement::message_box: Unknow code");
			}
		}
		break;
	default:
		uVar5 = 0;
		/* MCardMenus_Warning: Unknow code */
		uVar4 = 0;
		edDebugPrintf("MCardMenus_Warning: Unknow code");
	}

	return MenuMessageBoxDisplay(flags, uVar5, uVar4, uVar3, uVar2);
}

void CSaveManagement::clear_slot()
{
	this->slotID_0x28 = -1;
	return;
}
