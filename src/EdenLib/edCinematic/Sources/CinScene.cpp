#include "CinScene.h"
#include "CinResCollection.h"

#include "port/pointer_conv.h"
#include "MathOps.h"
#include <string>
#include "Cinematic.h"

#define CUTSCENE_LOG(level, format, ...) MY_LOG_CATEGORY("Cutscene", level, format, ##__VA_ARGS__)

#ifdef PLATFORM_WIN
#define CUTSCENE_LOG_WIN(level, format, ...) MY_LOG_CATEGORY("Cutscene", level, format, ##__VA_ARGS__)
#else
#define CUTSCENE_LOG_WIN(...)
#endif

struct edSceneCamera
{
	CameraInfo* pInternal;
	void Create(edCinGameInterface& pCinematic);
	void Initialize();
	bool Timeslice(float currentPlayTime);
};

void edSceneCamera::Create(edCinGameInterface& pCinematic)
{
	edCinCamInterface::CAMERA_CREATIONtag tag;
	char cameraName[32];
	float local_30;
	float local_20;
	float local_1c;
	float local_18;
	undefined4 local_14;
	float local_10;
	float local_c;
	float local_8;
	float local_4;
	CameraInfo* cameraInfoPtr;

	cameraInfoPtr = this->pInternal;
	//local_30 = (cameraInfoPtr->field_0x34).x;
	//local_20 = (cameraInfoPtr->field_0x34).y;
	//local_1c = (cameraInfoPtr->field_0x34).z;
	//local_18 = (cameraInfoPtr->field_0x34).w;
	//local_14 = 0x3f800000;
	//local_10 = (cameraInfoPtr->field_0x44).x;
	//local_c = (cameraInfoPtr->field_0x44).y;
	//local_8 = (cameraInfoPtr->field_0x44).z;
	//local_4 = (cameraInfoPtr->field_0x44).w;
	strcpy(cameraName, cameraInfoPtr->name);

	CUTSCENE_LOG(LogLevel::Info, "edSceneCamera::Create {}", cameraName);

	edCinCamInterface* cinCam;
	pCinematic.GetCamera(&cinCam, &tag);
	cameraInfoPtr->cinCam = STORE_SECTION(cinCam);
	return;
}

void edSceneCamera::Initialize()
{
	((edCinCamInterface*)LOAD_SECTION(pInternal->cinCam))->Activate();
}

float* edAnmSubControler::GetClosestKeyIndex(float time, int* outIndex)
{
	ushort uVar1;
	int iVar2;
	uint uVar3;
	edAnmSubControlerTag* peVar4;
	float* pfVar5;

	peVar4 = this->pData;
	uVar1 = peVar4->keyCount;
	pfVar5 = peVar4->keyTimes;
	uVar3 = (uint)uVar1;
	if (time < pfVar5[uVar3 - 1]) {
		IMPLEMENTATION_GUARD_LOG();
		peVar4 = peVar4 + 1;
		if ((((8 < uVar1) && (iVar2 = (int)(uVar1 - 1) >> 2, *(float*)(&peVar4->keyCount + iVar2 * 2) <= time)) &&
			(peVar4 = (edAnmSubControlerTag*)(&peVar4->keyCount + iVar2 * 2),
				*(float*)(&peVar4->keyCount + iVar2 * 2) <= time)) &&
			(peVar4 = (edAnmSubControlerTag*)(&peVar4->keyCount + iVar2 * 2),
				*(float*)(&peVar4->keyCount + iVar2 * 2) <= time)) {
			peVar4 = (edAnmSubControlerTag*)(&peVar4->keyCount + iVar2 * 2);
		}
		for (; *(float*)peVar4 <= time; peVar4 = (edAnmSubControlerTag*)&peVar4->keyTimes) {
		}
		peVar4 = (edAnmSubControlerTag*)((int)peVar4 - (int)pfVar5);
		if ((int)peVar4 < 0) {
			peVar4 = (edAnmSubControlerTag*)((int)&peVar4->field_0x2 + 1);
		}
		uVar3 = (int)peVar4 >> 2;
	}
	*outIndex = uVar3 - 1;
	return pfVar5 + (uVar3 - 1);
}

bool edAnimatedProperty::GetVector3Value(float time, float* outValue)
{
	ushort uVar1;
	edAnimatedPropertyTag* peVar2;
	bool bVar3;
	float* piVar4;
	float delta;
	int local_10;
	int local_c[2];

	peVar2 = this->pData;

	edAnmSubControlerTag* pTag = (edAnmSubControlerTag*)(peVar2 + 1);

	uVar1 = pTag->keyCount;

	edAnmSubControler local_4 = edAnmSubControler(pTag);

	piVar4 = pTag->keyTimes;
	if (time < pTag->keyTimes[0]) {
		bVar3 = false;
	}
	else {
		local_4.GetKeyIndicesAndRatioSafe(time, &local_10, local_c, 0);
		delta = piVar4[local_c[0]] - piVar4[local_10];
		if (delta != 0.0f) {
			delta = (time - piVar4[local_10]) / delta;
		}
		edF32Vector3LERPSoft(delta, (edF32VECTOR3*)outValue, (edF32VECTOR3*)(piVar4 + uVar1 + local_10 * 3), (edF32VECTOR3*)(piVar4 + uVar1 + local_c[0] * 3));
		bVar3 = true;
	}
	return bVar3;
}

bool edAnimatedProperty::GetKeyIndexAndTime(float currentTime, int* outIndex, float* outTime)
{
	bool bVar1;
	float* pfVar2;

	edAnmSubControlerTag* pSubControllerTag = (edAnmSubControlerTag*)(this->pData + 1);
	edAnmSubControler keyframeBufferStart = edAnmSubControler(pSubControllerTag);

	bVar1 = pSubControllerTag->keyTimes[0] <= currentTime;
	if (bVar1) {
		pfVar2 = keyframeBufferStart.GetClosestKeyIndexSafe(currentTime, outIndex);
		*outTime = currentTime - *pfVar2;
	}
	return bVar1;
}

