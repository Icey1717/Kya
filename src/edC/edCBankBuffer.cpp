#include "edCBankBuffer.h"
#include "edCBank.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

#if defined(PLATFORM_PS2)
#include <eekernel.h>
#endif

const char* sz_edCBankBuffer_Wait_0042fb70 = "edCBankBuffer::file_access: Wait for end of previous loading operation \n";

void ReadsBankFile(void)
{
	edCFiler* peVar1;
	edCFiler_28* peVar2;

	for (peVar1 = GetFirstEdFileHandler_00260e00(&g_edCFiler_MCPtr_00448fd8); peVar1 != (edCFiler*)0x0; peVar1 = (peVar1->baseData).pNextEd) {
		peVar2 = peVar1->GetGlobalC_0x1c();
		if (peVar2 != (edCFiler_28*)0x0) {
			ReadsBankFile(peVar2);
		}
	}
	return;
}

bool CopyBankFileAccessObjectHeader_002444a0(ReadBank_158* pReadBank, ReadBank_1C* pInBank)
{
	int iVar1;
	ReadBank_1C* pRVar2;

	iVar1 = pReadBank->loadedBanks;
	if (iVar1 != 0xc) {
		pRVar2 = pReadBank->field_0x8 + (pReadBank->field_0x4 + iVar1) % 0xc;
		pRVar2->pBankHeader_0x0 = pInBank->pBankHeader_0x0;
		pRVar2->pReadBuffer = pInBank->pReadBuffer;
		pRVar2->pDebugBankData = pInBank->pDebugBankData;
		pRVar2->pBankTypePairData_0xc = pInBank->pBankTypePairData_0xc;
		pRVar2->pObjectReference_0x10 = pInBank->pObjectReference_0x10;
		pRVar2->fileFunc_0x14 = pInBank->fileFunc_0x14;
		pRVar2->fileFlagB_0x18 = pInBank->fileFlagB_0x18;
		pReadBank->loadedBanks = pReadBank->loadedBanks + 1;
	}
	return iVar1 != 0xc;
}

uint GetFileSize_0025b330(DebugBankData_234* param_1)
{
	uint uVar1;

	uVar1 = 0;
	if ((param_1->openFlags & 6) == 0) {
		uVar1 = (param_1->field_0x10).fileSize;
	}
	return uVar1;
}

const char* sz_CannotOpenFile_0042fc60 = "edCBankBuffer: Can't open file\n";
const char* sz_CannotReplaceHeader_0042fbf0 = "edCBankBuffer::file_access: Can't replace header of bank of bank!!\n";
const char* sz_FileNotOpen_0042fc40 = "edCBankBuffer: File not open \n";

