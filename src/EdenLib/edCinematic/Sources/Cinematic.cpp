#include "Cinematic.h"

#include "CinResCollection.h"

#include "port/pointer_conv.h"
#include "CinScene.h"

bool edCinematic::Create(edCinGameInterface& pInterface, void* pCinFileBuffer, int bufferLength)
{
	bool bVar1;
	char* dataPtr;
	char* pSeekPos;
	edResCollection* resPtr;
	char* seekPos;
	int readData;

	resPtr = (edResCollection*)0x0;
	dataPtr = (char*)0x0;
	seekPos = (char*)pCinFileBuffer;
	/* This statement is always true? */
	if ((char*)pCinFileBuffer < (char*)pCinFileBuffer + bufferLength) {
		do {
			readData = *(int*)seekPos;
			/* If the data we read from the buffer == 'DATA' */
			if (readData == 0x41544144) {
				dataPtr = seekPos + 8;
			}
			else {
				/* If the data we read from the buffer == 'RES!' */
				if (readData == 0x21534552) {
					resPtr = (edResCollection*)(seekPos + 8);
				}
				else {
					/* If the data we read from the buffer == 'CIN!' */
					if ((readData == 0x214e4943) && (*(int*)(seekPos + 8) != 0x81)) {
						return false;
					}
				}
			}
			seekPos = seekPos + *(int*)(seekPos + 4) + 8;
		} while (seekPos < (((char*)pCinFileBuffer) + bufferLength));
	}
	bVar1 = false;
	if (dataPtr != (char*)0x0) {
		/* Assign some values to our cin file object */
		this->fileStart = (int*)pCinFileBuffer;
		this->pRes = resPtr;
		this->pCinTag = (edCinematicTag*)dataPtr;
		/* Set the read and data start points */
		pSeekPos = (char*)(this->pCinTag + 1);
		for (readData = pCinTag->srcCount; readData != 0; readData = readData + -1) {
			edResCollection edResCol = { (edResCollectionTag*)resPtr };
			edCinematicSource edSrc = { (edCinematicSourceInternal*)pSeekPos };
			edSrc.Create(pInterface, edResCol);
			pSeekPos = (char*)((ulong)pSeekPos + 0xc);
		}
		bVar1 = true;
	}
	return bVar1;
}

bool edCinematic::Create(edCinGameInterface& pInterface, const char* fileName)
{
	bool uVar1;
	char* pCinResource;
	int bufferLength;

	bufferLength = 0;
	/* Load the CIN file for the cutscene
	   Example: Airlift_To_Nativ_Cin_Montage_SceneMontage.cin */
	pCinResource = (char*)pInterface.GetResource((edResCollection::RES_TYPE)0xff, 0, fileName, &bufferLength);
	if (bufferLength < 1) {
		uVar1 = false;
	}
	else {
		uVar1 = Create(pInterface, pCinResource, bufferLength);
	}
	return uVar1;
}

bool edCinematicSource::Create(edCinGameInterface& loadObj, edResCollection& resPtr)
{
	char* pcVar1;
	int* local_10;
	int* local_c;
	int sceFileLength;
	edScene local_4;
	int dataValue;

	dataValue = this->pInternal->type;
	if (dataValue == 1) {
		local_4.pTag = (edSCENEtag*)0x0;
		/* Loads the SCE asset for the cutscene
		   Example: default.sce */
		pcVar1 = resPtr.LoadResource(loadObj, this->pInternal->offset, &sceFileLength);
		/* This will load all the assets from the SCE file */
		edSCENEtag* pTag = local_4.Create(pcVar1, sceFileLength, loadObj);
		this->pInternal->pTag = STORE_SECTION(pTag);
	}
	else {
		if (dataValue == 2) {
			IMPLEMENTATION_GUARD_AUDIO(
			local_c = (int*)0x0;
			(*(code*)loadObj->vt->GetSourceAudioInterface)(loadObj, &local_c);
			pcVar1 = edResCollection::GetResFilename(resPtr, (*ppSource)->offset);
			(**(code**)(*local_c + 8))(local_c, pcVar1);
			(*ppSource)->pFileData = (char*)local_c;)
		}
		else {
			if (dataValue == 3) {
				IMPLEMENTATION_GUARD_AUDIO(
				local_10 = (int*)0x0;
				(*(code*)loadObj->vt->GetSourceSubtitleInterface)(loadObj, &local_10);
				dataValue = (*ppSource)->offset;
				pcVar1 = edResCollection::GetResFilename(resPtr, dataValue);
				(**(code**)(*local_10 + 8))(local_10, pcVar1, ((uint)(*resPtr)[dataValue * 3 + 1].pData & 0x80000000) != 0);
				(*ppSource)->pFileData = (char*)local_10;)
			}
		}
	}
	return true;
}

