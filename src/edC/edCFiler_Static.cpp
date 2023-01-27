#include "edCFiler_Static.h"

#include <string.h>

edCFiler_Static g_edCFiler_Static_0046dea0;

char* sz_PC_DrivePath_00431468 = "<PC>";

edCFiler_Static::edCFiler_Static()
{
	bool bVar1;
	char cVar2;
	char cVar3;
	char* pcVar4;
	char* pcVar5;

	g_edCFiler_Static_0046dea0.field_0x0[0].field_0x0 = 0;
	g_edCFiler_Static_0046dea0.field_0x0[0].drivePath[0] = '\0';
	pcVar4 = g_edCFiler_Static_0046dea0.rootDrivePath;
	g_edCFiler_Static_0046dea0.count_0x1224 = 1;
	g_edCFiler_Static_0046dea0.field_0x0[0].field_0x4 = 0;
	g_edCFiler_Static_0046dea0.field_0x0[1].drivePath[0] = '\0';
	g_edCFiler_Static_0046dea0.field_0x0[0].field_0x8 = 0;
	g_edCFiler_Static_0046dea0.field_0x0[2].drivePath[0] = '\0';
	g_edCFiler_Static_0046dea0.field_0x0[0].field_0xc = 0;
	g_edCFiler_Static_0046dea0.field_0x0[3].drivePath[0] = '\0';
	g_edCFiler_Static_0046dea0.field_0x0[0].field_0x10 = 0;
	g_edCFiler_Static_0046dea0.field_0x0[4].drivePath[0] = '\0';
	g_edCFiler_Static_0046dea0.field_0x0[0].field_0x14 = 0;
	g_edCFiler_Static_0046dea0.field_0x0[5].drivePath[0] = '\0';
	g_edCFiler_Static_0046dea0.field_0x0[0].field_0x18 = 0;
	g_edCFiler_Static_0046dea0.field_0x0[6].drivePath[0] = '\0';
	g_edCFiler_Static_0046dea0.field_0x0[0].field_0x1c = 0;
	g_edCFiler_Static_0046dea0.field_0x0[7].drivePath[0] = '\0';
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
	//SetupEd10_00217720((undefined*)&g_edCFiler_Static_0046dea0, Destructor_00261780, &EdFileGlobal_10_0046dac8);
	return;
}


void edCFiler_Static::InitDriveType_00261530(char* param_2)
{
	char* pcVar1;
	int iVar2;
	edCFiler_Static* peVar3;
	uint objIndex;

	pcVar1 = SearchString(param_2, ':');
	objIndex = 0;
	peVar3 = this;
	do {
		iVar2 = memcmp(peVar3->field_0x0[0].drivePath, param_2, (long)(int)(pcVar1 + (1 - (int)param_2)));
		if (iVar2 == 0) {
			count_0x1224 = count_0x1224 + 1;
			*(int*)(field_0x0[0].drivePath + objIndex * 4 + -0x20) = count_0x1224;
			edStringCpyL(param_2, field_0x0[objIndex].drivePath);
			return;
		}
		objIndex = objIndex + 1;
		peVar3 = (edCFiler_Static*)(peVar3->field_0x0 + 1);
	} while (objIndex < 8);
	pcVar1[1] = '\\';
	pcVar1[2] = '\0';
	return;
}

int edCFiler_Static::CopyDrivePath(char* param_2)
{
	int length;

	length = edStringCpyL(param_2, rootDrivePath);
	return length;


}

int edCFiler_Static::SetDefaultFileLoad_00261610(char* param_2)
{
	bool bVar1;
	char* pcVar2;
	int iVar3;
	edCFiler_Static* peVar4;
	edCFiler_Static* peVar5;
	uint uVar6;
	uint uVar7;
	uint uVar8;

	pcVar2 = SearchString(param_2, ':');
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
		peVar5 = (edCFiler_Static*)(peVar5->field_0x0 + 1);
		peVar4 = (edCFiler_Static*)&peVar4->field_0x0[0].field_0x4;
		uVar8 = uVar7;
	} while (uVar6 < 8);
	if (!bVar1) {
		iVar3 = edStringCpyL(field_0x0[uVar8].drivePath, param_2);
		count_0x1224 = count_0x1224 + 1;
		*(int*)(field_0x0[0].drivePath + uVar8 * 4 + -0x20) = count_0x1224;
	}
	return iVar3;
}