float edAnmSubControler::GetKeyIndicesAndRatioSafe(float currentPlayTime, int* outKeyFrame, int* outNextKeyframe, uchar param_5)
{
	ushort uVar1;
	float* keyframeBufferStart;
	int KeyframeBLength;
	int keyframeALength;
	uint uVar2;
	float* nextKeyframeBPtr;
	float* nextKeyframeAPtr;
	uint uVar3;
	float nextKeyframeTime;
	float fVar4;

	uVar1 = this->pData->keyCount;
	keyframeBufferStart = this->pData->keyTimes;
	if (uVar1 < 2) {
		*outNextKeyframe = 0;
		*outKeyFrame = 0;
		fVar4 = 0.0;
	}
	else {
		uVar2 = (uint)uVar1;
		uVar3 = (int)uVar2 >> 1;
		if (param_5 == '\0') {
			if (keyframeBufferStart[uVar1 - 1] <= currentPlayTime) {
				*outNextKeyframe = uVar2 - 1;
				*outKeyFrame = uVar2 - 1;
				nextKeyframeTime = 0.0;
			}
			else {
				for (; (currentPlayTime < keyframeBufferStart[uVar3] && (1 < uVar3)); uVar3 = (int)uVar3 >> 1) {
				}
				if (uVar3 == 0) {
					uVar3 = 1;
				}
				/* Set the keyframe we are on */
				*outNextKeyframe = uVar3;
				/* Search for the next keyframe */
				nextKeyframeAPtr = keyframeBufferStart + uVar3;
				nextKeyframeTime = *nextKeyframeAPtr;
				while (nextKeyframeTime <= currentPlayTime) {
					nextKeyframeAPtr = nextKeyframeAPtr + 1;
					nextKeyframeTime = *nextKeyframeAPtr;
				}
				/* Found the next keyframe

				   Get the length between the current keyframe and the first keyframe */
				keyframeALength = (int)nextKeyframeAPtr - (int)keyframeBufferStart;
				/* If we went below zero, jump up 3 frames */
				if (keyframeALength < 0) {
					keyframeALength = keyframeALength + 3;
				}
				*outNextKeyframe = keyframeALength >> 2;
				*outKeyFrame = *outNextKeyframe + -1;
				/* Check if the out keyframe is the first key frame */
				if (*outKeyFrame == 0) {
					/* This is the first keyframe, so our percentage through the keyframe is just our current time /
					   next keyframe time */
					if (*nextKeyframeAPtr <= 0.0) {
						nextKeyframeTime = 0.0;
					}
					else {
						nextKeyframeTime = currentPlayTime / *nextKeyframeAPtr;
					}
				}
				else {
					/* We need to get the previous keyframe time, and subtract it from the current time before we
					   divide by the next keyframe time */
					nextKeyframeTime = *nextKeyframeAPtr - nextKeyframeAPtr[-1];
					if (nextKeyframeTime <= 0.0) {
						nextKeyframeTime = 0.0;
					}
					else {
						nextKeyframeTime = (currentPlayTime - nextKeyframeAPtr[-1]) / nextKeyframeTime;
					}
				}
			}
		}
		else {
			if ((keyframeBufferStart + uVar1)[-1] <= currentPlayTime) {
				*outKeyFrame = uVar2 - 1;
				*outNextKeyframe = 0;
				fVar4 = (keyframeBufferStart + uVar1)[-1];
				nextKeyframeTime = *keyframeBufferStart - fVar4;
				if (nextKeyframeTime <= 0.0) {
					nextKeyframeTime = 0.0;
				}
				else {
					nextKeyframeTime = (currentPlayTime - fVar4) / nextKeyframeTime;
				}
			}
			else {
				for (; (currentPlayTime < keyframeBufferStart[uVar3] && (1 < uVar3)); uVar3 = (int)uVar3 >> 1) {
				}
				if (uVar3 == 0) {
					uVar3 = 1;
				}
				*outNextKeyframe = uVar3;
				nextKeyframeBPtr = keyframeBufferStart + uVar3;
				nextKeyframeTime = *nextKeyframeBPtr;
				while (nextKeyframeTime <= currentPlayTime) {
					nextKeyframeBPtr = nextKeyframeBPtr + 1;
					nextKeyframeTime = *nextKeyframeBPtr;
				}
				KeyframeBLength = (int)nextKeyframeBPtr - (int)keyframeBufferStart;
				if (KeyframeBLength < 0) {
					KeyframeBLength = KeyframeBLength + 3;
				}
				*outNextKeyframe = KeyframeBLength >> 2;
				*outKeyFrame = *outNextKeyframe + -1;
				if (*outKeyFrame == 0) {
					if (*nextKeyframeBPtr <= 0.0) {
						nextKeyframeTime = 0.0;
					}
					else {
						nextKeyframeTime = currentPlayTime / *nextKeyframeBPtr;
					}
				}
				else {
					nextKeyframeTime = *nextKeyframeBPtr - nextKeyframeBPtr[-1];
					if (nextKeyframeTime <= 0.0) {
						nextKeyframeTime = 0.0;
					}
					else {
						nextKeyframeTime = (currentPlayTime - nextKeyframeBPtr[-1]) / nextKeyframeTime;
					}
				}
			}
		}
		/* Check that the percent through keyframe is above zero */
		if (nextKeyframeTime < 0.0f) {
			/* Below zero so just return zero */
			fVar4 = 0.0f;
		}
		else {
			/* Above zero, so default to 100%, and if its under 100% then set it to be what it really is */
			fVar4 = 1.0f;
			if (nextKeyframeTime <= 1.0f) {
				fVar4 = nextKeyframeTime;
			}
		}
	}
	return fVar4;
}

float* edAnmSubControler::GetClosestKeyIndexSafe(float time, int* outIndex)
{
	ushort uVar1;
	float* pKeyTimes;
	int iVar3;
	uint uVar4;
	edAnmSubControlerTag* peVar5;
	uint uVar6;

	peVar5 = this->pData;
	uVar1 = peVar5->keyCount;
	uVar6 = (uint)uVar1;
	pKeyTimes = peVar5->keyTimes;
	if (uVar6 < 2) {
		/* If we don't have enough keyframes to interpolate between then just return out */
		*outIndex = 0;
	}
	else {
		uVar4 = (uint)uVar1;
		if (time < pKeyTimes[uVar4 - 1]) {
			peVar5 = peVar5 + 1;

			if ((((8 < uVar1) && (iVar3 = (int)(uVar6 - 1) >> 2, *(float*)(&peVar5->keyCount + iVar3 * 2) <= time)) &&
				(peVar5 = (edAnmSubControlerTag*)(&peVar5->keyCount + iVar3 * 2),
					*(float*)(&peVar5->keyCount + iVar3 * 2) <= time)) &&
				(peVar5 = (edAnmSubControlerTag*)(&peVar5->keyCount + iVar3 * 2),
					*(float*)(&peVar5->keyCount + iVar3 * 2) <= time)) {
				peVar5 = (edAnmSubControlerTag*)(&peVar5->keyCount + iVar3 * 2);
			}

			for (; *(float*)peVar5 <= time; peVar5 = (edAnmSubControlerTag*)&peVar5->keyTimes) {
			}

			int diff = ((char*)peVar5 - (char*)pKeyTimes);

			if (diff < 0) {
				IMPLEMENTATION_GUARD();
				diff = ((int)&peVar5->field_0x2 + 1);
			}

			uVar4 = diff >> 2;
		}
		iVar3 = uVar4 - 1;
		*outIndex = iVar3;
		if (iVar3 < 0) {
			iVar3 = 0;
		}
		else {
			if (((int)uVar6 <= iVar3) && (iVar3 = iVar3 % (int)uVar6, uVar6 == 0)) {
#ifdef PLATFORM_PS2
				trap(7);
#else
				assert(false);
#endif
			}
		}
		pKeyTimes = pKeyTimes + iVar3;
	}
	return pKeyTimes;
}

bool edAnimatedProperty::GetQuaternionValue(float currentPlayTime, edF32VECTOR4* outVector)
{
	ushort uVar1;
	bool ret;
	edF32VECTOR4* targetRotationPtr;
	edF32VECTOR4* currentRotationPtr;
	float* baseRotationData;
	float nextKeyframeTime;
	edF32VECTOR4 targetRotation;
	edF32VECTOR4 currentRotation;
	int rotationCurrentKeyFrame;
	int rotationNextKeyframe;
	//edAnmSubControler rotationKeyframeData;
	edAnimatedPropertyTag* pTag;

	pTag = this->pData;
	edAnmSubControlerTag* pSubTag = (edAnmSubControlerTag*)(pTag + 1);
	edAnmSubControler rotationKeyframeData = edAnmSubControler(pSubTag);
	uVar1 = pSubTag->keyCount;
	baseRotationData = pSubTag->keyTimes;
	if (currentPlayTime < pSubTag->keyTimes[0]) {
		ret = false;
	}
	else {
		rotationKeyframeData.GetKeyIndicesAndRatioSafe(currentPlayTime, &rotationCurrentKeyFrame, &rotationNextKeyframe, '\0');
		nextKeyframeTime = baseRotationData[rotationNextKeyframe] - baseRotationData[rotationCurrentKeyFrame];
		if (nextKeyframeTime != 0.0) {
			nextKeyframeTime = (currentPlayTime - baseRotationData[rotationCurrentKeyFrame]) / nextKeyframeTime;
		}
		currentRotationPtr = (edF32VECTOR4*)(baseRotationData + uVar1 + rotationCurrentKeyFrame * 4);
		currentRotation.x = currentRotationPtr->x;
		targetRotationPtr = (edF32VECTOR4*)(baseRotationData + uVar1 + rotationNextKeyframe * 4);
		currentRotation.y = currentRotationPtr->y;
		currentRotation.z = currentRotationPtr->z;
		currentRotation.w = currentRotationPtr->w;
		targetRotation.x = targetRotationPtr->x;
		targetRotation.y = targetRotationPtr->y;
		targetRotation.z = targetRotationPtr->z;
		targetRotation.w = targetRotationPtr->w;
		edQuatShortestSLERPHard(nextKeyframeTime, outVector, &currentRotation, &targetRotation);
		ret = true;
	}
	return ret;
}

