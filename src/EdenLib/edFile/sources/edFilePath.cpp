#include "edFilePath.h"

#include "edFileFiler.h"
#include "edStr.h"

edCFilePathManager edFilePathManager;

char* sz_PC_DrivePath_00431468 = "<PC>";

// Inlined
edCFilePathManager::edCFilePathManager()
{
	bool bVar1;
	char cVar2;
	char cVar3;
	char* pcVar4;
	char* pcVar5;

	edFilePathManager.field_0x0[0].field_0x0 = 0;
	edFilePathManager.field_0x0[0].drivePath[0] = '\0';
	pcVar4 = edFilePathManager.rootDrivePath;
	edFilePathManager.count_0x1224 = 1;
	edFilePathManager.field_0x0[0].field_0x4 = 0;
	edFilePathManager.field_0x0[1].drivePath[0] = '\0';
	edFilePathManager.field_0x0[0].field_0x8 = 0;
	edFilePathManager.field_0x0[2].drivePath[0] = '\0';
	edFilePathManager.field_0x0[0].field_0xc = 0;
	edFilePathManager.field_0x0[3].drivePath[0] = '\0';
	edFilePathManager.field_0x0[0].field_0x10 = 0;
	edFilePathManager.field_0x0[4].drivePath[0] = '\0';
	edFilePathManager.field_0x0[0].field_0x14 = 0;
	edFilePathManager.field_0x0[5].drivePath[0] = '\0';
	edFilePathManager.field_0x0[0].field_0x18 = 0;
	edFilePathManager.field_0x0[6].drivePath[0] = '\0';
	edFilePathManager.field_0x0[0].field_0x1c = 0;
	edFilePathManager.field_0x0[7].drivePath[0] = '\0';
	pcVar5 = sz_PC_DrivePath_00431468;
	cVar2 = sz_PC_DrivePath_00431468[0];
	if (sz_PC_DrivePath_00431468[0] != '\0') {
		do {
			pcVar5 = pcVar5 + 1;
			bVar1 = false;
			if (('`' < cVar2) && (cVar2 < '{')) {
				bVar1 = true;
			}
			cVar3 = cVar2;
			if (bVar1) {
				cVar3 = cVar2 + -0x20;
			}
			*pcVar4 = cVar3;
			pcVar4 = pcVar4 + 1;
			if (cVar2 == '>') break;
			cVar2 = *pcVar5;
		} while (*pcVar5 != '\0');
	}
	*pcVar4 = '\0';
	return;
}

void edCFilePathManager::get_path(char* param_2)
{
	char* pcVar1;
	int iVar2;
	edCFilePathManager* peVar3;
	uint objIndex;

	pcVar1 = edStrChr(param_2, ':');
	objIndex = 0;
	peVar3 = this;
	do {
		iVar2 = memcmp(peVar3->field_0x0[0].drivePath, param_2, (long)(int)(pcVar1 + (1 - (int)param_2)));
		if (iVar2 == 0) {
			count_0x1224 = count_0x1224 + 1;
			*(int*)(field_0x0[0].drivePath + objIndex * 4 + -0x20) = count_0x1224;
			edStrCopy(param_2, field_0x0[objIndex].drivePath);
			return;
		}
		objIndex = objIndex + 1;
		peVar3 = (edCFilePathManager*)(peVar3->field_0x0 + 1);
	} while (objIndex < 8);
	pcVar1[1] = '\\';
	pcVar1[2] = '\0';
	return;
}

int edCFilePathManager::set_path(char* param_2)
{
	bool bVar1;
	char* pcVar2;
	int iVar3;
	edCFilePathManager* peVar4;
	edCFilePathManager* peVar5;
	uint uVar6;
	uint uVar7;
	uint uVar8;

	pcVar2 = edStrChr(param_2, ':');
	if ((pcVar2[1] != '\\') || (bVar1 = true, pcVar2[2] != '\0')) {
		bVar1 = false;
	}
	uVar7 = 0;
	uVar6 = 0;
	peVar4 = this;
	peVar5 = this;
	do {
		iVar3 = memcmp(peVar5->field_0x0[0].drivePath, param_2, (long)(int)(pcVar2 + (1 - (int)param_2)));
		if (iVar3 == 0) {
			uVar8 = uVar6;
			if (bVar1) {
				field_0x0[uVar6].drivePath[0] = '\0';
				*(undefined4*)(field_0x0[0].drivePath + uVar7 * 4 + -0x20) = 0;
				return 0;
			}
			break;
		}
		if ((uint)peVar4->field_0x0[0].field_0x0 < *(uint*)(field_0x0[0].drivePath + uVar7 * 4 + -0x20)) {
			uVar7 = uVar6;
		}
		uVar6 = uVar6 + 1;
		peVar5 = (edCFilePathManager*)(peVar5->field_0x0 + 1);
		peVar4 = (edCFilePathManager*)&peVar4->field_0x0[0].field_0x4;
		uVar8 = uVar7;
	} while (uVar6 < 8);
	if (!bVar1) {
		iVar3 = edStrCopy(field_0x0[uVar8].drivePath, param_2);
		count_0x1224 = count_0x1224 + 1;
		*(int*)(field_0x0[0].drivePath + uVar8 * 4 + -0x20) = count_0x1224;
	}
	return iVar3;
}

