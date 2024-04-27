#include "edBankFile.h"

#include "edFile.h"

#ifdef PLATFORM_WIN
#include "port.h"
#endif

char FileBuffer[512] = { 0 };
char* FileNamePtr = NULL;
int CurrentIndex = 0;

char* edBankFilerReadHeader(EHeap heapID, char* filePath, uint flags, edFILEH** ppFile)
{
	byte bVar1;
	edFILEH* pFile;
	uint uVar2;
	char* __dest;
	char* pReadBuffer;
	int size;
	char acStack4096[4096];
	uint __n;

	pReadBuffer = acStack4096;
	if (((ulong)pReadBuffer & 0x7ff) != 0) {
		pReadBuffer = (char*)((((ulong)pReadBuffer >> 0xb) + 1) * 0x800);
	}

	pFile = edFileOpen(filePath, flags);

	if (pFile == (edFILEH*)0x0) {
		__dest = (char*)0x0;
	}
	else {
		uVar2 = edFileGetSize(pFile);

		if (uVar2 < 0x40) {
			__dest = (char*)0x0;
		}
		else {
			memset(pReadBuffer, 0, 0x800);

			bVar1 = edFileRead(pFile, pReadBuffer, 0x800);
			if (bVar1 == 0) {
				__dest = (char*)0x0;
			}
			else {
				edFileFlush();
				__n = *(uint*)(pReadBuffer + 0x44);
				size = __n;
				if ((0x800 < __n) && ((__n & 0x7ff) != 0)) {
					size = ((__n >> 0xb) + 1) * 0x800;
				}
				__dest = (char*)edMemAlloc(heapID, size);
				if (__dest == (char*)0x0) {
					IMPLEMENTATION_GUARD();
					//FUN_0025b420(pDebugBank);
					__dest = (char*)0x0;
				}
				else {
					if (__n < 0x801) {
						memcpy(__dest, pReadBuffer, __n);
					}
					else {
						memcpy(__dest, pReadBuffer, 0x800);
						edFileRead(pFile, __dest + 0x800, size - 0x800);
						edFileFlush();
						if (__n != size) {
							edMemShrink(__dest, __n);
						}
					}
					if (ppFile == (edFILEH**)0x0) {
						edFileClose(pFile);
					}
					else {
						*ppFile = pFile;
					}
				}
			}
		}
	}

	return __dest;
}

void edCBankFileHeader::analyse()
{

}

