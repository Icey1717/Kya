#include "_ps2/_edFileFilerCDVD.h"

#include <string.h>
#include <stdio.h>
#include "edMem.h"

#if defined(PLATFORM_PS2)
#include <sifdev.h>
#include <eekernel.h>
#else
#include <vector>
#endif

#include "edSystem.h"
#include "edFile.h"

#include <assert.h>
#include "edStr.h"
#include "edFilePath.h"

edCFiler_CDVD edFiler_CDVD;

char* sz_CDVD_Drive_004312f8 = "<CDVD>";
char* sz_DriveLetter_00431300 = "0:";

edCFiler_CDVD::edCFiler_CDVD()
{
	int iVar1;
	int iVar2;

	iVar2 = 0;
	do {
		aContainerInUse[iVar2] = 0;
		memset(&aFileContainers[iVar2], 0, sizeof(CDFileContainer));
		iVar2 = iVar2 + 1;
	} while (iVar2 < 0x10);

	field_0x4a4[0].freeIndexes = 0;
	field_0x4a4[0].currentIndex = 0;
	field_0x4a4[0].nextAction = LOAD;
	toc.bLoaded = 0;
	toc.pCurrentFolder = (edCdlFolder*)0x0;
	toc.offset = 0;
	toc.pNextFreeEntry = (edCdlFolder*)0x0;
	toc.objCount_0x14 = 0;
	return;
}

bool edCFiler_CDVD::configure(char* path, ETableOfContentsInitMode mode, edFILE_PARAMETER* param_4)
{
	if (mode == IM_INIT) {
		//toc.Initialize((edCdlFolder*)param_4->field_0x0, (int)param_4->field_0x8);
	}
	else {
		if (mode != IM_CALC_SIZE) {
			return false;
		}
		/* Work out how many files we need. */
		*param_4->field_0x8 = ((int)param_4->field_0x0 + 1) * 0x2c;
	}
	return true;
}

bool edCFiler_CDVD::initialize()
{
	this->pDriveName_0x0 = sz_CDVD_Drive_004312f8;
	this->iopBuf = 0;
	set_default_unit(sz_DriveLetter_00431300);
	this->field_0x4 = 0x61;
	return true;
}

edCFiler_28* edCFiler_CDVD::GetGlobalC_0x1c()
{
	return field_0x4a4;
}

#if defined(PLATFORM_WIN)
char* FormatForPC(char* inString)
{
	size_t len = strlen(inString);
	// remove the first character from the string
	memmove(inString, inString + 1, len - 1);
	// remove the last three characters from the string
	inString[len - 3] = '\0';
	return inString;
}
#endif

DebugBankDataInternal* GetDebugBankInternal(edFILEH* param_1)
{
	DebugBankDataInternal* pDVar1;

	pDVar1 = &param_1->field_0x10;
	if ((param_1->openFlags & 6) != 0) {
		pDVar1 = (DebugBankDataInternal*)0x0;
	}
	return pDVar1;
}

#ifdef PLATFORM_WIN
std::vector<FILE*> g_FileDescriptors;
#endif

char* sz_FileNotFound_00431280 = "edCFiler_CDVD::open: File not found... retry... %s\n";
char* sz_cdrom_00431268 = "cdrom";
char* sz_DriveLetter_00431270 = ";1";

