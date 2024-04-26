#include "edFileFiler.h"
#include "edFile.h"
#include "edStr.h"
#include "edFilePath.h"

edCFilerList edFilerList;

bool edCFiler::getfreespace(/*missing args*/)
{
	return false;
}

uint edCFiler::getalignedsize(uint inSize)
{
	return inSize;
}

bool edCFiler::get_physical_filename(char* outFilePath, char* pathBuff)
{
	return false;
}

bool edCFiler::configure(uint)
{
	return false;
}

bool edCFiler::unmount_unit(char* filePath)
{
	return false;
}

bool edCFiler::mount_unit(char* filePath, char* bankPath)
{
	return false;
}

bool edCFiler::format(/*missing args*/)
{
	return false;
}

bool edCFiler::setattr(/*missing args*/)
{
	return false;
}

bool edCFiler::mkdir(/*missing args*/)
{
	return false;
}

bool edCFiler::cmdbreak(/*missing args*/)
{
	return false;
}

bool edCFiler::isnowaitcmdend(edCFiler_28_Internal* pEdFilerInternal)
{
	return false;
}

bool edCFiler::waitcmdend(/*missing args*/)
{
	return false;
}

bool edCFiler::erase(/*missing args*/)
{
	return false;
}

bool edCFiler::seek(edFILEH* pDebugBank)
{
	return false;
}

bool edCFiler::write(/*missing args*/)
{
	return false;
}

uint edCFiler::read(edFILEH* pDebugBank, char* destination, uint requiredSize)
{
	return false;
}

bool edCFiler::close(edFILEH* pDebugBank)
{
	return false;
}

bool edCFiler::create(/*missing args*/)
{
	return false;
}

bool edCFiler::open(edFILEH* outFile, char* unformatedFilePath)
{
	return false;
}

bool edCFiler::findfile()
{
	return false;
}

int edCFiler::get_default_unit(char* outString)
{
	int default_unit;

	default_unit = edStrCopy(outString, this->filePath);
	return default_unit;
}

void edCFiler::set_default_unit(char* szDriveLetter)
{
	edFilePathSplit((char*)0x0, filePath, (char*)0x0, (char*)0x0, szDriveLetter);
	return;
}

bool edCFiler::configure(char* path, ETableOfContentsInitMode mode, edFILE_PARAMETER* param_4)
{
	return false;
}

edCFiler_28 edCFiler_28_ARRAY_004697a0[24] = { 0 };

edCFiler_28* edCFiler::GetGlobalC_0x1c()
{
	return edCFiler_28_ARRAY_004697a0;
}

bool edCFiler::terminate()
{
	return false;
}

bool edCFiler::initialize()
{
	return false;
}

edCFiler::~edCFiler()
{
	this->pDriveName_0x0 = (char*)0x0;
	this->pPrevEd = (edCFiler*)0x0;
	this->pNextEd = (edCFiler*)0x0;
	this->field_0x4 = 0;
	filePath[0] = '\0';
}

edCFiler::edCFiler()
{
	this->pDriveName_0x0 = (char*)0x0;
	this->pPrevEd = (edCFiler*)0x0;
	this->pNextEd = (edCFiler*)0x0;
	this->field_0x4 = 0;
	filePath[0] = '\0';
	return;
}

edCFiler* edCFilerList::get_root()
{
	return this->pRoot;
}

void edCFilerList::remove_filer(edCFiler* pFiler)
{
	edCFiler* pCurRoot;

	if (pFiler != (edCFiler*)0x0) {
		for (pCurRoot = this->pRoot; (pCurRoot != (edCFiler*)0x0 && (pCurRoot != pFiler)); pCurRoot = pCurRoot->pNextEd) {
		}
		if (pCurRoot->pPrevEd != (edCFiler*)0x0) {
			pCurRoot->pPrevEd->pNextEd = pCurRoot->pNextEd;
		}
		if (pCurRoot->pNextEd != (edCFiler*)0x0) {
			pCurRoot->pNextEd->pPrevEd = pCurRoot->pPrevEd;
		}
		if (pCurRoot == this->pRoot) {
			if (pCurRoot->pPrevEd == (edCFiler*)0x0) {
				if (pCurRoot->pNextEd == (edCFiler*)0x0) {
					this->pRoot = (edCFiler*)0x0;
				}
				else {
					this->pRoot = pCurRoot->pNextEd;
				}
			}
			else {
				this->pRoot = pCurRoot->pPrevEd;
			}
		}
	}
	return;
}