void edCBankFileHeader::unpack()
{
	int iVar1;
	int iVar2;
	int iVar3;
	int iVar4;
	char* pcVar5;
	uint uVar6;
	char* pcVar7;
	uint uVar8;
	uint uVar9;
	int iVar10;
	uint* piVar11;
	int iVar12;
	uint uVar13;
	byte* pbVar14;
	char* piVar10;
	byte* __src;
	undefined auStack256[256];

	if ((this->flags_0x4 & 1) != 0) {
		/* \n ==> UNPACKING \n */
		edDebugPrintf("\n ==> UNPACKING \n");
		uVar6 = this->fileCount;
		piVar10 = (char*)&this[-1].field_0x34;
		if (uVar6 != 0) {
			iVar12 = uVar6 << 4;
			while (uVar6 != 0) {
				iVar1 = this->fileHeaderDataOffset;
				iVar2 = iVar12 + -0x10;
				uVar13 = uVar6 - 1;
				if (iVar1 == 0) {
					pcVar5 = (char*)0x0;
				}
				else {
					pcVar5 = piVar10 + iVar1;
				}
				if (iVar1 == 0) {
					pcVar7 = (char*)0x0;
				}
				else {
					pcVar7 = piVar10 + iVar1;
				}
				uVar8 = *(int*)(pcVar7 + uVar6 * 0x10 + 8) - *(int*)(pcVar5 + iVar2 + 8);
				if (iVar1 == 0) {
					pcVar7 = (char*)0x0;
				}
				else {
					pcVar7 = piVar10 + iVar1;
				}
				iVar3 = *(int*)(pcVar7 + iVar12 + -8);
				if ((this->flags_0x4 & 1) == 0) {
					if (iVar1 == 0) {
						pcVar7 = (char*)0x0;
					}
					else {
						pcVar7 = piVar10 + iVar1;
					}
					iVar12 = *(int*)(pcVar7 + iVar12 + -8);
				}
				else {
					pcVar7 = (char*)0x0;
					if (iVar1 != 0) {
						pcVar7 = piVar10 + iVar1;
					}
					iVar12 = *(int*)(pcVar7 + 8);
					uVar6 = 0;
					if (uVar13 != 0) {
						iVar10 = 0;
						uVar9 = this->field_0x1c - 1;
						do {
							pcVar7 = (char*)0x0;
							if (iVar1 != 0) {
								pcVar7 = piVar10 + iVar1;
							}
							uVar6 = uVar6 + 1;
							iVar4 = iVar10 + 0xc;
							iVar10 = iVar10 + 0x10;
							iVar12 = iVar12 + (~uVar9 & *(int*)(pcVar7 + iVar4) + uVar9);
						} while (uVar6 < uVar13);
					}
				}
				uVar9 = *(uint*)(pcVar5 + iVar2 + 0xc);
				pbVar14 = (byte*)(piVar10 + iVar12);
				uVar6 = uVar13;
				iVar12 = iVar2;
				if (uVar8 < uVar9) {
					__src = pbVar14 + uVar9;
					memcpy(auStack256, __src, 0x100);
					iVar1 = 0x100 - uVar8;
					while (uVar8 != 0) {
						uVar8 = uVar8 - 1;
						(__src + iVar1)[uVar8] = (piVar10 + iVar3)[uVar8];
					}
					edDataLZ77Unpack(pbVar14, __src + iVar1, uVar9);
					memcpy(__src, auStack256, 0x100);
				}
				else {
					while (uVar8 != 0) {
						uVar8 = uVar8 - 1;
						pbVar14[uVar8] = (piVar10 + iVar3)[uVar8];
					}
				}
			}
		}
		uVar6 = 0;
		iVar12 = 0;
		piVar11 = &this[-1].field_0x34;
		do {
			iVar1 = this->fileHeaderDataOffset;
			iVar2 = 0;
			if (iVar1 != 0) {
				iVar2 = (int)piVar11 + iVar1;
			}
			if ((this->flags_0x4 & 1) == 0) {
				iVar3 = 0;
				if (iVar1 != 0) {
					iVar3 = (int)piVar11 + iVar1;
				}
				iVar3 = *(int*)(iVar3 + iVar12 + 8);
			}
			else {
				iVar3 = 0;
				if (iVar1 != 0) {
					iVar3 = (int)piVar11 + iVar1;
				}
				iVar3 = *(int*)(iVar3 + 8);
				uVar13 = 0;
				if (uVar6 != 0) {
					iVar10 = 0;
					uVar8 = this->field_0x1c - 1;
					do {
						iVar4 = 0;
						if (iVar1 != 0) {
							iVar4 = (int)piVar11 + iVar1;
						}
						iVar4 = iVar4 + iVar10;
						uVar13 = uVar13 + 1;
						iVar10 = iVar10 + 0x10;
						iVar3 = iVar3 + (~uVar8 & *(int*)(iVar4 + 0xc) + uVar8);
					} while (uVar13 < uVar6);
				}
			}
			*(int*)(iVar2 + iVar12 + 8) = iVar3;
			uVar6 = uVar6 + 1;
			iVar12 = iVar12 + 0x10;
		} while (uVar6 <= this->fileCount);
	}
	return;
}

