#include "ed3D.h"
#include "edMem.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "edDlist.h"

#if defined(PLATFORM_PS2)
#include <eekernel.h>
#endif

char* s_ed3D_Initialsation_004333a0 = "ed3D Initialsation\n";

ed3D_Params::ed3D_Params()
	: meshHeaderCountB(0x3E0)
	, initialStaticMeshMasterCount(0xF)
	, field_0x8(0x10)
	, meshHeaderCountBAlt(0x2800)
	, meshHeaderCountA(0x1F4)
	, field_0x14(0xFA)
	, field_0x18(0x7D0)
	, meshHeaderCountAA(0xA)
	, meshHeaderCountBB(0x19)
	, bEnableProfile(0x0)
	, field_0x25(0x0)
	, field_0x26(0x64)
	, field_0x27(0x0)
	, field_0x28(0xC8)
	, field_0x2c(0x2)
	, field_0x2d(0x1)
	, field_0x2e(0x0)
	, field_0x2f(0x0)
	, meshTransformDataCount(0x3E8)
	, field_0x34(0x0)
{

}

namespace ed3D
{
	TextureInfoSmall* TexturePool_004491cc;
	int LoadedTextureCount_004491c4;
	
	ulong* LoadedTextureData_004491e0;
	ulong* LoadedTextureDataBase_004491d8;

	ed3D_Params Params_00424e30;