bool edCFiler_CDVD::open(edFILEH* outFile, char* unformatedFilePath)
{
	u_char uVar1;
	u_char uVar2;
	byte bVar3;
	byte bVar4;
	uint openFlags;
	bool openSuccess;
	int fileDirectoryLength;
	char* pcVar6;
	int fileNameLength;
	char* postColonFilePath;
	bool success;
	DebugBankDataInternal* pDVar7;
	int newFd;
	void* pNewIopBuf;
	byte* pbVar8;
	char* outString;
	ulong ret;
	CDFileContainer* bufferStart;
	char fileDirectory[512];
	char fullFilePath[524];
#if defined(PLATFORM_PS2)
	sceCdlLOCCD local_4;
#endif
	int fileDescriptor;

	MY_LOG("edCFiler_CDVD::open {}\n", unformatedFilePath);

	/* First we have to change the format of the file path from <CDVD>0:\CDEURO\LEVEL\ to cdrom0:\CDEURO\LEVEL\ */
	openFlags = outFile->openFlags;
	fileDirectoryLength = 0;

	do {
		if (aContainerInUse[fileDirectoryLength] == 0) {
			aContainerInUse[fileDirectoryLength] = 1;
			bufferStart = &aFileContainers[fileDirectoryLength];
			memset(bufferStart, 0, sizeof(CDFileContainer));
			break;
		}
		fileDirectoryLength = fileDirectoryLength + 1;
		bufferStart = (CDFileContainer*)0x0;
	} while (fileDirectoryLength < 0x10);

	openSuccess = false;

	if (bufferStart != (CDFileContainer*)0x0) {
		edFilePathSplit((char*)0x0, fileDirectory, (char*)0x0, (char*)0x0, unformatedFilePath);
		/* File path starts with 'cdrom' */
		fileDirectoryLength = edStrCopy(fullFilePath, sz_cdrom_00431268);
		outString = fullFilePath + fileDirectoryLength;
		fileDirectoryLength = edStrCopy(outString, fileDirectory);
		pcVar6 = edFilePathGetFilePath(unformatedFilePath);
		fileNameLength = edStrCopyUpper(outString + fileDirectoryLength, pcVar6);
		/* Find the first empty char after our current buffer pos and append this suffix ';1' */
		edStrCat(outString + fileDirectoryLength + fileNameLength, sz_DriveLetter_00431270);
		/* At this point we will have a path like cdrom0:\CDEURO\LEVEL\PREINTRO\CINE\1c9de79b.bnk */

		MY_LOG("edCFiler_CDVD::open Formatted path {} toc: {}\n", fullFilePath, toc.bLoaded);

#ifdef PLATFORM_WIN
		// For windows, create a file handle.
		FILE* fp = NULL;
#endif

		ret = 0;
		if (toc.bLoaded == 0) {
			while (ret == 0) {
#if defined(PLATFORM_PS2)
				MY_LOG("edCFiler_CDVD::open Searching\n");
				sceCdDiskReady(0);
				pcVar6 = edFilePathGetFilePath(fullFilePath);
				ret = sceCdSearchFile(&bufferStart->file, pcVar6);
				if (ret == 0) {
					/* edCFiler_CDVD::open: File not found... retry... %s\n */
					edDebugPrintf(sz_FileNotFound_00431280);
				}
#else
				pcVar6 = edFilePathGetFilePath(fullFilePath);

				char* pcFileFull = FormatForPC(pcVar6);

				fp = fopen(pcFileFull, "rb");

				if (fp) {
					fseek(fp, 0, SEEK_END);
					(bufferStart->file).size = ftell(fp);
					fseek(fp, 0, SEEK_SET);
					ret = 1;
				}
				else {
					assert(fp);
				}
#endif
			}
		}
		else {
			/* seek along the path to the first colon (cdrom0:(HERE)\CDEURO\LEVEL\PREINTRO\CINE\1c9de79b.bnk) */
			postColonFilePath = edFilePathGetFilePath(unformatedFilePath);
			success = toc.LoadFromTOC(&bufferStart->file, postColonFilePath);
			ret = (long)success & 0xff;
			if (ret == 0) {
				/* edCFiler_CDVD::open: File not found... retry... %s\n */
				edDebugPrintf(sz_FileNotFound_00431280);
			}
		}

		fileDirectoryLength = 0;

		if (ret == 0) {
			pbVar8 = aContainerInUse;
			do {
				if ((CDFileContainer*)(pbVar8 + 0x10) == bufferStart) {
					aContainerInUse[fileDirectoryLength] = 0;
					memset(aFileContainers + fileDirectoryLength, 0, sizeof(CDFileContainer));
					break;
				}
				fileDirectoryLength = fileDirectoryLength + 1;
				pbVar8 = pbVar8 + 0x28;
			} while (fileDirectoryLength < 0x10);
			//PrintCdError();
			//LoadFailFunc_0025d270(edCFiler_CDVD);
			openSuccess = false;
		}
		else {
			/* Found file successfully */
			outFile->pFileData = bufferStart;
			pDVar7 = GetDebugBankInternal(outFile);
			pDVar7->fileSize = (bufferStart->file).size;
			pDVar7->field_0x14 = 0x22;
			uVar1 = (bufferStart->file).date[3];
			uVar2 = (bufferStart->file).date[2];
			*(u_char*)pDVar7 = (bufferStart->file).date[1];
			*(u_char*)&pDVar7->field_0x1 = uVar2;
			*(u_char*)&pDVar7->field_0x2 = uVar1;
			bVar3 = (bufferStart->file).date[7];
			bVar4 = (bufferStart->file).date[6];
			uVar1 = (bufferStart->file).date[5];
			*(u_char*)&pDVar7->field_0x4 = (bufferStart->file).date[4];
			*(u_char*)&pDVar7->field_0x5 = uVar1;
			*(ushort*)&pDVar7->field_0x6 = (ushort)bVar4 + (ushort)bVar3 * 0x100;
			*(undefined*)&pDVar7->field_0x8 = *(undefined*)pDVar7;
			*(undefined*)&pDVar7->field_0x9 = *(undefined*)&pDVar7->field_0x1;
			*(undefined*)&pDVar7->field_0xa = *(undefined*)&pDVar7->field_0x2;
			*(undefined*)&pDVar7->field_0xb = *(undefined*)&pDVar7->field_0x3;
			*(undefined*)&pDVar7->field_0xc = *(undefined*)&pDVar7->field_0x4;
			*(undefined*)&pDVar7->field_0xd = *(undefined*)&pDVar7->field_0x5;
			/* Copy file name into object */
			*(undefined2*)&pDVar7->field_0xe = *(undefined2*)&pDVar7->field_0x6;
			edStrCopy(pDVar7->name, (bufferStart->file).name);
			if ((openFlags & 8) == 0) {
#if defined(PLATFORM_PS2)
				bufferStart->fd = -1;
				fileDescriptor = bufferStart->fd;
				while (fileDescriptor < 0) {
					sceCdDiskReady(0);
					newFd = sceOpen(fullFilePath, 1);
					bufferStart->fd = newFd;
					fileDescriptor = bufferStart->fd;
				}
				outFile->pFileData = bufferStart;
				if ((openFlags & 0x10) != 0) {
					pNewIopBuf = sceSifAllocSysMemory(0, 0x28010, (void*)0x0);
					iopBuf = (char*)pNewIopBuf;
					sceCdStInit(0x50, 5, (uint)(iopBuf + 0xf) & 0xfffffff0);
					local_4.minute = 0;
					local_4.second = 0;
					local_4.sector = 0;
					sceCdIntToPos((bufferStart->file).lsn, &local_4);
				}	
#endif
			}

#ifdef PLATFORM_WIN
			if (fp) {
				bufferStart->fd = g_FileDescriptors.size();
				MY_LOG("edCFiler_CDVD::open FILE: {}\n", bufferStart->fd);
				g_FileDescriptors.push_back(fp);
			}
#endif

			openSuccess = true;
		}
	}

	return openSuccess;
}

