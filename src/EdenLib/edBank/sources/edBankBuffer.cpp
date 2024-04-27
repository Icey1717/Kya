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
	return (edCFiler_Bnk_static_header*)(&this->aBankBufferEntries->pBankBuffer + index * 9);
}

void edCBankBuffer::initialize(int size, int nbBankBufferEntries, edCBankInstall* pBankInstall)
{
	int fileSize;
	edCBankBufferEntry* pBankBufferEntry;
	char* pFileBuffer;
	char* pBufferStart;
	uint bankBufferEntryIndex;
	uint bankBufferSize;

	edBankDebug.buffer_link(this);

	this->size = size;
	this->fileFlagA = pBankInstall->fileFlagA;
	this->nbBankBufferEntries = nbBankBufferEntries;
	this->pBankCallback = (edCBankCallback*)0x0;
	this->flagB = 0;
	this->nbBankBufferEntriesInUse = 0;
	this->firstField = 0;
	this->heapID = pBankInstall->heapID;

	if (this->heapID == TO_HEAP(H_INVALID)) {
		this->heapID = TO_HEAP(H_MAIN);
	}

	if (_edBankAllowNoWait == 0) {
		this->fileFlagA = this->fileFlagA & 0xfffffffb;
	}

	if ((this->fileFlagA & 1U) != 0) {
		this->nbBankBufferEntries = this->nbBankBufferEntries + 1;
	}

	bankBufferSize = this->nbBankBufferEntries * sizeof(edCBankBufferEntry);
	if ((bankBufferSize & 0x7ff) == 0) {
		fileSize = this->size;
	}
	else {
		bankBufferSize = ((bankBufferSize >> 0xb) + 1) * 0x800;
		fileSize = this->size;
	}

	this->aBankBufferEntries = (edCBankBufferEntry*)edMemAllocAlign(this->heapID, fileSize + bankBufferSize, 0x800);
	memset(this->aBankBufferEntries, 0, bankBufferSize);

	if ((this->fileFlagA & 1U) != 0) {
		bankBufferEntryIndex = 0;
		for (pBankBufferEntry = this->aBankBufferEntries; (bankBufferEntryIndex < this->nbBankBufferEntries && (pBankBufferEntry->pFileBuffer != (char*)0x0)); pBankBufferEntry = pBankBufferEntry + 1) {
			bankBufferEntryIndex = bankBufferEntryIndex + 1;
		}

		if (bankBufferEntryIndex == this->nbBankBufferEntries) {
			pBankBufferEntry = (edCBankBufferEntry*)0x0;
		}
		else {
			pBankBufferEntry->headerComponent = 0;
			pBankBufferEntry->pBankBuffer = this;
			bankBufferSize = this->nbBankBufferEntries * sizeof(edCBankBufferEntry);

			if ((bankBufferSize & 0x7ff) == 0) {
				pBufferStart = reinterpret_cast<char*>(this->aBankBufferEntries);
			}
			else {
				bankBufferSize = ((bankBufferSize >> 0xb) + 1) * 0x800;
				pBufferStart = reinterpret_cast<char*>(this->aBankBufferEntries);
			}

			pBankBufferEntry->pFileBuffer = pBufferStart + this->flagB + bankBufferSize;
			this->nbBankBufferEntriesInUse = this->nbBankBufferEntriesInUse + 1;
		}

		bankBufferSize = 1;
		pBankBufferEntry->headerComponent = 1;

		if ((this->fileFlagA & 4U) != 0) {
			bankBufferSize = 9;
		}

		pFileBuffer = edBankFilerReadHeader(this->heapID, pBankInstall->filePath, bankBufferSize, &pBankBufferEntry->pFile);
		pBankBufferEntry->pFileBuffer = pFileBuffer;
		pBankBufferEntry->pFileHeader = reinterpret_cast<edCBankFileHeader*>(pFileBuffer + FILE_DATA_TAG_SIZE);
		pBankBufferEntry->field_0x14 = *reinterpret_cast<int*>(pBankBufferEntry->pFileHeader + 1);
		pBankBufferEntry->pFileHeader->analyse();
	}
	return;
}

void edCBankBuffer::bank_buffer_setcb(edCBankCallback* pNewBankCallback)
{
	this->pBankCallback = pNewBankCallback;
	return;
}

