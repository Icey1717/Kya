#include "edCBankBuffer.h"
#include "edCBank.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

#if defined(PLATFORM_PS2)
#include <eekernel.h>
#endif
#include "../edStr.h"

#define IO_LOG(level, format, ...) MY_LOG_CATEGORY("IO", level, format, ##__VA_ARGS__)

const char* sz_edCBankBuffer_Wait_0042fb70 = "edCBankBufferEntry::file_access: Wait for end of previous loading operation \n";

void edFileNoWaitStackFlush(void)
{
	edCFiler* peVar1;
	edCFiler_28* peVar2;

	for (peVar1 = GetFirstEdFileHandler_00260e00(&g_edCFiler_MCPtr_00448fd8); peVar1 != (edCFiler*)0x0; peVar1 = (peVar1->baseData).pNextEd) {
		peVar2 = peVar1->GetGlobalC_0x1c();
		if (peVar2 != (edCFiler_28*)0x0) {
			edFileNoWaitStackCallBack(peVar2);
		}
	}
	return;
}

bool QueueBankRead_002444a0(edCBankStack* pReadBank, ReadBank_1C* pInBank)
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

uint edFileGetSize(edFILEH* param_1)
{
	uint uVar1;

	uVar1 = 0;
	if ((param_1->openFlags & 6) == 0) {
		uVar1 = (param_1->field_0x10).fileSize;
	}
	return uVar1;
}

uint edCBankFileHeader::get_index(int inIndex, int mode)
{
	uint uVar1;
	char* pcVar2;

	if (this == (edCBankFileHeader*)0x0) {
		/* edCBankFileHeader::get_index: NULL object\n */
		edDebugPrintf("edCBankFileHeader::get_index: NULL object\n");
		uVar1 = 0;
	}
	else {
		if (mode == 0) {
			pcVar2 = (char*)0x0;
			if (this->field_0x34 != 0) {
				pcVar2 = this->header + (this->field_0x34 - 8);
			}
		}
		else {
			pcVar2 = (char*)0x0;
			if (this->field_0x38 != 0) {
				pcVar2 = this->header + this->field_0x38 + -8;
			}
		}
		pcVar2 = pcVar2 + 8;
		if (this->fileCount < 0x100) {
			uVar1 = (uint)(byte)pcVar2[inIndex];
		}
		else {
			if (this->fileCount < 0x10000) {
				uVar1 = (uint) * (ushort*)(pcVar2 + inIndex * 2);
			}
			else {
				uVar1 = *(uint*)(pcVar2 + inIndex * 4);
			}
		}
	}
	return uVar1;
}

