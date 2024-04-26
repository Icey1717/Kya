#include "edBankBuffer.h"
#include "edBankFile.h"
#include "edBankDebug.h"
#include "edBankStackFile.h"
#include "edBank.h"
#include "edFile.h"
#include "edStr.h"

const char* sz_CannotOpenFile_0042fc60 = "edCBankBufferEntry: Can't open file\n";
const char* sz_CannotReplaceHeader_0042fbf0 = "edCBankBufferEntry::file_access: Can't replace header of bank of bank!!\n";
const char* sz_FileNotOpen_0042fc40 = "edCBankBufferEntry: File not open \n";
char* sz_CloseBankBufferError_0042fbc0 = "Can't close header of bank of bank!!\n";

edCFiler_Bnk_static_header* edCBankBuffer::get_entry(int index)
{
	return (edCFiler_Bnk_static_header*)(&this->pBankBuffer->header + index * 9);
}

void edCBankBuffer::initialize(int size, int param_3, edCBankInstall* bankPathContainer)
{
	int iVar1;
	edCBankBufferEntry* peVar2;
	int* piVar3;
	edCBankBufferEntry* peVar4;
	uint bankFlags;
	uint uVar5;
	edCBankBuffer* local_s2_lo_56;

	edBankDebug.buffer_link(this);
	this->size = size;
	this->fileFlagA = bankPathContainer->fileFlagA;
	this->createFlagA = param_3;
	this->pBankTypePairData = (TypePairData*)0x0;
	this->flagB = 0;
	this->flagC = 0;
	this->firstField = 0;
	this->heapID = bankPathContainer->heapID;
	if (this->heapID == TO_HEAP(H_INVALID)) {
		this->heapID = TO_HEAP(H_MAIN);
	}
	if (_edBankAllowNoWait == 0) {
		this->fileFlagA = this->fileFlagA & 0xfffffffb;
	}
	if ((this->fileFlagA & 1U) != 0) {
		this->createFlagA = this->createFlagA + 1;
	}
	uVar5 = this->createFlagA * 0x24;
	if ((uVar5 & 0x7ff) == 0) {
		iVar1 = this->size;
	}
	else {
		uVar5 = ((uVar5 >> 0xb) + 1) * 0x800;
		iVar1 = this->size;
	}
	peVar2 = (edCBankBufferEntry*)edMemAllocAlign(this->heapID, (long)(int)(iVar1 + uVar5), 0x800);
	this->pBankBuffer = peVar2;
	memset(this->pBankBuffer, 0, uVar5);
	if ((this->fileFlagA & 1U) != 0) {
		bankFlags = 0;
		for (peVar2 = this->pBankBuffer; (bankFlags < (uint)this->createFlagA && (peVar2->field_0x4 != (int*)0x0)); peVar2 = (edCBankBufferEntry*)&peVar2->field_0x24) {
			bankFlags = bankFlags + 1;
		}
		if (bankFlags == this->createFlagA) {
			peVar2 = (edCBankBufferEntry*)0x0;
		}
		else {
			peVar2->headerComponent = 0;
			peVar2->header = this;
			uVar5 = this->createFlagA * 0x24;
			if ((uVar5 & 0x7ff) == 0) {
				peVar4 = this->pBankBuffer;
			}
			else {
				uVar5 = ((uVar5 >> 0xb) + 1) * 0x800;
				peVar4 = this->pBankBuffer;
			}
			peVar2->field_0x4 = (int*)((int)&peVar4->header + this->flagB + uVar5);
			this->flagC = this->flagC + 1;
		}
		uVar5 = 1;
		peVar2->headerComponent = 1;
		if ((this->fileFlagA & 4U) != 0) {
			uVar5 = 9;
		}
		piVar3 = (int*)edBankFilerReadHeader(this->heapID, bankPathContainer->filePath, uVar5, &peVar2->pLoadedData);
		peVar2->field_0x4 = piVar3;
		peVar2->fileBuffer = (edCBankFileHeader*)(piVar3 + 2);
		peVar2->field_0x14 = piVar3[0x11];
		((edCBankFileHeader*)(piVar3 + 2))->analyse();
	}
	return;
}

void edCBankBuffer::bank_buffer_setcb(TypePairData* pTypePairData)
{
	this->pBankTypePairData = pTypePairData;
	return;
}

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
	edCBankStackElement local_20;
	int fileBufferData;

	if (pBankFileAccessObject->accessFlag != 0) {
	
		edDebugPrintf("edCBankBufferEntry::file_access: Wait for end of previous loading operation \n");
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
	edBankStack.add_file(&local_20);
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
		edDebugPrintf("edCBankBufferEntry::file_access: Wait for end of previous loading operation \n");
		fileBufferData = pBankFileAccessObject->accessFlag;
		while (fileBufferData != 0) {
			edFileNoWaitStackFlush();
			fileBufferData = pBankFileAccessObject->accessFlag;
		}
	}
	return true;
}