void edDataLZ77Unpack(byte* param_1, byte* param_2, int param_3)
{
	byte bVar1;
	byte bVar2;
	long lVar3;
	uint uVar4;
	uint in_t2_lo;

	uVar4 = 0;
	if (param_3 != 0) {
		do {
			if (uVar4 == 0) {
				in_t2_lo = (uint)*param_2;
				uVar4 = 1;
				param_2 = param_2 + 1;
			}
			if ((in_t2_lo & 0xff & uVar4) == 0) {
				bVar1 = param_2[1];
				bVar2 = *param_2;
				param_2 = param_2 + 2;
				lVar3 = ((ulong)bVar2 & 0xf) + 3;
				param_3 = param_3 - (int)lVar3;
				if (lVar3 != 0) {
					do {
						lVar3 = lVar3 + -1;
						*param_1 = param_1[-((int)((long)((ulong)bVar2 | (long)(int)((uint)bVar1 << 8)) >> 4) + 1)];
						param_1 = param_1 + 1;
					} while (0 < lVar3);
				}
			}
			else {
				param_3 = param_3 + -1;
				*param_1 = *param_2;
				param_2 = param_2 + 1;
				param_1 = param_1 + 1;
			}
			uVar4 = (uVar4 & 0x7f) << 1;
		} while (param_3 != 0);
	}
	return;
}

int edCBankFileHeader::get_entryindex_from_filename(const char* inFileName)
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

	IO_LOG(LogLevel::Info, "GetIndexFromFileHeader {}", inFileName);

	if (this->field_0x30 == 0) {
		pHeaderBase = (char*)0x0;
	}
	else {
		pHeaderBase = this->header + this->field_0x30 + -8;
	}
	headerBasePath = pHeaderBase + 8;
	formattedFilename[0] = '\0';
	/* Formats file in the format extension / file name */
	fileNameLength = TreeInfo_OptimizeFilePath(formattedFilename + 1, inFileName);
	IO_LOG_DISABLED(LogLevel::VeryVerbose, "GetIndexFromFileHeader formatted {} | {} {}", formattedFilename + 1, headerBasePath, fileNameLength);
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
		uVar2 = this->get_index(inIndex, 0);
	}
	return (int)uVar2;
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

char* sz_edCBankFileHeader_NullObj_0042ff90 = "edCBankFileHeader::get_entry_typepair: NULL object\n";

FileTypeData* edCBankFileHeader::get_entry_typepair(int inFileIndex)
{
	bool bVar1;
	FileTypeData* pFVar2;

	if (this == (edCBankFileHeader*)0x0) {
		/* edCBankFileHeader::get_entry_typepair: NULL object\n */
		edDebugPrintf(sz_edCBankFileHeader_NullObj_0042ff90);
		pFVar2 = (FileTypeData*)0x0;
	}
	else {
		if (((this->typeData).stype != 0) || (bVar1 = true, (this->typeData).type != 0)) {
			bVar1 = false;
		}
		if (bVar1) {
			pFVar2 = (FileTypeData*)(this->header + inFileIndex * 4 + this->fileTypeDataOffset);
		}
		else {
			pFVar2 = &this->typeData;
		}
	}
	return pFVar2;
}

bool edCBankFileHeader::is_heterogeneous()
{
	bool bVar1;

	if (((this->typeData).stype != 0) || (bVar1 = true, (this->typeData).type != 0)) {
		bVar1 = false;
	}
	return bVar1;
}

char* edCBankFileHeader::get_entry_filename(int inIndex)
{
	char cVar1;
	char* pcVar2;
	char local_108[264];

	pcVar2 = (char*)0x0;

	if (this->field_0x30 != 0) {
		pcVar2 = this->header + this->field_0x30 + -8;
	}

	FileNamePtr = local_108 + 0x107;
	local_108[263] = '\0';
	cVar1 = pcVar2[8];
	pcVar2 = pcVar2 + 8;
	CurrentIndex = inIndex;

	while ((cVar1 != '\0' && (pcVar2 = TreeInfo_Recurse(pcVar2), pcVar2 != (char*)0x0))) {
		cVar1 = *pcVar2;
	}

	if (*FileNamePtr == '\\') {
		FileNamePtr = FileNamePtr + 1;
	}

	TreeInfo_UnOptimizeFilePath(FileBuffer, FileNamePtr);

	return FileBuffer;
}