bool edCinematicSource::Initialize()
{
	edCinematicSourceInternal* peVar1;
	int iVar2;
	edScene eStack4;

	peVar1 = this->pInternal;
	iVar2 = peVar1->type;
	if (iVar2 == 1) {
		eStack4 = edScene((edSCENEtag*)LOAD_SECTION(peVar1->pTag));
		eStack4.Initialize();
	}
	else {
		if (iVar2 == 2) {
			IMPLEMENTATION_GUARD_AUDIO(
			(**(code**)(*(int*)peVar1->pFileData + 0xc))();)
		}
		else {
			if (iVar2 == 3) {
				IMPLEMENTATION_GUARD_AUDIO(
				(**(code**)(*(int*)peVar1->pFileData + 0xc))();)
			}
		}
	}
	return true;
}

bool edCinematicSource::Timeslice(float currentPlayTime, uint param_3)
{
	if (this->pInternal->type == 1) {
		/* Stores a pointer to the SCE file in the input variable, then returns the pointer to the
		   variable */
		edScene scene = edScene((edSCENEtag*)LOAD_SECTION(this->pInternal->pTag));
		scene.Timeslice(currentPlayTime, param_3);
	}
	return true;
}

bool edCinematic::Initialize()
{
	int* piVar1;
	int iVar2;
	int* local_4;

	piVar1 = (int*)(this->pCinTag + 1);
	for (iVar2 = this->pCinTag->srcCount; iVar2 != 0; iVar2 = iVar2 + -1) {
		edCinematicSource edSrc = { (edCinematicSourceInternal*)piVar1 };
		edSrc.Initialize();
		piVar1 = piVar1 + 3;
	}
	return true;
}

PACK(
struct CinKey {
	ushort keyType;
	ushort field_0x2;
	int keySizeBytes;
	float field_0x8;
});

PACK(
	struct CinKeyB {
	float field_0x0;
	int field_0x4;
});

PACK(
	struct CinKeyBPre {
	ushort field_0x0;
	ushort field_0x2;
});

PACK(
struct CinKeyC {
	float field_0x0;
	int field_0x4;
	uint field_0x8;
	float field_0xc;
	float field_0x10;
});