bool edCFiler_CDVD::close(edFILEH* pDebugBank)
{
	bool bVar1;
	CDFileContainer* piVar2;
	int iVar2;

	piVar2 = (CDFileContainer*)GetInternalData_0025b2e0(pDebugBank);
	if (piVar2 == (CDFileContainer*)0x0) {
		bVar1 = false;
	}
	else {
		MY_LOG("edCFiler_CDVD::close FILE: {}\n", piVar2->fd);

#ifdef PLATFORM_PS2
		if ((pDebugBank->openFlags & 8) == 0) {
			if ((pDebugBank->openFlags & 0x10) != 0) {
				sceCdDiskReady(0);
				sceCdStStop();
				sceSifFreeIopHeap(this->iopBuf);
				this->iopBuf = (char*)0x0;
			}
			do {
				sceCdDiskReady(0);
				iVar2 = sceClose(piVar2->fd);
			} while (iVar2 < 0);
			bVar1 = true;
		}
		else {
			sceCdDiskReady(0);
			bVar1 = true;
		}
#else
		fclose(g_FileDescriptors[piVar2->fd]);
		bVar1 = true;
#endif
	}
	return bVar1;
}

uint edCFiler_CDVD::read(edFILEH* pDebugBank, char* destination, uint requiredSize)
{
	CDFileContainer* paVar1;
	int iVar2;
	uint readBytes;
	uint readError;
#ifdef PLATFORM_PS2
	sceCdRMode local_4;
#endif

	paVar1 = (CDFileContainer*)GetInternalData_0025b2e0(pDebugBank);
	if (paVar1 == (CDFileContainer*)0x0) {
		readBytes = 0;
	}
	else {
		MY_LOG("edCFiler_CDVD::read FILE: {} Offset: %u\n", paVar1->fd, requiredSize);

		if ((pDebugBank->openFlags & 8) == 0) {
			iVar2 = 0;
			if ((pDebugBank->openFlags & 0x10) == 0) {
				while (iVar2 == 0) {
#ifdef PLATFORM_PS2
					sceCdDiskReady(0);
					iVar2 = sceRead(paVar1->fd, destination, requiredSize);
#else
					iVar2 = fread(destination, 1, requiredSize, g_FileDescriptors[paVar1->fd]);
#endif
				}
			}
			else {
#ifdef PLATFORM_PS2
				do {
					readBytes = sceCdStRead(requiredSize >> 0xb, (u_int*)destination, 1, &readError);
				} while ((readBytes & 0x1fffff) == 0);
#else
				IMPLEMENTATION_GUARD();
#endif
			}
		}
		else {
#ifdef PLATFORM_PS2
			local_4.trycount = 0;
			local_4.spindlctrl = 1;
			readBytes = requiredSize >> 0xb;
			local_4.datapattern = 0;
			if ((requiredSize & 0x7ff) != 0) {
				readBytes = readBytes + 1;
			}
			do {
				sceCdDiskReady(0);
				iVar2 = sceCdRead((paVar1->file).lsn + ((uint)pDebugBank->seekOffset >> 0xb), readBytes, destination, &local_4);
			} while (iVar2 == 0);
#else
			iVar2 = fread(destination, 1, requiredSize, g_FileDescriptors[paVar1->fd]);
			assert(iVar2 > 0);
#endif
		}
		readBytes = 1;
	}
	return readBytes;
}