bool edCBankBuffer_file_access(edCBank* pBankHeader, edCBankBuffer* pBankFileAccessObject, BankFilePathContainer* filePathPtr, long param_4, bool someFlag)
{
	edCBankFileHeader* bankBufferObj;
	uint* puVar1;
	DebugBankData_234* pDebugBank;
	uint uVar2;
	edCBankBuffer* peVar3;
	ulong uVar4;
	uint uVar5;
	char* pReadBuffer;
	uint unaff_s7_lo;
	ReadBank_1C local_20;
	int fileBufferData;

	if (pBankFileAccessObject->accessFlag != 0) {
		/* edCBankBuffer::file_access: Wait for end of previous loading operation \n */
		PrintString(sz_edCBankBuffer_Wait_0042fb70);
		fileBufferData = pBankFileAccessObject->accessFlag;
		while (fileBufferData != 0) {
			ReadsBankFile();
			fileBufferData = pBankFileAccessObject->accessFlag;
		}
	}
	if (someFlag == false) {
		uVar2 = 1;
		if (((pBankHeader->fileFlagA & 4U) != 0) || ((filePathPtr->fileFlagA & 4U) != 0)) {
			uVar2 = 9;
		}
		pDebugBank = LoadFile(filePathPtr->filePath, uVar2 | 1);
		if (pDebugBank == (DebugBankData_234*)0x0) {
			/* edCBankBuffer: Can't open file\n */
			PrintString(sz_CannotOpenFile_0042fc60);
			return false;
		}
		uVar2 = GetFileSize_0025b330(pDebugBank);
	}
	else {
		if (pBankFileAccessObject->headerComponent != 0) {
			/* edCBankBuffer::file_access: Can't replace header of bank of bank!!\n */
			PrintString(sz_CannotReplaceHeader_0042fbf0);
			return false;
		}
		assert(false);
		/* Dereference the input pointer to get the actual file path */
		//uVar4 = SEXT48((int)filePathPtr->filePath);
		//bankBufferObj = pBankHeader->pBankBuffer->fileBuffer;
		//if (((ulong)(long)(int)bankBufferObj->field_0x20 <= uVar4) && (uVar4 = GetIndexFromFileHeader(bankBufferObj, filePathPtr->filePath), uVar4 == 0xffffffffffffffff)) {
		//	return false;
		//}
		//puVar1 = (uint*)edCBankFileHeader_FindFileDataInHeader(bankBufferObj, (int)uVar4);
		//uVar2 = puVar1[1];
		//unaff_s7_lo = *puVar1;
		//pDebugBank = pBankHeader->pBankBuffer->pLoadedData;
		//if (pDebugBank == (DebugBankData_234*)0x0) {
		//	/* edCBankBuffer: File not open \n */
		//	PrintString(sz_FileNotOpen_0042fc40);
		//	return false;
		//}
	}
	pBankFileAccessObject->pObjectReference = filePathPtr->pObjectReference;
	if ((param_4 == 0) || ((long)(int)pBankFileAccessObject->fileBuffer == 0)) {
		if ((uint)(pBankHeader->size - pBankHeader->flagB) < uVar2) {
			/* // edCBankBuffer: Not enough space in the bank buffer to add a bank \n */
			PrintString("// edCBankBuffer: Not enough space in the bank buffer to add a bank \n");
			return false;
		}
		uVar5 = pBankHeader->createFlagA * 0x24;
		if ((uVar5 & 0x7ff) == 0) {
			peVar3 = pBankHeader->pBankBuffer;
		}
		else {
			uVar5 = ((uVar5 >> 0xb) + 1) * 0x800;
			peVar3 = pBankHeader->pBankBuffer;
		}
		pReadBuffer = (char*)((char*)&peVar3->header + pBankHeader->flagB + uVar5);
		pBankFileAccessObject->field_0x4 = (int*)pReadBuffer;
		pBankFileAccessObject->fileBuffer = (char*)(pReadBuffer + 8);
		pBankFileAccessObject->pLoadedData = (DebugBankData_234*)0x0;
		pBankFileAccessObject->field_0x14 = uVar2;
	}
	else {
		assert(false);
		//edCBankFileHeader_get_entry_typepair((long)(int)pBankFileAccessObject->fileBuffer, pBankHeader->pBankTypePairData, 1, pBankFileAccessObject->pObjectReference);
		pReadBuffer = (char*)pBankFileAccessObject->field_0x4;
	}
	pBankFileAccessObject->accessFlag = 1;
	local_20.pBankTypePairData_0xc = pBankHeader->pBankTypePairData;
	local_20.pObjectReference_0x10 = filePathPtr->pObjectReference;
	local_20.fileFlagB_0x18 = filePathPtr->fileFlagA;
	local_20.pDebugBankData = pDebugBank;
	if (someFlag != false) {
		local_20.pDebugBankData = (DebugBankData_234*)0x0;
	}
	local_20.fileFunc_0x14 = filePathPtr->fileFunc;
	local_20.pBankHeader_0x0 = pBankFileAccessObject;
	local_20.pReadBuffer = pReadBuffer;
	CopyBankFileAccessObjectHeader_002444a0(&g_ReadBank_00466e60, &local_20);
	if (filePathPtr->fileFlagE == 0) {
		pBankHeader->flagB = pBankHeader->flagB + uVar2;
	}
	else {
		pBankHeader->flagB = pBankHeader->flagB + filePathPtr->fileFlagE;
	}
	if (someFlag != false) {
		DebugBankSeek(pDebugBank, unaff_s7_lo, ED_SEEK_SET);
	}
	UsedInFileLoad(pDebugBank, pReadBuffer, uVar2);
	if (someFlag == false) {
		DebugBankClose(pDebugBank);
	}
	if ((((pBankHeader->fileFlagA & 4U) == 0) && ((filePathPtr->fileFlagA & 4U) == 0)) && (pBankFileAccessObject->accessFlag != 0)) {
		/* edCBankBuffer::file_access: Wait for end of previous loading operation \n */
		PrintString(sz_edCBankBuffer_Wait_0042fb70);
		fileBufferData = pBankFileAccessObject->accessFlag;
		while (fileBufferData != 0) {
			ReadsBankFile();
			fileBufferData = pBankFileAccessObject->accessFlag;
		}
	}
	return true;
}