bool edCinematic::Timeslice(float deltaTime, FrameInfo* pFrameInfo)
{
	bool bVar1;
	float* pfVar2;
	int iVar3;
	uint uVar4;
	ushort* puVar5;
	int i;
	CinKey* peVar5;
	float fVar6;
	float currentPlayTime;
	float fVar7;
	CutsceneHoldsDurations adjustedDelta;
	float local_28;
	undefined4 local_24;
	undefined4 local_20;
	float local_1c;
	int local_18;
	edAnimatedProperty local_14;
	edCinematicSource sceFileB;
	int local_c;
	//edAnmSubControler local_8;
	edCinematicTag* pCinFileHeader;

	/* Make sure that our cutscene has a duration */
	adjustedDelta.durationA = this->pCinTag->field_0x0 - 1e-06f;
	bVar1 = deltaTime <= adjustedDelta.durationA;
	if (bVar1) {
		adjustedDelta.durationA = deltaTime;
	}
	if (adjustedDelta.durationA < 0.0) {
		adjustedDelta.durationA = 0.0;
	}
	/* Make sure the struct we passed in is all zeroed */
	if (pFrameInfo != (FrameInfo*)0x0) {
		pFrameInfo->field_0x4 = 0.0;
		pFrameInfo->field_0xc = 0.0;
		pFrameInfo->field_0x14 = 0;
		pFrameInfo->field_0x18 = 0.0;
		pFrameInfo->field_0x10 = 0.0f;
		pFrameInfo->field_0x0 = (float*)0x0;
	}
	pCinFileHeader = this->pCinTag;
	i = pCinFileHeader->field_0x10 + -1;

	CinKey* pKeyBuffer = (CinKey*)(pCinFileHeader + 1);
	peVar5 = &pKeyBuffer[pCinFileHeader->srcCount];

	if (-1 < i) {
		do {
			if (peVar5->keyType == 2) {
				CinKeyBPre* pre = (CinKeyBPre*)(peVar5 + 1);
				uVar4 = (uint)pre->field_0x0;
				if ((pre->field_0x2 & 1) == 0) {
					iVar3 = uVar4 * 2 + 2;
				}
				else {
					if ((pre->field_0x0 & 1) != 0) {
						uVar4 = uVar4 - 1;
					}
					iVar3 = uVar4 + 4;
				}

				CinKeyB* keyB = (CinKeyB*)(((char*)(peVar5 + 1)) + iVar3 + sizeof(CinKeyBPre));

				edCinematicSource sceFileA = { (edCinematicSourceInternal*)(&pKeyBuffer[keyB->field_0x4])};
				sceFileA.Timeslice(adjustedDelta.durationA, (uint)&adjustedDelta);
				if (pFrameInfo != (FrameInfo*)0x0) {
					pFrameInfo->field_0x10 = keyB->field_0x0;
					if (0 < keyB->field_0x4) {
						pFrameInfo->field_0x18 = adjustedDelta.durationA / keyB->field_0x0;
					}
				}
			}
			else {
				if (peVar5->keyType == 1) {
					edAnmSubControler local_8 = edAnmSubControler((edAnmSubControlerTag*)(peVar5 + 1));
					pfVar2 = local_8.GetClosestKeyIndex(adjustedDelta.durationA, &local_c);
					uVar4 = (uint)local_8.pData->keyCount;
					fVar7 = adjustedDelta.durationA - *pfVar2;
					if ((local_8.pData->field_0x2 & 1) == 0) {
						puVar5 = (ushort*)(((char*)local_8.pData) + (uVar4 * 4) + 4);
					}
					else {
						if ((local_8.pData->keyCount & 1) != 0) {
							uVar4 = uVar4 - 1;
						}
						IMPLEMENTATION_GUARD();
						puVar5 = (ushort*)(((char*)local_8.pData) + (uVar4 * 2) + 4);
					}
					CinKeyC* keyC = (CinKeyC*)(puVar5 + local_c * 0x1e);
					fVar6 = fVar7 / keyC->field_0x0;
					edCinematicSource sceFileB = { (edCinematicSourceInternal*)(&pKeyBuffer[keyC->field_0x4]) };
					currentPlayTime = keyC->field_0xc + fVar6 * (keyC->field_0x10 - keyC->field_0xc);
					sceFileB.Timeslice(currentPlayTime, keyC->field_0x8);
					if (pFrameInfo != (FrameInfo*)0x0) {
						pFrameInfo->field_0x0 = (float*)keyC;
						pFrameInfo->field_0x4 = fVar7;
						pFrameInfo->field_0xc = fVar6;
						pFrameInfo->field_0x8 = currentPlayTime;
					}
				}
			}
			i = i + -1;
			peVar5 = (CinKey*)(((char*)peVar5) + peVar5->keySizeBytes);
		} while (-1 < i);
	}
	i = this->pCinTag->field_0x14;
	while (i = i + -1, -1 < i) {
		edAnimatedPropertyTag* pAnimTag = (edAnimatedPropertyTag*)peVar5;
		edAnimatedProperty local_14 = edAnimatedProperty(pAnimTag);
		if (pAnimTag->propType == 3) {
			local_18 = 0;
			bool bValidKey = local_14.GetKeyIndexAndTime(adjustedDelta.durationA, &local_18, &local_1c);
			if (bValidKey != false) {
				IMPLEMENTATION_GUARD();
				//pfVar2 = &this->headerPtr->field_0x0 +
				//	*(int*)((int)((uint) * (ushort*)(local_14 + 0xc) * 4 + local_14) + local_18 * 0x14 + 0x10) * 3;
				//adjustedDelta.durationB = (float)this->pRes[(int)pfVar2[0xc] * 3 + 2].pData;
				//(**(code**)(*(int*)pfVar2[0xd] + 0x14))(local_1c);
			}
		}
		else {
			bool bValidVector;
			if ((pAnimTag->propType == 4) &&
				(bValidVector = local_14.GetVector3Value(adjustedDelta.durationA, &local_28), bValidVector != false)) {
				IMPLEMENTATION_GUARD();
				//(**(code**)(**(int**)(&this->headerPtr[1].field_0x8 + *(int*)(local_14 + 8) * 0xc) + 0x10))
				//	(local_28, local_24, local_20);
			}
		}
		pAnimTag = (edAnimatedPropertyTag*)(((char*)pAnimTag) + pAnimTag->size);
	}
	return bVar1;
}