bool edSceneCamera::Timeslice(float currentPlayTime)
{
	short sVar1;
	CameraInfo* pCVar2;
	bool bVar3;
	bool bVar4;
	bool bVar5;
	bool ret;
	long lVar6;
	int trackIndex;
	//float* cutsceneElementPtr;
	float* baseKeyframeData;
	float currentKeyframePlayTime;
	float unaff_f20;
	edF32VECTOR4 outVector;
	int local_48;
	int local_44;
	int locationCurrentKeyFrame;
	int locationNextKeyframe;
	int locationCurrentKeyframeB;
	int locationNextKeyframeB;
	int scaleCurrentKeyFrame;
	int scaleNextKeyframe;
	edF32VECTOR3 outLocation;
	//float* local_18;
	//float* locationKeyFrameData;
	//float* keyframeCountPtr;
	//float* scaleKeyframeData;
	//float* rotationBaseData;
	uint local_4;
	edCinCamInterface* objectPtr;
	uint type;
	ushort keyframeCount;

	objectPtr = (edCinCamInterface*)LOAD_SECTION(this->pInternal->cinCam);
	local_4 = 0;
	lVar6 = objectPtr->Initialize(true, &local_4);
	if (lVar6 != 0) {
		bVar3 = false;
		pCVar2 = this->pInternal;
		bVar4 = false;
		bVar5 = false;
		trackIndex = pCVar2->trackCount;
		edAnimatedPropertyTag* pAnimProp = (edAnimatedPropertyTag*)(((char*)pCVar2) + pCVar2->trackDataOffset);
	
		if (0 < trackIndex) {
			do {
				trackIndex = trackIndex + -1;
				type = pAnimProp->type;
				//rotationBaseData = cutsceneElementPtr;
				if (type == 0x64c8d3b1) {
					edAnimatedProperty anmProp = edAnimatedProperty(pAnimProp);
					sVar1 = pAnimProp->propType;
					if (sVar1 == 2) {
						ret = anmProp.GetQuaternionValue(currentPlayTime, &outVector);
						if (ret != false) {
							/* Apply Rotation */
							objectPtr->SetHeadingQuat(outVector.x, outVector.y, outVector.z, outVector.w);
							bVar3 = true;
						}
					}
					else {
						if (sVar1 == 1) {
							edAnmSubControlerTag* pTag = (edAnmSubControlerTag*)(pAnimProp + 1);
							keyframeCount = pTag->keyCount;
							baseKeyframeData = pTag->keyTimes;
							if (currentPlayTime < pTag->keyTimes[0]) {
								ret = false;
							}
							else {
								edAnmSubControler local_18 = edAnmSubControler(pTag);
								local_18.GetKeyIndicesAndRatioSafe(currentPlayTime, &local_48, &local_44, 0);
								currentKeyframePlayTime = baseKeyframeData[local_44] - baseKeyframeData[local_48];
								if (currentKeyframePlayTime != 0.0) {
									currentKeyframePlayTime = (currentPlayTime - baseKeyframeData[local_48]) / currentKeyframePlayTime;
								}
								edF32Vector3LERPSoft(currentKeyframePlayTime, &outVector.xyz,
									(edF32VECTOR3*)(baseKeyframeData + keyframeCount + local_48 * 3),
									(edF32VECTOR3*)(baseKeyframeData + keyframeCount + local_44 * 3));
								ret = true;
							}
							if (ret) {
								objectPtr->SetHeadingEuler(outVector.x, outVector.y, outVector.z, true);
								bVar3 = true;
							}
						}
						else {
							if (sVar1 == 0) {
								edAnmSubControlerTag* pTag = (edAnmSubControlerTag*)(pAnimProp + 1);
								keyframeCount = pTag->keyCount;
								baseKeyframeData = pTag->keyTimes;
								if (currentPlayTime < pTag->keyTimes[0]) {
									ret = false;
								}
								else {
									edAnmSubControler locationKeyFrameData = edAnmSubControler(pTag);
									locationKeyFrameData.GetKeyIndicesAndRatioSafe
									(currentPlayTime, &locationCurrentKeyFrame,
										&locationNextKeyframe, 0);
									/* Work out the duration through the current keyframe (again, this is already done inside the
									   function above) */
									currentKeyframePlayTime =
										baseKeyframeData[locationNextKeyframe] - baseKeyframeData[locationCurrentKeyFrame];
									if (currentKeyframePlayTime != 0.0) {
										currentKeyframePlayTime =
											(currentPlayTime - baseKeyframeData[locationCurrentKeyFrame]) / currentKeyframePlayTime;
									}
									/* Work out a transform based on the two key frames and current keyframe play time */
									edF32Vector3LERPSoft
									(currentKeyframePlayTime, &outVector.xyz,
										(edF32VECTOR3*)(baseKeyframeData + keyframeCount + locationCurrentKeyFrame * 3),
										(edF32VECTOR3*)(baseKeyframeData + keyframeCount + locationNextKeyframe * 3));
									ret = true;
								}
								/* Apply the transform */
								if (ret) {
									objectPtr->SetHeadingEuler(outVector.x, outVector.y, outVector.z, false);
									bVar3 = true;
								}
							}
						}
					}
				}
				else {
					if (type == 0xd7e2d8c4) {
						edAnmSubControlerTag* pTag = (edAnmSubControlerTag*)(pAnimProp + 1);
						keyframeCount = pTag->keyCount;
						baseKeyframeData = pTag->keyTimes;
						if (currentPlayTime < pTag->keyTimes[0]) {
							ret = false;
						}
						else {
							edAnmSubControler keyframeCountPtr = edAnmSubControler(pTag);
							/* Camera goes in here */
							keyframeCountPtr.GetKeyIndicesAndRatioSafe(currentPlayTime, &locationCurrentKeyframeB, &locationNextKeyframeB, 0);
							/* Work out the duration through the current keyframe (again, this is already done inside the
							   function above) */
							currentKeyframePlayTime = baseKeyframeData[locationNextKeyframeB] - baseKeyframeData[locationCurrentKeyframeB];

							if (currentKeyframePlayTime != 0.0f) {
								currentKeyframePlayTime = (currentPlayTime - baseKeyframeData[locationCurrentKeyframeB]) / currentKeyframePlayTime;
							}
							/* Work out a transform based on the two key frames and current keyframe play time  */
							edF32Vector3LERPSoft(currentKeyframePlayTime, &outLocation,
								(edF32VECTOR3*)(baseKeyframeData + keyframeCount + locationCurrentKeyframeB * 3),
								(edF32VECTOR3*)(baseKeyframeData + keyframeCount + locationNextKeyframeB * 3));
							ret = true;
						}
						if (ret) {
							/* Update location needed */
							objectPtr->SetPos(outLocation.x, outLocation.y, outLocation.z);
							bVar5 = true;
						}
					}
					else {
						if (type == 0x766f46) {
							edAnmSubControlerTag* pTag = (edAnmSubControlerTag*)(pAnimProp + 1);
							baseKeyframeData = pTag->keyTimes;
							if (currentPlayTime < pTag->keyTimes[0]) {
								ret = false;
							}
							else {
								/* Maybe scale? */
								edAnmSubControler scaleKeyframeData = edAnmSubControler(pTag);
								keyframeCount = pTag->keyCount;
								scaleKeyframeData.GetKeyIndicesAndRatioSafe(currentPlayTime, &scaleCurrentKeyFrame, &scaleNextKeyframe, 0);
								currentKeyframePlayTime = baseKeyframeData[scaleNextKeyframe] - baseKeyframeData[scaleCurrentKeyFrame];
								if (currentKeyframePlayTime != 0.0f) {
									currentKeyframePlayTime =
										(currentPlayTime - baseKeyframeData[scaleCurrentKeyFrame]) / currentKeyframePlayTime;
								}
								ret = true;
								unaff_f20 = currentKeyframePlayTime * (baseKeyframeData + keyframeCount)[scaleNextKeyframe] +
									(1.0f - currentKeyframePlayTime) * (baseKeyframeData + keyframeCount)[scaleCurrentKeyFrame];
							}
							if (ret) {
								objectPtr->SetFov(unaff_f20);
								bVar4 = true;
							}
						}
					}
				}
				pAnimProp = (edAnimatedPropertyTag*)(((char*)pAnimProp) + pAnimProp->size);
			} while (0 < trackIndex);
		}

		if ((local_4 & 1) != 0) {
			pCVar2 = this->pInternal;
			if (!bVar4) {
				objectPtr->SetFov(pCVar2->fov);
			}
			if (!bVar5) {
				objectPtr->SetPos((pCVar2->position).x, (pCVar2->position).y, (pCVar2->position).z);
			}
			if (!bVar3) {
				objectPtr->SetHeadingQuat((pCVar2->heading).x, (pCVar2->heading).y, (pCVar2->heading).z, (pCVar2->heading).w);
			}
		}
		objectPtr->Initialize(false, &local_4);
	}
	return true;
}