char* DebugFindFilePath(edCBankFileHeader* pBVar3, int inFileIndex)
{
	int inIndex = 0;
	char* returnFileBufferStart = NULL;
	if (pBVar3->fileCount != 0) {
		do {
			uint uVar2 = pBVar3->get_index(inIndex, 0);
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

bool edCBankBuffer::file_access(edCBankBufferEntry* pBankFileAccessObject, edCBankInstall* filePathPtr, long param_4, bool someFlag)
{
	edCBankFileHeader* bankBufferObj;
	uint* puVar1;
	edFILEH* pDebugBank;
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
			edFileNoWaitStackFlush();
			fileBufferData = pBankFileAccessObject->accessFlag;
		}
	}
	if (someFlag == false) {
		uVar2 = 1;
		if (((this->fileFlagA & 4U) != 0) || ((filePathPtr->fileFlagA & 4U) != 0)) {
			uVar2 = 9;
		}
		pDebugBank = edFileOpen(filePathPtr->filePath, uVar2 | 1);
		if (pDebugBank == (edFILEH*)0x0) {
			/* edCBankBuffer: Can't open file\n */
			edDebugPrintf(sz_CannotOpenFile_0042fc60);
			return false;
		}
		uVar2 = edFileGetSize(pDebugBank);
	}
	else {
		if (pBankFileAccessObject->headerComponent != 0) {
			/* edCBankBuffer::file_access: Can't replace header of bank of bank!!\n */
			edDebugPrintf(sz_CannotReplaceHeader_0042fbf0);
			return false;
		}
		IMPLEMENTATION_GUARD();
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
		//if (pDebugBank == (edFILEH*)0x0) {
		//	/* edCBankBuffer: File not open \n */
		//	edDebugPrintf(sz_FileNotOpen_0042fc40);
		//	return false;
		//}
	}
	pBankFileAccessObject->pObjectReference = filePathPtr->pObjectReference;
	if ((param_4 == 0) || ((long)(int)pBankFileAccessObject->fileBuffer == 0)) {
		if ((uint)(this->size - this->flagB) < uVar2) {
			/* // edCBankBuffer: Not enough space in the bank buffer to add a bank \n */
			edDebugPrintf("// edCBankBufferEntry: Not enough space in the bank buffer to add a bank \n");
			return false;
		}
		uVar5 = this->createFlagA * 0x24;
		if ((uVar5 & 0x7ff) == 0) {
			peVar3 = this->pBankBuffer;
		}
		else {
			uVar5 = ((uVar5 >> 0xb) + 1) * 0x800;
			peVar3 = this->pBankBuffer;
		}
		pReadBuffer = (char*)((char*)&peVar3->header + this->flagB + uVar5);
		pBankFileAccessObject->field_0x4 = (int*)pReadBuffer;
		pBankFileAccessObject->fileBuffer = (edCBankFileHeader*)(pReadBuffer + 8);
		pBankFileAccessObject->pLoadedData = (edFILEH*)0x0;
		pBankFileAccessObject->field_0x14 = uVar2;
	}
	else {
		IMPLEMENTATION_GUARD();
		//edCBankFileHeader_get_entry_typepair((long)(int)pBankFileAccessObject->fileBuffer, pBankHeader->pBankTypePairData, 1, pBankFileAccessObject->pObjectReference);
		pReadBuffer = (char*)pBankFileAccessObject->field_0x4;
	}
	pBankFileAccessObject->accessFlag = 1;
	local_20.pBankTypePairData_0xc = this->pBankTypePairData;
	local_20.pObjectReference_0x10 = filePathPtr->pObjectReference;
	local_20.fileFlagB_0x18 = filePathPtr->fileFlagA;
	local_20.pDebugBankData = pDebugBank;
	if (someFlag != false) {
		local_20.pDebugBankData = (edFILEH*)0x0;
	}
	local_20.fileFunc_0x14 = filePathPtr->fileFunc;
	local_20.pBankHeader_0x0 = pBankFileAccessObject;
	local_20.pReadBuffer = pReadBuffer;
	QueueBankRead_002444a0(&g_edCBankStack_00466e60, &local_20);
	if (filePathPtr->fileFlagE == 0) {
		this->flagB = this->flagB + uVar2;
	}
	else {
		this->flagB = this->flagB + filePathPtr->fileFlagE;
	}
	if (someFlag != false) {
		edFileSeek(pDebugBank, unaff_s7_lo, ED_SEEK_SET);
	}
	edFileRead(pDebugBank, pReadBuffer, uVar2);
	if (someFlag == false) {
		edFileClose(pDebugBank);
	}
	if ((((this->fileFlagA & 4U) == 0) && ((filePathPtr->fileFlagA & 4U) == 0)) && (pBankFileAccessObject->accessFlag != 0)) {
		/* edCBankBuffer::file_access: Wait for end of previous loading operation \n */
		edDebugPrintf(sz_edCBankBuffer_Wait_0042fb70);
		fileBufferData = pBankFileAccessObject->accessFlag;
		while (fileBufferData != 0) {
			edFileNoWaitStackFlush();
			fileBufferData = pBankFileAccessObject->accessFlag;
		}
	}
	return true;
}

bool edCBankBufferEntry::load(edCBankInstall* file)
{
	bool ret;
	int fileOutputStart;

	if ((file == (edCBankInstall*)0x0) || (this->header == (edCBankBuffer*)0x0)) {
		ret = false;
	}
	else {
		if (this->accessFlag != 0) {
			/* edCBankBuffer::file_access: Wait for end of previous loading operation \n */
			edDebugPrintf(sz_edCBankBuffer_Wait_0042fb70);
			fileOutputStart = this->accessFlag;
			while (fileOutputStart != 0) {
				edFileNoWaitStackFlush();
				fileOutputStart = this->accessFlag;
			}
		}
		ret = this->header->file_access(this, file, 0, (this->header->fileFlagA & 1U) != 0);
	}
	return ret;
}

int edCBankBufferEntry::get_element_count()
{
	int ret;

	if (this->fileBuffer == (edCBankFileHeader*)0x0) {
		ret = 0;
	}
	else {
		ret = this->fileBuffer->fileCount;
	}
	return ret;
}

char* sz_edCBankFileHeader_NullObj_0042ff90 = "edCBankFileHeader::get_entry_typepair: NULL object\n";

FileTypeData* edCBankFileHeader_GetFileTypeData(edCBankFileHeader* fileBuffer, int inFileIndex)
{
	bool bVar1;
	FileTypeData* pFVar2;

	if (fileBuffer == (edCBankFileHeader*)0x0) {
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

void edCBankFileHeader::apply_callback(TypePairData* pTypePairData, int mode)
{
	bool bVar1;
	char* pFileHeaderStart;
	FileTypeData* puVar3;
	TypePairData* puVar8;
	uint uVar3;
	int iVar4;
	int iVar5;

	IO_LOG(LogLevel::Info, "Looking up type pair entry: %d | file count: %d\n", mode, this->fileCount);

	if ((pTypePairData != (TypePairData*)0x0) && (uVar3 = 0, this->fileCount != 0)) {
		iVar5 = 0;
		iVar4 = 0;
		do {
			if (this == (edCBankFileHeader*)0x0) {
				/* edCBankFileHeader::get_entry_typepair: NULL object\n */
				edDebugPrintf(sz_edCBankFileHeader_NullObj_0042ff90);
				puVar3 = (FileTypeData*)0x0;
			}
			else {
				if (((this->typeData).stype != 0) || (bVar1 = true, (this->typeData).type != 0)) {
					bVar1 = false;
				}
				if (bVar1) {
					puVar3 = (FileTypeData*)(this->header + iVar5 + this->fileTypeDataOffset);
				}
				else {
					puVar3 = &this->typeData;
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
				if (this->fileHeaderDataOffset != 0) {
					pFileHeaderStart = (char*)(((char*)this - 8) + this->fileHeaderDataOffset);
				}

				IO_LOG(LogLevel::Info, "Executing for %s\n", DebugFindFilePath(this, uVar3));

				FileHeaderFileData* pFileHeaderData = (FileHeaderFileData*)(pFileHeaderStart + iVar4 + 8);
				puVar8->pFunction[mode](this->header + pFileHeaderData->offset - 8, pFileHeaderData->size);
			}
			uVar3 = uVar3 + 1;
			iVar5 = iVar5 + 4;
			iVar4 = iVar4 + 0x10;
		} while (uVar3 < this->fileCount);
	}
	return;
}

FileHeaderFileData* get_entry(edCBankFileHeader* pFileHeader, int fileIndex)
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

char* edCBankFileHeader_GetFileBufferStartFromFileIndex(edCBankFileHeader* pFileData, int fileIndex)
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

char* edCBankFileHeader_GetIopPath_00246460(edCBankFileHeader* fileBuffer, int inIndex)
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


bool edCBankBufferEntry::get_info(int inFileIndex, edBANK_ENTRY_INFO* outFileData, char* outIopPath)
{
	bool bVar1;
	FileTypeData* pTypeData;
	FileHeaderFileData* pFileHeader;
	char* returnFileBufferStart;
	uint uVar2;
	edCBankFileHeader* pBVar3;
	uint inIndex;

	IO_LOG(LogLevel::Info, "get_info for %d = %s\n", inFileIndex, DebugFindFilePath(this->fileBuffer, inFileIndex));

	pBVar3 = this->fileBuffer;
	if (pBVar3 == (edCBankFileHeader*)0x0) {
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
				pFileHeader = get_entry(this->fileBuffer, inFileIndex);
				if (pFileHeader == (FileHeaderFileData*)0x0) {
					bVar1 = false;
				}
				else {
					outFileData->size = pFileHeader->size;
					outFileData->crc = pFileHeader->crc;
					returnFileBufferStart = edCBankFileHeader_GetFileBufferStartFromFileIndex(this->fileBuffer, inFileIndex);
					outFileData->fileBufferStart = returnFileBufferStart;
					if (outIopPath != (char*)0x0) {
						*outIopPath = '\0';
						pBVar3 = this->fileBuffer;
						inIndex = 0;
						if (pBVar3->fileCount != 0) {
							do {
								uVar2 = pBVar3->get_index(inIndex, 0);
								if ((uVar2 == inFileIndex) &&
									(returnFileBufferStart = edCBankFileHeader_GetIopPath_00246460(this->fileBuffer, inIndex),
										returnFileBufferStart != (char*)0x0)) {
									edStrCopy(outIopPath, returnFileBufferStart);
									break;
								}
								pBVar3 = this->fileBuffer;
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

bool edCBankBufferEntry::install(edCBankInstall* pLoadData)
{
	int iVar1;
	bool bVar2;
	LoadBankFileFunc pLoadFunc;

	if ((pLoadData == (edCBankInstall*)0x0) || (this->header == (edCBankBuffer*)0x0)) {
		bVar2 = false;
	}
	else {
		if (this->accessFlag != 0) {
			/* edCBankBuffer::file_access: Wait for end of previous loading operation \n */
			edDebugPrintf(sz_edCBankBuffer_Wait_0042fb70);
			iVar1 = this->accessFlag;
			while (iVar1 != 0) {
				edFileNoWaitStackFlush();
				iVar1 = this->accessFlag;
			}
		}
		pLoadFunc = pLoadData->fileFunc;
		if (pLoadFunc != (LoadBankFileFunc)0x0) {
			(*pLoadFunc)(false, pLoadData->pObjectReference);
		}
		((edCBankFileHeader*)this->fileBuffer)->apply_callback(this->header->pBankTypePairData, 0);
		if (pLoadFunc != (LoadBankFileFunc)0x0) {
			(*pLoadFunc)(true, pLoadData->pObjectReference);
		}
		((edCBankFileHeader*)this->fileBuffer)->analyse();
		bVar2 = true;
	}
	return bVar2;
}

char* sz_CloseBankBufferError_0042fbc0 = "Can't close header of bank of bank!!\n";

bool edCBankBufferEntry::close()
{
	int iVar1;
	edCBankBuffer* peVar2;
	edCBankFileHeader* fileBuffer;
	bool bVar3;

	if (this->header == (edCBankBuffer*)0x0) {
		bVar3 = false;
	}
	else {
		if (this->accessFlag != 0) {
			/* edCBankBuffer::file_access: Wait for end of previous loading operation \n */
			edDebugPrintf(sz_edCBankBuffer_Wait_0042fb70);
			iVar1 = this->accessFlag;
			while (iVar1 != 0) {
				edFileNoWaitStackFlush();
				iVar1 = this->accessFlag;
			}
		}
		peVar2 = this->header;
		if (this->accessFlag != 0) {
			/* edCBankBuffer::file_access: Wait for end of previous loading operation \n */
			edDebugPrintf(sz_edCBankBuffer_Wait_0042fb70);
			iVar1 = this->accessFlag;
			while (iVar1 != 0) {
				edFileNoWaitStackFlush();
				iVar1 = this->accessFlag;
			}
		}
		if (this->headerComponent == 0) {
			fileBuffer = this->fileBuffer;
			peVar2->flagB = peVar2->flagB - fileBuffer->sizePacked;
			peVar2->flagC = peVar2->flagC + -1;
			fileBuffer->apply_callback(peVar2->pBankTypePairData, 5);
			memset(this, 0, sizeof(edCBankBufferEntry));
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

int TreeInfo_OptimizeFilePath(char* outFileName, const char* inFileName)
{
	const char* pcVar1;
	const char* pcVar2;
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

char* TreeInfo_RecurseWhileCountingIndexesUsingReference(char* headerBaseFilePath, int* outIndex, char* fileName, char* searchBuffer)
{
	bool bVar1;
	char cVar2;
	byte bVar3;
	long long lVar4;
	char* basePathChar;
	uint uVar5;
	int iVar6;
	long long lVar7;
	uint uVar8;

	IO_LOG(LogLevel::VeryVerbose, "TreeInfo_RecurseWhileCountingIndexesUsingReference headerBaseFilePath %s\n", headerBaseFilePath);

	/* This will recursively search for the input file in the header folder
	   structure */
	cVar2 = *headerBaseFilePath;
	lVar4 = (long long)cVar2;
	basePathChar = headerBaseFilePath + 1;
	if (lVar4 < 0) {
		lVar4 = (long long)-(int)cVar2 << 0x38;
		lVar7 = lVar4 >> 0x38;
		IO_LOG(LogLevel::VeryVerbose, "%d %d\n", lVar4, lVar7);
		if (lVar7 == 0x80) {
			lVar7 = (long long)((int)(lVar4 >> 0x38) + (int)*basePathChar);
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

		IO_LOG(LogLevel::VeryVerbose, "%u %u %d\n", uVar8, uVar5, lVar7);

		iVar6 = (int)lVar7;
		if (lVar7 != 0) {
			do {
				iVar6 = iVar6 + -1;
				*searchBuffer = *basePathChar;
				basePathChar = basePathChar + 1;
				searchBuffer = searchBuffer + 1;
			} while (iVar6 != 0);
		}
		IO_LOG(LogLevel::VeryVerbose, "TreeInfo_RecurseWhileCountingIndexesUsingReference basePathChar2: %s\n", basePathChar);
		*searchBuffer = '\\';
		do {
			if (uVar8 == 0) {
				return basePathChar;
			}
			uVar8 = uVar8 - 1;
			basePathChar = TreeInfo_RecurseWhileCountingIndexesUsingReference(basePathChar, outIndex, fileName, searchBuffer + 1);
		} while (basePathChar != (char*)0x0);
		IO_LOG(LogLevel::VeryVerbose, "TreeInfo_RecurseWhileCountingIndexesUsingReference FOUND\n");
		basePathChar = (char*)0x0;
	}
	else {
		if (lVar4 == 0x7f) {
			lVar4 = (long long)((int)cVar2 + (int)*basePathChar);
			basePathChar = headerBaseFilePath + 2;
		}
		while (bVar1 = lVar4 != 0, lVar4 = (long long)((int)lVar4 + -1), bVar1) {
			*searchBuffer = *basePathChar;
			basePathChar = basePathChar + 1;
			searchBuffer = searchBuffer + 1;
		}
		IO_LOG(LogLevel::VeryVerbose,"TreeInfo_RecurseWhileCountingIndexesUsingReference searchBufferB %s\n", basePathChar);
		do {
			fileName = fileName + -1;
			searchBuffer = searchBuffer + -1;
			IO_LOG(LogLevel::VeryVerbose,"TreeInfo_RecurseWhileCountingIndexesUsingReference searchBufferB %s\n", searchBuffer);
			IO_LOG(LogLevel::VeryVerbose,"TreeInfo_RecurseWhileCountingIndexesUsingReference filenameB %s\n", fileName);
			if (*fileName == 0) {
				IO_LOG(LogLevel::VeryVerbose,"TreeInfo_RecurseWhileCountingIndexesUsingReference FOUND B\n");
				return (char*)0x0;
			}
		} while ((*searchBuffer != 0) && ((*fileName & 0xdfU) == (*searchBuffer & 0xdfU)));
		*outIndex = *outIndex + 1;
	}
	IO_LOG(LogLevel::VeryVerbose,"TreeInfo_RecurseWhileCountingIndexesUsingReference basePathChar %s\n", basePathChar);
	IO_LOG(LogLevel::VeryVerbose,"TreeInfo_RecurseWhileCountingIndexesUsingReference fileName %s\n", fileName);
	IO_LOG(LogLevel::VeryVerbose,"TreeInfo_RecurseWhileCountingIndexesUsingReference searchBuffer %s\n", searchBuffer);
	return basePathChar;
}

int get_entryindex_from_filename(edCBankFileHeader* bankBufferObj, const char* inFileName)
{
	char* pHeaderBase;
	int fileNameLength;
	char* headerBasePath;
	uint uVar2;
	int inIndex;
	char formattedFilename[272];
	char fullHeaderFilePath[267];
	int outIndex;
	char cVar1;

	IO_LOG(LogLevel::Info, "GetIndexFromFileHeader %s\n", inFileName);

	if (bankBufferObj->field_0x30 == 0) {
		pHeaderBase = (char*)0x0;
	}
	else {
		pHeaderBase = bankBufferObj->header + bankBufferObj->field_0x30 + -8;
	}
	headerBasePath = pHeaderBase + 8;
	formattedFilename[0] = '\0';
	/* Formats file in the format extension / file name */
	fileNameLength = TreeInfo_OptimizeFilePath(formattedFilename + 1, inFileName);
	IO_LOG(LogLevel::VeryVerbose, "GetIndexFromFileHeader formatted %s | %s %d\n", formattedFilename + 1, headerBasePath, fileNameLength);
	outIndex = 0;
	cVar1 = *headerBasePath;
	while (cVar1 != '\0') {
		/* Search through the header object and find which index the input file name is
			*/
		headerBasePath =
			TreeInfo_RecurseWhileCountingIndexesUsingReference(headerBasePath, &outIndex, formattedFilename + 1 + fileNameLength,
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
		uVar2 = bankBufferObj->get_index(inIndex, 0);
	}
	return (int)uVar2;
}

int edCBankBufferEntry::get_index(const char* inFileName)
{
	int iVar1;

	iVar1 = 0;
	if (this->fileBuffer != (edCBankFileHeader*)0x0) {
		iVar1 = get_entryindex_from_filename(this->fileBuffer, inFileName);
	}
	return iVar1;
}

char* get_element(edCBankBufferEntry* bankObj, int fileIndex)
{
	return edCBankFileHeader_GetFileBufferStartFromFileIndex(bankObj->fileBuffer, fileIndex);
}

void edCBankBufferEntry::wait()
{
	int iVar1;

	if (this->accessFlag != 0) {
		/* edCBankBuffer::file_access: Wait for end of previous loading operation \n */
		edDebugPrintf(sz_edCBankBuffer_Wait_0042fb70);
		iVar1 = this->accessFlag;
		while (iVar1 != 0) {
			edFileNoWaitStackFlush();
			iVar1 = this->accessFlag;
		}
	}
	return;
}

bool edCBankBufferEntry::is_loaded()
{
	return this->accessFlag == 0;
}