bool edCFiler_CDVD::seek(edFILEH* pDebugBank)
{
	bool bVar1;
	CDFileContainer* piVar2;
	int iVar2;

	piVar2 = (CDFileContainer*)GetInternalData_0025b2e0(pDebugBank);
	if (piVar2 == (CDFileContainer*)0x0) {
		bVar1 = false;
	}
	else {
		MY_LOG("edCFiler_CDVD::seek FILE: {} Offset: {}\n", piVar2->fd, pDebugBank->seekOffset);

		if ((pDebugBank->openFlags & 8) == 0) {
			do {
#ifdef PLATFORM_PS2
				sceCdDiskReady(0);
				iVar2 = sceLseek(piVar2->fd, pDebugBank->seekOffset, 0);
				sceCdSync(0);
#else 
				iVar2 = fseek(g_FileDescriptors[piVar2->fd], pDebugBank->seekOffset, 0);
#endif
			} while (iVar2 == -1);
		}
		bVar1 = true;
	}
	return bVar1;
}

bool edCFiler_CDVD::isnowaitcmdend(edCFiler_28_Internal* pEdFilerInternal)
{
	bool bVar1;
	CDFileContainer* pInternalData;
	int result;
	byte* pbVar2;
	int iVar3;
	EBankAction action;

	pInternalData = (CDFileContainer*)GetInternalData_0025b2e0(pEdFilerInternal->pDataBank);
	if (pInternalData == (CDFileContainer*)0x0) {
		bVar1 = false;
	}
	else {
		action = pEdFilerInternal->nextAction;
		if (((action == BANK_ACTION_3) || (action == READ_STREAM)) || (action == SEEK)) {
#ifdef PLATFORM_PS2
			iVar3 = sceCdSync(1);
			if (iVar3 == 1) {
				bVar1 = false;
			}
			else {
				field_0x88d = 1;
				field_0x88c = 0;
				result = sceCdGetError();
				if (result != 0x0) {
					/* Errors:  */
					edDebugPrintf("Errors: ");
					if (result == 0x1) {
						edDebugPrintf("Abort command received\n");
					}
					else {
						if (result == 0x13) {
							/* Processing Command\n */
							edDebugPrintf("Processing command\n");
						}
						else {
							if (result == 0x14) {
								/* Not appropriate for disc in drive\n */
								edDebugPrintf("Not appropriate for disc in drive\n");
								field_0x88d = 0;
							}
							else {
								if (result == 0x30) {
									/* Problem occured during read\n */
									edDebugPrintf("Problem occured during read\n");
								}
								else {
									if (result == 0x31) {
										/* Cover opened during playback\n */
										edDebugPrintf("Cover opened during playback\n");
										field_0x88c = 1;
										edSysHandlersCall(edFileHandlers.mainIdentifier,
											edFileHandlers.entries,
											edFileHandlers.maxEventID, 0xd, (void*)0x0);
									}
									else {
										if (result == 0x32) {
											/* Outermost track reached during playback\n */
											edDebugPrintf("Outermost track reached during playback\n");
										}
										else {
											if (result == 0x0) {
												/* No error\n */
												edDebugPrintf("No error\n");
											}
											else {
												if (result == -1) {
													/* sceCdGetError() function issue failed\n */
													edDebugPrintf("sceCdGetError() function issue failed\n");
													edSysHandlersCall(edFileHandlers.mainIdentifier,
														edFileHandlers.entries,
														edFileHandlers.maxEventID, 0xd, (void*)0x0);
												}
												else {
													/* unknown error\n */
													edDebugPrintf("unknown error\n");
												}
											}
										}
									}
								}
							}
						}
					}
					edSysHandlersCall(edFileHandlers.mainIdentifier,
						edFileHandlers.entries,
						edFileHandlers.maxEventID, 0xf, (void*)0x0);
				}
				bVar1 = true;
			}
#else
			bVar1 = true;
#endif
		}
		else {
			if (action == CLOSE) {
				iVar3 = 0;
				pbVar2 = this->aContainerInUse;
				do {
					if ((CDFileContainer*)(pbVar2 + 0x10) == pInternalData) {
						this->aContainerInUse[iVar3] = 0;
						memset(this->aFileContainers + iVar3, 0, sizeof(CDFileContainer));
						break;
					}
					iVar3 = iVar3 + 1;
					pbVar2 = pbVar2 + 0x28;
				} while (iVar3 < 0x10);
				bVar1 = true;
			}
			else {
				if (action == LOAD) {
					bVar1 = true;
				}
				else {
					bVar1 = true;
				}
			}
		}
	}
	return bVar1;
}