bool edCBankBuffer_file_access(edCBankBuffer* pBankBuffer, BankFilePathContainer* file)
{
	bool ret;
	int fileOutputStart;

	if ((file == (BankFilePathContainer*)0x0) || (pBankBuffer->header == (edCBank*)0x0)) {
		ret = false;
	}
	else {
		if (pBankBuffer->accessFlag != 0) {
			/* edCBankBuffer::file_access: Wait for end of previous loading operation \n */
			PrintString(sz_edCBankBuffer_Wait_0042fb70);
			fileOutputStart = pBankBuffer->accessFlag;
			while (fileOutputStart != 0) {
				ReadsBankFile();
				fileOutputStart = pBankBuffer->accessFlag;
			}
		}
		ret = edCBankBuffer_file_access(pBankBuffer->header, pBankBuffer, file, 0, (pBankBuffer->header->fileFlagA & 1U) != 0);
	}
	return ret;
}

int CheckFileLoadAndGetParam(edCBankBuffer* pBankBuffer)
{
	int ret;

	if (pBankBuffer->fileBuffer == (char*)0x0) {
		ret = 0;
	}
	else {
		ret = *(int*)(pBankBuffer->fileBuffer + 0x20);
	}
	return ret;
}

char* sz_edCBankFileHeader_NullObj_0042ff90 = "edCBankFileHeader::get_entry_typepair: NULL object\n";

char* edCBankFileHeader_get_entry_typepair(char* fileBuffer, int inFileIndex)
{
	bool bVar1;
	char* pcVar2;

	if (fileBuffer == (char*)0x0) {
		/* edCBankFileHeader::get_entry_typepair: NULL object\n */
		PrintString(sz_edCBankFileHeader_NullObj_0042ff90);
		pcVar2 = (char*)0x0;
	}
	else {
		if ((*(short*)(fileBuffer + 0xc) != 0) || (bVar1 = true, *(short*)(fileBuffer + 0xe) != 0)) {
			bVar1 = false;
		}
		if (bVar1) {
			pcVar2 = fileBuffer + inFileIndex * 4 + *(int*)(fileBuffer + 0x2c);
		}
		else {
			pcVar2 = fileBuffer + 0xc;
		}
	}
	return pcVar2;
}

PACK(struct BankTypePairEntryPacked {
	ushort field_0x0;
	ushort field_0x2;
});

PACK(struct BankFile_Internal {
	char header[4];
	undefined field_0x4;
	undefined field_0x5;
	undefined field_0x6;
	undefined field_0x7;
	undefined field_0x8;
	undefined field_0x9;
	undefined field_0xa;
	undefined field_0xb;
	ushort field_0xc;
	ushort field_0xe;
	int field_0x10;
	int field_0x14;
	undefined field_0x18;
	undefined field_0x19;
	undefined field_0x1a;
	undefined field_0x1b;
	undefined field_0x1c;
	undefined field_0x1d;
	undefined field_0x1e;
	undefined field_0x1f;
	uint fileCount;
	undefined field_0x24;
	undefined field_0x25;
	undefined field_0x26;
	undefined field_0x27;
	int field_0x28;
	int field_0x2c;
});