void edCFilerList::add_filer(edCFiler* pFiler)
{
	edCFiler* peVar1;
	edCFiler* peVar2;

	if (pFiler != (edCFiler*)0x0) {
		peVar1 = this->pRoot;

		if (peVar1 == (edCFiler*)0x0) {
			pFiler->pPrevEd = (edCFiler*)0x0;
			pFiler->pNextEd = (edCFiler*)0x0;
			this->pRoot = pFiler;
		}
		else {
			for (peVar2 = peVar1->pNextEd; peVar2 != (edCFiler*)0x0; peVar2 = peVar2->pNextEd) {
				peVar1 = peVar2;
			}

			pFiler->pPrevEd = peVar1;
			pFiler->pNextEd = (edCFiler*)0x0;
			peVar1->pNextEd = pFiler;
		}
	}

	return;
}

bool edFileFilerConfigure(char* path, ETableOfContentsInitMode mode, void* param_3, int* param_4)
{
	bool bVar1;
	edCFiler* peVar1;
	int iVar2;
	char acStack592[528];
	ETableOfContentsInitMode EStack64;
	long local_30;
	long local_28;

	local_28 = (long)(int)param_4;
	local_30 = (long)(int)param_3;
	EStack64 = mode;
	peVar1 = edFileGetFiler(acStack592, path, 1);
	if (peVar1 == (edCFiler*)0x0) {
		bVar1 = false;
	}
	else {
		if (true) {
			iVar2 = 0x30;
		}
		else {
			iVar2 = 0;
		}

		edFILE_PARAMETER params;
		params.field_0x0 = param_3;
		params.field_0x8 = param_4;

		bVar1 = peVar1->configure(acStack592, EStack64, &params);
	}
	return bVar1;
}