char* sz_CDVD_Toc_Init_00431310 = "edCFiler_CDVD_Toc::Initialize: ** Test of CheckISO\n";
char* sz_CDVD_Toc_Init_HasISO_00431390 = "edCFiler_CDVD_Toc::Initialize: ISO Image Found\n\n";
char* sz_CDVD_Toc_Init_NoISO_00431350 = "edCFiler_CDVD_Toc::Initialize: CD does not have an ISO image\n";
char* sz_CD001_00431348 = "CD001";

bool edCFiler_CDVD_Toc::Initialize(edCdlFolder* pFolder, int size)
{
#if defined(PLATFORM_PS2)
	bool bVar3;
	int iVar4;
	uint uVar5;
	long lVar6;
	char uStack2112[2108];
	char* auStack1954 = uStack2112 + 158;
	char* auStack1946 = uStack2112 + 158 + 8;
	sceCdRMode local_4;
	uint uVar1;
	uint uVar2;

	if (pFolder == (edCdlFolder*)0x0) {
		bLoaded = 0;
		bVar3 = true;
		pBaseFolder = (edCdlFolder*)0x0;
		pCurrentFolder = (edCdlFolder*)0x0;
		offset = 0;
	}
	else {
		iVar4 = sceCdGetDiskType();
		if (iVar4 == 0) {
			bVar3 = false;
		}
		else {
			/* edCFiler_CDVD_Toc::Initialize: ** Test of CheckISO\n */
			printf(sz_CDVD_Toc_Init_00431310);
			local_4.trycount = 0;
			local_4.spindlctrl = 1;
			local_4.datapattern = 0;
			do {
				sceCdSync(0);
				iVar4 = sceCdRead(0x10, 1, &uStack2112, &local_4);
				lVar6 = (long)iVar4;
			} while (lVar6 != 1);
			sceCdSync(0);
			uVar5 = (uint)lVar6;
			/* CD001 */
			iVar4 = strncmp(uStack2112 + 1, sz_CD001_00431348, 5);
			if (iVar4 == 0) {
				/* "edCFiler_CDVD_Toc::Initialize: ISO Image Found\n\n" */
				printf(sz_CDVD_Toc_Init_HasISO_00431390);
				pBaseFolder = pFolder;
				pCurrentFolder = (edCdlFolder*)(pFolder->name + size);
				offset = size;
				memset(pBaseFolder, 0, sizeof(edCdlFolder));
				uVar2 = (uint)auStack1954 & 3;
				uVar1 = (uint)(auStack1954 + 3) & 3;
				uVar1 = *(int*)(auStack1954 + 3 + -uVar1) << (3 - uVar1) * 8 | (uVar5 & -1 << (4 - uVar2) * 8 | *(uint*)(auStack1954 + -uVar2) >> uVar2 * 8) & 0xffffffffU >> (uVar1 + 1) * 8;
				pBaseFolder->lsn = uVar1;
				uVar2 = (uint)auStack1946 & 3;
				uVar5 = (uint)(auStack1946 + 3) & 3;
				pBaseFolder->size =
					*(int*)(auStack1946 + 3 + -uVar5) << (3 - uVar5) * 8 | (uVar1 & -1 << (4 - uVar2) * 8 | *(uint*)(auStack1946 + -uVar2) >> uVar2 * 8) & 0xffffffffU >> (uVar5 + 1) * 8;
				pNextFreeEntry = pBaseFolder + 1;
				bVar3 = InitTOC_0025d530(pBaseFolder);
				if (bVar3 == false) {
					bVar3 = false;
				}
				else {
					bVar3 = true;
					bLoaded = 1;
				}
			}
			else {
				/* edCFiler_CDVD_Toc::Initialize: CD does not have an ISO image\n */
				printf(sz_CDVD_Toc_Init_NoISO_00431350);
				bVar3 = false;
			}
		}
	}
	return bVar3;
#else
	return true;
#endif
}