edScene::edScene(edSCENEtag* pTag)
{
	this->pTag = pTag;
}

struct edSceneActor {
	edSceneActor(CineCreatureObject* pInObj) : pObj(pInObj) {}

	CineCreatureObject* pObj;

	void Initialize();
	virtual bool Create(edCinGameInterface& cinGameInterface, edResCollection& resCollection) { return true; }
	virtual bool Timeslice(float currentPlayTime, edResCollection& resCollection);
};

void edSceneActor::Initialize()
{
	edCinActorInterface* pCinActor = (edCinActorInterface*)LOAD_SECTION(this->pObj->pCinActorInterface);
	pCinActor->Initialize();
}

struct edSceneActorVirtual : public edSceneActor {
	edSceneActorVirtual(CineCreatureObject* pInObj) : edSceneActor(pInObj) {}
	virtual bool Create(edCinGameInterface& cinGameInterface, edResCollection& resCollection);
};

bool edSceneActorVirtual::Create(edCinGameInterface& cinGameInterface, edResCollection& resCollection)
{
	bool bVar1;
	edCinGameInterface::ACTORV_CREATIONtag creationTag;
	CineCreatureObject* cineCreatureObject;

	/* This sets up objects for a cutscene, and gives them a name
	   Example: SC_SOUND_EMITTER */
	cineCreatureObject = this->pObj;
	creationTag.position.xyz = cineCreatureObject->position;
	creationTag.position.w = 1.0;
	creationTag.heading = cineCreatureObject->heading;
	creationTag.scale.xyz = cineCreatureObject->scale;
	creationTag.scale.w = 1.0f;
	creationTag.boundingSphere = cineCreatureObject->boundingSphere;

	/* Loads the name of the player into this buffer.
	   Example: ARAIGNOSBLACK_TOONPLAYER_L0 */
	strcpy((char*)&creationTag, cineCreatureObject->name);

	CUTSCENE_LOG(LogLevel::Info, "\nedSceneActorVirtual::Create name: {}", cineCreatureObject->name);

	CUTSCENE_LOG_WIN(LogLevel::Info, "edSceneActorVirtual::Create position: {}", creationTag.position.ToString());
	CUTSCENE_LOG_WIN(LogLevel::Info, "edSceneActorVirtual::Create heading: {}", creationTag.heading.ToString());
	CUTSCENE_LOG_WIN(LogLevel::Info, "edSceneActorVirtual::Create scale: {}", creationTag.scale.ToString());

	if (cineCreatureObject->textureID == -1) {
		creationTag.textureName = 0;
		creationTag.bHasTexture = false;
	}
	else {
		creationTag.textureName = resCollection.GetResFilename(cineCreatureObject->textureID);

		CUTSCENE_LOG(LogLevel::Info, "edSceneActorVirtual::Create texture {}", creationTag.textureName);

		creationTag.bHasTexture = (resCollection.pData->aTags[cineCreatureObject->textureID].flags & 0x80000000U) != 0;
	}
	if (cineCreatureObject->meshID == -1) {
		creationTag.meshName = 0;
		creationTag.bHasMesh = false;
	}
	else {
		creationTag.meshName = resCollection.GetResFilename(cineCreatureObject->meshID);

		CUTSCENE_LOG(LogLevel::Info, "edSceneActorVirtual::Create mesh {}", creationTag.meshName);

		creationTag.bHasMesh = (resCollection.pData->aTags[cineCreatureObject->meshID].flags & 0x80000000U) != 0;
	}
	creationTag.field_0x30 = (cineCreatureObject->boundingSphere).w;
	bVar1 = cinGameInterface.CreateActor((edCinActorInterface**)&this->pObj->pCinActorInterface, &creationTag);
	return bVar1 != false;
}

struct edSceneActorBehavior : public edSceneActor {
	edSceneActorBehavior(CineCreatureObject* pInObj) : edSceneActor(pInObj) {}
	virtual bool Create(edCinGameInterface& cinGameInterface, edResCollection& resCollection);
};