char* edCBankFileHeader::get_entry_data(int fileIndex)
{
	char* pcVar1;

	pcVar1 = (char*)0x0;

	if (this->fileHeaderDataOffset != 0) {
		pcVar1 = this->header + this->fileHeaderDataOffset + -8;
	}

	FileHeaderFileData* pFileHeaderData = (FileHeaderFileData*)(pcVar1 + fileIndex * 0x10 + 8);

	return this->header + pFileHeaderData->offset + -8;
}


FileHeaderFileData* edCBankFileHeader::get_entry(int fileIndex)
{
	char* pcVar1;

	if (this->fileHeaderDataOffset == 0) {
		pcVar1 = (char*)0x0;
	}
	else {
		pcVar1 = this->header + this->fileHeaderDataOffset + -8;
	}
	return (FileHeaderFileData*)(pcVar1 + fileIndex * 0x10 + 8);
}

void edCBankFileHeader::apply_callback(edCBankCallback* pTypePairData, int mode)
{
	bool bVar1;
	char* pFileHeaderStart;
	FileTypeData* puVar3;
	edCBankCallback* puVar8;
	uint uVar3;
	int iVar4;
	int iVar5;

	IO_LOG(LogLevel::Info, "Looking up type pair entry: %d | file count: %d\n", mode, this->fileCount);

	if ((pTypePairData != (edCBankCallback*)0x0) && (uVar3 = 0, this->fileCount != 0)) {
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
			puVar8 = (edCBankCallback*)0x0;
		LAB_00246698:
			if ((puVar8 != (edCBankCallback*)0x0) && (puVar8->pFunction[mode] != NULL)) {
				pFileHeaderStart = 0;
				if (this->fileHeaderDataOffset != 0) {
					pFileHeaderStart = (char*)(((char*)this - 8) + this->fileHeaderDataOffset);
				}

				IO_LOG(LogLevel::Info, "Executing for {}\n", DebugFindFilePath(this, uVar3));

				NAME_NEXT_OBJECT(DebugFindFilePath(this, uVar3));

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

	IO_LOG_DISABLED(LogLevel::VeryVerbose, "TreeInfo_RecurseWhileCountingIndexesUsingReference headerBaseFilePath %s\n", headerBaseFilePath);

	/* This will recursively search for the input file in the header folder
	   structure */
	cVar2 = *headerBaseFilePath;
	lVar4 = (long long)cVar2;
	basePathChar = headerBaseFilePath + 1;
	if (lVar4 < 0) {
		lVar4 = (long long)-(int)cVar2 << 0x38;
		lVar7 = lVar4 >> 0x38;
		IO_LOG_DISABLED(LogLevel::VeryVerbose, "%d %d\n", lVar4, lVar7);
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

		IO_LOG_DISABLED(LogLevel::VeryVerbose, "%u %u %d\n", uVar8, uVar5, lVar7);

		iVar6 = (int)lVar7;
		if (lVar7 != 0) {
			do {
				iVar6 = iVar6 + -1;
				*searchBuffer = *basePathChar;
				basePathChar = basePathChar + 1;
				searchBuffer = searchBuffer + 1;
			} while (iVar6 != 0);
		}
		IO_LOG_DISABLED(LogLevel::VeryVerbose, "TreeInfo_RecurseWhileCountingIndexesUsingReference basePathChar2: %s\n", basePathChar);
		*searchBuffer = '\\';
		do {
			if (uVar8 == 0) {
				return basePathChar;
			}
			uVar8 = uVar8 - 1;
			basePathChar = TreeInfo_RecurseWhileCountingIndexesUsingReference(basePathChar, outIndex, fileName, searchBuffer + 1);
		} while (basePathChar != (char*)0x0);
		IO_LOG_DISABLED(LogLevel::VeryVerbose, "TreeInfo_RecurseWhileCountingIndexesUsingReference FOUND\n");
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
		IO_LOG_DISABLED(LogLevel::VeryVerbose, "TreeInfo_RecurseWhileCountingIndexesUsingReference searchBufferB %s\n", basePathChar);
		do {
			fileName = fileName + -1;
			searchBuffer = searchBuffer + -1;
			IO_LOG_DISABLED(LogLevel::VeryVerbose, "TreeInfo_RecurseWhileCountingIndexesUsingReference searchBufferB %s\n", searchBuffer);
			IO_LOG_DISABLED(LogLevel::VeryVerbose, "TreeInfo_RecurseWhileCountingIndexesUsingReference filenameB %s\n", fileName);
			if (*fileName == 0) {
				IO_LOG_DISABLED(LogLevel::VeryVerbose, "TreeInfo_RecurseWhileCountingIndexesUsingReference FOUND B\n");
				return (char*)0x0;
			}
		} while ((*searchBuffer != 0) && ((*fileName & 0xdfU) == (*searchBuffer & 0xdfU)));
		*outIndex = *outIndex + 1;
	}
	IO_LOG_DISABLED(LogLevel::VeryVerbose, "TreeInfo_RecurseWhileCountingIndexesUsingReference basePathChar %s\n", basePathChar);
	IO_LOG_DISABLED(LogLevel::VeryVerbose, "TreeInfo_RecurseWhileCountingIndexesUsingReference fileName %s\n", fileName);
	IO_LOG_DISABLED(LogLevel::VeryVerbose, "TreeInfo_RecurseWhileCountingIndexesUsingReference searchBuffer %s\n", searchBuffer);
	return basePathChar;
}

// Recursive function that returns a pointer to a string
// Takes a pointer to a null-terminated string as input
char* TreeInfo_Recurse(char* input)
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
			pcVar3 = TreeInfo_Recurse(pcVar3);
		} while (pcVar3 != (char*)0x0);
		pcVar2 = pcVar2 + iVar7;
		if (rightShifteValue != 0) {
			do {
				iVar7 = iVar7 + -1;
				pcVar2 = pcVar2 + -1;
				pcVar3 = FileNamePtr + -1;
				FileNamePtr = FileNamePtr + -1;
				*pcVar3 = *pcVar2;
			} while (iVar7 != 0);
		}
		pcVar2 = (char*)0x0;
		pcVar3 = FileNamePtr + -1;
		FileNamePtr = FileNamePtr + -1;
		*pcVar3 = '\\';
	}
	else {
		if (lVar5 == 0x7f) {
			lVar5 = (long)((int)cVar1 + (int)*pcVar2);
			pcVar2 = input + 2;
		}
		iVar7 = (int)lVar5;
		if (CurrentIndex == 0) {
			pcVar2 = pcVar2 + iVar7;
			if (lVar5 != 0) {
				do {
					iVar7 = iVar7 + -1;
					pcVar2 = pcVar2 + -1;
					pcVar3 = FileNamePtr + -1;
					FileNamePtr = FileNamePtr + -1;
					*pcVar3 = *pcVar2;
				} while (iVar7 != 0);
			}
			pcVar2 = (char*)0x0;
			pcVar3 = FileNamePtr + -1;
			FileNamePtr = FileNamePtr + -1;
			*pcVar3 = '\\';
		}
		else {
			CurrentIndex = CurrentIndex + -1;
			pcVar2 = pcVar2 + iVar7;
		}
	}
	return pcVar2;
}

void TreeInfo_UnOptimizeFilePath(char* param_1, char* param_2)
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

char* DebugFindFilePath(edCBankFileHeader* pBVar3, int inFileIndex)
{
	int inIndex = 0;
	char* returnFileBufferStart = NULL;
	if (pBVar3->fileCount != 0) {
		do {
			uint uVar2 = pBVar3->get_index(inIndex, 0);
			if ((uVar2 == inFileIndex) &&
				(returnFileBufferStart = pBVar3->get_entry_filename(inIndex),
					returnFileBufferStart != (char*)0x0)) {
				break;
			}
			inIndex = inIndex + 1;
		} while (inIndex < pBVar3->fileCount);
	}
	return returnFileBufferStart;
}