edCFiler* edFileGetFiler(char* szOutPath, char* szFilePath, long mode)
{
	char* pcVar1;
	bool bVar2;
	char* pcVar3;
	char* bufferPos;
	int length;
	//ulong lVar4;
	char cVar5;
	//long lVar6;
	char cVar7;
	char* currentCharacter;
	char* pcVar8;
	edCFiler* pFiler;
	char local_200[512];

	/* This function takes in a raw file path, and puts it in the correct format for finding the file on the disc.
	   Example:
	   In: CDEURO/frontend/kyatitle.g2d
	   Out: <CDVD>0:\CDEURO\FRONTEND\kyatitle.g2d
	   Return: 3A (58) */
	if (szFilePath == (char*)0x0) {
		szFilePath = "";
	}
	cVar7 = *szFilePath;
	if (cVar7 == '<') {
		szFilePath = szFilePath + 1;
		currentCharacter = szOutPath;
		if (true) {
			do {
				bVar2 = false;
				if (('`' < cVar7) && (cVar7 < '{')) {
					bVar2 = true;
				}
				cVar5 = cVar7;
				if (bVar2) {
					cVar5 = cVar7 + -0x20;
				}
				*currentCharacter = cVar5;
				currentCharacter = currentCharacter + 1;
				if (cVar7 == '>') break;
				cVar7 = *szFilePath;
				szFilePath = szFilePath + 1;
			} while (cVar7 != '\0');
		}
		*currentCharacter = '\0';
		pFiler = edFilerList.pRoot;
	}
	else {
		length = edFilePathManager.get_default_filer(szOutPath);
		MY_LOG("Inserted {} length {}", szOutPath, length);
		currentCharacter = szOutPath + length;
		pFiler = edFilerList.pRoot;
	}
	for (; pFiler != (edCFiler*)0x0; pFiler = pFiler->pNextEd) {
		bufferPos = pFiler->pDriveName_0x0;
		if ((bufferPos != (char*)0x0) && (length = edStrCmp(szOutPath, bufferPos), length == 0)) goto LAB_00261138;
	}
	pFiler = (edCFiler*)0x0;
LAB_00261138:
	if (pFiler == (edCFiler*)0x0) {
		*szOutPath = '\0';
		pFiler = (edCFiler*)0x0;
	}
	else {
		char nextChar = *szFilePath;
		if (nextChar == '|') {
			bufferPos = szFilePath + 1;
			if (true) {
				do {
					bVar2 = false;
					if (('`' < nextChar) && (nextChar < '{')) {
						bVar2 = true;
					}
					if (bVar2) {
						nextChar = nextChar - 0x20;
					}
					*currentCharacter = nextChar;
					nextChar = *bufferPos;
					currentCharacter = currentCharacter + 1;
					bufferPos = bufferPos + 1;
				} while ((nextChar != '|') && (nextChar != '\0'));
			}
			if (*bufferPos != '\0') {
				return (edCFiler*)0x0;
			}
		}
		else {
			bufferPos = edStrChr(szFilePath, ':');
			if (bufferPos == (char*)0x0) {
				length = pFiler->get_default_unit(currentCharacter);
				currentCharacter = currentCharacter + length;
			}
			else {
				cVar7 = *szFilePath;
				while (szFilePath = szFilePath + 1, cVar7 != '\0') {
					bVar2 = false;
					if (('`' < cVar7) && (cVar7 < '{')) {
						bVar2 = true;
					}
					cVar5 = cVar7;
					if (bVar2) {
						cVar5 = cVar7 + -0x20;
					}
					*currentCharacter = cVar5;
					currentCharacter = currentCharacter + 1;
					if (cVar7 == ':') break;
					cVar7 = *szFilePath;
				}
				*currentCharacter = '\0';
			}
			local_200[0] = '\0';
			bufferPos = (char*)0x0;
			/* Try and find the next / or \\ */
			if ((mode == 0) && ((bufferPos = edStrChr(szFilePath, '/'), bufferPos != (char*)0x0 || (pcVar3 = edStrChr(szFilePath, '\\'), bufferPos = szFilePath, pcVar3 != (char*)0x0)))) {
				/* Go back through the file and try find '/' and '\\' */
				for (bufferPos = edStrReturnEndPtr(szFilePath); (*bufferPos != '/' && (*bufferPos != '\\')); bufferPos = bufferPos + -1) {
				}
				bufferPos = bufferPos + 1;

				MY_LOG("Found filename: {}", bufferPos);

			}
			if ((*szFilePath == '/') || (*szFilePath == '\\')) {
				*currentCharacter = '\\';
				szFilePath = szFilePath + 1;
				currentCharacter = currentCharacter + 1;
			}
			else {
				edFilePathManager.get_path(szOutPath);
				currentCharacter = edStrReturnEndPtr(szOutPath);

				MY_LOG("Added default unit {}", szOutPath);
			}
			cVar7 = '\0';
			pcVar3 = local_200;

			// Convert to lowercase and forward slashes
			for (; (szFilePath != bufferPos && ((long)*szFilePath != 0)); szFilePath = szFilePath + 1) {
				char nextChar = *szFilePath;
				bVar2 = false;
				if (('`' < nextChar) && (nextChar < '{')) {
					bVar2 = true;
				}
				if (bVar2) {
					// LOWERCASE
					nextChar = nextChar - 0x20;
				}
				cVar5 = nextChar;
				if (nextChar == '/') {
					cVar5 = '\\';
				}
				if ((cVar5 != '\\') || (cVar7 != '\\')) {
					*pcVar3 = cVar5;
					pcVar3 = pcVar3 + 1;
					cVar7 = cVar5;
				}
			}

			if ((cVar7 != '\0') && (cVar7 != '\\')) {
				*pcVar3 = '\\';
				pcVar3 = pcVar3 + 1;
			}
			*pcVar3 = '\0';
			pcVar3 = local_200;
			while (local_200[0] != '\0') {
				if (local_200[0] == '.') {
					if (pcVar3[1] == '\\') {
						pcVar3 = pcVar3 + 2;
					}
					else {
						if ((pcVar3[1] == '.') && (pcVar3 = pcVar3 + 3, pcVar8 = currentCharacter, currentCharacter[-2] != ':')) {
							do {
								currentCharacter = pcVar8 + -1;
								pcVar1 = pcVar8 + -2;
								pcVar8 = currentCharacter;
							} while (*pcVar1 != '\\');
						}
					}
				}
				else {
					do {
						cVar7 = *pcVar3;
						*currentCharacter = cVar7;
						pcVar3 = pcVar3 + 1;
						currentCharacter = currentCharacter + 1;
						if (cVar7 == '\0') break;
					} while (cVar7 != '\\');
				}
				local_200[0] = *pcVar3;
			}
			if (bufferPos != (char*)0x0) {
				cVar7 = *bufferPos;
				while (cVar7 != '\0') {
					bufferPos = bufferPos + 1;
					*currentCharacter = cVar7;
					currentCharacter = currentCharacter + 1;
					cVar7 = *bufferPos;
				}
			}
		}
		*currentCharacter = '\0';
	}
	return pFiler;
}

void edFileGetFiler(edCFiler* pFiler)
{
	bool bVar1;
	edCFiler* peVar2;
	edCFiler_28* peVar3;

	do {
		bVar1 = true;
		for (peVar2 = edFilerList.get_root(); peVar2 != (edCFiler*)0x0; peVar2 = peVar2->pNextEd) {
			peVar3 = peVar2->GetGlobalC_0x1c();
			if ((peVar3 != (edCFiler_28*)0x0) && (peVar3->freeIndexes != 0)) {
				edFileNoWaitStackCallBack(peVar3);
				if (pFiler == (edCFiler*)0x0) {
					bVar1 = false;
				}
				else {
					if (peVar2 == pFiler) {
						bVar1 = false;
					}
				}
			}
		}
	} while (!bVar1);
	return;
}