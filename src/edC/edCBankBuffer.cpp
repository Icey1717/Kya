#include "edCBankBuffer.h"
#include "edCBank.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

#if defined(PLATFORM_PS2)
#include <eekernel.h>
#endif

const char* sz_edCBankBuffer_Wait_0042fb70 = "edCBankBufferEntry::file_access: Wait for end of previous loading operation \n";

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

bool QueueBankRead_002444a0(AsyncBankReader* pReadBank, ReadBank_1C* pInBank)
{
	int iVar1;
	ReadBank_1C* pRVar2;

	iVar1 = pReadBank->loadedBanks;
	if (iVar1 != 0xc) {
		pRVar2 = pReadBank->aBankQueue + (pReadBank->currentIndex + iVar1) % 0xc;
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

uint edCBankFileHeader_get_index(BankFile_Internal* pFileData, int inIndex, int mode)
{
	uint uVar1;
	char* pcVar2;

	if (pFileData == (BankFile_Internal*)0x0) {
		/* edCBankFileHeader::get_index: NULL object\n */
		edDebugPrintf("edCBankFileHeader::get_index: NULL object\n");
		uVar1 = 0;
	}
	else {
		if (mode == 0) {
			pcVar2 = (char*)0x0;
			if (pFileData->field_0x34 != 0) {
				pcVar2 = pFileData->header + (pFileData->field_0x34 - 8);
			}
		}
		else {
			pcVar2 = (char*)0x0;
			if (pFileData->field_0x38 != 0) {
				pcVar2 = pFileData->header + pFileData->field_0x38 + -8;
			}
		}
		pcVar2 = pcVar2 + 8;
		if (pFileData->fileCount < 0x100) {
			uVar1 = (uint)(byte)pcVar2[inIndex];
		}
		else {
			if (pFileData->fileCount < 0x10000) {
				uVar1 = (uint) * (ushort*)(pcVar2 + inIndex * 2);
			}
			else {
				uVar1 = *(uint*)(pcVar2 + inIndex * 4);
			}
		}
	}
	return uVar1;
}

char* DebugFindFilePath(BankFile_Internal* pBVar3, int inFileIndex)
{
	int inIndex = 0;
	char* returnFileBufferStart = NULL;
	if (pBVar3->fileCount != 0) {
		do {
			uint uVar2 = edCBankFileHeader_get_index(pBVar3, inIndex, 0);
			if ((uVar2 == inFileIndex) &&
				(returnFileBufferStart = edCBankFileHeader_GetIopPath_00246460(pBVar3, inIndex),
					returnFileBufferStart != (char*)0x0)) {
				break;
			}
			inIndex = inIndex + 1;
		} while (inIndex < pBVar3->fileCount);
	}
	return returnFileBufferStart;
}

const char* sz_CannotOpenFile_0042fc60 = "edCBankBufferEntry: Can't open file\n";
const char* sz_CannotReplaceHeader_0042fbf0 = "edCBankBufferEntry::file_access: Can't replace header of bank of bank!!\n";
const char* sz_FileNotOpen_0042fc40 = "edCBankBufferEntry: File not open \n";

bool load(edCBank* pBankHeader, edCBankBufferEntry* pBankFileAccessObject, BankFilePathContainer* filePathPtr, long param_4, bool someFlag)
{
	BankFile_Internal* bankBufferObj;
	uint* puVar1;
	DebugBankData_234* pDebugBank;
	uint uVar2;
	edCBankBufferEntry* peVar3;
	ulong uVar4;
	uint uVar5;
	char* pReadBuffer;
	uint unaff_s7_lo;
	ReadBank_1C local_20;
	int fileBufferData;

	if (pBankFileAccessObject->accessFlag != 0) {
		/* edCBankBuffer::file_access: Wait for end of previous loading operation \n */
		edDebugPrintf(sz_edCBankBuffer_Wait_0042fb70);
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
		pDebugBank = edFileLoadSize(filePathPtr->filePath, uVar2 | 1);
		if (pDebugBank == (DebugBankData_234*)0x0) {
			/* edCBankBuffer: Can't open file\n */
			edDebugPrintf(sz_CannotOpenFile_0042fc60);
			return false;
		}
		uVar2 = GetFileSize_0025b330(pDebugBank);
	}
	else {
		if (pBankFileAccessObject->headerComponent != 0) {
			/* edCBankBuffer::file_access: Can't replace header of bank of bank!!\n */
			edDebugPrintf(sz_CannotReplaceHeader_0042fbf0);
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
		//	edDebugPrintf(sz_FileNotOpen_0042fc40);
		//	return false;
		//}
	}
	pBankFileAccessObject->pObjectReference = filePathPtr->pObjectReference;
	if ((param_4 == 0) || ((long)(int)pBankFileAccessObject->fileBuffer == 0)) {
		if ((uint)(pBankHeader->size - pBankHeader->flagB) < uVar2) {
			/* // edCBankBuffer: Not enough space in the bank buffer to add a bank \n */
			edDebugPrintf("// edCBankBufferEntry: Not enough space in the bank buffer to add a bank \n");
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
		pBankFileAccessObject->fileBuffer = (BankFile_Internal*)(pReadBuffer + 8);
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
	QueueBankRead_002444a0(&g_AsyncBankReader_00466e60, &local_20);
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
		edDebugPrintf(sz_edCBankBuffer_Wait_0042fb70);
		fileBufferData = pBankFileAccessObject->accessFlag;
		while (fileBufferData != 0) {
			ReadsBankFile();
			fileBufferData = pBankFileAccessObject->accessFlag;
		}
	}
	return true;
}

bool load(edCBankBufferEntry* pBankBuffer, BankFilePathContainer* file)
{
	bool ret;
	int fileOutputStart;

	if ((file == (BankFilePathContainer*)0x0) || (pBankBuffer->header == (edCBank*)0x0)) {
		ret = false;
	}
	else {
		if (pBankBuffer->accessFlag != 0) {
			/* edCBankBuffer::file_access: Wait for end of previous loading operation \n */
			edDebugPrintf(sz_edCBankBuffer_Wait_0042fb70);
			fileOutputStart = pBankBuffer->accessFlag;
			while (fileOutputStart != 0) {
				ReadsBankFile();
				fileOutputStart = pBankBuffer->accessFlag;
			}
		}
		ret = load(pBankBuffer->header, pBankBuffer, file, 0, (pBankBuffer->header->fileFlagA & 1U) != 0);
	}
	return ret;
}

int get_element_count(edCBankBufferEntry* pBankBuffer)
{
	int ret;

	if (pBankBuffer->fileBuffer == (BankFile_Internal*)0x0) {
		ret = 0;
	}
	else {
		ret = pBankBuffer->fileBuffer->fileCount;
	}
	return ret;
}

char* sz_edCBankFileHeader_NullObj_0042ff90 = "edCBankFileHeader::get_entry_typepair: NULL object\n";

FileTypeData* edCBankFileHeader_GetFileTypeData(BankFile_Internal* fileBuffer, int inFileIndex)
{
	bool bVar1;
	FileTypeData* pFVar2;

	if (fileBuffer == (BankFile_Internal*)0x0) {
		/* edCBankFileHeader::get_entry_typepair: NULL object\n */
		edDebugPrintf(sz_edCBankFileHeader_NullObj_0042ff90);
		pFVar2 = (FileTypeData*)0x0;
	}
	else {
		if (((fileBuffer->typeData).stype != 0) || (bVar1 = true, (fileBuffer->typeData).type != 0)) {
			bVar1 = false;
		}
		if (bVar1) {
			pFVar2 = (FileTypeData*)(fileBuffer->header + inFileIndex * 4 + fileBuffer->fileTypeDataOffset);
		}
		else {
			pFVar2 = &fileBuffer->typeData;
		}
	}
	return pFVar2;
}

void edCBankFileHeader_get_entry_typepair(BankFile_Internal* fileBuffer, TypePairData* pTypePairData, int mode)
{
	bool bVar1;
	char* pFileHeaderStart;
	FileTypeData* puVar3;
	TypePairData* puVar8;
	uint uVar3;
	int iVar4;
	int iVar5;

	MY_LOG("Looking up type pair entry: %d | file count: %d\n", mode, fileBuffer->fileCount);

	if ((pTypePairData != (TypePairData*)0x0) && (uVar3 = 0, fileBuffer->fileCount != 0)) {
		iVar5 = 0;
		iVar4 = 0;
		do {
			if (fileBuffer == (BankFile_Internal*)0x0) {
				/* edCBankFileHeader::get_entry_typepair: NULL object\n */
				edDebugPrintf(sz_edCBankFileHeader_NullObj_0042ff90);
				puVar3 = (FileTypeData*)0x0;
			}
			else {
				if (((fileBuffer->typeData).stype != 0) || (bVar1 = true, (fileBuffer->typeData).type != 0)) {
					bVar1 = false;
				}
				if (bVar1) {
					puVar3 = (FileTypeData*)(fileBuffer->header + iVar5 + fileBuffer->fileTypeDataOffset);
				}
				else {
					puVar3 = &fileBuffer->typeData;
				}
			}
			if (pTypePairData->type != 0xffffffff) {
				puVar8 = pTypePairData;
				do {
					if ((puVar8->type == (uint)puVar3->type) && (puVar8->stype == (uint)puVar3->stype))
						goto LAB_00246698;
					puVar8 = puVar8 + 1;
				} while (puVar8->type != 0xffffffff);
			}
			puVar8 = (TypePairData*)0x0;
		LAB_00246698:
			if ((puVar8 != (TypePairData*)0x0) && (puVar8->pFunction[mode] != NULL)) {
				pFileHeaderStart = 0;
				if (fileBuffer->fileHeaderDataOffset != 0) {
					pFileHeaderStart = (char*)(((char*)fileBuffer - 8) + fileBuffer->fileHeaderDataOffset);
				}

				MY_LOG("Executing for %s\n", DebugFindFilePath(fileBuffer, uVar3));

				FileHeaderFileData* pFileHeaderData = (FileHeaderFileData*)(pFileHeaderStart + iVar4 + 8);
				puVar8->pFunction[mode](fileBuffer->header + pFileHeaderData->offset - 8, pFileHeaderData->size);
			}
			uVar3 = uVar3 + 1;
			iVar5 = iVar5 + 4;
			iVar4 = iVar4 + 0x10;
		} while (uVar3 < fileBuffer->fileCount);
	}
	return;
}

FileHeaderFileData* edCBankFileHeader_FindFileDataInHeader(BankFile_Internal* pFileHeader, int fileIndex)
{
	char* pcVar1;

	if (pFileHeader->fileHeaderDataOffset == 0) {
		pcVar1 = (char*)0x0;
	}
	else {
		pcVar1 = pFileHeader->header + pFileHeader->fileHeaderDataOffset + -8;
	}
	return (FileHeaderFileData*)(pcVar1 + fileIndex * 0x10 + 8);
}

char* edCBankFileHeader_GetFileBufferStartFromFileIndex(BankFile_Internal* pFileData, int fileIndex)
{
	char* pcVar1;

	pcVar1 = (char*)0x0;
	if (pFileData->fileHeaderDataOffset != 0) {
		pcVar1 = pFileData->header + pFileData->fileHeaderDataOffset + -8;
	}
	FileHeaderFileData* pFileHeaderData = (FileHeaderFileData*)(pcVar1 + fileIndex * 0x10 + 8);
	return pFileData->header + pFileHeaderData->offset + -8;
}

char sz_DefaultIopPath_00466d50[512] = { 0 };
char* PTR_DAT_00448f44 = NULL;
int INT_00448f48 = 0;

void CopyIopPath_00246b50(char* param_1, char* param_2)
{
	char cVar1;
	char* pcVar2;
	char* pcVar3;

	pcVar3 = param_2;
	while (*param_2 != '\0') {
		if (*param_2 == '\\') {
			param_2 = param_2 + 1;
			if (*pcVar3 == '.') {
				for (; (cVar1 = *param_2, pcVar2 = pcVar3, cVar1 != '\0' && (cVar1 != '\\')); param_2 = param_2 + 1) {
					*param_1 = cVar1;
					param_1 = param_1 + 1;
				}
				for (; (cVar1 = *pcVar2, pcVar3 = param_2, cVar1 != '\0' && (cVar1 != '\\')); pcVar2 = pcVar2 + 1) {
					*param_1 = cVar1;
					param_1 = param_1 + 1;
				}
			}
			else {
				for (; pcVar3 < param_2; pcVar3 = pcVar3 + 1) {
					*param_1 = *pcVar3;
					param_1 = param_1 + 1;
				}
			}
		}
		else {
			param_2 = param_2 + 1;
		}
	}
	for (; pcVar3 < param_2; pcVar3 = pcVar3 + 1) {
		*param_1 = *pcVar3;
		param_1 = param_1 + 1;
	}
	*param_1 = '\0';
	return;
}


// Recursive function that returns a pointer to a string
// Takes a pointer to a null-terminated string as input
char* DecodeStringFromBinaryTree_002469a0(char* input)
{
	char cVar1;
	char* pcVar2;
	char* pcVar3;
	uint uVar4;
	long lVar5;
	uint uVar6;
	int iVar7;
	ulong leftShiftedValue;
	ulong rightShifteValue;

	cVar1 = *input;
	lVar5 = (long)cVar1;
	pcVar2 = input + 1;
	if (lVar5 < 0) {
		leftShiftedValue = (ulong)-(int)cVar1 << 0x38;
		rightShifteValue = leftShiftedValue >> 0x38;
		if (rightShifteValue == 0x80) {
			rightShifteValue = (long)((int)(leftShiftedValue >> 0x38) + (int)*pcVar2);
			pcVar2 = input + 2;
		}
		uVar6 = 0;
		uVar4 = 0;
		do {
			cVar1 = *pcVar2;
			pcVar2 = pcVar2 + 1;
			uVar6 = uVar6 | ((int)cVar1 & 0x7fU) << (uVar4 & 0x1f);
			uVar4 = uVar4 + 7;
		} while (cVar1 < '\0');
		iVar7 = (int)rightShifteValue;
		pcVar3 = pcVar2 + iVar7;
		do {
			if (uVar6 == 0) {
				return pcVar3;
			}
			uVar6 = uVar6 - 1;
			pcVar3 = DecodeStringFromBinaryTree_002469a0(pcVar3);
		} while (pcVar3 != (char*)0x0);
		pcVar2 = pcVar2 + iVar7;
		if (rightShifteValue != 0) {
			do {
				iVar7 = iVar7 + -1;
				pcVar2 = pcVar2 + -1;
				pcVar3 = PTR_DAT_00448f44 + -1;
				PTR_DAT_00448f44 = PTR_DAT_00448f44 + -1;
				*pcVar3 = *pcVar2;
			} while (iVar7 != 0);
		}
		pcVar2 = (char*)0x0;
		pcVar3 = PTR_DAT_00448f44 + -1;
		PTR_DAT_00448f44 = PTR_DAT_00448f44 + -1;
		*pcVar3 = '\\';
	}
	else {
		if (lVar5 == 0x7f) {
			lVar5 = (long)((int)cVar1 + (int)*pcVar2);
			pcVar2 = input + 2;
		}
		iVar7 = (int)lVar5;
		if (INT_00448f48 == 0) {
			pcVar2 = pcVar2 + iVar7;
			if (lVar5 != 0) {
				do {
					iVar7 = iVar7 + -1;
					pcVar2 = pcVar2 + -1;
					pcVar3 = PTR_DAT_00448f44 + -1;
					PTR_DAT_00448f44 = PTR_DAT_00448f44 + -1;
					*pcVar3 = *pcVar2;
				} while (iVar7 != 0);
			}
			pcVar2 = (char*)0x0;
			pcVar3 = PTR_DAT_00448f44 + -1;
			PTR_DAT_00448f44 = PTR_DAT_00448f44 + -1;
			*pcVar3 = '\\';
		}
		else {
			INT_00448f48 = INT_00448f48 + -1;
			pcVar2 = pcVar2 + iVar7;
		}
	}
	return pcVar2;
}

char* edCBankFileHeader_GetIopPath_00246460(BankFile_Internal* fileBuffer, int inIndex)
{
	char cVar1;
	char* pcVar2;
	char local_108[264];

	pcVar2 = (char*)0x0;
	if (fileBuffer->field_0x30 != 0) {
		pcVar2 = fileBuffer->header + fileBuffer->field_0x30 + -8;
	}
	PTR_DAT_00448f44 = local_108 + 0x107;
	local_108[263] = '\0';
	cVar1 = pcVar2[8];
	pcVar2 = pcVar2 + 8;
	INT_00448f48 = inIndex;
	while ((cVar1 != '\0' && (pcVar2 = DecodeStringFromBinaryTree_002469a0(pcVar2), pcVar2 != (char*)0x0))) {
		cVar1 = *pcVar2;
	}
	if (*PTR_DAT_00448f44 == '\\') {
		PTR_DAT_00448f44 = PTR_DAT_00448f44 + 1;
	}
	CopyIopPath_00246b50(sz_DefaultIopPath_00466d50, PTR_DAT_00448f44);
	return sz_DefaultIopPath_00466d50;
}


bool get_info(edCBankBufferEntry* pBankBuffer, int inFileIndex, edBANK_ENTRY_INFO* outFileData, char* outIopPath)
{
	bool bVar1;
	FileTypeData* pTypeData;
	FileHeaderFileData* pFileHeader;
	char* returnFileBufferStart;
	uint uVar2;
	BankFile_Internal* pBVar3;
	uint inIndex;

	MY_LOG("get_info for %d = %s\n", inFileIndex, DebugFindFilePath(pBankBuffer->fileBuffer, inFileIndex));

	pBVar3 = pBankBuffer->fileBuffer;
	if (pBVar3 == (BankFile_Internal*)0x0) {
		bVar1 = false;
	}
	else {
		bVar1 = false;
		if ((uint)inFileIndex < pBVar3->fileCount) {
			pTypeData = edCBankFileHeader_GetFileTypeData(pBVar3, inFileIndex);
			if (pTypeData == (FileTypeData*)0x0) {
				bVar1 = false;
			}
			else {
				outFileData->type = (uint)pTypeData->type;
				outFileData->stype = (uint)pTypeData->stype;
				pFileHeader = edCBankFileHeader_FindFileDataInHeader(pBankBuffer->fileBuffer, inFileIndex);
				if (pFileHeader == (FileHeaderFileData*)0x0) {
					bVar1 = false;
				}
				else {
					outFileData->size = pFileHeader->size;
					outFileData->crc = pFileHeader->crc;
					returnFileBufferStart = edCBankFileHeader_GetFileBufferStartFromFileIndex(pBankBuffer->fileBuffer, inFileIndex);
					outFileData->fileBufferStart = returnFileBufferStart;
					if (outIopPath != (char*)0x0) {
						*outIopPath = '\0';
						pBVar3 = pBankBuffer->fileBuffer;
						inIndex = 0;
						if (pBVar3->fileCount != 0) {
							do {
								uVar2 = edCBankFileHeader_get_index(pBVar3, inIndex, 0);
								if ((uVar2 == inFileIndex) &&
									(returnFileBufferStart = edCBankFileHeader_GetIopPath_00246460(pBankBuffer->fileBuffer, inIndex),
										returnFileBufferStart != (char*)0x0)) {
									edStringCpyL(outIopPath, returnFileBufferStart);
									break;
								}
								pBVar3 = pBankBuffer->fileBuffer;
								inIndex = inIndex + 1;
							} while (inIndex < pBVar3->fileCount);
						}
					}
					bVar1 = true;
				}
			}
		}
	}
	return bVar1;
}

bool edCBankBuffer_file_access_002450e0(edCBankBufferEntry* pBankBuffer, BankFilePathContainer* pLoadData)
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
			edDebugPrintf(sz_edCBankBuffer_Wait_0042fb70);
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

bool edCBankBuffer_close(edCBankBufferEntry* pBankBuffer)
{
	int iVar1;
	edCBank* peVar2;
	BankFile_Internal* fileBuffer;
	bool bVar3;

	if (pBankBuffer->header == (edCBank*)0x0) {
		bVar3 = false;
	}
	else {
		if (pBankBuffer->accessFlag != 0) {
			/* edCBankBuffer::file_access: Wait for end of previous loading operation \n */
			edDebugPrintf(sz_edCBankBuffer_Wait_0042fb70);
			iVar1 = pBankBuffer->accessFlag;
			while (iVar1 != 0) {
				ReadsBankFile();
				iVar1 = pBankBuffer->accessFlag;
			}
		}
		peVar2 = pBankBuffer->header;
		if (pBankBuffer->accessFlag != 0) {
			/* edCBankBuffer::file_access: Wait for end of previous loading operation \n */
			edDebugPrintf(sz_edCBankBuffer_Wait_0042fb70);
			iVar1 = pBankBuffer->accessFlag;
			while (iVar1 != 0) {
				ReadsBankFile();
				iVar1 = pBankBuffer->accessFlag;
			}
		}
		if (pBankBuffer->headerComponent == 0) {
			fileBuffer = pBankBuffer->fileBuffer;
			peVar2->flagB = peVar2->flagB - fileBuffer->sizePacked;
			peVar2->flagC = peVar2->flagC + -1;
			edCBankFileHeader_get_entry_typepair(fileBuffer, peVar2->pBankTypePairData, 5);
			memset(pBankBuffer, 0, sizeof(edCBankBufferEntry));
			bVar3 = true;
		}
		else {
			/* Can't close header of bank of bank!!\n */
			edDebugPrintf(sz_CloseBankBufferError_0042fbc0);
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

int GetIndexFromFileHeader(BankFile_Internal* bankBufferObj, char* inFileName)
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

	if (bankBufferObj->field_0x30 == 0) {
		pHeaderBase = (char*)0x0;
	}
	else {
		pHeaderBase = bankBufferObj->header + bankBufferObj->field_0x30 + -8;
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
		uVar2 = edCBankFileHeader_get_index(bankBufferObj, inIndex, 0);
	}
	return (int)uVar2;
}

int get_index(edCBankBufferEntry* headerObj, char* inFileName)
{
	int iVar1;

	iVar1 = 0;
	if (headerObj->fileBuffer != (BankFile_Internal*)0x0) {
		iVar1 = GetIndexFromFileHeader(headerObj->fileBuffer, inFileName);
	}
	return iVar1;
}

char* get_element(edCBankBufferEntry* bankObj, int fileIndex)
{
	return edCBankFileHeader_GetFileBufferStartFromFileIndex(bankObj->fileBuffer, fileIndex);
}

void load(edCBankBufferEntry* pBankAccessObject)
{
	int iVar1;

	if (pBankAccessObject->accessFlag != 0) {
		/* edCBankBuffer::file_access: Wait for end of previous loading operation \n */
		edDebugPrintf(sz_edCBankBuffer_Wait_0042fb70);
		iVar1 = pBankAccessObject->accessFlag;
		while (iVar1 != 0) {
			ReadsBankFile();
			iVar1 = pBankAccessObject->accessFlag;
		}
	}
	return;
}

bool edCBankBuffer_CheckAccessFlag(edCBankBufferEntry* bankAccessObj)
{
	return bankAccessObj->accessFlag == 0;
}