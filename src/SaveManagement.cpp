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
#include "EdenLib/edFile/include/edFile.h"
#include "EdenLib/edFile/include/edFileNoWaitStack.h"
#include "EdenLib/edFile/include/edFileCRC32.h"

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
	gSaveManagement.pFiler = edFileGetFiler(acStack512, gSaveManagement.memCardAccessPath, 1);
	if (gVideoConfig.omode == 3) {
		edStrCopy(gSaveManagement.serialNumber, "BESLES-51473-KYA");
	}
	else {
		edStrCopy(gSaveManagement.serialNumber, "BASLUS-20440-KYA");
	}

	if ((gSaveManagement.boot_check_load()) != 0) {
		gSaveManagement.field_0x0 = 0;
		const int deviceRoomCheckResult = gSaveManagement.test_device_has_enough_room();
		if (((deviceRoomCheckResult == 3) || (deviceRoomCheckResult == 2)) && (gSaveManagement.load_settings() != false)) {
			if (gSettings.LoadFromBuffer(gSaveManagement.pSettings_0x38) != false) {
				gSettings.SetSettingsToGlobal();
			}
			gSaveManagement.field_0x0 = 1;
		}
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

			deviceRoomCheckResult = test_device_has_enough_room();
			if (deviceRoomCheckResult == ROOM_CHECK_RESULT_OK) {
				this->field_0x0 = 1;
				return true;
			}

			if (deviceRoomCheckResult != ROOM_CHECK_RESULT_NOT_ENOUGH_ROOM) break;

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

int CSaveManagement::test_device_has_enough_room()
{
	edCFiler* peVar1;
	bool bSaveDirExists;
	int iVar3;
	edCFileNoWaitStack* peVar4;
	uint uVar5;
	long lVar6;
	uint uVar7;
	char* outString;
	edFILE_INFO fileInfo;
	char szPath[512];

	iVar3 = edFileIsIdle(this->memCardAccessPath, 1);

	if ((iVar3 != 0) || (this->bBootCheckLoadComplete != 0)) {
#ifdef PLATFORM_PS2
		scePcStop();
#endif
		if (this->pFiler != (edCFiler*)0x0) {
			// Run twice, once for each memory card slot.
			edFileNoWaitStackFlush(this->pFiler);
			this->pFiler->getnowaitfilestack()->AddFilerSync(this->pFiler);

			edFileNoWaitStackFlush(this->pFiler);
			this->pFiler->getnowaitfilestack()->AddFilerSync(this->pFiler);

			edFileNoWaitStackFlush(this->pFiler);
		}

		this->fileExistsFlags = 0;
		this->freeBytes = 0;
		this->field_0x3 = 0;
		this->field_0x1 = 0;
		this->deviceRoomState = 0;
		this->nbSaveFiles = 0;
		this->bBootCheckLoadComplete = 0;

		this->field_0x848[0].levelId = 0x10;
		this->field_0x848[1].levelId = 0x10;
		this->field_0x848[2].levelId = 0x10;
		this->field_0x848[3].levelId = 0x10;

		// Now that we have synced we can get the free space of the MC.
		if (edFileGetFreeSpace(this->memCardAccessPath, &this->freeBytes, 0, &this->field_0x2) != false) {
			this->field_0x1 = 1;
			if (this->field_0x2 == 0) {
				this->freeBytes = 0xfffffffff4000000;
				this->field_0x3 = 1;
				this->deviceRoomState = ROOM_CHECK_RESULT_ERROR;
			}
			else {
				this->field_0x10 = this->freeBytes;
				iVar3 = edStrCopy(szPath, this->memCardDriveString);
				outString = szPath + iVar3;
				iVar3 = edStrCopy(outString, this->memCardPathEnd);
				edStrCopy(outString + iVar3, this->serialNumber);

				bSaveDirExists = edFileFindFirst(&fileInfo, szPath);
				if (bSaveDirExists != false) {
					edFileFindClose();

					this->fileExistsFlags = this->fileExistsFlags | 1;
					this->field_0x10 = this->field_0x10 + 0x800;

					if (file_exists("icon.sys", gSaveManagementInfoIconSys.size) != false) {
						this->fileExistsFlags = this->fileExistsFlags | 2;
					}

					if (file_exists("KYA.ICO", gSaveManagementInfoFileIco.size) != false) {
						this->fileExistsFlags = this->fileExistsFlags | 4;
					}

					if (file_exists(this->serialNumber, 0x400) != false) {
						this->fileExistsFlags = this->fileExistsFlags | 8;
					}

					if (file_exists("settings.dat", CSettings::GetMaxSaveBufferSize() + 0x1c) != false) {
						this->fileExistsFlags = this->fileExistsFlags | 0x10;
					}

					if (file_exists("slot_0.dat", 0x10000) != false) {
						this->fileExistsFlags = this->fileExistsFlags | 0x20;
						read_slot_info(0);
					}

					if (file_exists("slot_1.dat", 0x10000) != false) {
						this->fileExistsFlags = this->fileExistsFlags | 0x40;
						read_slot_info(1);
					}

					if (file_exists("slot_2.dat", 0x10000) != false) {
						this->fileExistsFlags = this->fileExistsFlags | 0x80;
						read_slot_info(2);
					}

					if (file_exists("slot_3.dat", 0x10000) != false) {
						this->fileExistsFlags = this->fileExistsFlags | 0x100;
						read_slot_info(3);
					}

					this->field_0x10 = this->field_0x10 + ((long)(int)((this->nbSaveFiles + 1U >> 1) << 10) & 0xffffffffU);
				}

				const int iconSysSize = gSaveManagementInfoIconSys.size + 0x3ff;
				const int iconSize = gSaveManagementInfoFileIco.size + 0x3ff;
				if (this->field_0x10 < ((long)(int)(((iconSysSize >> 10) + 0x10b + (iconSize >> 10) + (CSettings::GetMaxSaveBufferSize() + 0x41bU >> 10)) * 0x400) & 0xffffffffU)) {
					this->deviceRoomState = ROOM_CHECK_RESULT_NOT_ENOUGH_ROOM;
				}
				else {
					this->deviceRoomState = ROOM_CHECK_RESULT_OK;
				}
			}
		}
	}

	return this->deviceRoomState;
}

bool CSaveManagement::file_exists(char* name, uint size)
{
	bool bResult;
	int iVar2;
	int iVar3;
	char* pbVar4;
	edFILE_INFO fileInfo;
	char szFullPath[512];

	iVar2 = edStrCopy(szFullPath, this->memCardDriveString);
	pbVar4 = szFullPath + iVar2;
	iVar2 = edStrCopy(pbVar4, this->memCardPathEnd);
	pbVar4 = pbVar4 + iVar2;
	iVar2 = edStrCopy(pbVar4, this->serialNumber);
	iVar3 = edStrCopy(pbVar4 + iVar2, "\\");
	edStrCopy(pbVar4 + iVar2 + iVar3, name);

	bResult = edFileFindFirst(&fileInfo, szFullPath);
	if (bResult == false) {
		bResult = false;
	}
	else {
		edFileFindClose();

		if (fileInfo.fileSize < size) {
			edFileDelete(szFullPath);
			bResult = false;
		}
		else {
			bResult = true;
			this->field_0x10 = this->field_0x10 + ((ulong)(int)(fileInfo.fileSize + 0x3ff & 0xfffffc00) & 0xffffffffU);
			this->nbSaveFiles = this->nbSaveFiles + 1;
		}
	}

	return bResult;
}

void CSaveManagement::read_slot_info(int slotId)
{

}

struct SettingsHeader
{
	uint hash;
	uint headerCrc;
	int headerSize;
	uint settingsCrc;
	int settingsSize;
	int field_0x14;
	int field_0x18;
};

static_assert(sizeof(SettingsHeader) == 0x1c, "Incorrect padding in SettingsHeader structure");

bool CSaveManagement::load_settings()
{
	byte bVar1;
	int pathLen;
	int fileNameLen;
	edFILEH* settingsFilePtr;
	uint uVar2;
	char* filePath;
	bool bResult;
	SettingsHeader local_220;
	char memCardPath[512];

	bResult = false;
	pathLen = edStrCopy(memCardPath, this->memCardDriveString);
	filePath = memCardPath + pathLen;
	pathLen = edStrCopy(filePath, this->memCardPathEnd);
	fileNameLen = edStrCopy(filePath + pathLen, this->serialNumber);
	edStrCopy(filePath + pathLen + fileNameLen, "\\settings.dat");

	/* Tries to load <mc>0:BESLES-51473-KYA\settings.dat */
	settingsFilePtr = edFileOpen(memCardPath, 1);
	if (settingsFilePtr != (edFILEH*)0x0) {
		bVar1 = edFileRead(settingsFilePtr, (char*)&local_220, 0x1c);
		if ((((((bVar1 != 0) && (local_220.hash == 0x4544454e)) && (local_220.headerSize == 0x1c)) && (local_220.headerCrc == edFileComputeCRC32(&local_220.headerSize, 0x14))) &&
			(((local_220.settingsSize != 0 || (local_220.field_0x18 != 0)) || ((local_220.settingsCrc != 0 || (local_220.field_0x14 != 0)))))) &&
			(((this->settingsSize_0x40 == local_220.settingsSize && (edFileRead(settingsFilePtr, (char*)this->pSettings_0x38, this->settingsSize_0x40) != false)) &&
				(edFileComputeCRC32(this->pSettings_0x38, this->settingsSize_0x40) == local_220.settingsCrc)))) {
			bResult = true;
		}

		edFileClose(settingsFilePtr);
	}

	return bResult;
}

void CSaveManagement::clear_slot()
{
	this->slotID_0x28 = -1;
	return;
}

#define MESSAGE_BOX_CATEGORY_SAVE 5
#define MESSAGE_BOX_SAVE_ID_RETRY_CANCEL 2
#define MESSAGE_BOX_SAVE_ID_IN_PROGRESS 5

bool CSaveManagement::save_sequence()
{
	void* pData;
	SaveBigAlloc* pData_00;
	char* pLVar1;
	bool bVar2;
	char cVar3;
	int iVar4;
	int iVar5;
	edFILEH* peVar6;
	char* pcVar7;
	uint uVar8;
	undefined auStack1568[1024];
	SettingsHeader settingsHeader;
	char acStack512[512];

	this->field_0x0 = 0;
LAB_002f3a30:
	do {
		while (true) {
			while (true) {
				while (true) {
#ifdef PLATFORM_PS2
					scePcStop();
#endif
					this->bBootCheckLoadComplete = 1;
					iVar4 = test_device_has_enough_room();
					if (iVar4 != 3) {
						return false;
					}

					if (this->fileExistsFlags == 0x1ff) {
						return true;
					}

					message_box(MESSAGE_BOX_CATEGORY_SAVE, MESSAGE_BOX_SAVE_ID_IN_PROGRESS);

					iVar4 = edStrCopy(acStack512, this->memCardDriveString);
					pcVar7 = acStack512 + iVar4;
					iVar4 = edStrCopy(pcVar7, this->memCardPathEnd);
					iVar5 = edStrCopy(pcVar7 + iVar4, this->serialNumber);
					pcVar7 = pcVar7 + iVar4 + iVar5;

					if ((this->fileExistsFlags & 1) != 0) break;

					bVar2 = edFileMakeDir(acStack512);
					if ((bVar2 == false) && (bVar2 = message_box(MESSAGE_BOX_CATEGORY_SAVE, MESSAGE_BOX_SAVE_ID_RETRY_CANCEL), bVar2 == true)) {
						return false;
					}
				}

				if ((this->fileExistsFlags & 2) == 0) break;
				iVar4 = edStrCopy(pcVar7, this->memCardDriveString);
				pcVar7 = acStack512 + iVar4;
				iVar4 = edStrCopy(pcVar7, this->memCardPathEnd);
				iVar5 = edStrCopy(pcVar7 + iVar4, this->serialNumber);
				edStrCopy(pcVar7 + iVar4 + iVar5, "\\icon.sys");
				bVar2 = edFileDelete(acStack512);
				if ((bVar2 == false) && (bVar2 = message_box(MESSAGE_BOX_CATEGORY_SAVE, MESSAGE_BOX_SAVE_ID_RETRY_CANCEL), bVar2 == true)) {
					return false;
				}
			}

			settingsHeader.settingsCrc = 0;
			settingsHeader.settingsSize = 0;
			settingsHeader.field_0x14 = 0;
			settingsHeader.field_0x18 = 0;
			settingsHeader.hash = 0x4544454e;
			settingsHeader.headerSize = 0x1c;
			settingsHeader.headerCrc = edFileComputeCRC32(&settingsHeader.headerSize, 0x14);

			if ((this->fileExistsFlags & 0x10) != 0) break;

			iVar4 = edStrCopy(pcVar7, this->memCardDriveString);
			pcVar7 = acStack512 + iVar4;
			iVar4 = edStrCopy(pcVar7, this->memCardPathEnd);
			pcVar7 = pcVar7 + iVar4;
			iVar4 = edStrCopy(pcVar7, this->serialNumber);
			iVar5 = edStrCopy(pcVar7 + iVar4, "\\settings.dat");
			pcVar7 = pcVar7 + iVar4 + iVar5;
			iVar4 = CSettings::GetMaxSaveBufferSize();
			pData = this->pSettings_0x38;
#ifdef PLATFORM_PS2
			scePcStop();
#endif			
			peVar6 = edFileOpen(acStack512, 6);
			if (peVar6 == (edFILEH*)0x0) {
			LAB_002f3cb8:
				bVar2 = false;
			}
			else {
				bool bWriteSuccess = edFileWrite(peVar6, &settingsHeader, 0x1c);
				if ((bWriteSuccess != false) && (iVar4 + 0x1c != 0)) {
					bWriteSuccess = edFileWrite(peVar6, pData, iVar4 + 0x1c);
				}

				edFileClose(peVar6);

				if (bWriteSuccess == false) goto LAB_002f3cb8;

				bVar2 = true;
			}

			if (bVar2) break;

			bVar2 = message_box(MESSAGE_BOX_CATEGORY_SAVE, MESSAGE_BOX_SAVE_ID_RETRY_CANCEL);
			if (bVar2 == true) {
				return false;
			}
		}
		bVar2 = true;
		uVar8 = 0;
		do {
			if ((this->fileExistsFlags & 0x20 << (uVar8 & 0x1f)) == 0) {
				iVar4 = edStrCopy(pcVar7, this->memCardDriveString);
				pcVar7 = acStack512 + iVar4;
				iVar4 = edStrCopy(pcVar7, this->memCardPathEnd);
				pcVar7 = pcVar7 + iVar4;
				iVar4 = edStrCopy(pcVar7, this->serialNumber);
				pcVar7 = pcVar7 + iVar4;
				iVar4 = edStrCopy(pcVar7, "\\slot_");
				pcVar7 = pcVar7 + iVar4;
				*pcVar7 = (char)uVar8 + '0';
				iVar4 = edStrCopy(pcVar7 + 1, ".dat");
				pData_00 = this->pBigAlloc_0x34;
				pcVar7 = pcVar7 + 1 + iVar4;
#ifdef PLATFORM_PS2
				scePcStop();
#endif				
				peVar6 = edFileOpen(acStack512, 6);
				if (peVar6 != (edFILEH*)0x0) {
					if (edFileWrite(peVar6, &settingsHeader, 0x1c) != false) {
						cVar3 = edFileWrite(peVar6, pData_00, 0x10000);
					}

					edFileClose(peVar6);

					if (cVar3 != false) {
						bVar2 = true;
						goto LAB_002f3df0;
					}
				}

				bVar2 = false;
			}
		LAB_002f3df0:
		} while ((bVar2) && (uVar8 = uVar8 + 1, (int)uVar8 < 4));

		if (bVar2) {
			if ((this->fileExistsFlags & 8) == 0) {
				iVar4 = edStrCopy(pcVar7, this->memCardDriveString);
				pcVar7 = acStack512 + iVar4;
				iVar4 = edStrCopy(pcVar7, this->memCardPathEnd);
				pcVar7 = pcVar7 + iVar4;
				iVar4 = edStrCopy(pcVar7, this->serialNumber);
				pcVar7 = pcVar7 + iVar4;
				iVar4 = edStrCopy(pcVar7, "\\");
				iVar5 = edStrCopy(pcVar7 + iVar4, this->serialNumber);
				pcVar7 = pcVar7 + iVar4 + iVar5;
#ifdef PLATFORM_PS2
				scePcStop();
#endif				
				peVar6 = edFileOpen(acStack512, 6);
				if (peVar6 == (edFILEH*)0x0) {
				LAB_002f3ee8:
					bVar2 = false;
				}
				else {
					bVar2 = edFileWrite(peVar6, auStack1568, 0x400);
					edFileClose(peVar6);

					if (bVar2 == false) goto LAB_002f3ee8;

					bVar2 = true;
				}

				if (!bVar2) {
					bVar2 = message_box(5, 2);
					if (bVar2 == true) {
						return false;
					}

					goto LAB_002f3a30;
				}
			}

			if ((this->fileExistsFlags & 4) == 0) {
				iVar4 = edStrCopy(pcVar7, this->memCardDriveString);
				pcVar7 = acStack512 + iVar4;
				iVar4 = edStrCopy(pcVar7, this->memCardPathEnd);
				pcVar7 = pcVar7 + iVar4;
				iVar4 = edStrCopy(pcVar7, this->serialNumber);
				iVar5 = edStrCopy(pcVar7 + iVar4, "\\KYA.ICO");
				pLVar1 = gSaveManagementInfoFileIco.fileBufferStart;
				uVar8 = gSaveManagementInfoFileIco.size;
				pcVar7 = pcVar7 + iVar4 + iVar5;
#ifdef PLATFORM_PS2
				scePcStop();
#endif				
				peVar6 = edFileOpen(acStack512, 6);
				if (peVar6 == (edFILEH*)0x0) {
				LAB_002f3fd0:
					bVar2 = false;
				}
				else {
					bVar2 = edFileWrite(peVar6, pLVar1, uVar8);
					edFileClose(peVar6);

					if (bVar2 == false) goto LAB_002f3fd0;

					bVar2 = true;
				}

				if (!bVar2) {
					bVar2 = message_box(5, 2);
					if (bVar2 == true) {
						return false;
					}

					goto LAB_002f3a30;
				}
			}

			if (this->fileExistsFlags == 0x1fd) {
				iVar4 = edStrCopy(pcVar7, this->memCardDriveString);
				pcVar7 = acStack512 + iVar4;
				iVar4 = edStrCopy(pcVar7, this->memCardPathEnd);
				iVar5 = edStrCopy(pcVar7 + iVar4, this->serialNumber);
				edStrCopy(pcVar7 + iVar4 + iVar5, "\\icon.sys");
				pLVar1 = gSaveManagementInfoIconSys.fileBufferStart;
				uVar8 = gSaveManagementInfoIconSys.size;
#ifdef PLATFORM_PS2
				scePcStop();
#endif				
				peVar6 = edFileOpen(acStack512, 6);
				if (peVar6 == (edFILEH*)0x0) {
				LAB_002f40b8:
					bVar2 = false;
				}
				else {
					bVar2 = edFileWrite(peVar6, pLVar1, uVar8);
					edFileClose(peVar6);

					if (bVar2 == false) goto LAB_002f40b8;

					bVar2 = true;
				}

				if (bVar2) {
					return true;
				}

				bVar2 = message_box(5, 2);
				if (bVar2 == true) {
					return false;
				}
			}
			goto LAB_002f3a30;
		}

		bVar2 = message_box(5, 2);
		if (bVar2 == true) {
			return false;
		}
	} while (true);
}

SaveData5* CSaveManagement::GetSaveData5(int index)
{
	SaveData5* pSVar1;

	if ((index < 0) || (3 < index)) {
		pSVar1 = (SaveData5*)0x0;
	}
	else {
		pSVar1 = this->field_0x848 + index;
	}

	return pSVar1;
}