void edCBankFileHeader_get_entry_typepair(BankFile_Internal* fileBuffer, TypePairData* pTypePairData, int param_3)
{
	bool bVar1;
	char* iVar2;
	BankTypePairEntryPacked* puVar3;
	TypePairData* puVar8;
	uint uVar3;
	int iVar4;
	int iVar5;

	MY_LOG("Looking up type pair entry: %d | file count: %d\n", param_3, fileBuffer->fileCount);

	if ((pTypePairData != (TypePairData*)0x0) && (uVar3 = 0, fileBuffer->fileCount != 0)) {
		iVar5 = 0;
		iVar4 = 0;
		do {
			if (fileBuffer == (BankFile_Internal*)0x0) {
				/* edCBankFileHeader::get_entry_typepair: NULL object\n */
				PrintString(sz_edCBankFileHeader_NullObj_0042ff90);
				puVar3 = (BankTypePairEntryPacked*)0x0;
			}
			else {
				if ((fileBuffer->field_0xc != 0) || (bVar1 = true, fileBuffer->field_0xe != 0)) {
					bVar1 = false;
				}
				if (bVar1) {
					puVar3 = (BankTypePairEntryPacked*)(fileBuffer->header + iVar5 + fileBuffer->field_0x2c);
				}
				else {
					puVar3 = (BankTypePairEntryPacked*)&fileBuffer->field_0xc;
				}
			}
			if (pTypePairData->field_0x0 != 0xffffffff) {
				puVar8 = pTypePairData;
				do {
					if ((puVar8->field_0x0 == (uint)puVar3->field_0x2) && (puVar8->field_0x4 == (uint)puVar3->field_0x0))
						goto LAB_00246698;
					puVar8 = puVar8 + 1;
				} while (puVar8->field_0x0 != 0xffffffff);
			}
			puVar8 = (TypePairData*)0x0;
		LAB_00246698:
			if ((puVar8 != (TypePairData*)0x0) && (puVar8->pFunction[param_3] != NULL)) {
				iVar2 = 0;
				if (fileBuffer->field_0x28 != 0) {
					iVar2 = (char*)((char*)&fileBuffer[-1].field_0x28 + fileBuffer->field_0x28);
				}

				int off = *(int*)(iVar2 + iVar4 + 8);

				puVar8->pFunction[param_3]((char*)&fileBuffer[-1].field_0x28 + off, *(int*)(iVar2 + iVar4 + 0xc));
			}
			uVar3 = uVar3 + 1;
			iVar5 = iVar5 + 4;
			iVar4 = iVar4 + 0x10;
		} while (uVar3 < fileBuffer->fileCount);
	}
	return;
}

char* edCBankFileHeader_FindFileDataInHeader(char* pFileHeader, int inFileIndex)
{
	char* iVar1;

	if (*(int*)(pFileHeader + 0x28) == 0) {
		iVar1 = (char*)0x0;
	}
	else {
		iVar1 = pFileHeader + *(int*)(pFileHeader + 0x28) + -8;
	}
	return iVar1 + inFileIndex * 0x10 + 8;
}

char* edCBankFileHeader_GetFileBufferStartFromFileIndex(char* pFileData, int fileIndex)
{
	char* pcVar1;

	pcVar1 = (char*)0x0;
	if (*(int*)(pFileData + 0x28) != 0) {
		pcVar1 = pFileData + *(int*)(pFileData + 0x28) + -8;
	}
	return pFileData + *(int*)(pcVar1 + fileIndex * 0x10 + 8) + -8;
}