	void Init(void)
	{
		TextureInfoSmall* pTVar1;
		uint uVar2;
		int size;

		PrintString(s_ed3D_Initialsation_004333a0);
		//FUN_002936b0();
		//DAT_004491b0 = 1;
		//CameraPanMasterHeaderA_004491d0 = AllocateCameraAndMesh_00290a10(1, Params_00424e30.meshHeaderCountAA, 1);
		//CameraPanMasterHeaderB_004491d4 = AllocateCameraAndMesh_00290a10(1, Params_00424e30.meshHeaderCountBB, 1);
		//PTR_DAT_004491c8 = (undefined*)edMemAlloc(1, Params_00424e30.meshHeaderCountAA << 6);
		//memset(PTR_DAT_004491c8, 0, Params_00424e30.meshHeaderCountAA << 6);
		TexturePool_004491cc = (TextureInfoSmall*)edMemAlloc(1, Params_00424e30.meshHeaderCountBB * sizeof(TextureInfoSmall));
		memset(TexturePool_004491cc, 0, Params_00424e30.meshHeaderCountBB * 0x30);
		//pTVar1 = TexturePool_004491cc;
		//INT_004491b4 = 0;
		LoadedTextureCount_004491c4 = 0;
		//*(undefined8*)TexturePool_004491cc = 0x3fff00000000;
		//*(undefined8*)&pTVar1->textureHeaderStart = 0x50;
		//*(undefined8*)&pTVar1->textureBufferStart = 0x7ffe000000000;
		//memset(&DAT_0048c3e0, 0, 0x200);
		//cameraPanStaticMasterArray_00449378 = (StaticMeshMaster*)edMemAlloc(1, Params_00424e30.initialStaticMeshMasterCount * 0x1c0 + 1);
		//memset(cameraPanStaticMasterArray_00449378, 0, Params_00424e30.initialStaticMeshMasterCount * 0x1c0);
		//PTR_MeshTransformParentHeader_0044937c = AllocateCameraPanHeader_00290af0(Params_00424e30.field_0x8 + Params_00424e30.initialStaticMeshMasterCount * 3, 1);
		//PTR_DAT_00449380 = (undefined*)edMemAlloc(1, Params_00424e30.field_0x8 * 0x30);
		//memset(PTR_DAT_00449380, 0, Params_00424e30.field_0x8 * 0x30);
		//for (uVar2 = 0; uVar2 < Params_00424e30.initialStaticMeshMasterCount; uVar2 = uVar2 + 1) {
		//	CameraPanMasterHeader::Setup_00290bf0(&cameraPanStaticMasterArray_00449378[uVar2].field_0x30, PTR_MeshTransformParentHeader_0044937c, 0, 0);
		//	CameraPanMasterHeader::Setup_00290bf0(&cameraPanStaticMasterArray_00449378[uVar2].field_0x10, PTR_MeshTransformParentHeader_0044937c, 0, 0);
		//}
		///* Add render handler */
		//edSysHandlers::edSysHandlersAdd(edSysHandlersNodeParent_0048cee0.pNodeTable, edSysHandlersNodeParent_0048cee0.pTypeArray_0x4, g_SysHandlersMainMaxEventID_0048cf10, ESHT_RenderScene, RenderScene, 1, 1);
		//edSysHandlers::edSysHandlersAdd
		//(edSysHandlersNodeParent_0048cee0.pNodeTable, edSysHandlersNodeParent_0048cee0.pTypeArray_0x4, g_SysHandlersMainMaxEventID_0048cf10, ESHT_Sound, EdSoundHandler_002b2730, 1, 0);
		//if (Params_00424e30.meshHeaderCountBAlt < 0x180) {
		//	Params_00424e30.meshHeaderCountBAlt = Params_00424e30.meshHeaderCountBAlt + 0x180;
		//}
		LoadedTextureDataBase_004491d8 = (ulong*)edMemAlloc(1, Params_00424e30.meshHeaderCountBAlt);
		//PTR_DAT_004491e4 = PTR_PTR_DAT_004491d8 + Params_00424e30.meshHeaderCountBAlt;
		//size = (Params_00424e30.field_0x28 * 2 + Params_00424e30.field_0x18 + 0x8c0 + Params_00424e30.meshHeaderCountA * 0x38 + Params_00424e30.field_0x34 * 8) * 0x10;
		LoadedTextureData_004491e0 = LoadedTextureDataBase_004491d8;
		//PTR_DAT_00449248 = (undefined*)edMemAlloc(1, size);
		//memset(PTR_DAT_00449248, 0, size);
		//PTR_DAT_00449250 = PTR_DAT_00449248 + size;
		//PTR_DAT_0044924c = PTR_DAT_00449248;
		//PTR_DAT_004491b8 = (undefined*)FUN_0029ce60(16000);
		//PTR_DAT_004491c0 = PTR_DAT_004491b8 + 16000;
		//PTR_DAT_004491bc = PTR_DAT_004491b8;
		//FUN_002b12a0(Params_00424e30.field_0x14, Params_00424e30.meshHeaderCountA);
		//FUN_002ad860();
		//InitFunc_002b13b0(Params_00424e30.field_0x14, Params_00424e30.meshHeaderCountB);
		//FUN_0029c220(Params_00424e30.field_0x18 + Params_00424e30.field_0x34 * 8, Params_00424e30.field_0x28);
		//thunk_FUN_002ab6d0();
		//FUN_0029cf60();
		//if (DAT_004491f8 == '\0') {
		//	DAT_004491f8 = '\x01';
		//}
		//g_RenderActiveCameraObj_006f7d80 = (CameraObj_28*)0x0;
		//FUN_0029ced0();
		//ResetMemoryPtr_0029c060();
		//FUN_002b1140();
		//RenderSetupFunc_002b1220();
		//FUN_002b6b20();
		//InitFunc_002abd00();
		//FUN_0029ffd0(&DAT_0048c380, 1);
		//FUN_0029ffd0(&DAT_0048c370, 1);
		//FUN_0029ffd0(&DAT_0048c3a0, 1);
		//FUN_0029ffd0(&DAT_0048c390, 1);
		//FUN_0029ffd0(&DAT_0048c3c0, 1);
		//FUN_0029ffd0(&DAT_0048c3b0, 1);
		//DAT_0044897c = 0xffffffff;
		//PTR_DAT_004491dc = PTR_DAT_004491e0;
		//if (Params_00424e30.bEnableProfile == 1) {
		//	g_3DRendProfileObjIndex_004491e8 = CreateNamedProfileObj_00281340(1, 0, 0, 0x80, s_3DRend_004333b8);
		//	g_3DFlushProfileObjIndex_004491ec = CreateNamedProfileObj_00281340(1, 0x80, 0, 0, s_3DFlush_004333c0);
		//}
		//PrintString(s_ed3D_Init_total_allocation_ammou_004333d0,
		//	Params_00424e30.meshHeaderCountA * 0x220 +
		//	Params_00424e30.field_0x18 * 0x10 + Params_00424e30.meshHeaderCountBAlt + Params_00424e30.meshHeaderCountAA * 0x40 + Params_00424e30.meshHeaderCountBB * 0x30 +
		//	Params_00424e30.field_0x14 * 0x60);
		//return;
	}