int edCFilePathManager::get_default_filer(char* param_2)
{
	int length;

	length = edStrCopy(param_2, rootDrivePath);
	return length;
}

int* FUN_002617d0(char* param_1, byte* param_2)
{
	// Todo
	return 0;
}

void edFileSetPath(char* mode)
{
	bool bVar1;
	char* pcVar2;
	edCFiler* peVar3;
	char cVar4;
	char* pRootDrivePathChar;
	char local_200[512];
	char weirdChange;

	peVar3 = edFileGetFiler(local_200, mode, 1);
	if (peVar3 != (edCFiler*)0x0) {
		edFilePathManager.set_path(local_200);
		pcVar2 = local_200;
		pRootDrivePathChar = edFilePathManager.rootDrivePath;
		weirdChange = local_200[0];
		while (pcVar2 = pcVar2 + 1, weirdChange != '\0') {
			bVar1 = false;
			if (('`' < weirdChange) && (weirdChange < '{')) {
				bVar1 = true;
			}
			cVar4 = weirdChange;
			if (bVar1) {
				cVar4 = weirdChange + -0x20;
			}
			*pRootDrivePathChar = cVar4;
			pRootDrivePathChar = pRootDrivePathChar + 1;
			if (weirdChange == '>') break;
			weirdChange = *pcVar2;
		}
		*pRootDrivePathChar = '\0';
		peVar3->set_default_unit(local_200);
	}
	return;
}

void edFilePathSplit(char* param_1, char* param_2, char* param_3, char* param_4, char* param_5)
{
	char cVar1;
	char* pcVar2;

	if ((*param_5 != '\0') && (*param_5 == '<')) {
		do {
			cVar1 = *param_5;
			if (param_1 != (char*)0x0) {
				*param_1 = cVar1;
				param_1 = param_1 + 1;
			}
		} while ((cVar1 != '\0') && (param_5 = param_5 + 1, cVar1 != '>'));
	}
	if (param_1 != (char*)0x0) {
		*param_1 = '\0';
	}
	if ((*param_5 != '\0') && (pcVar2 = edStrChr(param_5, ':'), pcVar2 != (char*)0x0)) {
		do {
			cVar1 = *param_5;
			if (param_2 != (char*)0x0) {
				*param_2 = cVar1;
				param_2 = param_2 + 1;
			}
		} while ((cVar1 != '\0') && (param_5 = param_5 + 1, cVar1 != ':'));
	}
	if (param_2 != (char*)0x0) {
		*param_2 = '\0';
	}
	if ((*param_5 != '\0') && (pcVar2 = edStrChr(param_5, '\\'), pcVar2 != (char*)0x0)) {
		pcVar2 = edStrReturnEndPtr(param_5);
		cVar1 = *pcVar2;
		while (cVar1 != '\\') {
			pcVar2 = pcVar2 + -1;
			cVar1 = *pcVar2;
		}
		while ((param_5 != pcVar2 + 1 && (cVar1 = *param_5, cVar1 != '\0'))) {
			if (param_3 != (char*)0x0) {
				*param_3 = cVar1;
				param_3 = param_3 + 1;
			}
			if (cVar1 != '\0') {
				param_5 = param_5 + 1;
			}
		}
	}
	if (param_3 != (char*)0x0) {
		*param_3 = '\0';
	}
	cVar1 = *param_5;
	if (cVar1 != '\0') {
		while (cVar1 != '\0') {
			if (param_4 != (char*)0x0) {
				*param_4 = cVar1;
				param_4 = param_4 + 1;
			}
			if (cVar1 != '\0') {
				param_5 = param_5 + 1;
			}
			cVar1 = *param_5;
		}
	}
	if (param_4 != (char*)0x0) {
		*param_4 = '\0';
	}
	return;
}

char* edFilePathGetFilePath(char* inString)
{
	char* ret;

	/* Takes a string and finds the first colon, then returns the address of the first character after the found colon */
	ret = edStrChr(inString, ':');
	if (ret != (char*)0x0) {
		ret = ret + 1;
	}
	return ret;
}