bool edCFiler_CDVD_Toc::InitTOC_0025d530(edCdlFolder* pFolder)
{
#if defined(PLATFORM_PS2)
	char cVar1;
	uint uVar2;
	uint uVar3;
	bool bVar4;
	int iVar5;
	edCdlFolder** ppeVar6;
	uint* puVar7;
	byte bVar8;
	uint* puVar9;
	undefined4* puVar10;
	edCdlFolder* peVar11;
	edCdlFolder* peVar12;
	uint uVar13;
	edCdlFolder* peVar14;
	byte* pbVar15;
	uint uVar16;
	uint uVar17;
	edCdlFolder* peVar18;
	edCdlFolder* peVar19;
	edCdlFolder* peVar20;
	byte local_840[2108];
	sceCdRMode local_4;

	MY_LOG("Processing folder: {}\n", pFolder->name);

	pFolder->pChild = this->pNextFreeEntry;
	uVar16 = pFolder->size + 0x7ff >> 0xb;
	uVar17 = 0;
	if (uVar16 != 0) {
		do {
			uVar13 = pFolder->lsn;
			local_4.trycount = 0;
			local_4.spindlctrl = 1;
			local_4.datapattern = 0;
			do {
				sceCdSync(0);
				iVar5 = sceCdRead(uVar13 + uVar17, 1, local_840, &local_4);
			} while (iVar5 != 1);
			sceCdSync(0);
			pbVar15 = local_840;
			bVar8 = local_840[0];
			while (bVar8 != 0) {
				if ((pbVar15[0x20] != 1) || (1 < pbVar15[0x21])) {
					peVar14 = pFolder->pChild + pFolder->folderCount;
					if ((pbVar15[0x19] & 2) == 0) {
						uVar13 = pFolder->fileCount;
						pFolder->fileCount = uVar13 + 1;
						peVar14 = peVar14 + uVar13;
						peVar14->flag = (uint)pbVar15[0x19];
						*(byte*)&peVar14->pChild = pbVar15[0x17];
						*(byte*)((int)&peVar14->pChild + 1) = pbVar15[0x16];
						*(byte*)((int)&peVar14->pChild + 2) = pbVar15[0x15];
						*(byte*)&peVar14->pParent = pbVar15[0x14];
						*(byte*)((int)&peVar14->pParent + 1) = pbVar15[0x13];
						*(ushort*)((int)&peVar14->pParent + 2) = pbVar15[0x12] + 0x76c;
						*(undefined*)&peVar14->folderCount = *(undefined*)&peVar14->pChild;
						*(undefined*)((int)&peVar14->folderCount + 1) = *(undefined*)((int)&peVar14->pChild + 1);
						*(undefined*)((int)&peVar14->folderCount + 2) = *(undefined*)((int)&peVar14->pChild + 2);
						*(undefined*)((int)&peVar14->folderCount + 3) = *(undefined*)((int)&peVar14->pChild + 3);
						*(undefined*)&peVar14->fileCount = *(undefined*)&peVar14->pParent;
						*(undefined*)((int)&peVar14->fileCount + 1) = *(undefined*)((int)&peVar14->pParent + 1);
						*(undefined2*)((int)&peVar14->fileCount + 2) = *(undefined2*)((int)&peVar14->pParent + 2);
					}
					else {
						uVar13 = pFolder->fileCount;
						if (uVar13 != 0) {
							peVar12 = peVar14 + (uVar13 - 1);
							puVar10 = &peVar14[uVar13 - 1].field_0xc;
							puVar9 = &peVar14[uVar13 - 1].lsn;
							puVar7 = &peVar14[uVar13 - 1].size;
							ppeVar6 = &peVar14[uVar13 - 1].pChild;
							peVar11 = peVar14 + uVar13;
							do {
								iVar5 = 6;
								peVar18 = peVar12;
								peVar19 = peVar11;
								do {
									iVar5 = iVar5 + -1;
									cVar1 = peVar18->name[1];
									peVar19->name[0] = peVar18->name[0];
									peVar18 = (edCdlFolder*)(peVar18->name + 2);
									peVar19->name[1] = cVar1;
									peVar19 = (edCdlFolder*)(peVar19->name + 2);
								} while (0 < iVar5);
								uVar13 = uVar13 - 1;
								peVar12 = peVar12 + -1;
								peVar11->field_0xc = *puVar10;
								puVar10 = puVar10 + -0xb;
								peVar11->lsn = *puVar9;
								puVar9 = puVar9 + -0xb;
								peVar11->size = *puVar7;
								puVar7 = puVar7 + -0xb;
								peVar20 = ppeVar6[1];
								peVar18 = ppeVar6[2];
								peVar19 = ppeVar6[3];
								peVar11->pChild = *ppeVar6;
								peVar11->pParent = peVar20;
								peVar11->folderCount = (uint)peVar18;
								peVar11->fileCount = (uint)peVar19;
								peVar11->flag = (uint)ppeVar6[4];
								ppeVar6 = ppeVar6 + -0xb;
								peVar11 = peVar11 + -1;
							} while (uVar13 != 0);
						}
						pFolder->folderCount = pFolder->folderCount + 1;
						peVar14->pParent = pFolder;
						peVar14->pChild = (edCdlFolder*)0x0;
						peVar14->fileCount = 0;
						peVar14->folderCount = 0;
					}
					this->objCount_0x14 = this->objCount_0x14 + 1;
					bVar4 = peVar14 < this->pCurrentFolder;
					if (!bVar4) {
						return false;
					}
					uVar2 = (uint)(pbVar15 + 2) & 3;
					iVar5 = 0;
					uVar13 = (uint)(pbVar15 + 5) & 3;
					uVar2 = *(int*)(pbVar15 + 5 + -uVar13) << (3 - uVar13) * 8 | ((uint)bVar4 & -1 << (4 - uVar2) * 8 | *(uint*)(pbVar15 + 2 + -uVar2) >> uVar2 * 8) & 0xffffffffU >> (uVar13 + 1) * 8;
					peVar14->lsn = uVar2;
					uVar3 = (uint)(pbVar15 + 10) & 3;
					uVar13 = (uint)(pbVar15 + 0xd) & 3;
					peVar14->size = *(int*)(pbVar15 + 0xd + -uVar13) << (3 - uVar13) * 8 | (uVar2 & -1 << (4 - uVar3) * 8 | *(uint*)(pbVar15 + 10 + -uVar3) >> uVar3 * 8) & 0xffffffffU >> (uVar13 + 1) * 8;
					peVar14->field_0xc = 0;
					uVar13 = (uint)pbVar15[0x20];
					do {
						bVar8 = 0;
						if (uVar13 != 0) {
							bVar8 = (pbVar15 + iVar5)[0x21];
							if ((bVar8 == 0x3b) && ((pbVar15 + iVar5)[0x22] == 0x31)) {
								uVar13 = 0;
								bVar8 = 0;
							}
							else {
								uVar13 = uVar13 - 1;
							}
						}
						peVar14->name[iVar5] = bVar8;
						iVar5 = iVar5 + 1;
					} while (iVar5 < 0xc);

					MY_LOG("    Copied name: {}\n", peVar14->name);
				}
				pbVar15 = pbVar15 + *pbVar15;
				bVar8 = *pbVar15;
			}
			uVar17 = uVar17 + 1;
		} while (uVar17 < uVar16);
	}
	this->pNextFreeEntry = this->pNextFreeEntry + pFolder->folderCount + pFolder->fileCount;
	peVar14 = pFolder->pChild;
	uVar16 = 0;
	if (pFolder->folderCount != 0) {
		do {
			bVar4 = InitTOC_0025d530(peVar14);
			if (bVar4 == false) {
				return false;
			}
			uVar16 = uVar16 + 1;
			peVar14 = peVar14 + 1;
		} while (uVar16 < pFolder->folderCount);
	}
#endif
	return true;
}

