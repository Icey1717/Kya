#include "CinScene.h"
#include "CinResCollection.h"
#include "Cinematic.h"

#include "port/pointer_conv.h"
#include "MathOps.h"
#include <string>

#define CUTSCENE_LOG(level, format, ...) MY_LOG_CATEGORY("Cutscene", level, format, ##__VA_ARGS__)

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

	CUTSCENE_LOG(LogLevel::Info, "edSceneCamera::Create %s", cameraName);

	edCinCamInterface* cinCam;
	pCinematic.GetCamera(&cinCam, &tag);
	cameraInfoPtr->cinCam = STORE_SECTION(cinCam);
	return;
}

void edSceneCamera::Initialize()
{
	((edCinCamInterface*)LOAD_SECTION(pInternal->cinCam))->Activate();
}

float* edAnmSubControler::GetClosestKeyIndex(float param_1, int* param_3)
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
	if (param_1 < pfVar5[uVar3 - 1]) {
		IMPLEMENTATION_GUARD();
		peVar4 = peVar4 + 1;
		if ((((8 < uVar1) && (iVar2 = (int)(uVar1 - 1) >> 2, *(float*)(&peVar4->keyCount + iVar2 * 2) <= param_1)) &&
			(peVar4 = (edAnmSubControlerTag*)(&peVar4->keyCount + iVar2 * 2),
				*(float*)(&peVar4->keyCount + iVar2 * 2) <= param_1)) &&
			(peVar4 = (edAnmSubControlerTag*)(&peVar4->keyCount + iVar2 * 2),
				*(float*)(&peVar4->keyCount + iVar2 * 2) <= param_1)) {
			peVar4 = (edAnmSubControlerTag*)(&peVar4->keyCount + iVar2 * 2);
		}
		for (; *(float*)peVar4 <= param_1; peVar4 = (edAnmSubControlerTag*)&peVar4->keyTimes) {
		}
		peVar4 = (edAnmSubControlerTag*)((int)peVar4 - (int)pfVar5);
		if ((int)peVar4 < 0) {
			peVar4 = (edAnmSubControlerTag*)((int)&peVar4->field_0x2 + 1);
		}
		uVar3 = (int)peVar4 >> 2;
	}
	*param_3 = uVar3 - 1;
	return pfVar5 + (uVar3 - 1);
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
	edAnmSubControler rotationKeyframeData = { pSubTag };
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
	edF32VECTOR4 outLocation;
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
		edAnimatedPropertyTag* pAnimProp = (edAnimatedPropertyTag*)(pCVar2->name + pCVar2->field_0x4 + -0x14);
		//cutsceneElementPtr = (float*)(pCVar2->name + pCVar2->field_0x4 + -0x14);
		if (0 < trackIndex) {
			do {
				trackIndex = trackIndex + -1;
				type = pAnimProp->type;
				//rotationBaseData = cutsceneElementPtr;
				if (type == 0x64c8d3b1) {
					edAnimatedProperty anmProp = { pAnimProp };
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
								edAnmSubControler local_18 = { pTag };
								local_18.GetKeyIndicesAndRatioSafe(currentPlayTime, &local_48, &local_44, 0)
									;
								currentKeyframePlayTime = baseKeyframeData[local_44] - baseKeyframeData[local_48];
								if (currentKeyframePlayTime != 0.0) {
									currentKeyframePlayTime = (currentPlayTime - baseKeyframeData[local_48]) / currentKeyframePlayTime;
								}
								edF32Vector3LERPSoft(currentKeyframePlayTime, &outVector,
									(edF32VECTOR4*)(baseKeyframeData + keyframeCount + local_48 * 3),
									(edF32VECTOR4*)(baseKeyframeData + keyframeCount + local_44 * 3));
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
									edAnmSubControler locationKeyFrameData = { pTag };
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
									(currentKeyframePlayTime, &outVector,
										(edF32VECTOR4*)(baseKeyframeData + keyframeCount + locationCurrentKeyFrame * 3),
										(edF32VECTOR4*)(baseKeyframeData + keyframeCount + locationNextKeyframe * 3));
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
							edAnmSubControler keyframeCountPtr = { pTag };
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
								(edF32VECTOR4*)(baseKeyframeData + keyframeCount + locationCurrentKeyframeB * 3),
								(edF32VECTOR4*)(baseKeyframeData + keyframeCount + locationNextKeyframeB * 3));
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
								edAnmSubControler scaleKeyframeData = { pTag };
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

PACK(
	struct CineCreatureObjectHeader {
	int field_0x0;
	int field_0x4;
	undefined field_0x8;
	undefined field_0x9;
	undefined field_0xa;
	undefined field_0xb;
	undefined field_0xc;
	undefined field_0xd;
	undefined field_0xe;
	undefined field_0xf;
	int field_0x10;
	int cutNameActor; // CutsceneNameActorB*
});

PACK(
	struct CineCreatureObject : public edCinActorInterface {
	CineCreatureObjectHeader header;
	char name[32];
	edF32VECTOR4 field_0x38;
	int meshID;
	int textureID;
	edF32VECTOR4 field_0x50;
	edF32VECTOR4 field_0x60;
	undefined field_0x70;
	undefined field_0x71;
	undefined field_0x72;
	undefined field_0x73;
	undefined field_0x74;
	undefined field_0x75;
	undefined field_0x76;
	undefined field_0x77;
	undefined field_0x78;
	undefined field_0x79;
	undefined field_0x7a;
	undefined field_0x7b;
	undefined field_0x7c;
	undefined field_0x7d;
	undefined field_0x7e;
	undefined field_0x7f;
	undefined field_0x80;
	undefined field_0x81;
	undefined field_0x82;
	undefined field_0x83;
	undefined field_0x84;
	undefined field_0x85;
	undefined field_0x86;
	undefined field_0x87;
	undefined field_0x88;
	undefined field_0x89;
	undefined field_0x8a;
	undefined field_0x8b;
	undefined field_0x8c;
	undefined field_0x8d;
	undefined field_0x8e;
	undefined field_0x8f;
	undefined field_0x90;
	undefined field_0x91;
	undefined field_0x92;
	undefined field_0x93;
	undefined field_0x94;
	undefined field_0x95;
	undefined field_0x96;
	undefined field_0x97;
	undefined field_0x98;
	undefined field_0x99;
	undefined field_0x9a;
	undefined field_0x9b;
	undefined field_0x9c;
	undefined field_0x9d;
	undefined field_0x9e;
	undefined field_0x9f;
	undefined field_0xa0;
	undefined field_0xa1;
	undefined field_0xa2;
	undefined field_0xa3;
	undefined field_0xa4;
	undefined field_0xa5;
	undefined field_0xa6;
	undefined field_0xa7;
	undefined field_0xa8;
	undefined field_0xa9;
	undefined field_0xaa;
	undefined field_0xab;
	undefined field_0xac;
	undefined field_0xad;
	undefined field_0xae;
	undefined field_0xaf;
	undefined field_0xb0;
	undefined field_0xb1;
	undefined field_0xb2;
	undefined field_0xb3;
	undefined field_0xb4;
	undefined field_0xb5;
	undefined field_0xb6;
	undefined field_0xb7;
	undefined field_0xb8;
	undefined field_0xb9;
	undefined field_0xba;
	undefined field_0xbb;
	undefined field_0xbc;
	undefined field_0xbd;
	undefined field_0xbe;
	undefined field_0xbf;
	undefined field_0xc0;
	undefined field_0xc1;
	undefined field_0xc2;
	undefined field_0xc3;
	undefined field_0xc4;
	undefined field_0xc5;
	undefined field_0xc6;
	undefined field_0xc7;
	undefined field_0xc8;
	undefined field_0xc9;
	undefined field_0xca;
	undefined field_0xcb;
	undefined field_0xcc;
	undefined field_0xcd;
	undefined field_0xce;
	undefined field_0xcf;
	undefined field_0xd0;
	undefined field_0xd1;
	undefined field_0xd2;
	undefined field_0xd3;
	undefined field_0xd4;
	undefined field_0xd5;
	undefined field_0xd6;
	undefined field_0xd7;
	undefined field_0xd8;
	undefined field_0xd9;
});

struct edSceneActorVirtual {
	CineCreatureObject* pObj;
	bool Create(edCinGameInterface& cinGameInterface, edResCollection& resCollection);
};

bool edSceneActorVirtual::Create(edCinGameInterface& cinGameInterface, edResCollection& resCollection)
{
	bool bVar1;
	edCinGameInterface::ACTORV_CREATIONtag creationTag;
	CineCreatureObject* cineCreatureObject;

	/* This sets up objects for a cutscene, and gives them a name
	   Example: SC_SOUND_EMITTER */
	cineCreatureObject = this->pObj;
	creationTag.vectorFieldA.x = (cineCreatureObject->field_0x50).x;
	creationTag.vectorFieldA.y = (cineCreatureObject->field_0x50).y;
	creationTag.vectorFieldA.z = (cineCreatureObject->field_0x50).z;
	creationTag.vectorFieldA.w = 1.0;
	creationTag.vectorFieldB.x = (cineCreatureObject->field_0x50).w;
	creationTag.vectorFieldB.y = (cineCreatureObject->field_0x60).x;
	creationTag.vectorFieldB.z = (cineCreatureObject->field_0x60).y;
	creationTag.vectorFieldB.w = (cineCreatureObject->field_0x60).z;
	creationTag.vectorFieldC.x = (cineCreatureObject->field_0x60).w;
	creationTag.vectorFieldC.y = *(float*)&cineCreatureObject->field_0x70;
	creationTag.vectorFieldC.z = *(float*)&cineCreatureObject->field_0x74;
	creationTag.vectorFieldC.w = 1.0;
	creationTag.field_0x40 = (cineCreatureObject->field_0x38).x;
	creationTag.field_0x44 = (cineCreatureObject->field_0x38).y;
	creationTag.field_0x48 = (cineCreatureObject->field_0x38).z;
	creationTag.field_0x4c = (cineCreatureObject->field_0x38).w;
	/* Loads the name of the player into this buffer.
	   Example: ARAIGNOSBLACK_TOONPLAYER_L0 */
	strcpy((char*)&creationTag, cineCreatureObject->name);

	CUTSCENE_LOG(LogLevel::Info, "edSceneActorVirtual::Create name: %s", cineCreatureObject->name);

	if (cineCreatureObject->textureID == -1) {
		creationTag.textureName = 0;
		creationTag.bHasTexture = false;
	}
	else {
		creationTag.textureName = resCollection.GetResFilename(cineCreatureObject->textureID);

		CUTSCENE_LOG(LogLevel::Info, "edSceneActorVirtual::Create texture %s", creationTag.textureName);

		creationTag.bHasTexture = (resCollection.pData[cineCreatureObject->meshID * 3 + 1].field_0x0 & 0x80000000U) != 0;
	}
	if (cineCreatureObject->meshID == -1) {
		creationTag.meshName = 0;
		creationTag.bHasMesh = false;
	}
	else {
		creationTag.meshName = resCollection.GetResFilename(cineCreatureObject->meshID);

		CUTSCENE_LOG(LogLevel::Info, "edSceneActorVirtual::Create texture %s", creationTag.meshName);

		creationTag.bHasMesh = (resCollection.pData[cineCreatureObject->meshID * 3 + 1].field_0x0 & 0x80000000U) != 0;
	}
	creationTag.field_0x30 = (cineCreatureObject->field_0x38).w;
	bVar1 = cinGameInterface.CreateActor((edCinActorInterface**)&this->pObj, &creationTag);
	return bVar1 != false;
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
	edResCollectionInternal* resPtr;
	char* fileBuffer;

	if (inFileBuffer == (void*)0x0) {
		dataPtr = (edSCENEtag*)0x0;
	}
	else {
		resPtr = (edResCollectionInternal*)0x0;
		dataPtr = (edSCENEtag*)0x0;
		fileBuffer = (char*)inFileBuffer;
		if ((char*)inFileBuffer < (char*)inFileBuffer + fileLength) {
			do {
				seekCounter = *(int*)fileBuffer;
				/* If the data we read from the buffer == 'DATA' */
				if (seekCounter == 0x21534552) {
					resPtr = (edResCollectionInternal*)(fileBuffer + 8);
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
			int pcVar1 = resPtr->field_0x0;
			filePath = (char*)((ulong)&resPtr[(ulong)pcVar1 * 3 + 1].field_0x0 + 1);
			if (0 < pcVar1) {
				seekIncrement = 0;
				do {
					internalSeekPos = (char*)((ulong)&resPtr->field_0x0 + seekIncrement);
					uVar2 = *(uint*)(internalSeekPos + 4) & 0x7fffffff;
					if (uVar2 == 7) {
						/* Set some value along the res ptr */
						*(char**)(internalSeekPos + 8) = filePath;
					}
					else {
						if ((uVar2 != 1) && (uVar2 != 4)) {
							/* Will call Load Cutscene File */
							returnPtr = loadObj.GetResource((edResCollection::RES_TYPE)uVar2,
								(ulong)((*(uint*)(internalSeekPos + 4) & 0x80000000) != 0), filePath,
								(int*)(internalSeekPos + 0xc));
							/* Store the pointer we got back */
							*(char**)(internalSeekPos + 8) = returnPtr;
							/* If the pointer we got back was null, then exit */
							if (*(int*)(internalSeekPos + 8) == 0) {
								return (edSCENEtag*)0x0;
							}
						}
					}
					seekCounter = seekCounter + 1;
					seekIncrement = seekIncrement + 0xc;
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
								CUTSCENE_LOG(LogLevel::Warning, "Skipping edSceneScenery::Create\n");
								//cachedReturn = (CameraInfo*)pCVar3;
								//edSceneScenery::Create((int*)&cachedReturn, loadObj, &resPtr);
							}
							else {
								/* Sets up cutscene named elements
								   Example: SC_SOUND_EMITTER or A983538304 or ARAIGNOSBLACK_TOONPLAYER_L0 */
								if (iVar1 == 0x395f05b1) {

									CUTSCENE_LOG(LogLevel::Warning, "Skipping Virtual Actor\n");
									edSceneActorVirtual virtualActor;
									virtualActor.pObj = (CineCreatureObject*)pCVar3;
									edResCollection resCol = { resPtr };
									virtualActor.Create(loadObj, resCol);
									//local_1c = &edSceneLight_VTable_00441ca0;
									//cineCreature = (CameraInfo*)pCVar3;
									//(*(code*)edSceneLight_VTable_00441ca0.Create)(&cineCreature, loadObj);
								}
								else {
									if (iVar1 == 0x3d4c64aa) {
										CUTSCENE_LOG(LogLevel::Warning, "Skipping Behaviour Actor\n");
										//local_14 = &edSceneLight_VTable_00441cb0;
										//local_18 = (CameraInfo*)pCVar3;
										//(*(code*)edSceneLight_VTable_00441cb0.Create)(&local_18, loadObj);
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
						CUTSCENE_LOG(LogLevel::Warning, "Skipping Actor\n");
						//local_14 = &edSceneActorVirtual_VTable_00441ca0;
						//local_18 = (CameraInfo*)pBuffer;
						//edSceneActor::Initialize((edSceneActor*)&local_18);
					}
					else {
						if (iVar2 == 0x3d4c64aa) {
							CUTSCENE_LOG(LogLevel::Warning, "Skipping Actor\n");
							//local_c = &edSceneLight_VTable_00441cb0;
							//local_10 = (CameraInfo*)pBuffer;
							//edSceneActor::Initialize((edSceneActor*)&local_10);
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
	//edSceneActor local_18;
	//edSceneLight_VTable* local_14;
	//edSceneActor sceSeekPtr;
	//edSceneLight_VTable* local_c;
	int elementType;
	int numElements;

	iVar1 = 0;
	numElements = this->pTag->size;
	edResCollection local_4 = { (edResCollectionInternal*)LOAD_SECTION(this->pTag->pCollection) };
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
						//local_14 = &edSceneActorVirtual_VTable_00441ca0;
						//local_18 = (int)sceSeek;
						///* Runs in tunnel cutscene */
						//edSceneActor::Timeslice(currentPlayTime, &local_18, &local_4);
					}
					else {
						if (elementType == 0x3d4c64aa) {
							//local_c = &edSceneLight_VTable_00441cb0;
							//sceSeekPtr = (int)sceSeek;
							//edSceneActor::Timeslice(currentPlayTime, &sceSeekPtr, &local_4);
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