void edCBankBufferEntry::wait()
{
	int iVar1;

	if (this->accessFlag != 0) {
		/* edCBankBuffer::file_access: Wait for end of previous loading operation \n */
		edDebugPrintf("edCBankBufferEntry::file_access: Wait for end of previous loading operation \n");
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

void edCBankBuffer::terminate()
{
	edBankDebug.buffer_unlink(this);

	if ((this->fileFlagA & 1U) != 0) {
		edFileClose(this->pBankBuffer->pLoadedData);
		edMemFree(this->pBankBuffer->field_0x4);
		memset(this->pBankBuffer, 0, 0x24);
	}

	edMemFree(this->pBankBuffer);
	memset(this, 0, 0x38);
	return;
}

edCBankBufferEntry* edCBankBuffer::get_free_entry()
{
	edCBankBufferEntry* returnBankPtr;
	edCBankBufferEntry* peVar1;
	uint uVar2;

	uVar2 = 0;
	for (returnBankPtr = this->pBankBuffer; (uVar2 < (uint)this->createFlagA && (returnBankPtr->field_0x4 != (int*)0x0)); returnBankPtr = (edCBankBufferEntry*)&returnBankPtr->field_0x24) {
		uVar2 = uVar2 + 1;
	}
	if (uVar2 == this->createFlagA) {
		returnBankPtr = (edCBankBufferEntry*)0x0;
	}
	else {
		returnBankPtr->headerComponent = 0;
		returnBankPtr->header = this;
		uVar2 = this->createFlagA * 0x24;
		if ((uVar2 & 0x7ff) == 0) {
			peVar1 = this->pBankBuffer;
		}
		else {
			uVar2 = ((uVar2 >> 0xb) + 1) * 0x800;
			peVar1 = this->pBankBuffer;
		}
		returnBankPtr->field_0x4 = (int*)((int)&peVar1->header + this->flagB + uVar2);
		this->flagC = this->flagC + 1;
	}
	return returnBankPtr;
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

	IO_LOG(LogLevel::Info, "get_info for {} = {}", inFileIndex, DebugFindFilePath(this->fileBuffer, inFileIndex));

	pBVar3 = this->fileBuffer;
	if (pBVar3 == (edCBankFileHeader*)0x0) {
		bVar1 = false;
	}
	else {
		bVar1 = false;
		if ((uint)inFileIndex < pBVar3->fileCount) {
			pTypeData = pBVar3->get_entry_typepair(inFileIndex);
			if (pTypeData == (FileTypeData*)0x0) {
				bVar1 = false;
			}
			else {
				outFileData->type = (uint)pTypeData->type;
				outFileData->stype = (uint)pTypeData->stype;
				pFileHeader = this->fileBuffer->get_entry(inFileIndex);
				if (pFileHeader == (FileHeaderFileData*)0x0) {
					bVar1 = false;
				}
				else {
					outFileData->size = pFileHeader->size;
					outFileData->crc = pFileHeader->crc;
					returnFileBufferStart = this->fileBuffer->get_entry_data(inFileIndex);
					outFileData->fileBufferStart = returnFileBufferStart;
					if (outIopPath != (char*)0x0) {
						*outIopPath = '\0';
						pBVar3 = this->fileBuffer;
						inIndex = 0;
						if (pBVar3->fileCount != 0) {
							do {
								uVar2 = pBVar3->get_index(inIndex, 0);
								if ((uVar2 == inFileIndex) &&
									(returnFileBufferStart = this->fileBuffer->get_entry_filename(inIndex),
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

char* edCBankBufferEntry::get_element(int fileIndex)
{
	return this->fileBuffer->get_entry_data(fileIndex);
}


int edCBankBufferEntry::get_index(const char* inFileName)
{
	int iVar1;

	iVar1 = 0;
	if (this->fileBuffer != (edCBankFileHeader*)0x0) {
		iVar1 = this->fileBuffer->get_entryindex_from_filename(inFileName);
	}
	return iVar1;
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
			edDebugPrintf("edCBankBufferEntry::file_access: Wait for end of previous loading operation \n");
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
			edDebugPrintf("edCBankBufferEntry::file_access: Wait for end of previous loading operation \n");
			iVar1 = this->accessFlag;
			while (iVar1 != 0) {
				edFileNoWaitStackFlush();
				iVar1 = this->accessFlag;
			}
		}
		peVar2 = this->header;
		if (this->accessFlag != 0) {
			/* edCBankBuffer::file_access: Wait for end of previous loading operation \n */
			edDebugPrintf("edCBankBufferEntry::file_access: Wait for end of previous loading operation \n");
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
			edDebugPrintf("edCBankBufferEntry::file_access: Wait for end of previous loading operation \n");
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