uint edCBankFileHeader_get_index(char* pFileData, int inIndex, int mode)
{
	char* pcVar1;
	uint uVar2;

	if (pFileData == (char*)0x0) {
		/* edCBankFileHeader::get_index: NULL object\n */
		PrintString("edCBankFileHeader::get_index: NULL object\n");
		uVar2 = 0;
	}
	else {
		if (mode == 0) {
			pcVar1 = (char*)0x0;
			if (*(int*)(pFileData + 0x34) != 0) {
				pcVar1 = pFileData + *(int*)(pFileData + 0x34) + -8;
			}
		}
		else {
			pcVar1 = (char*)0x0;
			if (*(int*)(pFileData + 0x38) != 0) {
				pcVar1 = pFileData + *(int*)(pFileData + 0x38) + -8;
			}
		}
		pcVar1 = pcVar1 + 8;
		if (*(uint*)(pFileData + 0x20) < 0x100) {
			uVar2 = (uint)(byte)pcVar1[inIndex];
		}
		else {
			if (*(uint*)(pFileData + 0x20) < 0x10000) {
				uVar2 = (uint) * (ushort*)(pcVar1 + inIndex * 2);
			}
			else {
				uVar2 = *(uint*)(pcVar1 + inIndex * 4);
			}
		}
	}
	return uVar2;
}

char* sz_DefaultIopPath_00466d50 = "d:\\projects\\b - witch\\resource\\build\\level\\cinematiques\\loading\\rs";

char* edCBankFileHeader_GetIopPath_00246460(char* fileBuffer, int inIndex)
{
	//char cVar1;
	//char* pcVar2;
	//undefined uStack1;
	//
	//pcVar2 = (char*)0x0;
	//if (*(int*)(fileBuffer + 0x30) != 0) {
	//	pcVar2 = fileBuffer + *(int*)(fileBuffer + 0x30) + -8;
	//}
	//PTR_DAT_00448f44 = &uStack1;
	//uStack1 = 0;
	//cVar1 = pcVar2[8];
	//pcVar2 = pcVar2 + 8;
	//DAT_00448f48 = inIndex;
	//while ((cVar1 != '\0' && (pcVar2 = FUN_002469a0(pcVar2), pcVar2 != (char*)0x0))) {
	//	cVar1 = *pcVar2;
	//}
	//if (*PTR_DAT_00448f44 == '\\') {
	//	PTR_DAT_00448f44 = PTR_DAT_00448f44 + 1;
	//}
	//CopyIopPath_00246b50(s_d:\projects\b - witch\resource\bui_00466d50, PTR_DAT_00448f44);
	return sz_DefaultIopPath_00466d50;
}


bool GetFileDataForIndex(edCBankBuffer* pBankBuffer, int inFileIndex, BankFileData* outFileData, char* outIopPath)
{
	bool bVar1;
	ushort* returnBufferPtr;
	char* headerFileDataPtr;
	uint uVar2;
	uint inIndex;

	headerFileDataPtr = pBankBuffer->fileBuffer;
	if (headerFileDataPtr == (char*)0x0) {
		bVar1 = false;
	}
	else {
		bVar1 = false;
		if ((uint)inFileIndex < *(uint*)(headerFileDataPtr + 0x20)) {
			returnBufferPtr = (ushort*)edCBankFileHeader_get_entry_typepair(headerFileDataPtr, inFileIndex);
			if (returnBufferPtr == (ushort*)0x0) {
				bVar1 = false;
			}
			else {
				outFileData->unknownA = (uint)returnBufferPtr[1];
				outFileData->unknownB = (uint)*returnBufferPtr;
				headerFileDataPtr = edCBankFileHeader_FindFileDataInHeader(pBankBuffer->fileBuffer, inFileIndex);
				if (headerFileDataPtr == (char*)0x0) {
					bVar1 = false;
				}
				else {
					outFileData->length = *(uint*)(headerFileDataPtr + 4);
					outFileData->unknownC = *(uint*)(headerFileDataPtr + 0xc);
					headerFileDataPtr = edCBankFileHeader_GetFileBufferStartFromFileIndex(pBankBuffer->fileBuffer, inFileIndex);
					outFileData->fileBufferStart = headerFileDataPtr;
					if (outIopPath != (char*)0x0) {
						*outIopPath = '\0';
						headerFileDataPtr = pBankBuffer->fileBuffer;
						inIndex = 0;
						if (*(int*)(headerFileDataPtr + 0x20) != 0) {
							do {
								uVar2 = edCBankFileHeader_get_index(headerFileDataPtr, inIndex, 0);
								if ((uVar2 == inFileIndex) && (headerFileDataPtr = edCBankFileHeader_GetIopPath_00246460(pBankBuffer->fileBuffer, inIndex), headerFileDataPtr != (char*)0x0)) {
									edStringCpyL(outIopPath, headerFileDataPtr);
									break;
								}
								headerFileDataPtr = pBankBuffer->fileBuffer;
								inIndex = inIndex + 1;
							} while (inIndex < *(uint*)(headerFileDataPtr + 0x20));
						}
					}
					bVar1 = true;
				}
			}
		}
	}
	return bVar1;
}