bool edCBankBuffer::file_access(edCBankBufferEntry* pBankBufferEntry, edCBankInstall* pBankInstall, long param_4, bool someFlag)
{
	edCBankFileHeader* pBankFileHeader;
	uint* puVar1;
	edFILEH* pFile;
	uint uVar2;
	char* peVar3;
	ulong uVar4;
	uint uVar5;
	char* pReadBuffer;
	uint unaff_s7_lo;
	edCBankStackElement bankStackElement;
	int accessFlag;

	if (pBankBufferEntry->accessFlag != 0) {	
		edDebugPrintf("edCBankBufferEntry::file_access: Wait for end of previous loading operation \n");
		accessFlag = pBankBufferEntry->accessFlag;
		while (accessFlag != 0) {
			edFileNoWaitStackFlush();
			accessFlag = pBankBufferEntry->accessFlag;
		}
	}

	if (someFlag == false) {
		uVar2 = 1;
		if (((this->fileFlagA & 4U) != 0) || ((pBankInstall->fileFlagA & 4U) != 0)) {
			uVar2 = 9;
		}

		pFile = edFileOpen(pBankInstall->filePath, uVar2 | 1);
		if (pFile == (edFILEH*)0x0) {
			/* edCBankBuffer: Can't open file\n */
			edDebugPrintf(sz_CannotOpenFile_0042fc60);
			return false;
		}
		uVar2 = edFileGetSize(pFile);
	}
	else {
		if (pBankBufferEntry->headerComponent != 0) {
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
	pBankBufferEntry->pObjectReference = pBankInstall->pObjectReference;
	if ((param_4 == 0) || ((long)(int)pBankBufferEntry->pFileHeader == 0)) {
		if ((uint)(this->size - this->flagB) < uVar2) {
			/* // edCBankBuffer: Not enough space in the bank buffer to add a bank \n */
			edDebugPrintf("// edCBankBufferEntry: Not enough space in the bank buffer to add a bank \n");
			return false;
		}

		uVar5 = this->nbBankBufferEntries * sizeof(edCBankBufferEntry);
		if ((uVar5 & 0x7ff) == 0) {
			peVar3 = reinterpret_cast<char*>(this->aBankBufferEntries);
		}
		else {
			uVar5 = ((uVar5 >> 0xb) + 1) * 0x800;
			peVar3 = reinterpret_cast<char*>(this->aBankBufferEntries);
		}

		pReadBuffer = peVar3 + this->flagB + uVar5;
		pBankBufferEntry->pFileBuffer = pReadBuffer;
		pBankBufferEntry->pFileHeader = reinterpret_cast<edCBankFileHeader*>(pReadBuffer + FILE_DATA_TAG_SIZE);
		pBankBufferEntry->pFile = (edFILEH*)0x0;
		pBankBufferEntry->field_0x14 = uVar2;
	}
	else {
		IMPLEMENTATION_GUARD();
		//edCBankFileHeader_get_entry_typepair((long)(int)pBankFileAccessObject->fileBuffer, pBankHeader->pBankTypePairData, 1, pBankFileAccessObject->pObjectReference);
		pReadBuffer = pBankBufferEntry->pFileBuffer;
	}

	pBankBufferEntry->accessFlag = 1;
	bankStackElement.pBankTypePairData_0xc = this->pBankCallback;
	bankStackElement.pObjectReference_0x10 = pBankInstall->pObjectReference;
	bankStackElement.fileFlagB_0x18 = pBankInstall->fileFlagA;
	bankStackElement.pDebugBankData = pFile;
	if (someFlag != false) {
		bankStackElement.pDebugBankData = (edFILEH*)0x0;
	}
	bankStackElement.fileFunc_0x14 = pBankInstall->fileFunc;
	bankStackElement.pBankHeader_0x0 = pBankBufferEntry;
	bankStackElement.pReadBuffer = pReadBuffer;
	edBankStack.add_file(&bankStackElement);

	if (pBankInstall->fileFlagE == 0) {
		this->flagB = this->flagB + uVar2;
	}
	else {
		this->flagB = this->flagB + pBankInstall->fileFlagE;
	}

	if (someFlag != false) {
		edFileSeek(pFile, unaff_s7_lo, ED_SEEK_SET);
	}

	edFileRead(pFile, pReadBuffer, uVar2);

	if (someFlag == false) {
		edFileClose(pFile);
	}

	if ((((this->fileFlagA & 4U) == 0) && ((pBankInstall->fileFlagA & 4U) == 0)) && (pBankBufferEntry->accessFlag != 0)) {
		/* edCBankBuffer::file_access: Wait for end of previous loading operation \n */
		edDebugPrintf("edCBankBufferEntry::file_access: Wait for end of previous loading operation \n");
		accessFlag = pBankBufferEntry->accessFlag;
		while (accessFlag != 0) {
			edFileNoWaitStackFlush();
			accessFlag = pBankBufferEntry->accessFlag;
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
		edFileClose(this->aBankBufferEntries->pFile);
		edMemFree(this->aBankBufferEntries->pFileBuffer);
		memset(this->aBankBufferEntries, 0, sizeof(edCBankBufferEntry));
	}

	edMemFree(this->aBankBufferEntries);
	memset(this, 0, sizeof(edCBankBuffer));
	return;
}

edCBankBufferEntry* edCBankBuffer::get_free_entry()
{
	edCBankBufferEntry* pFreeEntry;
	char* peVar1;
	uint nextFreeEntryId;
	uint offset;

	nextFreeEntryId = 0;

	for (pFreeEntry = this->aBankBufferEntries; (nextFreeEntryId < (uint)this->nbBankBufferEntries && (pFreeEntry->pFileBuffer != (char*)0x0)); pFreeEntry = pFreeEntry + 1) {
		nextFreeEntryId = nextFreeEntryId + 1;
	}

	if (nextFreeEntryId == this->nbBankBufferEntries) {
		pFreeEntry = (edCBankBufferEntry*)0x0;
	}
	else {
		pFreeEntry->headerComponent = 0;
		pFreeEntry->pBankBuffer = this;
		offset = this->nbBankBufferEntries * sizeof(edCBankBufferEntry);

		if ((offset & 0x7ff) == 0) {
			peVar1 = reinterpret_cast<char*>(this->aBankBufferEntries);
		}
		else {
			offset = ((offset >> 0xb) + 1) * 0x800;
			peVar1 = reinterpret_cast<char*>(this->aBankBufferEntries);
		}

		pFreeEntry->pFileBuffer = peVar1 + this->flagB + offset;

		this->nbBankBufferEntriesInUse = this->nbBankBufferEntriesInUse + 1;
	}

	return pFreeEntry;
}

bool edCBankBufferEntry::get_info(int inFileIndex, edBANK_ENTRY_INFO* pBankEntryInfoOut, char* outIopPath)
{
	bool bVar1;
	FileTypeData* pTypeData;
	FileHeaderFileData* pFileHeader;
	char* returnFileBufferStart;
	uint uVar2;
	edCBankFileHeader* pBVar3;
	uint inIndex;

	IO_LOG(LogLevel::Info, "get_info for {} = {}", inFileIndex, DebugFindFilePath(this->pFileHeader, inFileIndex));

	pBVar3 = this->pFileHeader;
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
				pBankEntryInfoOut->type = (uint)pTypeData->type;
				pBankEntryInfoOut->stype = (uint)pTypeData->stype;
				pFileHeader = this->pFileHeader->get_entry(inFileIndex);
				if (pFileHeader == (FileHeaderFileData*)0x0) {
					bVar1 = false;
				}
				else {
					pBankEntryInfoOut->size = pFileHeader->size;
					pBankEntryInfoOut->crc = pFileHeader->crc;
					returnFileBufferStart = this->pFileHeader->get_entry_data(inFileIndex);
					pBankEntryInfoOut->fileBufferStart = returnFileBufferStart;
					if (outIopPath != (char*)0x0) {
						*outIopPath = '\0';
						pBVar3 = this->pFileHeader;
						inIndex = 0;
						if (pBVar3->fileCount != 0) {
							do {
								uVar2 = pBVar3->get_index(inIndex, 0);
								if ((uVar2 == inFileIndex) &&
									(returnFileBufferStart = this->pFileHeader->get_entry_filename(inIndex),
										returnFileBufferStart != (char*)0x0)) {
									edStrCopy(outIopPath, returnFileBufferStart);
									break;
								}
								pBVar3 = this->pFileHeader;
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
	return this->pFileHeader->get_entry_data(fileIndex);
}


int edCBankBufferEntry::get_index(const char* inFileName)
{
	int index;

	index = 0;
	if (this->pFileHeader != (edCBankFileHeader*)0x0) {
		index = this->pFileHeader->get_entryindex_from_filename(inFileName);
	}
	return index;
}


int edCBankBufferEntry::get_element_count()
{
	int ret;

	if (this->pFileHeader == (edCBankFileHeader*)0x0) {
		ret = 0;
	}
	else {
		ret = this->pFileHeader->fileCount;
	}

	return ret;
}

bool edCBankBufferEntry::install(edCBankInstall* pBankInstall)
{
	int iVar1;
	bool bSuccess;
	LoadBankFileFunc pLoadFunc;

	if ((pBankInstall == (edCBankInstall*)0x0) || (this->pBankBuffer == (edCBankBuffer*)0x0)) {
		bSuccess = false;
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

		pLoadFunc = pBankInstall->fileFunc;
		if (pLoadFunc != (LoadBankFileFunc)0x0) {
			(*pLoadFunc)(false, pBankInstall->pObjectReference);
		}

		this->pFileHeader->apply_callback(this->pBankBuffer->pBankCallback, 0);

		if (pLoadFunc != (LoadBankFileFunc)0x0) {
			(*pLoadFunc)(true, pBankInstall->pObjectReference);
		}

		this->pFileHeader->analyse();

		bSuccess = true;
	}

	return bSuccess;
}

bool edCBankBufferEntry::close()
{
	int access;
	edCBankBuffer* pBankBuff;
	edCBankFileHeader* fileBuffer;
	bool bSuccess;

	if (this->pBankBuffer == (edCBankBuffer*)0x0) {
		bSuccess = false;
	}
	else {
		if (this->accessFlag != 0) {
			/* edCBankBuffer::file_access: Wait for end of previous loading operation \n */
			edDebugPrintf("edCBankBufferEntry::file_access: Wait for end of previous loading operation \n");
			access = this->accessFlag;
			while (access != 0) {
				edFileNoWaitStackFlush();
				access = this->accessFlag;
			}
		}

		pBankBuff = this->pBankBuffer;
		if (this->accessFlag != 0) {
			/* edCBankBuffer::file_access: Wait for end of previous loading operation \n */
			edDebugPrintf("edCBankBufferEntry::file_access: Wait for end of previous loading operation \n");
			access = this->accessFlag;
			while (access != 0) {
				edFileNoWaitStackFlush();
				access = this->accessFlag;
			}
		}
		if (this->headerComponent == 0) {
			fileBuffer = this->pFileHeader;
			pBankBuff->flagB = pBankBuff->flagB - fileBuffer->sizePacked;
			pBankBuff->nbBankBufferEntriesInUse = pBankBuff->nbBankBufferEntriesInUse + -1;
			fileBuffer->apply_callback(pBankBuff->pBankCallback, 5);
			memset(this, 0, sizeof(edCBankBufferEntry));
			bSuccess = true;
		}
		else {
			/* Can't close header of bank of bank!!\n */
			edDebugPrintf(sz_CloseBankBufferError_0042fbc0);
			bSuccess = false;
		}
	}
	return bSuccess;
}

bool edCBankBufferEntry::load(edCBankInstall* pBankInstall)
{
	bool ret;
	int access;

	if ((pBankInstall == (edCBankInstall*)0x0) || (this->pBankBuffer == (edCBankBuffer*)0x0)) {
		ret = false;
	}
	else {
		if (this->accessFlag != 0) {
			/* edCBankBuffer::file_access: Wait for end of previous loading operation \n */
			edDebugPrintf("edCBankBufferEntry::file_access: Wait for end of previous loading operation \n");
			access = this->accessFlag;

			while (access != 0) {
				edFileNoWaitStackFlush();
				access = this->accessFlag;
			}
		}

		ret = this->pBankBuffer->file_access(this, pBankInstall, 0, (this->pBankBuffer->fileFlagA & 1U) != 0);
	}

	return ret;
}