edCdlFolder* edCFiler_CDVD_Toc::FindEdCFile(char* filePath)
{
	bool bVar1;
	int iVar2;
	char* folderName;
	uint uVar3;
	edCdlFolder* peVar4;
	edCdlFolder* pcVar4;
	char folderNameBuffer[512];
	char currentCharacter;

	/* seek through the file path looking for the first non \ character */
	pcVar4 = pBaseFolder;
	do {
		if (*filePath == '\\') {
			filePath = filePath + 1;
		}
		/* Point the folder name ptr at the first character of the folder name buffer */
		folderName = folderNameBuffer;
		while (true) {
			currentCharacter = *filePath;
			filePath = filePath + 1;
			/* seek through to the next slash, storing the name of the folder */
			if ((currentCharacter == '\0') || (currentCharacter == '\\')) break;
			*folderName = currentCharacter;
			folderName = folderName + 1;
		}
		/* Add null term to folder name */
		*folderName = '\0';
		/* If we have reached the end of the file path */
		peVar4 = pcVar4->pChild;
		if (currentCharacter == '\0') {
			uVar3 = 0;
			peVar4 = peVar4 + pcVar4->folderCount;
			if (pcVar4->fileCount != 0) {
				do {
					MY_LOG("File: Comparing {} to {}\n", folderNameBuffer, (char*)peVar4);
					iVar2 = edStrICmp((byte*)folderNameBuffer, (byte*)peVar4);
					if (iVar2 == 0) {
						return peVar4;
					}
					uVar3 = uVar3 + 1;
					peVar4 = peVar4 + 1;
				} while (uVar3 < pcVar4->fileCount);
			}
			return (edCdlFolder*)0x0;
		}
		/* Still in the middle of our file path */
		bVar1 = false;
		uVar3 = 0;
		if (pcVar4->folderCount != 0) {
			do {
				MY_LOG("Folder: Comparing {} to {}\n", folderNameBuffer, (char*)peVar4);
				iVar2 = edStrICmp((byte*)folderNameBuffer, (byte*)peVar4);
				if (iVar2 == 0) {
					bVar1 = true;
					pcVar4 = peVar4;
					break;
				}
				uVar3 = uVar3 + 1;
				peVar4 = peVar4 + 1;
			} while (uVar3 < pcVar4->folderCount);
		}
		if (!bVar1) {
			return (edCdlFolder*)0x0;
		}
	} while (true);
}