bool edCBankBuffer_file_access_002450e0(edCBankBuffer* pBankBuffer, BankFilePathContainer* pLoadData)
{
	int iVar1;
	bool bVar2;
	LoadBankFileFunc pLoadFunc;

	if ((pLoadData == (BankFilePathContainer*)0x0) || (pBankBuffer->header == (edCBank*)0x0)) {
		bVar2 = false;
	}
	else {
		if (pBankBuffer->accessFlag != 0) {
			/* edCBankBuffer::file_access: Wait for end of previous loading operation \n */
			PrintString(sz_edCBankBuffer_Wait_0042fb70);
			iVar1 = pBankBuffer->accessFlag;
			while (iVar1 != 0) {
				ReadsBankFile();
				iVar1 = pBankBuffer->accessFlag;
			}
		}
		pLoadFunc = pLoadData->fileFunc;
		if (pLoadFunc != (LoadBankFileFunc)0x0) {
			(*pLoadFunc)(false, pLoadData->pObjectReference);
		}
		edCBankFileHeader_get_entry_typepair((BankFile_Internal*)pBankBuffer->fileBuffer, pBankBuffer->header->pBankTypePairData, 0);
		if (pLoadFunc != (LoadBankFileFunc)0x0) {
			(*pLoadFunc)(true, pLoadData->pObjectReference);
		}
		//EmptyFunction();
		bVar2 = true;
	}
	return bVar2;
}

char* sz_CloseBankBufferError_0042fbc0 = "Can't close header of bank of bank!!\n";

bool edCBankBuffer_close(edCBankBuffer* pBankBuffer)
{
	int iVar1;
	edCBank* peVar2;
	char* fileBuffer;
	bool bVar3;

	if (pBankBuffer->header == (edCBank*)0x0) {
		bVar3 = false;
	}
	else {
		if (pBankBuffer->accessFlag != 0) {
			/* edCBankBuffer::file_access: Wait for end of previous loading operation \n */
			PrintString(sz_edCBankBuffer_Wait_0042fb70);
			iVar1 = pBankBuffer->accessFlag;
			while (iVar1 != 0) {
				ReadsBankFile();
				iVar1 = pBankBuffer->accessFlag;
			}
		}
		peVar2 = pBankBuffer->header;
		if (pBankBuffer->accessFlag != 0) {
			/* edCBankBuffer::file_access: Wait for end of previous loading operation \n */
			PrintString(sz_edCBankBuffer_Wait_0042fb70);
			iVar1 = pBankBuffer->accessFlag;
			while (iVar1 != 0) {
				ReadsBankFile();
				iVar1 = pBankBuffer->accessFlag;
			}
		}
		if (pBankBuffer->headerComponent == 0) {
			fileBuffer = pBankBuffer->fileBuffer;
			peVar2->flagB = peVar2->flagB - *(int*)(fileBuffer + 0x10);
			peVar2->flagC = peVar2->flagC + -1;
			edCBankFileHeader_get_entry_typepair((BankFile_Internal*)fileBuffer, peVar2->pBankTypePairData, 5);
			memset(pBankBuffer, 0, sizeof(edCBankBuffer));
			bVar3 = true;
		}
		else {
			/* Can't close header of bank of bank!!\n */
			PrintString(sz_CloseBankBufferError_0042fbc0);
			bVar3 = false;
		}
	}
	return bVar3;
}