	char* VerifyFileBufferRead(char* pBuffStart, char* pBuffEnd)
	{
		/* Checks that the end of the file is greater than the start of the file */
		if ((pBuffEnd != (char*)0x0) && (pBuffEnd <= pBuffStart)) {
			pBuffStart = (char*)0x0;
		}
		return pBuffStart;
	}

	int* GetNextFileSection(char* param_1, int* param_2)
	{
		int* piVar1;

		if ((param_2 == (int*)0x0) || (piVar1 = (int*)0x0, (int*)(param_1 + *(int*)(param_1 + 8)) < param_2)) {
			piVar1 = (int*)(param_1 + *(int*)(param_1 + 8));
		}
		return piVar1;
	}

	void LoadTextureMajorSections(TextureInfoSmall* textureInfoObj, char* fileBuffer, int length)
	{
		int* piVar1;
		int* nextSection;
		int readValue;

		/* Seek through the buffer */
		for (piVar1 = (int*)VerifyFileBufferRead(fileBuffer, fileBuffer + length); piVar1 != (int*)0x0; piVar1 = GetNextFileSection((char*)piVar1, (int*)(fileBuffer + length))) {
			readValue = *piVar1;
			/* Check the first value in the buffer is *2D* */
			if (readValue == 0x2a44322a) {
				/* Set the readValue to be the last section of the texture header */
				readValue = piVar1[2];
				textureInfoObj->textureHeaderStart = (char*)piVar1;
				for (nextSection = (int*)VerifyFileBufferRead((char*)(piVar1 + 4), (char*)(int*)((char*)piVar1 + readValue)); nextSection != (int*)0x0;
					nextSection = GetNextFileSection((char*)nextSection, (int*)((char*)piVar1 + readValue))) {
					/* Check if the value in the buffer is 'MATA' */
					if (*nextSection == 0x4154414d) {
						textureInfoObj->materialBufferStart = (char*)(nextSection + 4);
					}
				}
			}
			else {
				/* Check if the value in the buffer is 'ANMA' */
				if (readValue == 0x414d4e41) {
					textureInfoObj->animationBufferStart = (char*)piVar1;
				}
				else {
					/* Check if the value in the buffer is 'PALL' */
					if (readValue == 0x4c4c4150) {
						textureInfoObj->palleteBufferStart = (char*)piVar1;
					}
					else {
						/* Check if the value in the buffer is 'T2DA' */
						if (readValue == 0x41443254) {
							textureInfoObj->textureBufferStart = (char*)piVar1;
						}
					}
				}
			}
		}
		return;
	}


