#include "Cinematic.h"

#include "CinResCollection.h"

#include "port/pointer_conv.h"
#include "CinScene.h"

bool edCinematic::Create(edCinGameInterface& pInterface, void* pCinFileBuffer, int bufferLength)
{
	bool bVar1;
	char* dataPtr;
	char* pSeekPos;
	edResCollectionHeader* resPtr;
	char* seekPos;
	int readData;

	resPtr = (edResCollectionHeader*)0x0;
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
					resPtr = (edResCollectionHeader*)(seekPos + 8);
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
			edResCollection edResCol = { resPtr };
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
	edCinSourceSubtitleI* local_10;
	edCinSourceAudioI* local_c;
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
			local_c = (edCinSourceAudioI*)0x0;
			loadObj.GetSourceAudioInterface(&local_c);
			pcVar1 = resPtr.GetResFilename(this->pInternal->offset);
			local_c->Create(pcVar1);
			this->pInternal->pTag = STORE_SECTION(local_c);
		}
		else {
			if (dataValue == 3) {
				local_10 = (edCinSourceSubtitleI*)0x0;
				loadObj.GetSourceSubtitleInterface(&local_10);
				pcVar1 = resPtr.GetResFilename(this->pInternal->offset);

				uint offset = ((this->pInternal->offset * 2) + this->pInternal->offset) * 4;

				char* pOffsetData = reinterpret_cast<char*>(resPtr.pData) + offset;

				uint* pOffsetDataPtr = reinterpret_cast<uint*>(pOffsetData);

				local_10->Create(pcVar1, (pOffsetDataPtr[1] & 0x80000000) != 0);
				this->pInternal->pTag = STORE_SECTION(local_10);
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
		eStack4 = edScene(LOAD_SECTION_CAST(edSCENEtag*, peVar1->pTag));
		eStack4.Initialize();
	}
	else {
		if (iVar2 == 2) {
			edCinSourceAudioI* pAudioInterface = LOAD_SECTION_CAST(edCinSourceAudioI*, peVar1->pTag);
			pAudioInterface->Play();
		}
		else {
			if (iVar2 == 3) {
				edCinSourceSubtitleI* pSubtitleInterface = LOAD_SECTION_CAST(edCinSourceSubtitleI*, peVar1->pTag);
				pSubtitleInterface->Init();
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

bool edCinematicSource::Shutdown()
{
	edCinematicSourceInternal* peVar1;
	int iVar2;
	edScene eStack4;

	peVar1 = this->pInternal;
	iVar2 = peVar1->type;
	if (iVar2 == 1) {
		eStack4 = edScene((edSCENEtag*)LOAD_SECTION(peVar1->pTag));
		eStack4.Shutdown();
	}
	else {
		if (iVar2 == 2) {
			edCinSourceAudioI* pAudioInterface = LOAD_SECTION_CAST(edCinSourceAudioI*, peVar1->pTag);
			pAudioInterface->Stop();
		}
		else {
			if (iVar2 == 3) {
				edCinSourceSubtitleI* pSubtitleInterface = LOAD_SECTION_CAST(edCinSourceSubtitleI*, peVar1->pTag);
				pSubtitleInterface->Shutdown();
			}
		}
	}
	return true;
}

bool edCinematicSource::Destroy(edCinGameInterface& pCinGameInterface)
{
	edCinematicSourceInternal* pSrcTag;
	int type;
	edSCENEtag* peVar3;
	edScene eStack4;

	pSrcTag = this->pInternal;
	type = pSrcTag->type;
	if (type == 1) {
		eStack4 = edScene((edSCENEtag*)LOAD_SECTION(pSrcTag->pTag));
		eStack4.Destroy(pCinGameInterface);
		this->pInternal->pTag = 0x0;
	}
	else {
		if (type == 2) {
			edCinSourceAudioI* pAudioInterface = LOAD_SECTION_CAST(edCinSourceAudioI*, pSrcTag->pTag);
			pAudioInterface->Destroy();
			pCinGameInterface.ReleaseSourceAudioInterface(pAudioInterface);
			this->pInternal->pTag = 0x0;
		}
		else {
			if (type == 3) {
				edCinSourceSubtitleI* pSubtitleInterface = LOAD_SECTION_CAST(edCinSourceSubtitleI*, pSrcTag->pTag);
				pSubtitleInterface->Destroy();
				pCinGameInterface.ReleaseSourceSubtitleInterface(pSubtitleInterface);
				this->pInternal->pTag = 0x0;
			}
		}
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
	CinKey* pCurKey;
	float fVar6;
	float currentPlayTime;
	float fVar7;
	CutsceneHoldsDurations adjustedDelta;
	float local_28;
	undefined4 local_24;
	undefined4 local_20;
	float keyTime;
	int keyIndex;
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
	pCurKey = pKeyBuffer + pCinFileHeader->srcCount;

	if (-1 < i) {
		do {
			if (pCurKey->keyType == 2) {
				CinKeyBPre* pre = (CinKeyBPre*)(pCurKey + 1);
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

				CinKeyB* keyB = (CinKeyB*)(((char*)(pCurKey + 1)) + iVar3 + sizeof(CinKeyBPre));

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
				if (pCurKey->keyType == 1) {
					edAnmSubControler local_8 = edAnmSubControler((edAnmSubControlerTag*)(pCurKey + 1));
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
			pCurKey = reinterpret_cast<CinKey*>(reinterpret_cast<char*>(pCurKey) + pCurKey->keySizeBytes);
		} while (-1 < i);
	}

	i = this->pCinTag->field_0x14;
	while (i = i + -1, -1 < i) {
		edAnimatedPropertyTag* pAnimProp = (edAnimatedPropertyTag*)pCurKey;
		edAnimatedProperty animatedProperty = edAnimatedProperty(pAnimProp);
		if (pAnimProp->propType == 3) {
			keyIndex = 0;
			bool bValidKey = animatedProperty.GetKeyIndexAndTime(adjustedDelta.durationA, &keyIndex, &keyTime);
			if (bValidKey != false) {
				// Subtitles
				edAnmSubControlerTag* pSubControllerTag = (edAnmSubControlerTag*)(pAnimProp + 1);
				edCinSourceSubtitleI::SUBTITLE_PARAMStag subtitleParams;

				// Advance past the keys where the subtitle tags are stored.
				edCinSourceSubtitleI::SUBTITLE_PARAMStag* pSubtitleTags = reinterpret_cast<edCinSourceSubtitleI::SUBTITLE_PARAMStag*>(pSubControllerTag->keyTimes + pSubControllerTag->keyCount);

				subtitleParams.field_0x0 = pSubtitleTags[keyIndex].field_0x0; // Not sure if this is correct.
				subtitleParams.keyA = pSubtitleTags[keyIndex].keyA;
				subtitleParams.keyB = pSubtitleTags[keyIndex].keyB;
				subtitleParams.time = pSubtitleTags[keyIndex].time;

				subtitleParams.flags = pSubtitleTags[keyIndex].flags;

				// Find the end of the cin tag cause that's where the 
				edCinematicSourceInternal* pSourceInternals = reinterpret_cast<edCinematicSourceInternal*>(this->pCinTag + 1);
				edCinSourceSubtitleI* pSubtitle = LOAD_SECTION_CAST(edCinSourceSubtitleI*, pSourceInternals[pSubtitleTags[keyIndex].field_0x0].pTag);

				pSubtitle->SetSubtitle(keyTime, &subtitleParams);
			}
		}
		else {
			bool bValidVector;
			if ((pAnimProp->propType == 4) &&
				(bValidVector = animatedProperty.GetVector3Value(adjustedDelta.durationA, &local_28), bValidVector != false)) {
				IMPLEMENTATION_GUARD();
				//(**(code**)(**(int**)(&this->pCinTag[1].field_0x8 + *(int*)(local_14 + 8) * 0xc) + 0x10))
				//	(local_28, local_24, local_20);
			}
		}
		pAnimProp = (edAnimatedPropertyTag*)(((char*)pAnimProp) + pAnimProp->size);
	}
	return bVar1;
}

bool edCinematic::Shutdown()
{
	int iVar1;
	edCinematicSourceInternal* eVar2 = (edCinematicSourceInternal*)(this->pCinTag + 1);

	for (iVar1 = this->pCinTag->srcCount; 0 < iVar1; iVar1 = iVar1 + -1) {
		edCinematicSource local_8 = { eVar2 };
		local_8.Shutdown();
		eVar2 = eVar2 + 1;
	}

	return true;
}

bool edCinematic::Destroy(edCinGameInterface& pCinGameInterface)
{
	int iVar1;
	edCinematicSourceInternal* eVar2;

	if (this->fileStart != (int*)0x0) {
		eVar2 = (edCinematicSourceInternal*)(this->pCinTag + 1);
		for (iVar1 = this->pCinTag->srcCount; 0 < iVar1; iVar1 = iVar1 + -1) {
			edCinematicSource local_8 = { eVar2 };
			local_8.Destroy(pCinGameInterface);
			eVar2 = eVar2 + 1;
		}
		edResCollection local_4 = { this->pRes };
		local_4.FlushAllResources(pCinGameInterface);
		pCinGameInterface.ReleaseResource(0xff, false, this->fileStart);
		this->fileStart = (int*)0x0;
		this->pRes = (edResCollectionHeader*)0x0;
		this->pCinTag = (edCinematicTag*)0x0;
	}
	return true;
}