int FormatFileName(char* outFileName, char* inFileName)
{
	char* pcVar1;
	char* pcVar2;
	char* finalBufferPos;
	char currentCharacter;

	/* This loads in the format extension/name
	   Example: arrow_pressed.g2d => g2d\arrow_pressed */
	finalBufferPos = outFileName;
	pcVar1 = inFileName;
	while (currentCharacter = *inFileName, currentCharacter != '\0') {
		if (currentCharacter == '\\') {
			inFileName = inFileName + 1;
			for (; pcVar1 < inFileName; pcVar1 = pcVar1 + 1) {
				*finalBufferPos = *pcVar1;
				finalBufferPos = finalBufferPos + 1;
			}
		}
		else {
			if (currentCharacter == '.') {
				for (; (currentCharacter = *inFileName, currentCharacter != '\0' &&
					(currentCharacter != '\\')); inFileName = inFileName + 1) {
					*finalBufferPos = currentCharacter;
					finalBufferPos = finalBufferPos + 1;
				}
				*finalBufferPos = '\\';
				currentCharacter = *pcVar1;
				pcVar2 = pcVar1;
				while (finalBufferPos = finalBufferPos + 1, pcVar1 = inFileName, currentCharacter != '.') {
					*finalBufferPos = currentCharacter;
					pcVar2 = pcVar2 + 1;
					currentCharacter = *pcVar2;
				}
			}
			else {
				inFileName = inFileName + 1;
			}
		}
	}
	for (; pcVar1 < inFileName; pcVar1 = pcVar1 + 1) {
		*finalBufferPos = *pcVar1;
		finalBufferPos = finalBufferPos + 1;
	}
	*finalBufferPos = '\0';
	return (int)(finalBufferPos + -(long long)outFileName);
}

char* FindHeaderIndex(char* headerBaseFilePath, int* outIndex, char* fileName, char* searchBuffer)
{
	bool bVar1;
	char cVar2;
	byte bVar3;
	long lVar4;
	char* basePathChar;
	uint uVar5;
	int iVar6;
	long lVar7;
	uint uVar8;

	/* This will recursively search for the input file in the header folder
	   structure */
	cVar2 = *headerBaseFilePath;
	lVar4 = (long)cVar2;
	basePathChar = headerBaseFilePath + 1;
	if (lVar4 < 0) {
		lVar4 = (long)-(int)cVar2 << 0x38;
		lVar7 = lVar4 >> 0x38;
		if (lVar7 == 0x80) {
			lVar7 = (long)((int)(lVar4 >> 0x38) + (int)*basePathChar);
			basePathChar = headerBaseFilePath + 2;
		}
		uVar8 = 0;
		uVar5 = 0;
		do {
			bVar3 = *basePathChar;
			basePathChar = (char*)((byte*)basePathChar + 1);
			uVar8 = uVar8 | ((int)(char)bVar3 & 0x7fU) << (uVar5 & 0x1f);
			uVar5 = uVar5 + 7;
		} while ((char)bVar3 < '\0');
		iVar6 = (int)lVar7;
		if (lVar7 != 0) {
			do {
				iVar6 = iVar6 + -1;
				*searchBuffer = *basePathChar;
				basePathChar = basePathChar + 1;
				searchBuffer = searchBuffer + 1;
			} while (iVar6 != 0);
		}
		*searchBuffer = '\\';
		do {
			if (uVar8 == 0) {
				return basePathChar;
			}
			uVar8 = uVar8 - 1;
			basePathChar = FindHeaderIndex(basePathChar, outIndex, fileName, searchBuffer + 1);
		} while (basePathChar != (char*)0x0);
		basePathChar = (char*)0x0;
	}
	else {
		if (lVar4 == 0x7f) {
			lVar4 = (long)((int)cVar2 + (int)*basePathChar);
			basePathChar = headerBaseFilePath + 2;
		}
		while (bVar1 = lVar4 != 0, lVar4 = (long)((int)lVar4 + -1), bVar1) {
			*searchBuffer = *basePathChar;
			basePathChar = (char*)((byte*)basePathChar + 1);
			searchBuffer = (char*)((byte*)searchBuffer + 1);
		}
		do {
			fileName = (char*)((byte*)fileName + -1);
			searchBuffer = (char*)((byte*)searchBuffer + -1);
			if (*fileName == 0) {
				return (char*)0x0;
			}
		} while ((*searchBuffer != 0) && ((*fileName & 0xdfU) == (*searchBuffer & 0xdfU)));
		*outIndex = *outIndex + 1;
	}
	return basePathChar;
}