	int* SetTextureSubsectionPointers(char* fileBuffer, int length, char* fileBufferCopy, int lengthCopy)
	{
		int iVar1;
		int* piVar2;
		int* nextSectionBuffer;
		int* nextSubSection;
		uint uVar3;
		uint uVar4;
		char* actualFileStart;
		char* pcVar5;
		int subSectionValue;

		pcVar5 = fileBuffer + (-lengthCopy - (long long)fileBufferCopy);
		actualFileStart = fileBufferCopy + -0x10;
		if (fileBuffer == fileBufferCopy) {
			pcVar5 = (char*)0x0;
		}
		nextSectionBuffer = (int*)VerifyFileBufferRead(fileBuffer, fileBuffer + length);
		while (true) {
			if (nextSectionBuffer == (int*)0x0) {
				return (int*)0x0;
			}
			/* Check if the value in the buffer is 'REAA'
			   If it is, and the next value isn't 1974, exit the loop (we reached the end of the file) */
			if ((*nextSectionBuffer == 0x41414552) && (*(short*)(nextSectionBuffer + 1) != 0x7b6)) break;
			nextSectionBuffer = GetNextFileSection((char*)nextSectionBuffer, (int*)(fileBuffer + length));
		}
		/* Jump ahead in the buffer */
		iVar1 = nextSectionBuffer[2];
		/* Mark the REAA buffer with 1974 */
		*(undefined2*)(nextSectionBuffer + 1) = 0x7b6;
		piVar2 = (int*)VerifyFileBufferRead((char*)(nextSectionBuffer + 4), (char*)(int*)((char*)nextSectionBuffer + iVar1));
		pcVar5 = actualFileStart + (int)pcVar5;
		for (; piVar2 != (int*)0x0; piVar2 = GetNextFileSection((char*)piVar2, (int*)((char*)nextSectionBuffer + iVar1))) {
			subSectionValue = *piVar2;
			nextSubSection = piVar2 + 4;
			uVar3 = piVar2[2] - 0x10U >> 2;
			/* Check if the value in the buffer is 'REAL' */
			if (subSectionValue == 0x4c414552) {
				for (uVar4 = 0; uVar4 < (uVar3 & 0xffff); uVar4 = uVar4 + 1 & 0xffff) {
					/* Jump forward one section */
					if (*nextSubSection != 0) {
						/* Store a pointer to the next sub section */
						*(char**)(actualFileStart + *nextSubSection) = actualFileStart + (int)*(char**)(actualFileStart + *nextSubSection);
					}
					nextSubSection = nextSubSection + 1;
				}
			}
			else {
				/* Is current value RSTR */
				if (subSectionValue == 0x52545352) {
					for (uVar4 = 0; uVar4 < (uVar3 & 0xffff); uVar4 = uVar4 + 1 & 0xffff) {
						if (*nextSubSection != 0) {
							/* Store a pointer to the next sub section */
							*(char**)(pcVar5 + *nextSubSection) = actualFileStart + (int)*(char**)(pcVar5 + *nextSubSection);
						}
						nextSubSection = nextSubSection + 1;
					}
				}
				else {
					/* Is current value ROBJ */
					if (subSectionValue == 0x4a424f52) {
						for (uVar4 = 0; uVar4 < (uVar3 & 0xffff); uVar4 = uVar4 + 1 & 0xffff) {
							if (*nextSubSection != 0) {
								/* Store a pointer to the next sub section */
								*(char**)(actualFileStart + *nextSubSection) = pcVar5 + (int)*(char**)(actualFileStart + *nextSubSection);
							}
							nextSubSection = nextSubSection + 1;
						}
					}
					else {
						/* Is current value RGEO */
						if (subSectionValue == 0x4f454752) {
							for (uVar4 = 0; uVar4 < (uVar3 & 0xffff); uVar4 = uVar4 + 1 & 0xffff) {
								if (*nextSubSection != 0) {
									/* Store a pointer to the next sub section */
									*(char**)(pcVar5 + *nextSubSection) = pcVar5 + (int)*(char**)(pcVar5 + *nextSubSection);
								}
								nextSubSection = nextSubSection + 1;
							}
						}
					}
				}
			}
		}
		return nextSectionBuffer;
	}