bool edSceneActorBehavior::Create(edCinGameInterface& cinGameInterface, edResCollection& resCollection)
{
	CineCreatureObject* cineCreatureObject;
	long lVar2;
	edCinGameInterface::ACTORV_CREATIONtag creationTag;

	cineCreatureObject = this->pObj;
	resCollection.pData = resCollection.pData;
	creationTag.position.x = (cineCreatureObject->position).x;
	creationTag.position.y = (cineCreatureObject->position).y;
	creationTag.position.z = (cineCreatureObject->position).z;
	creationTag.position.w = 1.0;
	creationTag.heading.x = (cineCreatureObject->heading).x;
	creationTag.heading.y = (cineCreatureObject->heading).y;
	creationTag.heading.z = (cineCreatureObject->heading).z;
	creationTag.heading.w = (cineCreatureObject->heading).w;
	creationTag.scale.x = (cineCreatureObject->scale).x;
	creationTag.scale.y = (cineCreatureObject->scale).y;
	creationTag.scale.z = (cineCreatureObject->scale).z;
	creationTag.scale.w = 1.0;
	creationTag.boundingSphere.x = (cineCreatureObject->boundingSphere).x;
	creationTag.boundingSphere.y = (cineCreatureObject->boundingSphere).y;
	creationTag.boundingSphere.z = (cineCreatureObject->boundingSphere).z;
	creationTag.boundingSphere.w = 1.0;

	CUTSCENE_LOG(LogLevel::Info, "\nedSceneActorBehavior::Create name: {}", cineCreatureObject->name);

	CUTSCENE_LOG_WIN(LogLevel::Info, "edSceneActorBehavior::Create position: {}", creationTag.position.ToString());
	CUTSCENE_LOG_WIN(LogLevel::Info, "edSceneActorBehavior::Create heading: {}", creationTag.heading.ToString());
	CUTSCENE_LOG_WIN(LogLevel::Info, "edSceneActorBehavior::Create scale: {}", creationTag.scale.ToString());

	if (cineCreatureObject->textureID == -1) {
		creationTag.textureName = (char*)0x0;
		creationTag.bHasTexture = false;
	}
	else {
		creationTag.textureName = resCollection.GetResFilename(cineCreatureObject->textureID);
		CUTSCENE_LOG(LogLevel::Info, "edSceneActorBehavior::Create texture {}", creationTag.textureName);
		creationTag.bHasTexture = (resCollection.pData->aTags[cineCreatureObject->textureID].flags & 0x80000000) != 0;
	}
	if (cineCreatureObject->meshID == -1) {
		creationTag.meshName = (char*)0x0;
		creationTag.bHasMesh = false;
	}
	else {
		creationTag.meshName = resCollection.GetResFilename(cineCreatureObject->meshID);
		CUTSCENE_LOG(LogLevel::Info, "edSceneActorBehavior::Create mesh {}", creationTag.meshName);
		creationTag.bHasMesh = (resCollection.pData->aTags[cineCreatureObject->meshID].flags & 0x80000000) != 0;
	}
	creationTag.field_0x30 = (cineCreatureObject->boundingSphere).w;
	strcpy(creationTag.name, cineCreatureObject->name);
	lVar2 = cinGameInterface.GetActor((edCinActorInterface**)&this->pObj->pCinActorInterface, this->pObj->actorHashCode, &creationTag);
	return lVar2 != 0;
}