int GetIndexFromFileHeader(edCBankFileHeader* bankBufferObj, char* inFileName)
{
	char* pHeaderBase;
	int fileNameLength;
	char* headerBasePath;
	uint uVar2;
	int inIndex;
	char formattedFilename[271];
	char fullHeaderFilePath[267];
	int outIndex;
	char cVar1;

	MY_LOG("GetIndexFromFileHeader %s\n", inFileName);

	if (bankBufferObj->usedInGetIndex == 0) {
		pHeaderBase = (char*)0x0;
	}
	else {
		pHeaderBase = (char*)((char*)bankBufferObj + bankBufferObj->usedInGetIndex + -8);
	}
	headerBasePath = pHeaderBase + 8;
	/* Formats file in the format extension / file name */
	fileNameLength = FormatFileName(formattedFilename, inFileName);
	outIndex = 0;
	cVar1 = *headerBasePath;
	while (cVar1 != '\0') {
		/* Search through the header object and find which index the input file name is
			*/
		headerBasePath =
			FindHeaderIndex(headerBasePath, &outIndex, formattedFilename + fileNameLength,
				fullHeaderFilePath);
		/* This will be true if we reached all the way through the path we were tryng to
		   search */
		inIndex = outIndex;
		if (headerBasePath == (char*)0x0) goto LAB_00246280;
		cVar1 = *headerBasePath;
	}
	inIndex = -1;
LAB_00246280:
	uVar2 = 0xffffffff;
	if (-1 < inIndex) {
		/* Ensure we found the right index? */
		uVar2 = edCBankFileHeader_get_index((char*)bankBufferObj, inIndex, 0);
	}
	return (int)uVar2;
}

int GetIndexForFileName(edCBankBuffer* headerObj, char* inFileName)
{
	int iVar1;

	iVar1 = 0;
	if ((edCBankFileHeader*)headerObj->fileBuffer != (edCBankFileHeader*)0x0) {
		iVar1 = GetIndexFromFileHeader((edCBankFileHeader*)headerObj->fileBuffer, inFileName);
	}
	return iVar1;
}

char* GetFilePointerFromFileIndex(edCBankBuffer* bankObj, int fileIndex)
{
	return edCBankFileHeader_GetFileBufferStartFromFileIndex(bankObj->fileBuffer, fileIndex);
}

void edCBankBuffer_file_access(edCBankBuffer* pBankAccessObject)
{
	int iVar1;

	if (pBankAccessObject->accessFlag != 0) {
		/* edCBankBuffer::file_access: Wait for end of previous loading operation \n */
		PrintString(sz_edCBankBuffer_Wait_0042fb70);
		iVar1 = pBankAccessObject->accessFlag;
		while (iVar1 != 0) {
			ReadsBankFile();
			iVar1 = pBankAccessObject->accessFlag;
		}
	}
	return;
}

bool edCBankBuffer_CheckAccessFlag(edCBankBuffer* bankAccessObj)
{
	return bankAccessObj->accessFlag == 0;
}