	void TextureSetup(TextureInfoSmall* textureInfoObj, ulong mode)
	{
		char* pcVar1;
		int iVar2;
		int iVar3;
		int* piVar4;
		int* piVar5;

		pcVar1 = textureInfoObj->textureHeaderStart;
		iVar2 = *(int*)(pcVar1 + 8);
		for (piVar4 = (int*)VerifyFileBufferRead(pcVar1 + 0x10, pcVar1 + iVar2); piVar4 != (int*)0x0; piVar4 = GetNextFileSection((char*)piVar4, (int*)(pcVar1 + iVar2))) {
			if (*piVar4 == 0x4154414d) {
				iVar3 = piVar4[2];
				for (piVar5 = (int*)VerifyFileBufferRead((char*)(piVar4 + 4), (char*)(int*)((int)piVar4 + iVar3)); piVar5 != (int*)0x0;
					piVar5 = GetNextFileSection((char*)piVar5, (int*)((int)piVar4 + iVar3))) {
					/* .TAM */
					if (*piVar5 == 0x2e54414d) {
#ifdef PLATFORM_WINDOWS
						assert(false); // untested
#endif
						//GenerateRenderingCommands_0029ffd0((TamPacked*)(piVar5 + 4), mode);
					}
				}
			}
		}
		return;
	}

	void LoadTextureLayer(TextureInfoSmall* textureInfoObj, ulong mode)
	{
		int iVar1;
		ulong* puVar2;
		LayerHeaderPacked* pBuffer;
		char* fileBufferStart;
		char* texHeaderStart;

		/* Can point at 004DA040 */
		puVar2 = ed3D::LoadedTextureData_004491e0;
		texHeaderStart = textureInfoObj->textureHeaderStart;
		iVar1 = *(int*)(texHeaderStart + 8);
		if ((textureInfoObj != (TextureInfoSmall*)0x0) && (fileBufferStart = textureInfoObj->textureFileBufferStart, (*(uint*)(fileBufferStart + 4) & 1) == 0)) {
#ifdef PLATFORM_WINDOWS
			assert(false); // untested
#endif
			for (pBuffer = (LayerHeaderPacked*)VerifyFileBufferRead(texHeaderStart + 0x10, texHeaderStart + iVar1); pBuffer != (LayerHeaderPacked*)0x0;
				pBuffer = (LayerHeaderPacked*)GetNextFileSection((char*)pBuffer, (int*)(texHeaderStart + iVar1))) {
				/* Check if the value in the buffer is 'LAYA' */
				if (*(int*)pBuffer == 0x4159414c) {
					//TextureSetup(pBuffer, textureInfoObj);
				}
			}
			TextureSetup(textureInfoObj, mode);
			textureInfoObj->textureFileLengthB = (int)ed3D::LoadedTextureData_004491e0 + (textureInfoObj->textureFileLengthB - (int)puVar2);
			*(uint*)(fileBufferStart + 4) = *(uint*)(fileBufferStart + 4) | 1;
		}
		return;
	}

	TextureInfoSmall* LoadTextureFromBuffer(char* fileBufferStart, int fileLength, int* outInt, TextureInfoSmall* pTextureInfo, ulong param_5)
	{
		bool bVar1;
		TextureInfoSmall* pOutTexture;
		char* fileBuffer;
		TextureInfoSmall* pTexturePool;

		/* Find some free area in the buffer? */
		pOutTexture = pTextureInfo;
		pTexturePool = TexturePool_004491cc;
		if (pTextureInfo == (TextureInfoSmall*)0x0) {
			while (pOutTexture = pTexturePool, pOutTexture->textureFileBufferStart != (char*)0x0) {
				pTexturePool = pOutTexture + 1;
			}
		}
		if ((pOutTexture == (TextureInfoSmall*)0x0) || (memset(pOutTexture, 0, 0x30), fileBufferStart == (char*)0x0)) {
			bVar1 = pOutTexture != pTextureInfo;
			pOutTexture = (TextureInfoSmall*)0x0;
			if (bVar1) {
				LoadedTextureCount_004491c4 = LoadedTextureCount_004491c4 + -1;
				pOutTexture = (TextureInfoSmall*)0x0;
			}
		}
		else {
			/* Offset the file pointer by 0x10 */
			fileBuffer = fileBufferStart + 0x10;
			LoadTextureMajorSections(pOutTexture, fileBuffer, fileLength + -0x10);
			SetTextureSubsectionPointers(fileBuffer, fileLength + -0x10, fileBuffer, fileLength + -0x10);
			pOutTexture->textureFileBufferStart = fileBufferStart;
			pOutTexture->textureFileLengthA = fileLength;
			pOutTexture->textureFileLengthB = fileLength;
			*outInt = fileLength - pOutTexture->textureFileLengthB;
			LoadTextureLayer(pOutTexture, param_5);
		}
		return pOutTexture;
	}

