#include "FrontEndBank.h"
#include "edStr.h"

CFrontendBank gFrontendBank;

CFrontendBank::CFrontendBank()
{
	this->field_0x44 = 0;

	memset(&this->texture, 0, sizeof(CFrontendBankSubObjA));
	memset(&this->mesh, 0, sizeof(CFrontendBankSubObjB));

	return;
}

void CFrontendBank::Game_Init()
{
	CFrontendBank* pCVar2;
	int iVar3;
	edCBankInstall bankInstallB;
	char filePath[128];
	edCBankInstall bankInstallA;
	int iStack8;
	int iStack4;

	memset(&bankInstallA, 0, sizeof(edCBankInstall));
	this->bankBuffer.initialize(0x64000, 1, &bankInstallA);
	this->bankBuffer.bank_buffer_setcb(&DAT_0040ed00);

	memset(&bankInstallB, 0, sizeof(edCBankInstall));
	edStrCatMulti(filePath, "CDEURO/Frontend/");
	bankInstallB.filePath = filePath;

	this->pBankBufferEntry = this->bankBuffer.get_free_entry();
	this->pBankBufferEntry->load(&bankInstallB);

	iVar3 = 0;
	pCVar2 = this;
	if (0 < (int)(this->bankBuffer).pBankFileAccessObject) {
		do {
			ed3DInstallG2D(*(char**)&pCVar2->texture, *(int*)&(pCVar2->texture).field_0x4, &iStack4,
				(ed_g2d_manager*)&(pCVar2->texture).field_0x8, 1);
			iVar3 = iVar3 + 1;
			pCVar2 = (CFrontendBank*)&(pCVar2->bankBuffer).pNextBank;
		} while (iVar3 < (int)(this->bankBuffer).pBankFileAccessObject);
	}

	ed3DInstallG3D(*(char**)&this->mesh, (this->mesh).meshFileLength_0x12c, 0, &iStack8, &(this->texture).textureInfo_0xf8,
		0xc, &(this->mesh).meshInfo_0x130);
	return;
}

void CFrontendBank::Game_Term()
{

}

void CFrontendBank::Level_Init()
{

}

void CFrontendBank::Level_Term()
{

}

void CFrontendBank::Level_ClearAll()
{

}

void CFrontendBank::Level_Manage()
{

}

void CFrontendBank::Level_ManagePaused()
{

}