bool edCFiler_CDVD_Toc::LoadFromTOC(sceCdlFILE* pOutFileData, char* filePath)
{
	edCdlFolder* pFoundFile;

	pFoundFile = FindEdCFile(filePath);
	if (pFoundFile != (edCdlFolder*)0x0) {
		pOutFileData->lsn = pFoundFile->lsn;
		pOutFileData->size = pFoundFile->size;
		pOutFileData->flag = pFoundFile->flag;
		pOutFileData->date[0] = '\0';
		pOutFileData->date[1] = *(u_char*)&pFoundFile->folderCount;
		pOutFileData->date[2] = *(u_char*)((int)&pFoundFile->folderCount + 1);
		pOutFileData->date[3] = *(u_char*)((int)&pFoundFile->folderCount + 2);
		pOutFileData->date[4] = *(u_char*)&pFoundFile->fileCount;
		pOutFileData->date[5] = *(u_char*)((int)&pFoundFile->fileCount + 1);
		pOutFileData->date[6] = (u_char) * (undefined2*)((int)&pFoundFile->fileCount + 2);
		pOutFileData->date[7] = (u_char)((uint) * (ushort*)((int)&pFoundFile->fileCount + 2) >> 8);
		strcpy(pOutFileData->name, (char*)pFoundFile);
	}
	else {
		memset(pOutFileData, 0, sizeof(sceCdlFILE));
	}
	return pFoundFile != (edCdlFolder*)0x0;
}

bool edCFiler_CDVD::get_physical_filename(char* filePathOut, char* pathBuff)
{
	bool bVar1;
	int iVar2;
	char* inString;
	int iVar3;
	char* outString;
	char acStack512[512];

	if ((filePathOut == (char*)0x0) || (pathBuff == (char*)0x0)) {
		bVar1 = false;
	}
	else {
		edFilePathSplit((char*)0x0, acStack512, (char*)0x0, (char*)0x0, pathBuff);
		iVar2 = edStrCopy(filePathOut, sz_cdrom_00431268);
		outString = filePathOut + iVar2;
		iVar2 = edStrCopy(outString, acStack512);
		inString = edFilePathGetFilePath(pathBuff);
		iVar3 = edStrCopyUpper(outString + iVar2, inString);
		edStrCat(outString + iVar2 + iVar3, sz_DriveLetter_00431270);
		bVar1 = true;
	}
	return bVar1;
}

void _edFileCDVDAddFiler(void)
{
	edFilerList.add_filer(&edFiler_CDVD);
	return;
}