bool edSceneActor::Timeslice(float currentPlayTime, edResCollection& resCollection)
{
	short sVar2;
	bool bUpdateRotationSuccess;
	float* currentKeyframePtr;
	bool bFrameDirected;
	float* keyframeBodyPtr;
	//int* trackSeekPos;
	int* pTrackDataStart;
	int numTracks;
	float fVar4;
	edF32VECTOR4 outRotation;
	int local_b8;
	int local_b4;
	int local_b0;
	int local_ac;
	int local_a8;
	int local_a4;
	int locationOutKeyframe;
	int locationOutNextKeyframe;
	int local_98;
	int local_94;
	int local_90;
	int soundFileInfoObj;
	float soundStart;
	float soundStop;
	int* local_78;
	int local_74;
	int local_70;
	edF32VECTOR3 local_68;
	edF32VECTOR4 locationOutVector;
	edAnmSubControler local_48;
	edAnmSubControler soundTrackBuffer;
	edAnmSubControler local_40;
	edAnmSubControler local_3c;
	edAnmSubControler local_38;
	edAnmSubControler local_34;
	edAnmSubControler local_30;
	edAnmSubControler local_2c;
	edAnmSubControler locationSceStart;
	edAnmSubControler local_24;
	int local_20;
	int local_1c;
	int local_18;
	int soundKeyframe;
	int local_10;
	int local_c;
	int local_8;
	edAnimatedProperty animatedProp;
	CineCreatureObject* pCineCreature;
	uint currentTrackType;
	edCinActorInterface* pCinActorInterface;

	/* Load the element start buffer into v1 */
	pCineCreature = this->pObj;
	pCinActorInterface = (edCinActorInterface*)LOAD_SECTION(pCineCreature->pCinActorInterface);

	numTracks = pCineCreature->trackCount;
	edAnimatedPropertyTag* pAnimProp = (edAnimatedPropertyTag*)(((char*)pCineCreature) + pCineCreature->trackDataOffset);
	/* Move along the seek position to the first track */
	bFrameDirected = pCinActorInterface->OnFrameDirected();
	if (bFrameDirected != 0) {
		while (bUpdateRotationSuccess = 0 < numTracks, numTracks = numTracks + -1, bUpdateRotationSuccess) {
			currentTrackType = pAnimProp->type;
			animatedProp = edAnimatedProperty(pAnimProp);
			if (currentTrackType == 0x73d8ccae) {
				IMPLEMENTATION_GUARD(
				locationOutVector.w = (float)(trackSeekPos + 3);
				local_1c = 0;
				if ((float)trackSeekPos[4] <= currentPlayTime) {
					currentKeyframePtr =
						edAnmSubControler::GetClosestKeyIndexSafe
						(currentPlayTime, (edAnmSubControler*)&locationOutVector.w, &local_1c);
					pTrackDataStart =
						(int*)((int)animatedProp + 0xc) +
						(uint) * (ushort*)((int)animatedProp + 0xc) + local_1c * 2;
					local_98 = pTrackDataStart[1];
					local_94 = resCollection->pData[pTrackDataStart[1]].pData;
					local_90 = pTrackDataStart[2];
					(*(code*)pCinActorInterface->vt->SetMessage)(currentPlayTime - *currentKeyframePtr, pCinActorInterface);
				})
			}
			else {
				if (currentTrackType == 0xd9cee9bc) {
					IMPLEMENTATION_GUARD(
					local_48 = (edAnmSubControlerTag*)(trackSeekPos + 3);
					local_18 = 0;
					if ((float)trackSeekPos[4] <= currentPlayTime) {
						currentKeyframePtr = edAnmSubControler::GetClosestKeyIndexSafe(currentPlayTime, &local_48, &local_18);
						(*(code*)pCinActorInterface->vt->SetSubtitle)(currentPlayTime - *currentKeyframePtr, pCinActorInterface);
					})
				}
				else {
					if (currentTrackType == 0x6e756fb7) {
						IMPLEMENTATION_GUARD_LOG(
						soundTrackBuffer = (edAnmSubControlerTag*)(trackSeekPos + 3);
						soundKeyframe = 0;
						/* Get the number of keyframes in this track */
						/* Check some time */
						if ((float)trackSeekPos[4] <= currentPlayTime) {
							currentKeyframePtr =
								edAnmSubControler::GetClosestKeyIndexSafe(currentPlayTime, &soundTrackBuffer, &soundKeyframe);
							keyframeBodyPtr =
								(float*)((int*)((int)animatedProp + 0xc) +
									(uint) * (ushort*)((int)animatedProp + 0xc) + soundKeyframe * 3);
							soundFileInfoObj = resCollection->pData[(int)keyframeBodyPtr[1]].pData;
							soundStart = keyframeBodyPtr[2];
							soundStop = keyframeBodyPtr[3];
							(*(code*)pCinActorInterface->vt->SetSound)(currentPlayTime - *currentKeyframePtr, pCinActorInterface);
						})
					}
					else {
						if (currentTrackType == 0xdbd3d7c5) {
							IMPLEMENTATION_GUARD_LOG(
							local_40 = (edAnmSubControlerTag*)(trackSeekPos + 3);
							local_10 = 0;
							if ((float)trackSeekPos[4] <= currentPlayTime) {
								currentKeyframePtr = edAnmSubControler::GetClosestKeyIndexSafe(currentPlayTime, &local_40, &local_10);
								(*(code*)pCinActorInterface->vt->SetLipsynch)
									((currentPlayTime - *currentKeyframePtr) +
										(float)((int*)((uint) * (ushort*)((int)animatedProp + 0xc) * 4 +
											(int)animatedProp))[local_10 * 2 + 5], pCinActorInterface);
							})
						}
						else {
							if (currentTrackType == 0xd9dec42c) {
								IMPLEMENTATION_GUARD_LOG(
								local_3c = (edAnmSubControlerTag*)(trackSeekPos + 3);
								local_c = 0;
								if ((float)trackSeekPos[4] <= currentPlayTime) {
									currentKeyframePtr = edAnmSubControler::GetClosestKeyIndexSafe(currentPlayTime, &local_3c, &local_c);
									pTrackDataStart =
										(int*)((int)animatedProp + 0xc) +
										(uint) * (ushort*)((int)animatedProp + 0xc) + local_c * 2;
									local_78 = pTrackDataStart + 1;
									local_74 = resCollection->pData[pTrackDataStart[1]].pData;
									local_70 = pTrackDataStart[2];
									(*(code*)pCinActorInterface->vt->SetParticles)
										(currentPlayTime - *currentKeyframePtr, pCinActorInterface);
								})
							}
							else {
								edAnmSubControlerTag* pTag = (edAnmSubControlerTag*)(pAnimProp + 1);
								if (currentTrackType == 0xdcd2e210) {
									local_38 = edAnmSubControler(pTag);
									float* pKeyTimes = pTag->keyTimes;
									local_8 = 0;
									if (pKeyTimes[0] <= currentPlayTime) {
										currentKeyframePtr = local_38.GetClosestKeyIndexSafe(currentPlayTime, &local_8);
										float fVar6 = currentPlayTime - *currentKeyframePtr;
										pTrackDataStart =(int*)((uint) * (ushort*)((ulong)pAnimProp + 0xc) * 4 + (ulong)pAnimProp) + 4;

										struct AnimTagData {
											int animResIndex;
											float field_0x4;
											float field_0x8;
											int field_0xc;
											float field_0x10;											
										};

										AnimTagData* piVar4 = (AnimTagData*)(pTrackDataStart + local_8 * 5);
										edCinActorInterface::ANIM_PARAMStag local_f0;
										local_f0.srcAnim.field_0x4 = piVar4->field_0x4 + fVar6 * piVar4->field_0x8;
										local_f0.srcAnim.pHdr = resCollection.pData->aTags[piVar4->animResIndex].pData;
										local_f0.srcAnim.field_0x8 = (piVar4->field_0xc & 1U) != 0;
										float fVar5 = piVar4->field_0x10;
										if ((fVar5 <= fVar6) || (local_8 < 1)) {
											local_f0.dstAnim.pHdr = 0;
										}
										else {
											pTrackDataStart = pTrackDataStart + local_8 * 5;
											if ((fVar5 == 0.0f) || (fVar6 <= 0.0f)) {
												local_f0.field_0x18 = 0.0f;
											}
											else {
												local_f0.field_0x18 = fVar6 / fVar5;
											}
											local_f0.dstAnim.pHdr = resCollection.pData->aTags[pTrackDataStart[-5]].pData;
											local_f0.dstAnim.field_0x8 = (pTrackDataStart[-2] & 1U) != 0;
											if (local_8 < 2) {
												local_f0.dstAnim.field_0x4 = (float)((int*)((ulong)pAnimProp + 0x10))[local_8];
											}
											else {
												currentKeyframePtr = (float*)((int*)((ulong)pAnimProp + 0x10) + local_8);
												local_f0.dstAnim.field_0x4 = *currentKeyframePtr - currentKeyframePtr[-1];
											}
											local_f0.dstAnim.field_0x4 = (local_f0.dstAnim.field_0x4 - 0.01818182f) * (float)pTrackDataStart[-3] + (float)pTrackDataStart[-4];
										}
										CUTSCENE_LOG(LogLevel::Info, "edSceneActor::Timeslice Key update name: {} anim: ", this->pObj->name);
										pCinActorInterface->SetAnim(&local_f0);
									}
								}
								else {
									if (currentTrackType == 0x64c8d3b1) {
										/* ROTATION */
										sVar2 = pAnimProp->propType;
										if (sVar2 == 2) {
											bUpdateRotationSuccess = animatedProp.GetQuaternionValue(currentPlayTime, &outRotation);
											if (bUpdateRotationSuccess != false) {
												CUTSCENE_LOG_WIN(LogLevel::Info, "edSceneActor::Timeslice Key update name: {} heading: {}", this->pObj->name, outRotation.ToString());
												pCinActorInterface->SetHeadingQuat(outRotation.x, outRotation.y, outRotation.z, outRotation.w);
											}
										}
										else {
											if (sVar2 == 1) {
												IMPLEMENTATION_GUARD(
												uVar1 = *(ushort*)(trackSeekPos + 3);
												pTrackDataStart = trackSeekPos + 4;
												if (currentPlayTime < (float)trackSeekPos[4]) {
													bUpdateRotationSuccess = false;
												}
												else {
													local_34 = (edAnmSubControlerTag*)(trackSeekPos + 3);
													edAnmSubControler::GetKeyIndicesAndRatioSafe
													(currentPlayTime, &local_34, &local_b8, &local_b4, '\0');
													fVar4 = (float)pTrackDataStart[local_b4] - (float)pTrackDataStart[local_b8];
													if (fVar4 != 0.0) {
														fVar4 = (currentPlayTime - (float)pTrackDataStart[local_b8]) / fVar4;
													}
													edF32Vector3LERPSoft
													(fVar4, &outRotation, (edF32VECTOR4*)(pTrackDataStart + uVar1 + local_b8 * 3),
														(edF32VECTOR4*)(pTrackDataStart + uVar1 + local_b4 * 3));
													bUpdateRotationSuccess = true;
												}
												if (bUpdateRotationSuccess) {
													(*(code*)pCinActorInterface->vt->SetHeadingQuat)
														(outRotation.x, outRotation.y, outRotation.z, pCinActorInterface);
												})
											}
											else {
												if (sVar2 == 0) {
													IMPLEMENTATION_GUARD(
													uVar1 = *(ushort*)(trackSeekPos + 3);
													pTrackDataStart = trackSeekPos + 4;
													if (currentPlayTime < (float)trackSeekPos[4]) {
														bUpdateRotationSuccess = false;
													}
													else {
														local_30 = (edAnmSubControlerTag*)(trackSeekPos + 3);
														edAnmSubControler::GetKeyIndicesAndRatioSafe
														(currentPlayTime, &local_30, &local_b0, &local_ac, '\0');
														fVar4 = (float)pTrackDataStart[local_ac] - (float)pTrackDataStart[local_b0];
														if (fVar4 != 0.0) {
															fVar4 = (currentPlayTime - (float)pTrackDataStart[local_b0]) / fVar4;
														}
														edF32Vector3LERPSoft
														(fVar4, &outRotation, (edF32VECTOR4*)(pTrackDataStart + uVar1 + local_b0 * 3),
															(edF32VECTOR4*)(pTrackDataStart + uVar1 + local_ac * 3));
														bUpdateRotationSuccess = true;
													}
													if (bUpdateRotationSuccess) {
														CUTSCENE_LOG_WIN(LogLevel::Info, "edSceneActor::Timeslice Key update name: {} heading: {}", this->pObj->name, outRotation.ToString());
														(*(code*)pCinActorInterface->vt->SetHeadingQuat)
															(outRotation.x, outRotation.y, outRotation.z, pCinActorInterface);
													})
												}
											}
										}
									}
									else {
										edAnmSubControlerTag* pTag = (edAnmSubControlerTag*)(pAnimProp + 1);
										if (currentTrackType == 0x6c6163b8) {
											ushort keyframeCount = pTag->keyCount;
											float* pKeyTimes = pTag->keyTimes;
											if (currentPlayTime < pKeyTimes[0]) {
												bUpdateRotationSuccess = false;
											}
											else {
												local_2c = edAnmSubControler(pTag);
												local_2c.GetKeyIndicesAndRatioSafe(currentPlayTime, &local_a8, &local_a4, 0);
												fVar4 = pKeyTimes[local_a4] - pKeyTimes[local_a8];
												if (fVar4 != 0.0f) {
													fVar4 = (currentPlayTime - pKeyTimes[local_a8]) / fVar4;
												}
												edF32Vector3LERPSoft(fVar4, &local_68, 
													(edF32VECTOR3*)(pKeyTimes + keyframeCount + local_a8 * 3),
													(edF32VECTOR3*)(pKeyTimes + keyframeCount + local_a4 * 3));
												bUpdateRotationSuccess = true;
											}
											if (bUpdateRotationSuccess) {
												CUTSCENE_LOG_WIN(LogLevel::Info, "edSceneActor::Timeslice Key update name: {} scale: {}", this->pObj->name, local_68.ToString());
												pCinActorInterface->SetScale(local_68.x, local_68.y, local_68.z);
											}
										}
										else {
											if (currentTrackType == 0xd7e2d8c4) {
												ushort keyframeCount = pTag->keyCount;
												float* pKeyTimes = pTag->keyTimes;
												if (currentPlayTime < pKeyTimes[0]) {
													bUpdateRotationSuccess = false;
												}
												else {
													locationSceStart = edAnmSubControler(pTag);
													/* LOCATION */
													locationSceStart.GetKeyIndicesAndRatioSafe(currentPlayTime, &locationOutKeyframe, &locationOutNextKeyframe, 0);
													
													fVar4 = pKeyTimes[locationOutNextKeyframe] - pKeyTimes[locationOutKeyframe];
													if (fVar4 != 0.0f) {
														fVar4 = (currentPlayTime - pKeyTimes[locationOutKeyframe]) / fVar4;
													}
													/* Also responsible for leaves */
													edF32Vector3LERPSoft(fVar4, &locationOutVector.xyz,
														(edF32VECTOR3*)(pKeyTimes + keyframeCount + locationOutKeyframe * 3),
														(edF32VECTOR3*)(pKeyTimes + keyframeCount + locationOutNextKeyframe * 3));
													bUpdateRotationSuccess = true;
												}
												if (bUpdateRotationSuccess) {
													CUTSCENE_LOG_WIN(LogLevel::Info, "edSceneActor::Timeslice Key update name: {} pos: {}", this->pObj->name, locationOutVector.ToString());
													pCinActorInterface->SetPos(locationOutVector.x, locationOutVector.y, locationOutVector.z);
												}
											}
											else {
												if ((currentTrackType == 0xd2df4b2c) && (pTag->keyTimes[0] <= currentPlayTime)) {
													ushort keyframeCount = pTag->keyCount;
													local_24 = edAnmSubControler(pTag);
													local_24.GetClosestKeyIndexSafe(currentPlayTime, &local_20);

													const bool visibilityValue = (*(char*)(((char*)pAnimProp) + local_20 + (uint)keyframeCount * 4 + 0x10)) != '\0';
													CUTSCENE_LOG(LogLevel::Info, "edSceneActor::Timeslice Key update name: {} vis: {}", this->pObj->name, visibilityValue);
													pCinActorInterface->SetVisibility(visibilityValue);
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
			/* Move to the next element section */
			pAnimProp = (edAnimatedPropertyTag*)(((char*)pAnimProp) + pAnimProp->size);
		}
		pCinActorInterface->OnFrameDirected();
	}
	return true;
}

edSCENEtag* edScene::Create(void* inFileBuffer, uint fileLength, edCinGameInterface& loadObj)
{
	int iVar1;
	edSCENEtag* dataPtr;
	char* returnPtr;
	uint uVar2;
	char* internalSeekPos;
	int seekCounter;
	char* pCVar3;
	char* filePath;
	int seekIncrement;
	CameraInfo* local_30;
	//edSceneLight_VTable* local_2c;
	CameraInfo* local_28;
	//edSceneLight_VTable* local_24;
	CameraInfo* cineCreature;
	//edSceneLight_VTable* local_1c;
	CameraInfo* local_18;
	//edSceneLight_VTable* local_14;
	CameraInfo* cachedReturn;
	CameraInfo* cineCamera;
	edResCollectionHeader* resPtr;
	char* fileBuffer;

	if (inFileBuffer == (void*)0x0) {
		dataPtr = (edSCENEtag*)0x0;
	}
	else {
		resPtr = (edResCollectionHeader*)0x0;
		dataPtr = (edSCENEtag*)0x0;
		fileBuffer = (char*)inFileBuffer;
		if ((char*)inFileBuffer < (char*)inFileBuffer + fileLength) {
			do {
				seekCounter = *(int*)fileBuffer;
				/* If the data we read from the buffer == 'DATA' */
				if (seekCounter == 0x21534552) {
					resPtr = (edResCollectionHeader*)(fileBuffer + 8);
				}
				else {
					/* If the data we read from the buffer == 'RES!' */
					if (seekCounter == 0x41544144) {
						dataPtr = (edSCENEtag*)(fileBuffer + 8);
					}
					else {
						/* If the data we read from the buffer == 'SCE!' */
						if ((seekCounter == 0x21454353) && (*(int*)(fileBuffer + 8) != 100)) {
							return (edSCENEtag*)0x0;
						}
					}
				}
				fileBuffer = fileBuffer + *(int*)(fileBuffer + 4) + 8;
			} while (fileBuffer < (char*)inFileBuffer + fileLength);
		}
		/* Seems to have searched through the file looking for Data4?
		   Also potentially found the RES section */
		if (dataPtr == (edSCENEtag*)0x0) {
			dataPtr = (edSCENEtag*)0x0;
		}
		else {
			this->pTag = dataPtr;
			seekCounter = 0;
			this->pTag->pCollection = STORE_SECTION(resPtr);
			int pcVar1 = resPtr->resCount;
			filePath = (char*)resPtr + pcVar1 * sizeof(edResCollectionTag) + sizeof(edResCollectionHeader) + 1;
			if (0 < pcVar1) {

				do {
					edResCollectionTag* pResData = &resPtr->aTags[seekCounter];

					uVar2 = pResData->flags & 0x7fffffff;
					if (uVar2 == 7) {
						/* Set some value along the res ptr */
						pResData->pData = STORE_SECTION(filePath);
					}
					else {
						if ((uVar2 != 1) && (uVar2 != 4)) {
							/* Will call Load Cutscene File */
							returnPtr = loadObj.GetResource((edResCollection::RES_TYPE)uVar2, (pResData->flags & 0x80000000) != 0, filePath, &pResData->size);
							/* Store the pointer we got back */
							pResData->pData = STORE_SECTION(returnPtr);
							/* If the pointer we got back was null, then exit */
							if (pResData->pData == 0) {
								return (edSCENEtag*)0x0;
							}
						}
					}
					seekCounter = seekCounter + 1;
					filePath = filePath + (byte)filePath[-1];
				} while (seekCounter < (ulong)pcVar1);
			}
			/* Load the rest of the cutscene assets */
			seekIncrement = 0;
			seekCounter = this->pTag->size;
			pCVar3 = (char*)(this->pTag + 1);
			if (0 < seekCounter) {
				do {
					iVar1 = *(int*)(pCVar3 + 8);
					if (iVar1 == -0x2725f4b1) {
						CUTSCENE_LOG(LogLevel::Warning, "Skipping Light\n");
						//local_2c = &edSceneLight_VTable_00441cc0;
						//local_30 = (CameraInfo*)pCVar3;
						//(*(code*)edSceneLight_VTable_00441cc0.Create)(&local_30, loadObj);
					}
					else {
						if (iVar1 == -0x596394d9) {
							CUTSCENE_LOG(LogLevel::Warning, "Skipping Light\n");
							//local_24 = &edSceneLight_VTable_00441cd0;
							//local_28 = (CameraInfo*)pCVar3;
							//(*(code*)edSceneLight_VTable_00441cd0.Create)(&local_28, loadObj);
						}
						else {
							if (iVar1 == 0x551369d) {
								edSceneScenery scenery = { (edSceneSceneryTag*)pCVar3 };
								edResCollection resCol = { resPtr };
								scenery.Create(loadObj, resCol);
							}
							else {
								/* Sets up cutscene named elements
								   Example: SC_SOUND_EMITTER or A983538304 or ARAIGNOSBLACK_TOONPLAYER_L0 */
								if (iVar1 == 0x395f05b1) {
									edSceneActorVirtual virtualActor = edSceneActorVirtual((CineCreatureObject*)pCVar3);
									edResCollection resCol = { resPtr };
									virtualActor.Create(loadObj, resCol);
								}
								else {
									if (iVar1 == 0x3d4c64aa) {
										edSceneActorBehavior behaviorActor = edSceneActorBehavior((CineCreatureObject*)pCVar3);
										edResCollection resCol = { resPtr };
										behaviorActor.Create(loadObj, resCol);
									}
									else {
										if (iVar1 == 0x2a043536) {
											edSceneCamera pCamTest = { (CameraInfo*)pCVar3 };
											pCamTest.Create(loadObj);
										}
									}
								}
							}
						}
					}
					seekIncrement = seekIncrement + 1;
					int offset = *(int*)pCVar3;
					pCVar3 = pCVar3 + offset;
				} while (seekIncrement < seekCounter);
			}
			dataPtr = this->pTag;
		}
	}
	return dataPtr;
}

bool edScene::Initialize()
{
	int iVar1;
	int iVar2;
	int iVar3;
	char* pBuffer;
	edSceneCamera local_28;
	//edSceneLight_VTable* local_24;
	edSceneCamera local_20;
	//edSceneLight_VTable* local_1c;
	edSceneCamera local_18;
	//edSceneLight_VTable* local_14;
	edSceneCamera local_10;
	//edSceneLight_VTable* local_c;
	edSceneCamera local_4;

	iVar3 = 0;
	iVar1 = this->pTag->size;
	pBuffer = (char*)(this->pTag + 1);
	if (0 < iVar1) {
		do {
			iVar2 = *(int*)((ulong)pBuffer + 8);
			if (iVar2 == -0x2725f4b1) {
				CUTSCENE_LOG(LogLevel::Warning, "Skipping Light\n");
				//local_24 = &edSceneLight_VTable_00441cc0;
				//local_28 = (CameraInfo*)pBuffer;
				//edSceneLight::Initialize((edSceneLight*)&local_28);
			}
			else {
				if (iVar2 == -0x596394d9) {
					CUTSCENE_LOG(LogLevel::Warning, "Skipping Light\n");
					//local_1c = &edSceneLight_VTable_00441cd0;
					//local_20 = (CameraInfo*)pBuffer;
					//edSceneLight::Initialize((edSceneLight*)&local_20);
				}
				else {
					if (iVar2 == 0x395f05b1) {
						edSceneActorVirtual virtualActor = edSceneActorVirtual((CineCreatureObject*)pBuffer);
						virtualActor.Initialize();
					}
					else {
						if (iVar2 == 0x3d4c64aa) {
							edSceneActorBehavior behaviorActor = edSceneActorBehavior((CineCreatureObject*)pBuffer);
							behaviorActor.Initialize();
						}
						else {
							if (iVar2 == 0x2a043536) {
								edSceneCamera sceneCamera = { (CameraInfo*)pBuffer };
								sceneCamera.Initialize();
							}
						}
					}
				}
			}
			iVar3 = iVar3 + 1;
			pBuffer = (char*)((ulong)pBuffer + *(int*)pBuffer);
		} while (iVar3 < iVar1);
	}
	return true;
}


bool edScene::Timeslice(float currentPlayTime, uint param_3)
{
	int iVar1;
	char* sceSeek;
	//edSceneLight local_28;
	//edSceneLight_VTable* local_24;
	//edSceneLight local_20;
	//edSceneLight_VTable* local_1c;
	//edSceneLight_VTable* local_14;
	//edSceneActor sceSeekPtr;
	//edSceneLight_VTable* local_c;
	int elementType;
	int numElements;

	iVar1 = 0;
	numElements = this->pTag->size;
	edResCollection local_4 = { (edResCollectionHeader*)LOAD_SECTION(this->pTag->pCollection) };
	sceSeek = (char*)(this->pTag + 1);
	if (0 < numElements) {
		do {
			elementType = *(int*)(sceSeek + 8);
			if (elementType == -0x2725f4b1) {
				//local_24 = &edSceneLight_VTable_00441cc0;
				//local_28 = (int)sceSeek;
				//edSceneLight::Timeslice(currentPlayTime, &local_28);
			}
			else {
				if (elementType == -0x596394d9) {
					//local_1c = &edSceneLight_VTable_00441cd0;
					//local_20 = (int)sceSeek;
					//edSceneLight::Timeslice(currentPlayTime, &local_20);
				}
				else {
					if (elementType == 0x395f05b1) {
						edSceneActorVirtual virtualActor = edSceneActorVirtual((CineCreatureObject*)sceSeek);
						virtualActor.Timeslice(currentPlayTime, local_4);
					}
					else {
						if (elementType == 0x3d4c64aa) {
							edSceneActorBehavior behaviorActor = edSceneActorBehavior((CineCreatureObject*)sceSeek);
							behaviorActor.Timeslice(currentPlayTime, local_4);
						}
						else {
							CameraInfo* camInfo = (CameraInfo*)sceSeek;
							edSceneCamera cam = { camInfo };
							if ((elementType == 0x2a043536) &&
								(param_3 == camInfo->field_0xc)) {
								cam.Timeslice(currentPlayTime);
							}
						}
					}
				}
			}
			iVar1 = iVar1 + 1;
			sceSeek = sceSeek + *(int*)sceSeek;
		} while (iVar1 < numElements);
	}
	return true;
}

void edSceneScenery::Create(edCinGameInterface& loadObj, edResCollection& collection)
{
	edCinGameInterface::SCENERY_CREATIONtag creationTag;
	edSceneSceneryTag* pSceneryTag;

	pSceneryTag = this->pTag;
	/* This name is the root name of the mesh and texture combo
	   Example: AIRLIFT_TO_NATIV.g2d and AIRLIFT_TO_NATIV.G3D would be
	   AIRLIFT_TO_NATIV */
	strcpy((char*)&creationTag, pSceneryTag->name);
	if (pSceneryTag->textureOffset == -1) {
		creationTag.szTexturePath = (char*)0x0;
		creationTag.textureType = 0;
	}
	else {
		creationTag.szTexturePath = collection.GetResFilename(pSceneryTag->textureOffset);
		creationTag.textureType = (collection.pData->aTags[pSceneryTag->textureOffset].flags & 0x80000000U) != 0;
	}
	if (pSceneryTag->meshOffset == -1) {
		creationTag.szMeshPath = (char*)0x0;
		creationTag.meshType = 0;
	}
	else {
		creationTag.szMeshPath = collection.GetResFilename(pSceneryTag->meshOffset);
		creationTag.meshType = (collection.pData->aTags[pSceneryTag->meshOffset].flags & 0x80000000U) != 0;
	}
	loadObj.CreateScenery((edCinSceneryInterface**)(this->pTag + 1), &creationTag);
	return;
}