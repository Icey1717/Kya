#include "FrontEndBank.h"
#include "edStr.h"
#include "edBank/edBankFile.h"

char* FRONTEND_BANK_NAME = "Frontend";

#ifdef PLATFORM_WIN
static std::string gDebugTexNames[4];
#endif

// This one is unused, instead the ones initialized in CScene constructor is used.
CFrontendBank gFrontendBank;

CFrontendBank::CFrontendBank()
{
	this->nbTextures = 0;
	this->nbMeshes = 0;

	memset(&this->texture, 0, sizeof(FrontendTextureList));
	memset(&this->mesh, 0, sizeof(FrontendMeshList));

	return;
}

bool BnkFrontendInstallG3D(char* pData, int size)
{
	CFrontendBank* pFrontendBank = CScene::ptable.g_FrontEndBank_00451674;

	pFrontendBank->mesh.aEntries[pFrontendBank->nbMeshes].pData = pData;
	pFrontendBank->mesh.aEntries[pFrontendBank->nbMeshes].size = size;

	pFrontendBank->nbMeshes = pFrontendBank->nbMeshes + 1;

	return false;
}

bool BnkFrontendInstallG2D(char* pData, int size)
{
	CFrontendBank* pFrontendBank = CScene::ptable.g_FrontEndBank_00451674;

	pFrontendBank->texture.aEntries[pFrontendBank->nbTextures].pData = pData;
	pFrontendBank->texture.aEntries[pFrontendBank->nbTextures].size = size;

#ifdef PLATFORM_WIN
	gDebugTexNames[pFrontendBank->nbTextures] = ObjectNaming::CopyObjectName();
#endif

	pFrontendBank->nbTextures = pFrontendBank->nbTextures + 1;

	return false;
}

edCBankCallback TableFrontendBankCallback[3] = 
{
	{ BANK_CALLBACK_MESH, 0x1, BnkFrontendInstallG3D, 0, 0, 0, 0, 0 },
	{ BANK_CALLBACK_TEXTURE, 0x1, BnkFrontendInstallG2D, 0, 0, 0, 0, 0 },
	{ -1, -1, 0x0, 0, 0, 0, 0, 0 }
};

void CFrontendBank::Game_Init()
{
	FrontendTextureListEntry* pCurEntry;
	int curIndex;
	edCBankInstall bankInstallB;
	char filePath[128];
	edCBankInstall bankInstallA;

	int iStack8;
	int iStack4;

	memset(&bankInstallA, 0, sizeof(edCBankInstall));
	this->bankBuffer.initialize(0x64000, 1, &bankInstallA);
	this->bankBuffer.bank_buffer_setcb(TableFrontendBankCallback);

	memset(&bankInstallB, 0, sizeof(edCBankInstall));
	edStrCatMulti(filePath, "CDEURO/Frontend/", FRONTEND_BANK_NAME, ".bnk", NULL);
	bankInstallB.filePath = filePath;

	this->pBankBufferEntry = this->bankBuffer.get_free_entry();
	this->pBankBufferEntry->load(&bankInstallB);

	curIndex = 0;
	pCurEntry = this->texture.aEntries;
	if (0 < this->nbTextures) {
		do {
			NAME_NEXT_OBJECT(gDebugTexNames[curIndex].c_str());
			ed3DInstallG2D(pCurEntry->pData, pCurEntry->size, &iStack4, &pCurEntry->manager, 1);
			curIndex = curIndex + 1;
			pCurEntry = pCurEntry + 1;
		} while (curIndex < this->nbTextures);
	}

	NAME_NEXT_OBJECT("CDEURO/Frontend/BANK_FILE_3D");
	ed3DInstallG3D((this->mesh).aEntries[0].pData, (this->mesh).aEntries[0].size, 0, &iStack8, &(this->texture).aEntries[3].manager, 0xc, &(this->mesh).aEntries[0].manager);

	return;
}

void CFrontendBank::Game_Term()
{
	int curIndex;

	curIndex = 0;
	FrontendTextureListEntry* pCurTextureEntry = this->texture.aEntries;
	if (0 < this->nbTextures) {
		do {
			ed3DUnInstallG2D(&pCurTextureEntry->manager);
			curIndex = curIndex + 1;
			pCurTextureEntry = pCurTextureEntry + 1;
		} while (curIndex < this->nbTextures);
	}

	curIndex = 0;
	FrontendMeshListEntry* pCurMeshEntry = this->mesh.aEntries;
	if (0 < this->nbMeshes) {
		do {
			ed3DUnInstallG3D(&pCurMeshEntry->manager);
			curIndex = curIndex + 1;
			pCurMeshEntry = pCurMeshEntry + 1;
		} while (curIndex < this->nbMeshes);
	}

	this->pBankBufferEntry->close();
	this->pBankBufferEntry = (edCBankBufferEntry*)0x0;

	this->bankBuffer.terminate();

	this->nbTextures = 0;
	this->nbMeshes = 0;

	memset(&this->texture, 0, sizeof(FrontendTextureList));
	memset(&this->mesh, 0, sizeof(FrontendMeshList));

	return;
}

void CFrontendBank::Level_Init()
{
	return;
}

char* CFrontendBank::GetResource(char* pFileName, edBANK_ENTRY_INFO* pEntryInfo)
{
	int fileIndex;
	char* pData;

	fileIndex = this->pBankBufferEntry->get_index(pFileName);
	if (fileIndex == -1) {
		edDebugPrintf("\r\nFile: %s\r\n", pFileName);
		pData = (char*)0x0;
	}
	else {
		pData = (char*)0x0;

		if (pEntryInfo == (edBANK_ENTRY_INFO*)0x0) {
			pData = this->pBankBufferEntry->get_element(fileIndex);
		}
		else {
			if (this->pBankBufferEntry->get_info(fileIndex, pEntryInfo, (char*)0x0) != false) {
				pData = pEntryInfo->fileBufferStart;
			}
		}
	}

	return pData;
}

ed_g2d_manager* CFrontendBank::GetG2D(int index)
{
	return &this->texture.aEntries[index].manager;
}

void CFrontendBank::Level_Term()
{
	return;
}

// Should be in: D:/Projects/b-witch/FrontendBank.h
void CFrontendBank::Level_ClearAll()
{
	return;
}

// Should be in: D:/Projects/b-witch/FrontendBank.h
void CFrontendBank::Level_Manage()
{
	return;
}

// Should be in: D:/Projects/b-witch/FrontendBank.h
void CFrontendBank::Level_ManagePaused()
{
	return;
}