	char* GetMaterialBufferStart(TextureInfoSmall* pTextureInfo, int count)
	{
		int* pBuffStart;
		int* nextSection;
		uint uVar1;
		int* piVar2;
		int* sectionEnd;

		pBuffStart = (int*)pTextureInfo->materialBufferStart;
		/* Ensure we are trying to read something from the 'MATA' section */
		if (pBuffStart[-4] == 0x4154414d) {
			/* Work out the section end address */
			sectionEnd = (int*)((char*)(pBuffStart + -4) + pBuffStart[-2]);
			for (nextSection = (int*)VerifyFileBufferRead((char*)pBuffStart, (char*)sectionEnd); nextSection != (int*)0x0; nextSection = GetNextFileSection((char*)nextSection, sectionEnd)) {
				/* Check the first value in the buffer is *MAT.* */
				if (*nextSection == 0x2e54414d) {
					if ((count == 0) && (*pBuffStart == 0x48534148)) {
						/* Check the first value in the buffer is *HASH* */
						piVar2 = pBuffStart + 4;
						uVar1 = (uint)((unsigned long long)(pBuffStart + -4) + (pBuffStart[-1] - (unsigned long long)piVar2)) >> 4;
						for (; uVar1 != 0; uVar1 = uVar1 - 1) {
#ifdef PLATFORM_PS2
							if ((int*)piVar2[2] == nextSection) {
#else
							unsigned long long* B = (unsigned long long*)piVar2;
							if ((int*)B[1] == nextSection) {
#endif
								return (char*)piVar2;
							}
							piVar2 = piVar2 + 4;
						}
					}
					count = count + -1;
				}
				/* Jump to the next section */
			}
		}
		return (char*)0x0;
	}

	void* GetMaterialInfo(MaterialInfo* outObj, char* materialBuffer, TextureInfoSmall* textureInfoObj, int mode)
	{
		void* pvVar1;
		DisplayList_0x10* someGlobalBuffer;
		uint counter;

		counter = 0;
		pvVar1 = memset(outObj, 0, sizeof(MaterialInfo));
#ifdef PLATFORM_PS2
		outObj->matSubsectionStart = (char*)(*(int*)(materialBuffer + 8) + 0x10);
#else
		outObj->matSubsectionStart = (char*)(*(unsigned long long*)(materialBuffer + 8) + 0x10);
#endif
		while (true) {
			if ((*(long*)(edDlist::g_DisplayListPtr_0044965c + counter) == 0) || (edDlist::g_DisplayListObjCount_004250e0 - 1U <= counter)) break;
			counter = counter + 1;
		}
		someGlobalBuffer = edDlist::g_DisplayListPtr_0044965c + counter;
		*(undefined8*)someGlobalBuffer = *(undefined8*)materialBuffer;
		someGlobalBuffer->field_0x8 = materialBuffer;
		outObj->Length = counter;
		edDlist::MaterialDisplayListCount_00449648 = edDlist::MaterialDisplayListCount_00449648 + 1;
		outObj->mode = mode;
		outObj->textureInfo = textureInfoObj;
		return pvVar1;
	}

	MaterialInfo* GetMaterialInfoFromTexture(MaterialInfo* outObj, int count, TextureInfoSmall* textureInfoObj, int mode)
	{
		char* materialBuffer;

		materialBuffer = GetMaterialBufferStart(textureInfoObj, count);
		if (materialBuffer == (char*)0x0) {
			outObj = (MaterialInfo*)0x0;
		}
		else {
			GetMaterialInfo(outObj, materialBuffer, textureInfoObj, mode);
		}
		return outObj;
	}
}
