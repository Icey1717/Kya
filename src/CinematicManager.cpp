#include "CinematicManager.h"
#include "CameraViewManager.h"
#include "CameraCinematic.h"
#include "MemoryStream.h"
#include "FileManager3D.h"
#include "Rendering/CameraPanMasterHeader.h"
#include "SectorManager.h"
#include "TimeController.h"
#include "Pause.h"
#include "MathOps.h"
#include "LevelScheduleManager.h"
#include "edStr.h"
#include "port/pointer_conv.h"

#include "edBankFile.h"
#include "edFile.h"

#ifdef PLATFORM_PS2
#include <eekernel.h>
#include <math.h>
#else
#include "port.h"
#endif

#include <assert.h>
#include <string.h>
#include "kya.h"
#include "FrontendDisp.h"
#include "InputManager.h"
#include "Actor_Cinematic.h"
#include "ActorManager.h"
#include "ActorHero.h"
#include "Actor.h"
#include "EventManager.h"
#include "Rendering/DisplayList.h"
#include "edVideo/VideoD.h"
#include "Rendering/edCTextFormat.h"
#include "Rendering/edCTextFont.h"
#include "edText.h"
#include "edVideo/VideoA.h"
#include "TranslatedTextData.h"
#include "EdFileBase.h"
#include "EdenLib/edParticles/include/edParticles.h"

CCinematicManager* g_CinematicManager_0048efc;

#define CUTSCENE_MAX_LOAD_TIME 15.0f

#define CUTSCENE_LOG(level, format, ...) MY_LOG_CATEGORY("Cutscene", level, format, ##__VA_ARGS__)

struct CinematicOption {
	char* name;
	undefined4 field_0x4;
	undefined4 field_0x8;
};

class BWCinActor {
public:
	static void InitCinMsgs();

	static int _gNbCinMsgs;
	static CinematicOption _gCinMsgs[36];
};

int BWCinActor::_gNbCinMsgs = 0;
CinematicOption BWCinActor::_gCinMsgs[36] = {
	{ "ANIM_SMOOTH_OFF", 0x1, 0x0 },
	{ "ANIM_SMOOTH_ON", 0x2, 0x0 },
	{ "AUTO_BLINK_OFF", 0x3, 0x0 },
	{ "AUTO_BLINK_ON", 0x4, 0x0 },
	{ "AUTO_MOOD_OFF", 0x6, 0x0 },
	{ "AUTO_MOOD_ON", 0x7, 0x0 },
	{ "BIG_MOTOR_HIGH_LONG", 0xD, 0x74 },
	{ "BIG_MOTOR_HIGH_MEDIUM", 0xD, 0x74 },
	{ "BIG_MOTOR_HIGH_SHORT", 0xD, 0x73 },
	{ "BIG_MOTOR_LOW_LONG", 0xD, 0x6F },
	{ "BIG_MOTOR_LOW_MEDIUM", 0xD, 0x6E },
	{ "BIG_MOTOR_LOW_SHORT", 0xD, 0x6FD},
	{ "BIG_MOTOR_MEDIUM_LONG", 0xD, 0x72 },
	{ "BIG_MOTOR_MEDIUM_MEDIUM", 0xD, 0x71 },
	{ "BIG_MOTOR_MEDIUM_SHORT", 0xD, 0x70 },
	{ "BLINK", 0x5, 0x0 },
	{ "BOOMY_CHANGED", 0xF, MESSAGE_BOOMY_CHANGED },
	{ "DRAW_MOOD_OFF", 0xE, 0x0 },
	{ "DRAW_MOOD_ON", 0xE, 0x1 },
	{ "FIGHT_RING_CHANGED", 0x10, MESSAGE_FIGHT_RING_CHANGED },
	{ "LIPSYNC_OFF", 0xC, 0x0 },
	{ "LIPSYNC_ON", 0xB, 0x0 },
	{ "MOOD_ANGER", 0x8, 0x1 },
	{ "MOOD_DISGUST", 0x8, 0x02 },
	{ "MOOD_FEAR", 0x8, 0x03 },
	{ "MOOD_JOY", 0x8, 0x04 },
	{ "MOOD_NONE", 0x8, 0x00 },
	{ "MOOD_SORROW", 0x8, 0x05 },
	{ "MOOD_SURPRISE", 0x8, 0x06 },
	{ "RECEPTACLE_CHANGED", 0x11, MESSAGE_RECEPTACLE_CHANGED },
	{ "SMALL_MOTOR_HIGH_LONG", 0xD, 0x78 },
	{ "SMALL_MOTOR_HIGH_MEDIUM", 0xD, 0x76 },
	{ "SMALL_MOTOR_HIGH_SHORT", 0xD, 0x76 },
	{ "STICK_GROUND_OFF", 0xA, 0x0 },
	{ "STICK_GROUND_ON", 0x9, 0x0 },
	{ NULL, 0x0, 0x0 }
};

void BWCinActor::InitCinMsgs(void)
{
	CinematicOption* pCVar1;

	_gNbCinMsgs = 0;
	if (_gCinMsgs[0].name != (char*)0x0) {
		pCVar1 = _gCinMsgs;
		do {
			pCVar1 = pCVar1 + 1;
			_gNbCinMsgs = _gNbCinMsgs + 1;
		} while (pCVar1->name != (char*)0x0);
	}
	return;
}


CCinematicManager::CCinematicManager()
{
	pCinematic = (CCinematic*)0x0;
	field_0x20 = -1;
	field_0x24 = -1;
	g_CinematicManager_0048efc = this;
	ppCinematicObjB_A = (CCinematic**)0x0;
	numCutscenes_0x8 = 0;
	ppCinematicObjB_B = (CCinematic**)0x0;
	activeCinematicCount = 0;
	pCurCinematic = (CCinematic*)0x0;
	field_0x28 = (CCinematic*)0x0;
	startTime = 0.0f;
	return;
}

void CCinematicManager::Game_Init()
{
	BWCinActor::InitCinMsgs();
	this->bInitialized = 1;
	return;
}

void CCinematicManager::LevelLoading_Begin()
{
	CCameraCinematic* pCinematicCamera;
	CCameraCinematic* pCamera;
	CCameraManager* pCameraManager;

	pCameraManager = CCameraManager::_gThis;
	pCinematicCamera = (CCameraCinematic*)CCameraManager::_gThis->GetDefGameCamera(CT_Cinematic);
	this->pCinematicCamera = pCinematicCamera;

	if (this->pCinematic != (CCinematic*)0x0) {
		this->pCinematicCamera->SetTransform(&this->matrix_0x70);
		pCinematicCamera->fov = this->field_0xb0;
		pCamera = this->pCinematicCamera;
		pCamera->field_0x8c = 0.0f;
		pCamera->switchMode = SWITCH_MODE_B;
		pCamera->field_0x98 = 0.0f;
		pCamera->field_0x94 = SWITCH_MODE_B;
		pCamera->field_0x9c = 0.0f;
		pCameraManager->PushCamera(this->pCinematicCamera, 1);
	}

	return;
}

void CCinematicManager::LevelLoading_End()
{
	CCinematic* pCinematic;
	bool bVar1;
	CCinematicManager* pCVar2;

	pCVar2 = g_CinematicManager_0048efc;

	pCinematic = this->pCinematic;
	if (pCinematic != (CCinematic*)0x0) {
		bVar1 = (pCinematic->cinematicLoadObject).BWCinSourceAudio_Obj.intFieldA != 0;
		if ((bVar1) && (bVar1 = true, (pCinematic->cinematicLoadObject).BWCinSourceAudio_Obj.field_0x8 != 0.0f)) {
			bVar1 = false;
		}

		if (bVar1) {
			g_CinematicManager_0048efc->pCurCinematic = pCinematic;
			pCinematic->cinematicLoadObject.BWCinSourceAudio_Obj.Stop();
			pCVar2->pCurCinematic = (CCinematic*)0x0;
		}

		bVar1 = (pCinematic)->pCineBankEntry != (edCBankBufferEntry*)0x0;
		if (bVar1) {
			bVar1 = pCinematic->cineBankLoadStage_0x2b4 != 4;
		}

		if (bVar1) {
			pCinematic->Load(0);
		}

		if (pCinematic->state != CS_Stopped) {
			pCinematic->Stop();
		}

		pCinematic->flags_0x8 = pCinematic->flags_0x8 & 0xfffffffb;
		pCinematic->pActor = (CActor*)0x0;

		GlobalDList_AddToView();

#ifndef PLATFORM_WIN
		edVideoFlip();
#endif

		delete this->pCinematic;
		this->pCinematic = (CCinematic*)0x0;
	}

	this->pCinematicCamera = (CCameraCinematic*)0x0;
	return;
}

bool CCinematicManager::LevelLoading_Manage()
{
	bool bVar1;

	if (this->pCinematic == (CCinematic*)0x0) {
		bVar1 = false;
	}
	else {
		this->pSubtitleText = (char*)0x0;
		this->field_0x40 = 0;
		this->pCinematic->Manage();

		bVar1 = false;
		if ((this->pCinematic->flags_0x4 & 8) == 0) {
			bVar1 = this->pCinematic->state != CS_Stopped;
		}
	}

	return bVar1;
}

void CCinematicManager::Level_Init()
{
	CCinematic** pCurCinematic;
	int curIndex;

	this->pCinematicCamera = reinterpret_cast<CCameraCinematic*>(CCameraManager::_gThis->GetDefGameCamera(CT_Cinematic));
	this->field_0x34 = 0;
	this->pSubtitleText = (char*)0x0;
	this->field_0x40 = 0;
	this->field_0x44 = 0;
	this->vector_0x50 = gF32Vertex4Zero;
	this->fadeDuration = 0.0f;
	this->fadeColor = gF32Vector4Zero.xyz;

	pCurCinematic = this->ppCinematicObjB_A;
	for (curIndex = this->numCutscenes_0x8; curIndex != 0; curIndex = curIndex + -1) {
		(*pCurCinematic)->Init();
		pCurCinematic = pCurCinematic + 1;
	}

	return;
}

void CCinematicManager::Level_AddAll(ByteCode* pByteCode)
{
	int iVar1;
	CCinematic** ppCVar2;
	CCinematic* this_00;
	int iVar4;
	long lVar3;

	edMemSetFlags(TO_HEAP(H_MAIN), 0x100);
	pByteCode->GetChunk();
	iVar1 = pByteCode->GetS32();
	this->numCutscenes_0x8 = iVar1;
	this->activeCinematicCount = 0;
	iVar1 = this->numCutscenes_0x8;
	if (iVar1 != 0) {
		ppCVar2 = new CCinematic*[iVar1];
		this->ppCinematicObjB_A = ppCVar2;
		ppCVar2 = new CCinematic*[this->numCutscenes_0x8];
		this->ppCinematicObjB_B = ppCVar2;
		iVar1 = this->numCutscenes_0x8;
		iVar4 = 0;
		if (0 < iVar1 + 1) {
			do {
				pByteCode->GetU32();
				iVar1 = this->numCutscenes_0x8;
				iVar4 = iVar4 + 1;
			} while (iVar4 < iVar1 + 1);
		}
		iVar4 = 0;
		if (0 < iVar1) {
			iVar1 = 0;
			do {
				this_00 = new CCinematic();
				this->ppCinematicObjB_A[iVar1] = this_00;
				this->ppCinematicObjB_A[iVar1]->Create(pByteCode);
				iVar4 = iVar4 + 1;
				iVar1 = iVar1 + 1;
			} while (iVar4 < this->numCutscenes_0x8);
		}
		Level_SectorChange(-1, -1);
	}
	edMemClearFlags(TO_HEAP(H_MAIN), 0x100);
	return;
}

void CCinematicManager::Level_Manage()
{
	int iVar1;

	this->pSubtitleText = (char*)0x0;
	this->field_0x40 = 0;
	this->fadeDuration = 0.0f;

	iVar1 = 0;
	if (0 < this->activeCinematicCount) {
		do {
			this->ppCinematicObjB_B[iVar1]->Manage();
			iVar1 = iVar1 + 1;
		} while (iVar1 < this->activeCinematicCount);
	}

	if ((this->pCinematic != (CCinematic*)0x0) && (this->pCinematic->state == CS_Stopped)) {
		this->pCinematic = (CCinematic*)0x0;
		this->field_0x20 = -1;
		this->field_0x24 = -1;
	}

	return;
}

void CCinematicManager::Level_SectorChange(int param_2, int param_3)
{
	CCinematic* pCVar1;
	bool bVar2;
	CCinematic** ppCVar3;
	int iVar4;

	iVar4 = this->numCutscenes_0x8;
	ppCVar3 = this->ppCinematicObjB_A;
	this->activeCinematicCount = 0;
	for (; iVar4 != 0; iVar4 = iVar4 + -1) {
		pCVar1 = *ppCVar3;
		bVar2 = false;
		if ((pCVar1->baseB == param_3) || (pCVar1->baseB == -1)) {
			bVar2 = true;
		}
		else {
			if (((param_3 != -1) && (this->pCinematic != pCVar1)) && (pCVar1->cineBankLoadStage_0x2b4 == 4)) {
				bVar2 = pCVar1->state != CS_Stopped;
				if ((bVar2) && (bVar2)) {
					pCVar1->flags_0x8 = pCVar1->flags_0x8 & 0xffffff7f;
					pCVar1->flags_0x8 = pCVar1->flags_0x8 | 0x100;
				}
				bVar2 = true;
			}
		}
		if (bVar2) {
			this->ppCinematicObjB_B[this->activeCinematicCount] = pCVar1;
			this->activeCinematicCount = this->activeCinematicCount + 1;
		}
		ppCVar3 = ppCVar3 + 1;
	}
	if (((param_2 == -1) && (pCVar1 = this->pCinematic, pCVar1 != (CCinematic*)0x0)) &&
		((bVar2 = pCVar1->state != CS_Stopped, bVar2 && (((pCVar1->flags_0x4 & 8) != 0 && (bVar2)))))) {
		pCVar1->flags_0x8 = pCVar1->flags_0x8 & 0xffffff7f;
		pCVar1->flags_0x8 = pCVar1->flags_0x8 | 0x100;
	}
	return;
}


void CCinematicManager::WillLoadCinematic()
{
	bool bVar1;
	S_NTF_TARGET_STREAM_REF* pSwitchList;
	int iVar3;
	int iVar4;
	CCinematic** pCVar6;
	int cutsceneID;
	CCinematic* pCinematic;

	pCVar6 = this->ppCinematicObjB_A;
	/* This int field is (2) for the openining cutscene */
	for (cutsceneID = this->numCutscenes_0x8; cutsceneID != 0; cutsceneID = cutsceneID + -1) {
		pCinematic = *pCVar6;
		if ((pCinematic->flags_0x4 & 0x800) != 0) {
			bVar1 = (pCinematic->flags_0x4 & 1) != 0;

			if (bVar1) {
				bVar1 = (pCinematic->flags_0x8 & 0x400) != 0;
			}

			if (!bVar1) {
				bVar1 = (pCinematic->flags_0x8 & 0x28) != 0;
			}

			bVar1 = (bool)(bVar1 ^ 1);
			if (!bVar1) {
				bVar1 = (pCinematic->flags_0x8 & 0x800) != 0;
			}

			if ((bVar1) && ((pCinematic->baseB == -1 || (pCinematic->baseB == ((CScene::ptable.g_SectorManager_00451670)->baseSector).desiredSectorID)))) {
				pCinematic->pActor = (CActor*)0x0;
				if ((pCinematic->flags_0x8 & 0x800) == 0) {
					pSwitchList = pCinematic->pSwitchListA;
					bVar1 = false;

					if ((pSwitchList != (S_NTF_TARGET_STREAM_REF*)0x0) && (pCinematic->pStreamEventCameraA != (S_STREAM_EVENT_CAMERA*)0x0)) {
						bVar1 = true;
					}

					if (bVar1) {
						pSwitchList->Switch((CActor*)0x0);
						pCinematic->pStreamEventCameraA->SwitchOn((CActor*)0x0);
						pSwitchList->PostSwitch((CActor*)0x0);
					}
				}

				pCinematic->Start();
			}
		}

		pCVar6 = pCVar6 + 1;
	}

	return;
}

CCinematic* CCinematicManager::GetCurCinematic()
{
	return this->pCurCinematic;
}

void CCinematicManager::NotifyCinematic(int cinematicIndex, CActor* pActor, int messageId, uint flags)
{
	bool bVar1;
	CCinematic* pCinematic;
	float fVar2;
	float fVar3;

	if ((cinematicIndex < 0) || (this->numCutscenes_0x8 <= cinematicIndex)) {
		pCinematic = (CCinematic*)0x0;
	}
	else {
		pCinematic = this->ppCinematicObjB_A[cinematicIndex];
	}
	if (pCinematic != (CCinematic*)0x0) {
		if (messageId == 3) {
			pCinematic->flags_0x8 = pCinematic->flags_0x8 | 0x20;
			bVar1 = pCinematic->state != CS_Stopped;
			if ((bVar1) && (bVar1)) {
				pCinematic->flags_0x8 = pCinematic->flags_0x8 & 0xffffff7f;
				pCinematic->flags_0x8 = pCinematic->flags_0x8 | 0x100;
			}
		}
		else {
			if (messageId == 0x3b) {
				if ((int)flags < 0) {
					fVar2 = (float)(flags >> 1 | flags & 1);
					fVar2 = fVar2 + fVar2;
				}
				else {
					fVar2 = (float)flags;
				}

				pCinematic->totalCutsceneDelta = fVar2 / 1000.0f;
				if ((pCinematic->flags_0x8 & 1) != 0) {
					fVar2 = pCinematic->totalCutsceneDelta;
					bVar1 = (pCinematic->cinematicLoadObject).BWCinSourceAudio_Obj.intFieldA != 0;
					if ((bVar1) && (bVar1 = true, (pCinematic->cinematicLoadObject).BWCinSourceAudio_Obj.field_0x8 == 0.0f)) {
						bVar1 = false;
					}

					if (bVar1) {
						IMPLEMENTATION_GUARD(
						fVar3 = CutsceneTypeTwoA((pCinematic->cinematicLoadObject).BWCinSourceAudio_Obj.intFieldA);
						(pCinematic->cinematicLoadObject).BWCinSourceAudio_Obj.field_0x8 = fVar3 - fVar2;
						CutsceneTypeTwoB(fVar2, (pCinematic->cinematicLoadObject).BWCinSourceAudio_Obj.intFieldA);)
					}
				}
			}
			else {
				if (messageId == 0x38) {
					if ((pCinematic->flags_0x8 & 0x40) == 0) {
						pCinematic->flags_0x8 = pCinematic->flags_0x8 | 0x40;
					}
					else {
						pCinematic->flags_0x8 = pCinematic->flags_0x8 & 0xffffffbf;
					}
				}
				else {
					if (messageId == 0x37) {
						pCinematic->flags_0x8 = pCinematic->flags_0x8 & 0xffffffbf;
					}
					else {
						if (messageId == 0x36) {
							pCinematic->flags_0x8 = pCinematic->flags_0x8 | 0x40;
						}
						else {
							if (messageId == 0x3c) {
								pCinematic->PreReset();
							}
							else {
								if (messageId == 0xe) {
									bVar1 = pCinematic->state != CS_Stopped;
									if (bVar1) {
										if (bVar1) {
											pCinematic->flags_0x8 = pCinematic->flags_0x8 & 0xffffff7f;
											pCinematic->flags_0x8 = pCinematic->flags_0x8 | 0x100;
										}
									}
									else {
										pCinematic->TryTriggerCutscene(pActor, 0);
									}
								}
								else {
									if (messageId == 0x10) {
										bVar1 = pCinematic->state != CS_Stopped;
										if ((bVar1) && (bVar1)) {
											pCinematic->flags_0x8 = pCinematic->flags_0x8 & 0xffffff7f;
											pCinematic->flags_0x8 = pCinematic->flags_0x8 | 0x100;
										}
									}
									else {
										if ((messageId == 0xf) && (pCinematic->state == CS_Stopped)) {
											pCinematic->TryTriggerCutscene(pActor, 0);
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
	return;
}

void CCinematicManager::Level_Draw()
{
	int iVar1;
	int iVar2;
	int iVar3;
	CCinematic* pCinematic;

	if ((GameFlags & 0x200) == 0) {
		iVar3 = 0;
		iVar2 = 0;
		if (0 < this->activeCinematicCount) {

			for (int i = 0; i < this->activeCinematicCount; i++) {
				pCinematic = this->ppCinematicObjB_B[i];
				pCinematic->Draw();
				if ((pCinematic->state != CS_Stopped) && ((pCinematic->flags_0x4 & 0x200) != 0)) {
					iVar3 = 1;
				}
			}
		}
		
		DrawBandsAndSubtitle(iVar3);
	}
	return;
}

void CCinematic::InitInternalData()
{
	this->baseB = -1;
	this->flags_0x4 = 0;
	*(undefined4*)&this->field_0x10 = 0;
	this->aActorCinematic = (CActorCinematic*)0x0;
	this->nbCinematicActors = 0;
	this->ppActorCinematics = (CActorCinematic**)0x0;
	this->nbActorRefs = 0;
	this->fileInfoStart = (CinFileContainer*)0x0;
	this->cinFileCount = 0;
	this->pCineBankEntry = (edCBankBufferEntry*)0x0;
	this->totalCutsceneDelta = 0.0;
	this->pActor = (CActor*)0x0;
	this->pMeshInfo = (ed_g3d_manager*)0x0;
	this->pMeshTransform = (edNODE*)0x0;
	this->soundCount_0x2b8 = 0;
	this->sound_0x2bc = 0;
	this->playingSounds_0x2c0 = 0;
	this->field_0x2c4 = 0;
	this->field_0x2c8 = -1.0f;
	//this->pCineSunHolderArray = (CineSunHolder*)0x0;
	this->count_0x224 = 0;
	//this->pCineSpotHolderArray = (CineSpotHolder*)0x0;
	this->count_0x22c = 0;
	this->numberOfParticles = 0;
	this->nbInstalledParticles = 0;
	this->particleSectionStart = (ParticleEntry*)0x0;
	this->buffer_0x2e4 = (ParticleInstance*)0x0;
	this->prtFileBufferPool = (void*)0x0;

	return;
}

int S_STREAM_NTF_TARGET_ONOFF::SwitchOff(CActor* pActor)
{
	uint uVar1;
	bool bVar2;
	int iVar3;

	uVar1 = this->flags;
	iVar3 = 0;
	bVar2 = true;

	if (((uVar1 & 1) != 0) && ((uVar1 & 0x40000000) != 0)) {
		bVar2 = false;
	}

	if (bVar2) {
		this->flags = this->flags | 0x40000000;
		if (this->field_0x14 != 0) {
			g_CinematicManager_0048efc->NotifyCinematic(this->cutsceneId, pActor, this->field_0x14, *(uint*)&this->field_0x18);
			if (pActor == (CActor*)0x0) {
				pActor = LOAD_SECTION_CAST(CActor*, this->pRef);
			}
			if (pActor != (CActor*)0x0) {
				iVar3 = pActor->DoMessage(LOAD_SECTION_CAST(CActor*, this->pRef), (ACTOR_MESSAGE)this->field_0x14, &this->field_0x18);
			}
		}
	}

	return iVar3;
}

void CCinematic::SetupInternalData()
{
	int* piVar2;
	CActorCinematic* pCVar3;
	CActorCinematic** ppCVar4;
	//BWCinSunLight* pBVar5;
	//BWCinSpotLight* pBVar6;
	void* pvVar7;
	int nbTotalRefs;

	if ((this->baseB == 0) || ((this->flags_0x4 & 0x40000) != 0)) {
		this->baseB = -1;
	}

	if (this->nbTotalCinematicActors == 0) {
		this->aActorCinematic = (CActorCinematic*)0x0;
	}
	else {
		this->aActorCinematic = new CActorCinematic[this->nbTotalCinematicActors];
	}

	nbTotalRefs = this->nbTotalLevelActors + this->nbTotalCinematicActors;
	if (nbTotalRefs == 0) {
		this->ppActorCinematics = (CActorCinematic**)0x0;
	}
	else {
		this->ppActorCinematics = new CActorCinematic*[nbTotalRefs];
	}

	//uVar1 = this->count_0x20;
	//if (uVar1 == 0) {
	//	this->pCineSunHolderArray = (BWCinSunLight*)0x0;
	//}
	//else {
	//	piVar2 = (int*)operator.new.array((long)(int)(uVar1 * 0xc0 + 0x10));
	//	pBVar5 = (BWCinSunLight*)
	//		__construct_new_array(piVar2, BWCinSunLight::BWCinSunLight, BWCinSunLight::~BWCinSunLight, 0xc0, uVar1);
	//	this->pCineSunHolderArray = pBVar5;
	//}
	//uVar1 = this->field_0x24;
	//if (uVar1 == 0) {
	//	this->pCineSpotHolderArray = (BWCinSpotLight*)0x0;
	//}
	//else {
	//	piVar2 = (int*)operator.new.array((long)(int)(uVar1 * 0xd0 + 0x10));
	//	pBVar6 = (BWCinSpotLight*)
	//		__construct_new_array(piVar2, BWCinSpotLight::BWCinSpotLight, BWCinSpotLight::~BWCinSpotLight, 0xd0, uVar1);
	//	this->pCineSpotHolderArray = pBVar6;
	//}

	if (this->field_0x28 == 0) {
		this->particleSectionStart = (ParticleEntry*)0x0;
		this->buffer_0x2e4 = (ParticleInstance*)0x0;
		this->prtFileBufferPool = (void*)0x0;
		this->nbInstalledParticles = 0;
	}
	else {
		this->particleSectionStart = new ParticleEntry[this->field_0x28];
		this->nbInstalledParticles = this->field_0x28;
		this->nbInstalledParticles = this->nbInstalledParticles << 1;
		if (this->nbInstalledParticles < 8) {
			this->nbInstalledParticles = 8;
		}

		this->buffer_0x2e4 = new ParticleInstance[this->nbInstalledParticles];
		this->prtFileBufferPool = edMemAlloc(TO_HEAP(H_MAIN), this->field_0x2c * this->nbInstalledParticles);
	}

	return;
}

CCinematic::CCinematic()
{
	this->cineActorConfigCount = 0;
	this->aCineActorConfig = (CCineActorConfig*)0x0;
	
	// CBWitchCin Constructor
	// CBWCinSourceAudio Constructor
	// CBWCinSourceSubtitle Constructor

	//(this->cinFileData).headerPtr = (CinematicFileDataHeader*)0x0;
	InitInternalData();
	return;
}

void CCinematic::Create(ByteCode* pByteCode)
{
	S_STREAM_FOG_DEF* pfVar1;
	int* piVar2;
	float fVar3;
	float fVar4;
	uint uVar5;
	char* pcVar6;
	int iVar7;
	CCineActorConfig* piVar7;
	CCineActorConfig* ppCVar8;
	int iVar8;
	uint uVar9;
	uint uVar10;
	ulong uVar11;
	int iVar12;
	CCineActorConfig* pCineActorConfig;
	float fVar13;

	this->flags_0x8 = 0;

	this->prtBuffer = pByteCode->GetU32();

	this->pBankName_0x48 = pByteCode->GetString();
	this->pBankName_0x50 = pByteCode->GetString();
	this->fileName = pByteCode->GetString();

	CUTSCENE_LOG(LogLevel::Info, "Cinematic::Create Bank A {}", this->pBankName_0x48);
	CUTSCENE_LOG(LogLevel::Info, "Cinematic::Create Bank B {}", this->pBankName_0x50);
	CUTSCENE_LOG(LogLevel::Info, "Cinematic::Create File {}", this->fileName);

	this->nbTotalCinematicActors = pByteCode->GetS32();

	CUTSCENE_LOG(LogLevel::Info, "Cinematic::Create Actor Cinematic Count {}", this->nbTotalCinematicActors);

	this->nbTotalLevelActors = pByteCode->GetS32();
	this->count_0x20 = pByteCode->GetS32();
	this->field_0x24 = pByteCode->GetS32();
	this->field_0x28 = pByteCode->GetS32();
	this->field_0x2c = pByteCode->GetS32();

	this->field_0x4c = pByteCode->GetU32();
	this->field_0x54 = pByteCode->GetU32();

	this->baseB = pByteCode->GetS32();

	*(uint*)&this->field_0x10 = pByteCode->GetU32();
	this->flags_0x4 = pByteCode->GetU32();

	this->field_0x30 = pByteCode->GetF32();

	pfVar1 = (S_STREAM_FOG_DEF*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)(pfVar1 + 1);
	this->streamFogDef = *pfVar1;

	this->zoneRefA.index = pByteCode->GetS32();
	this->triggerActorRef.index = pByteCode->GetS32();
	this->zoneRefB.index = pByteCode->GetS32();
	this->zoneRefC.index = pByteCode->GetS32();
	this->actorRefB.index = pByteCode->GetS32();

	this->field_0x58 = pByteCode->GetF32();

	this->field_0x5c = (SWITCH_MODE)pByteCode->GetS32();
	this->field_0x60 = pByteCode->GetF32();
	this->field_0x64 = (SWITCH_MODE)pByteCode->GetS32();
	this->field_0x68 = pByteCode->GetF32();

	this->cineActorConfigCount = pByteCode->GetS32();
	uVar5 = this->cineActorConfigCount;
	if (uVar5 != 0) {
		this->aCineActorConfig = new CCineActorConfig[uVar5];
		iVar7 = 0;
		if (0 < this->cineActorConfigCount) {
			do {
				pCineActorConfig = &this->aCineActorConfig[iVar7];
				pCineActorConfig->pActor.index = pByteCode->GetS32();
				pCineActorConfig->flags = pByteCode->GetU32();
				pCineActorConfig->field_0x8 = pByteCode->GetF32();
				pCineActorConfig->field_0xc = pByteCode->GetF32();

				iVar7 = iVar7 + 1;

				pCineActorConfig->field_0xc = pCineActorConfig->field_0xc * 0.01745329f;
				pCineActorConfig->postCinematicLocation = gF32Vertex4Zero;
				pCineActorConfig->postCinematicRotationEuler = gF32Vertex4Zero;
			} while (iVar7 < this->cineActorConfigCount);
		}
	}

	this->cond_0x248.Create(pByteCode);
	uVar11 = this->cond_0x248.IsVerified();
	if (uVar11 == 0) {
		this->flags_0x8 = this->flags_0x8 | 8;
	}

	if (2.16f <= CScene::_pinstance->field_0x1c) {
		this->defaultAudioTrackId = pByteCode->GetU32();
	}
	else {
		this->defaultAudioTrackId = -1;
	}

	uVar5 = pByteCode->GetU32();
	memset(this->aAudioTrackIds, 0xff, 0x14);

	for (; uVar5 != 0; uVar5 = uVar5 - 1) {
		const uint index = pByteCode->GetU32();
		this->aAudioTrackIds[index] = pByteCode->GetU32();
	}

	if (this->defaultAudioTrackId == -1) {
		this->defaultAudioTrackId = this->aAudioTrackIds[0];
	}

	this->endLevelId = pByteCode->GetS32();
	this->endElevatorId = pByteCode->GetS32();
	this->endCutsceneId = pByteCode->GetS32();
	this->field_0x78 = pByteCode->GetU32();
	this->field_0x7c = pByteCode->GetU32();
	this->field_0x80 = pByteCode->GetF32();

	piVar2 = (int*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)(piVar2 + 1);
	if (*piVar2 != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + *piVar2 * 4;
	}
	this->field_0x25c = piVar2;
	this->condArray_0x244.Create(pByteCode);

	S_NTF_TARGET_STREAM_REF::Create(&this->pSwitchListA, pByteCode);

	pcVar6 = pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = pcVar6 + sizeof(S_STREAM_EVENT_CAMERA);
	this->pStreamEventCameraA = (S_STREAM_EVENT_CAMERA*)pcVar6;

	S_NTF_TARGET_STREAM_REF::Create(&this->pSwitchListB, pByteCode);

	pcVar6 = pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = pcVar6 + sizeof(S_STREAM_EVENT_CAMERA);
	this->pStreamEventCameraB = (S_STREAM_EVENT_CAMERA*)pcVar6;

	if (this->field_0x4c == 0) {
		this->flags_0x8 = this->flags_0x8 | 8;
	}

	SetupInternalData();

	return;
}

template<>
void S_STREAM_REF<CActor>::Init()
{
	CActor* pActor;

	if (this->index == -1) {
		pActor = (CActor*)0x0;
	}
	else {
		pActor = ((CScene::ptable.g_ActorManager_004516a4)->aActors)[this->index];
	}

	this->pObj = STORE_SECTION(pActor);
	return;
}

template<>
void S_STREAM_REF<CSound>::Init()
{
	IMPLEMENTATION_GUARD_AUDIO();
}

template<>
void S_STREAM_REF<ed_zone_3d>::Init()
{
	ed_zone_3d* pZone;

	pZone = (ed_zone_3d*)0x0;

	if (this->index != -1) {
		pZone = edEventGetChunkZone((CScene::ptable.g_EventManager_006f5080)->activeChunkId, this->index);
	}
	this->pObj = STORE_SECTION(pZone);
	return;
}

void S_STREAM_NTF_TARGET_SWITCH::Reset()
{
	this->flags = this->flags & 1;
}

void S_STREAM_NTF_TARGET_BASE::Init()
{
	this->flags = this->flags & 1;

	CActor* pActor;

	if (this->index == -1) {
		pActor = (CActor*)0x0;
	}
	else {
		pActor = ((CScene::ptable.g_ActorManager_004516a4)->aActors)[this->index];
	}

	this->pRef = STORE_SECTION(pActor);
}

void CCinematic::Init()
{
	S_NTF_TARGET_STREAM_REF* pSwitchList;
	undefined4* puVar3;
	uint uVar4;
	int iVar5;
	int iVar6;

	zoneRefA.Init();
	triggerActorRef.Init();
	zoneRefB.Init();
	zoneRefC.Init();
	actorRefB.Init();

	iVar6 = 0;
	if (0 < this->cineActorConfigCount) {
		do {
			this->aCineActorConfig[iVar6].pActor.Init();
			iVar6 = iVar6 + 1;
		} while (iVar6 < this->cineActorConfigCount);
	}

	for (iVar6 = *this->field_0x25c; iVar6 != 0; iVar6 = iVar6 + -1) {
	}

	pSwitchList = this->pSwitchListA;

	pSwitchList->Init();
	this->pStreamEventCameraA->Init();

	pSwitchList = this->pSwitchListB;
	pSwitchList->Init();
	this->pStreamEventCameraB->Init();

	if (this->field_0x25c == (int*)0x0) {
		iVar6 = 0;
	}
	else {
		iVar6 = *this->field_0x25c;
	}
	if (iVar6 != 0) {
		this->flags_0x4 = this->flags_0x4 | 2;
	}
	uVar4 = 0x27f4;
	if ((this->flags_0x4 & 0x10000000) != 0) {
		uVar4 = 0x27d4;
	}
	this->flags_0x8 = this->flags_0x8 & ~uVar4;
	this->count_0x2d8 = 0;
	this->cineBankLoadStage_0x2b4 = 0;
	this->state = CS_Stopped;
	this->time_0x88 = GetTimer()->scaledTotalTime;
	if ((this->flags_0x8 & 0x10) != 0) {
		IMPLEMENTATION_GUARD(this->cinFileData.Shutdown());
		this->flags_0x8 = this->flags_0x8 & 0xffffffef;
	}
	this->count_0x224 = 0;
	this->count_0x22c = 0;

	iVar6 = 0;
	if (0 < this->nbInstalledParticles) {
		iVar5 = 0;
		do {
			ParticleInstance* pParticleInstance = &this->buffer_0x2e4[iVar6];

			pParticleInstance->id = -1;
			pParticleInstance->field_0x8 = 0.0f;
			pParticleInstance->field_0x4 = 0.0f;
			pParticleInstance->field_0xc = 0;
			pParticleInstance->pParticle = (_ed_particle_manager*)0x0;
			pParticleInstance->pParticleFileData = (ParticleFileData*)0x0;
			pParticleInstance->pActor = (CActor*)0x0;

			iVar6 = iVar6 + 1;
		} while (iVar6 < this->nbInstalledParticles);
	}

	if ((this->flags_0x4 & 0x2000000) != 0) {
		this->count_0x2d8 = this->count_0x2d8 + 1;
		this->Load(1);
	}

	return;
}

void CCinematic::Start()
{
	float fVar2;
	uint uVar3;
	uint uVar4;
	CActorHero* pAVar5;
	CCinematicManager* pCinematicManager;
	bool bShouldPlay;
	edNODE* pMVar9;
	int* piVar10;
	edF32MATRIX4* peVar11;
	edF32MATRIX4* peVar13;
	int** ppiVar14;
	int iVar15;
	int iVar16;
	int* piVar17;
	int* piVar18;
	int* piVar19;
	FrameInfo FStack224;
	edF32MATRIX4 eStack176;
	edF32MATRIX4 local_70;
	edBANK_ENTRY_INFO eStack48;
	CCinematic* local_18;
	undefined4 local_14;
	CCinematic** local_10;
	undefined4 local_c;
	int cinematicCompilerVersion;
	int cinematicLibraryVersion;

	CUTSCENE_LOG(LogLevel::Info, "Cinematic::Start");

	this->flags_0x8 = this->flags_0x8 & 0xffffff7f;

	if (this->state == CS_Stopped) {
		bShouldPlay = (this->flags_0x4 & 1) != 0;

		if (bShouldPlay) {
			bShouldPlay = (this->flags_0x8 & 0x400) != 0;
		}

		if (!bShouldPlay) {
			bShouldPlay = (this->flags_0x8 & 0x28) != 0;
		}

		bShouldPlay = (bool)(bShouldPlay ^ 1);
		if (!bShouldPlay) {
			bShouldPlay = (this->flags_0x8 & 0x800) != 0;
		}

		if (bShouldPlay) {
			if (this->cineBankLoadStage_0x2b4 != 4) {
				Load(0);
			}

			pCinematicManager = g_CinematicManager_0048efc;
			if (this->cineBankLoadStage_0x2b4 == 4) {
				if (this->zoneRefA.pObj != 0) {
					this->flags_0x8 = this->flags_0x8 | 0x200;
				}

				pCinematicManager->pCurCinematic = this;

				/* Takes in a bank header and file name, and returns index of file in bank
				   Example: Start_ff_Montage_SceneMontage.cin returns 59. */
				iVar16 = this->pCineBankEntry->get_index(this->fileName);
				if ((((iVar16 != -1) &&
					(bShouldPlay = this->pCineBankEntry->get_info(iVar16, &eStack48, (char*)0x0), bShouldPlay != false)) &&
					(bShouldPlay = ((edCinematic*)eStack48.fileBufferStart)->ExtractVersions(eStack48.size, &cinematicLibraryVersion, &cinematicCompilerVersion), bShouldPlay != false)) &&
					((cinematicLibraryVersion != 0x81 || (cinematicCompilerVersion != 0xa0)))) {
					edDebugPrintf("Cinematic library version: %d (should be %d)\n", cinematicLibraryVersion, 0x81);
					edDebugPrintf("Cinematic compiler version: %d (should be %d)\n", cinematicCompilerVersion, 0xa0);
				}

				if (this->defaultAudioTrackId != -1) {
					int audioTrackId = this->aAudioTrackIds[CMessageFile::get_default_language()];
					if (audioTrackId == -1) {
						audioTrackId = this->defaultAudioTrackId;
					}

					this->cinematicLoadObject.BWCinSourceAudio_Obj.SetAudioTrack(audioTrackId);
				}

				if ((this->prtBuffer == 1) || ((this->flags_0x4 & 8) != 0)) {
					edMemSetFlags(TO_HEAP(H_MAIN), 0x100);
				}

				this->nbActorRefs = 0;
				this->nbCinematicActors = 0;
				this->count_0x224 = 0;
				this->count_0x22c = 0;
				this->numberOfParticles = 0;

				/* This will take a cinematic file and load all of its assets
				   Example: AIRLIFT_TO_NATIV_Cin_Montage_SceneMontage.cin */
				this->cinFileData.Create(this->cinematicLoadObject, this->fileName);

				local_70 = gF32Matrix4Unit;

				if ((this->flags_0x4 & 0x2000) != 0) {
					CActor* pActorRef = (this->actorRefB).Get();
					if (pActorRef != (CActor*)0x0) {
						edF32Matrix4FromEulerSoft(&local_70, &pActorRef->pCinData->rotationEuler, "XYZ");
						local_70.rowT = pActorRef->baseLocation;
						edF32Matrix4GetInverseOrthoHard(&local_70, &local_70);
					}

					if (this->pActor != (CActor*)0x0) {
						edF32Matrix4FromEulerSoft(&eStack176, &this->pActor->rotationEuler.xyz, "XYZ");
						pActorRef = this->pActor;
						eStack176.rowT = pActorRef->currentLocation;
						edF32Matrix4MulF32Matrix4Hard(&local_70, &local_70, &eStack176);
					}
				}

				this->matrix_0x120 = local_70;
				
				if (this->prtBuffer == 1) {
					CScene::_pinstance->PushFogAndClippingSettings(0.0f, &this->streamFogDef);
				}

				if (this->pMeshInfo != (ed_g3d_manager*)0x0) {
					ed3DScenePushCluster(CScene::_scene_handleA, this->pMeshInfo);

					ed_g3d_hierarchy* pNewHier = ed3DG3DHierarchyGetFromHashcode(this->pMeshInfo, 0x43494d414e5944);
					if (pNewHier != 0) {
						ed3DG3DHierarchySetStripShadowReceiveFlag(pNewHier, 0xffff);
					}

					this->pMeshTransform = ed3DHierarchyAddToSceneByHashcode(CScene::_scene_handleA, this->pMeshInfo, 0x43494d414e5944);
					if (this->pMeshTransform != (edNODE*)0x0) {
						ed3DSetMeshTransformFlag_002abd80(this->pMeshTransform, 0xffff);
						ed3DSetMeshTransformFlag_002abff0(this->pMeshTransform, 0xffff);
					}
				}

				pAVar5 = CActorHero::_gThis;
				if (CActorHero::_gThis != (CActorHero*)0x0) {
					if ((this->flags_0x4 & 0x10) != 0) {
						local_c = 0;
						CActorHero::_gThis->DoMessage(CActorHero::_gThis, (ACTOR_MESSAGE)0x82, (MSG_PARAM)0x0);
					}
					if ((this->flags_0x4 & 0x10000) != 0) {
						pAVar5->flags = pAVar5->flags | 0x400000;
					}
				}

				if ((this->flags_0x4 & 0x400) != 0) {
					CScene::ptable.g_FrontendManager_00451680->SetActive(false);
					GameFlags = GameFlags | GAME_CUTSCENE_100;
				}

				if ((this->flags_0x4 & 0x20000200) != 0) {
					GameFlags = GameFlags | GAME_CUTSCENE_80;
				}

				uVar3 = this->flags_0x4;
				if ((uVar3 & 0x280000) != 0) {
					IMPLEMENTATION_GUARD_AUDIO(
					FUN_00182db0(0x3f800000, (int)CScene::ptable.g_AudioManager_00451698, uVar3 & 0x80000, uVar3 & 0x200000);)
				}

				if ((this->flags_0x4 & 0x100) == 0) {
					this->totalCutsceneDelta = 0.0;
				}

				if ((this->flags_0x4 & 0x1000000) != 0) {
					this->flags_0x8 = this->flags_0x8 | 0x40;
				}

				bShouldPlay = false;

				if ((this->flags_0x8 & 0x800) == 0) {
					int configCount = this->cineActorConfigCount;
					int currentConfigIndex = 0;
					if (0 < configCount) {
						do {
							CCineActorConfig* pCurrentConfig = &this->aCineActorConfig[currentConfigIndex];
							CActor* pConfigActor = (pCurrentConfig->pActor).Get();
							if (pConfigActor != (CActor*)0x0) {
								if ((pCurrentConfig->flags & 0x10) != 0) {
									pCurrentConfig->postCinematicLocation = pConfigActor->currentLocation;
									pCurrentConfig->postCinematicRotationEuler = pConfigActor->rotationEuler;
									if ((pConfigActor->flags & 0x1000) == 0) {
										pCurrentConfig->flags = (uint)((ulong)((ulong)(int)pCurrentConfig->flags << 0x21) >> 0x21);
									}
									else {
										pCurrentConfig->flags = pCurrentConfig->flags | 0x80000000;
									}
								}

								if ((pCurrentConfig->flags & 1) != 0) {
									pConfigActor->CinematicMode_Enter(true);
									bShouldPlay = true;
								}
							}

							currentConfigIndex = currentConfigIndex + 1;
						} while (currentConfigIndex < configCount);
					}

					if ((this->flags_0x4 & 0x1000) != 0) {
						bShouldPlay = true;
					}
				}

				if (bShouldPlay) {
					this->state = CS_Interpolate;
					this->time_0x88 = GetTimer()->scaledTotalTime;
				}
				else {
					this->state = CS_Playing;
					this->time_0x88 = GetTimer()->scaledTotalTime;
					this->cinFileData.Initialize();
					this->flags_0x8 = this->flags_0x8 | 0x10;
				}

				if ((this->flags_0x8 & 2) != 0) {
					this->cinematicLoadObject.BWCinCam_Obj.Activate();
					this->cinFileData.Timeslice(0.0f, &FStack224);
					CCameraManager::_gThis->PushCamera(pCinematicManager->pCinematicCamera, 1);
				}

				if ((this->prtBuffer == 1) || ((this->flags_0x4 & 8) != 0)) {
					edMemClearFlags(TO_HEAP(H_MAIN), 0x100);
				}

				pCinematicManager->pCurCinematic = (CCinematic*)0x0;

				if ((this->flags_0x4 & 2) == 0) {
					this->flags_0x8 = this->flags_0x8 | 0x1400;
				}

				if (this->pActor != (CActor*)0x0) {
					local_14 = 2;
					local_10 = &local_18;
					local_18 = this;
					this->pActor->ReceiveMessage((CActor*)0x0, (ACTOR_MESSAGE)0x7c, local_10);
				}
			}
			else {
				/* Cutscene failed to enter load state */
				g_CinematicManager_0048efc->field_0x28 = this;

				pCinematicManager->startTime = GetTimer()->totalTime;

				if ((this->flags_0x8 & 0x800) == 0) {
					S_NTF_TARGET_STREAM_REF* pSwitchList = this->pSwitchListA;

					bool bHasEventData = false;
					if ((pSwitchList != (S_NTF_TARGET_STREAM_REF*)0x0) && (this->pStreamEventCameraA != (S_STREAM_EVENT_CAMERA*)0x0)) {
						bHasEventData = true;
					}

					if (bHasEventData) {
						pSwitchList->Switch((CActor*)0x0);
						this->pStreamEventCameraA->SwitchOn((CActor*)0x0);
						pSwitchList->PostSwitch((CActor*)0x0);
					}
				}

				if ((this->flags_0x8 & 0x800) == 0) {
					this->condArray_0x244.Perform();

					S_NTF_TARGET_STREAM_REF* pSwitchList = this->pSwitchListB;

					bool bHasEventData = false;
					if ((pSwitchList != (S_NTF_TARGET_STREAM_REF*)0x0) && (this->pStreamEventCameraB != (S_STREAM_EVENT_CAMERA*)0x0)) {
						bHasEventData = true;
					}

					if (bHasEventData) {
						pSwitchList->Switch((CActor*)0x0);
						this->pStreamEventCameraB->SwitchOn((CActor*)0x0);
						pSwitchList->PostSwitch((CActor*)0x0);
					}

					if ((this == g_CinematicManager_0048efc->pCinematic) && (g_CinematicManager_0048efc->field_0x20 != -1)) {
						CLevelScheduler::gThis->FUN_002dc200(g_CinematicManager_0048efc->field_0x20, CLevelScheduler::gThis->currentLevelID, g_CinematicManager_0048efc->field_0x24);
					}
					else {
						CLevelScheduler::gThis->Level_Teleport((CActor*)0x0, this->endLevelId, this->endElevatorId, this->endCutsceneId, 0xffffffff);
					}
				}

				this->flags_0x8 = this->flags_0x8 & 0xfffff7ff;
			}
		}
	}

	return;
}

edCBankCallback _gCinTableBankCallback[1] = {-1, -1, 0x0, 0, 0, 0, 0, 0};

void CCinematic::Load(int mode)
{
	int iVar1;
	bool bVar2;
	uint uVar3;
	uint size;
	edCBankInstall eStack32;

	CUTSCENE_LOG(LogLevel::Info, "Cinematic::Load Current stage: {} mode: {}", this->cineBankLoadStage_0x2b4, mode);

	bool bBankLoaded = this->pCineBankEntry != (edCBankBufferEntry*)0x0;
	if (bBankLoaded) {
		bBankLoaded = this->cineBankLoadStage_0x2b4 != CINEMATIC_LOAD_STATE_COMPLETE;
	}

	CUTSCENE_LOG(LogLevel::Info, "Cinematic::Load Bank loaded: {}", bBankLoaded);

	if (bBankLoaded) {
		if (mode == 0) {
			while (this->cineBankLoadStage_0x2b4 != CINEMATIC_LOAD_STATE_COMPLETE) {
				CUTSCENE_LOG(LogLevel::Info, "Cinematic::Load Bank not loaded, loading... (state: {})", this->cineBankLoadStage_0x2b4);

				if (this->cineBankLoadStage_0x2b4 != CINEMATIC_LOAD_STATE_COMPLETE) {
					if (this->cineBankLoadStage_0x2b4 == 3) {
						if (this->pCineBankEntry->is_loaded() != false) {
							CUTSCENE_LOG(LogLevel::Info, "Cinematic::Load Complete!");
							this->cineBankLoadStage_0x2b4 = CINEMATIC_LOAD_STATE_COMPLETE;
						}
					}
					else {
						if (((this->cineBankLoadStage_0x2b4 == 2) && (bVar2 = edSoundAreAllSoundDataLoaded(), bVar2 != false)) &&
							(bVar2 = edMusicAreAllMusicDataLoaded(), bVar2 != false)) {
							this->pCineBankEntry->close();
							this->pCineBankEntry = (edCBankBufferEntry*)0x0;
							LoadCineBnk(1);
						}
					}
				}

				edFileNoWaitStackFlush();
			}
		}
	}
	else {
		size = this->field_0x4c;
		if (this->field_0x4c <= this->field_0x54) {
			size = this->field_0x54;
		}

		bVar2 = (this->flags_0x4 & 1) != 0;
		if (bVar2) {
			bVar2 = (this->flags_0x8 & 0x400) != 0;
		}

		if (!bVar2) {
			bVar2 = (this->flags_0x8 & 0x28) != 0;
		}

		bVar2 = (bool)(bVar2 ^ 1);
		if (!bVar2) {
			bVar2 = (this->flags_0x8 & 0x800) != 0;
		}

		if (((bVar2) && (bVar2 = StaticEdFileBase_004497f0.Check(), bVar2 != false)) &&
			(uVar3 = edMemGetMemoryAvailable(TO_HEAP(H_MAIN)), size + 0x2800 < uVar3)) {
			if ((this->prtBuffer == 1) || ((this->flags_0x4 & 8) != 0)) {
				edMemSetFlags(TO_HEAP(H_MAIN), 0x100);
			}

			memset(&eStack32, 0, sizeof(edCBankInstall));
			this->cineBank.initialize(size, 1, &eStack32);
			this->cineBank.bank_buffer_setcb(_gCinTableBankCallback);

			CUTSCENE_LOG(LogLevel::Info, "Cinematic::Load Loading bank...");

			if ((this->prtBuffer == 1) || ((this->flags_0x4 & 8) != 0)) {
				edMemClearFlags(TO_HEAP(H_MAIN), 0x100);
			}

			if (this->field_0x54 == 0) {
				this->LoadCineBnk(mode);
			}
			else {
				if ((LoadInternal(mode) != false) && (mode == 0)) {
					this->pCineBankEntry->close();
					this->pCineBankEntry = (edCBankBufferEntry*)0x0;
					this->LoadCineBnk(0);

					CUTSCENE_LOG(LogLevel::Info, "Cinematic::Load Closing bank...");
				}
			}
		}
	}

	return;
}

char* g_languageSuffixPtr[5] = {
	"US", "FR", "GER", "SP", "IT"
};


void _gCinSoundCallback(bool bSuccess, void* pObj)
{
	if (bSuccess != false) {
		((CCinematic*)pObj)->InstallSounds();
	}
	return;
}


bool CCinematic::LoadInternal(long mode)
{
	char cVar1;
	CLevelScheduler* pLVar2;
	bool bVar3;
	int iVar4;
	edFILEH* pDebugBank;
	edCBankBufferEntry* peVar5;
	char* pcVar6;
	edCBankInstall local_420;
	char acStack1024[512];
	char acStack512[512];

	pLVar2 = CLevelScheduler::gThis;
	edStrCopy(acStack1024, this->pBankName_0x50);
	iVar4 = edStrLength(acStack1024);
	pcVar6 = acStack1024 + iVar4;
	cVar1 = *pcVar6;
	while (cVar1 != '.') {
		pcVar6 = pcVar6 + -1;
		cVar1 = *pcVar6;
	}
	pcVar6[1] = '\0';
	iVar4 = CMessageFile::get_default_language();
	edStrCat(acStack1024, g_languageSuffixPtr[iVar4]);
	/* \\Cine\\ */
	edStrCatMulti(acStack512, pLVar2->levelPath, pLVar2->aLevelInfo[pLVar2->currentLevelID].levelName, "\\Cine\\", acStack1024, NULL);
	bVar3 = true;
	if (((gVideoConfig.omode != 2) && ((this->flags_0x4 & 0x8000000) != 0)) &&
		(pDebugBank = edFileOpen(acStack512, 9), pDebugBank != (edFILEH*)0x0)) {
		edFileClose(pDebugBank);
		bVar3 = false;
	}
	if (bVar3) {
		edStrCatMulti(acStack512, pLVar2->levelPath, pLVar2->aLevelInfo[pLVar2->currentLevelID].levelName, "\\Cine\\", this->pBankName_0x50, NULL);
	}
	memset(&local_420, 0, sizeof(edCBankInstall));
	local_420.filePath = acStack512;
	local_420.fileFunc = _gCinSoundCallback;
	local_420.pObjectReference = this;
	peVar5 = this->cineBank.get_free_entry();
	this->pCineBankEntry = peVar5;
	local_420.fileFlagA = 4;
	this->cineBankLoadStage_0x2b4 = 1;

	if (this->pCineBankEntry->load(&local_420) == false) {
		this->pCineBankEntry = (edCBankBufferEntry*)0x0;
		this->cineBank.terminate();
		this->cineBankLoadStage_0x2b4 = 0;
	}
	else {
		if (mode == 0) {
			this->pCineBankEntry->wait();
		}
	}

	return this->cineBankLoadStage_0x2b4 != 0;
}

void _gCinCallback(bool bSuccess, void* pObj)
{
	if (bSuccess != false) {
		((CCinematic*)pObj)->Install();
	}
	return;
}

void CCinematic::Install()
{
	this->cinFileCount = this->pCineBankEntry->get_element_count();

	if ((this->prtBuffer == 1) || ((this->flags_0x4 & 8) != 0)) {
		edMemSetFlags(TO_HEAP(H_MAIN), 0x100);
	}

	if (this->cinFileCount != 0) {
		this->fileInfoStart = new CinFileContainer[this->cinFileCount];
		int uVar4 = 0;
		if (this->cinFileCount != 0) {
			do {
				this->fileInfoStart[uVar4].pData = (char*)0x0;
				uVar4 = uVar4 + 1;
			} while (uVar4 < this->cinFileCount);
		}
	}

	if ((this->prtBuffer == 1) || ((this->flags_0x4 & 8) != 0)) {
		edMemClearFlags(TO_HEAP(H_MAIN), 0x100);
	}

	this->matrix_0x120 = gF32Matrix4Unit;

	if (this->defaultAudioTrackId != -1) {
		this->flags_0x8 = this->flags_0x8 | 1;
	}

	this->cineBankLoadStage_0x2b4 = 4;

	if (this->pActor != (CActor*)0x0) {
		CCinematic** local_4;// = &local_10;
		undefined4 local_c = 0;
		CCinematic* local_10 = this;
		this->pActor->ReceiveMessage(0, (ACTOR_MESSAGE)0x7c, local_4);
	}

	return;
}

bool CCinematic::LoadCineBnk(bool mode)
{
	edCBankInstall bankInstall;
	char cinematicFilePath[512];

	/* Loads a bank file from the cine folder
	   Example: CDEURO/LEVEL/PREINTRO/CINE/1c9de79b.bnk */
	edStrCatMulti(cinematicFilePath, CLevelScheduler::gThis->levelPath, CLevelScheduler::gThis->aLevelInfo[CLevelScheduler::gThis->currentLevelID].levelName,
		"\\Cine\\", this->pBankName_0x48, NULL);

	memset(&bankInstall, 0, sizeof(edCBankInstall));

	bankInstall.filePath = cinematicFilePath;
	bankInstall.fileFunc = _gCinCallback;
	bankInstall.pObjectReference = this;

	this->pCineBankEntry = this->cineBank.get_free_entry();

	bankInstall.fileFlagA = 4;
	this->cineBankLoadStage_0x2b4 = 3;

	if (this->pCineBankEntry->load(&bankInstall) == false) {
		this->pCineBankEntry = (edCBankBufferEntry*)0x0;
		this->cineBank.terminate();
		this->cineBankLoadStage_0x2b4 = 0;
	}
	else {
		if (mode == false) {
			this->pCineBankEntry->wait();
		}
	}

	return this->cineBankLoadStage_0x2b4 == 3;
}

struct CLipTrackManager;

int* CCinematic::InstallResource(edResCollection::RES_TYPE objectType, bool type2, const char* fileName, ed_g2d_manager* textureObj, int* bufferLengthOut)
{
	CLipTrackManager* pMVar1;
	bool bVar2;
	int result;
	ed_g2d_manager* outTextureInfo;
	ed_g3d_manager* outMeshInfo;
	edANM_HDR* outAnimationInfo;
	ulong uVar3;
	int iVar4;
	const char* pcVar5;
	const char* pcVar6;
	CinFileContainer* piVar7;
	uint counter;
	edBANK_ENTRY_INFO outFileData;
	char localisedAudioFileName[524];
	int iStack4;
	const char* suffix;
	edCBankBufferEntry* bankObj;

	outMeshInfo = (ed_g3d_manager*)0x0;
	/* Does not go in here for the air tunnel cutscene */
	if (objectType == (edResCollection::RES_TYPE)10) {
#if 0
		if (type2 == 0) {
			counter = 0;
			if (this->soundCount_0x2b8 != 0) {
				iVar4 = 0;
				do {
					result = edStrICmp(*(byte**)(this->playingSounds_0x2c0 + iVar4), (byte*)fileName);
					if (result == 0) {
						return (int*)(this->sound_0x2bc + counter * 0x18);
					}
					counter = counter + 1;
					iVar4 = iVar4 + 4;
				} while (counter < (uint)this->soundCount_0x2b8);
			}
		}
		else {
			outMeshInfo = (ed_g3d_manager*)AudioManager::GetSampleByName((int)Scene::ptable.g_GlobalSoundPtr_00451698, (byte*)fileName);
		}
#endif
	}
	else {
		/* Drops in here for the air tunnel cutscene */
		if (objectType == edResCollection::COT_Sound) {
			if (type2 == 0) {
				/* Background sound drops in here
				   This second check is probably for if the sound file contains voice audio */
				if ((gVideoConfig.omode != 2) && ((this->flags_0x4 & 0x8000000) != 0)) {
					iVar4 = edStrLength(fileName);
					pcVar5 = fileName + iVar4;
					do {
						suffix = pcVar5;
						pcVar5 = suffix + -1;
					} while (*pcVar5 != '\\');
					do {
						pcVar6 = pcVar5;
						pcVar5 = pcVar6 + -1;
					} while (*pcVar5 != '\\');
					edStrCopy(localisedAudioFileName, fileName);
					localisedAudioFileName[((char*)pcVar6) -((char*)fileName)] = '\0';
					iVar4 = CMessageFile::get_default_language();
					edStrCat(localisedAudioFileName, g_languageSuffixPtr[iVar4]);
					edStrCat(localisedAudioFileName, "\\");
					edStrCat(localisedAudioFileName, suffix);
					fileName = localisedAudioFileName;
				}
				counter = 0;
				/* Sound file will go through to here regardless of language */
				if (this->soundCount_0x2b8 != 0) {
					/* Background sound will drop in here */
					iVar4 = 0;
					do {
						/* This does a compare agains file name
						   Example:
						   d:\projects\b-witch\resource\build\level\cinematiques\loading\rsc_cin_airlift_to_native\wav\master_mono.vag
							*/
						result = edStrICmp(this->playingSounds_0x2c0[counter], fileName);
						if (result == 0) {
							/* We already have the sound file loaded, return it here */
							return (int*)(&this->sound_0x2bc[counter]);
						}
						counter = counter + 1;
						iVar4 = iVar4 + 4;
					} while (counter < (uint)this->soundCount_0x2b8);
				}
			}
			else {
				IMPLEMENTATION_GUARD(
				outMeshInfo = (ed_g3d_manager*)AudioManager::GetSampleByName((int)CScene::ptable.g_AudioManager_00451698, fileName);)
			}
		}
		else {
			/* First cutscene object load falls into here
			   Get the appropriate bank file */
			if (type2 == 0) {
				bankObj = this->pCineBankEntry;
			}
			else {
				bankObj = (CLevelScheduler::gThis)->pLevelBankBufferEntry;
			}
			/* Get the file index from the bank file */
			iVar4 = bankObj->get_index(fileName);
			if ((iVar4 == -1) || (bVar2 = bankObj->get_info(iVar4, &outFileData, (char*)0x0), bVar2 == false)) {
				/* We could not find the bank file, or we could not process the header. */
				*bufferLengthOut = 0;
				outMeshInfo = (ed_g3d_manager*)0x0;
			}
			else {
				/* Set a file length? */
				*bufferLengthOut = outFileData.size;
				CUTSCENE_LOG(LogLevel::Info, "File Size: 0x{:x} ({})", outFileData.size, outFileData.size);
				if (type2 == 0) {
					/* File info start has a section for each file in the cutscene.
					   We go to the correct section based on the file index we got out of the bank. */
					piVar7 = &this->fileInfoStart[iVar4];
					/* Check if the file has been loaded already */
					if (piVar7->pData == 0) {
						/* We are okay to start loading the file.
						   Structure for the file buffer is:

						   First 4 bytes: Type
						   Second 4 bytes: Out object pointer, or num objects.

						   First store what type of file it is. */
						piVar7->type = objectType;
						piVar7->pData = (char*)outFileData.fileBufferStart;
						pMVar1 = CScene::ptable.g_LipTrackManager_00451694;
						if (objectType == edResCollection::COT_Particle) {
							iVar4 = -1;
							if (this->particleSectionStart[this->numberOfParticles].InstallFromDSC(outFileData.fileBufferStart, outFileData.size, bankObj) != false) {
								iVar4 = this->numberOfParticles;
								this->numberOfParticles = iVar4 + 1;
							}
							
							/* For particles we store the number of particle objects. */
							piVar7->pData = (char*)(iVar4 + 1);
						}
						else {
							if (objectType == edResCollection::COT_Text) {
								this->textData.select_language(bankObj, fileName, AUTO);
							}
							else {
								if (objectType == edResCollection::COT_LipTrack) {
									IMPLEMENTATION_GUARD_LIP(
									iVar4 = FUN_001ad730((int)CScene::ptable.g_LipTrackManager_00451694, (int)outFileData.fileBufferStart);
									piVar7->pData = *(int*)(&pMVar1->field_0x8 + iVar4 * 4);)
								}
								else {
									if (objectType == edResCollection::COT_Animation) {
										outAnimationInfo = edAnmAnim::LoadFromMem(outFileData.fileBufferStart, outFileData.size);
										piVar7->pData = (char*)outAnimationInfo;
									}
									else {
										if (objectType == edResCollection::COT_MeshModel) {
											NAME_NEXT_OBJECT(fileName);
											outMeshInfo = ed3DInstallG3D(outFileData.fileBufferStart, outFileData.size, 0, &iStack4,
												textureObj, 0xc, (ed_g3d_manager*)0x0);
											piVar7->pData = (char*)outMeshInfo;
										}
										else {
											if (objectType == edResCollection::COT_MeshTexture) {
												NAME_NEXT_OBJECT(fileName);
												outTextureInfo =
													ed3DInstallG2D(outFileData.fileBufferStart, outFileData.size, &iStack4, (ed_g2d_manager*)0x0,
														1);
												piVar7->pData = (char*)outTextureInfo;
											}
										}
									}
								}
							}
						}
					}
					outMeshInfo = (ed_g3d_manager*)piVar7->pData;
				}
				else {
					if (objectType == edResCollection::COT_Animation) {
						outMeshInfo = (ed_g3d_manager*)edAnmAnim::LoadFromMem(outFileData.fileBufferStart, outFileData.size);
					}
					else {
						if (objectType == edResCollection::COT_MeshModel) {
							outMeshInfo = CScene::ptable.g_C3DFileManager_00451664->GetG3DManager(outFileData.fileBufferStart);
						}
						else {
							outMeshInfo = (ed_g3d_manager*)outFileData.fileBufferStart;
							if (objectType == edResCollection::COT_MeshTexture) {
								outMeshInfo = (ed_g3d_manager*)CScene::ptable.g_C3DFileManager_00451664->GetG2DManager(outFileData.fileBufferStart);
							}
						}
					}
				}
			}
		}
	}
	return (int*)outMeshInfo;
}

void S_STREAM_NTF_TARGET_ONOFF::Reset()
{
	this->flags = this->flags & 1;
	return;
}

bool S_STREAM_NTF_TARGET_ONOFF::SwitchOn(CActor* pActor)
{
	uint uVar1;
	bool bVar2;
	bool uVar3;

	uVar1 = this->flags;
	uVar3 = false;
	bVar2 = true;

	if (((uVar1 & 1) != 0) && ((uVar1 & 0x40000000) != 0)) {
		bVar2 = false;
	}

	if (bVar2) {
		this->flags = this->flags | 0x40000000;

		if (this->messageId != 0) {
			g_CinematicManager_0048efc->NotifyCinematic(this->cutsceneId, pActor, this->messageId, this->messageFlags);

			if (pActor == (CActor*)0x0) {
				pActor = (CActor*)LOAD_SECTION(this->pRef);
			}

			if (pActor != (CActor*)0x0) {
				uVar3 = pActor->DoMessage(pActor, (ACTOR_MESSAGE)this->messageId, (MSG_PARAM)this->messageFlags);
			}
		}
	}

	return uVar3;
}

CActorCinematic* CCinematic::NewCinematicActor(const edCinGameInterface::ACTORV_CREATIONtag* pTag, ed_g3d_manager* pG3D, ed_g2d_manager* pG2D)
{
	int curIndex;
	CActorCinematic* pNewCinematicActor;

	CUTSCENE_LOG(LogLevel::Info, "CCinematic::NewCinematicActor");

	pNewCinematicActor = (CActorCinematic*)0x0;

	curIndex = 0;
	if (0 < this->nbCinematicActors) {
		do {
			if (edStrICmp(this->aActorCinematic[curIndex].cinematicName, pTag->name) == 0) {
				pNewCinematicActor = this->aActorCinematic + curIndex;
			}
			curIndex = curIndex + 1;
		} while (curIndex < this->nbCinematicActors);
	}

	if (pNewCinematicActor == (CActorCinematic*)0x0) {
		assert(this->nbCinematicActors >= 0 && this->nbCinematicActors < this->nbTotalCinematicActors);

		FLUSH_LOG();

		pNewCinematicActor = &this->aActorCinematic[this->nbCinematicActors];
		pNewCinematicActor->Create(pTag, pG3D, pG2D, CScene::_scene_handleA);
		pNewCinematicActor->PreInit();
		pNewCinematicActor->Init();

		this->nbCinematicActors = this->nbCinematicActors + 1;
		this->ppActorCinematics[this->nbActorRefs] = pNewCinematicActor;
		this->nbActorRefs = this->nbActorRefs + 1;
	}

	return pNewCinematicActor;
}

CCineActorConfig* CCinematic::GetActorConfig(CActor* pActor)
{
	CCineActorConfig* pCVar1;
	int iVar2;

	if (pActor->typeID != 1) {
		iVar2 = 0;
		if (0 < this->cineActorConfigCount) {
			pCVar1 = this->aCineActorConfig;
			do {
				if (pActor == pCVar1->pActor.Get()) {
					return pCVar1;
				}
				iVar2 = iVar2 + 1;
				pCVar1 = pCVar1 + 1;
			} while (iVar2 < this->cineActorConfigCount);
		}
	}
	return (CCineActorConfig*)0x0;
}

char* g_ParticleReturnChars = " \r\n\t";

bool CCinematic::ParticleEntry::InstallFromDSC(char* pFileBuffer, int fileLength, edCBankBufferEntry* pBankBufferEntry)
{
	uint uVar1;
	int numChildFiles;
	int iVar2;
	ed_g2d_manager* pManager;
	ed_hash_code* peVar3;
	size_t sVar4;
	char* fileNameSeekPos;
	TextureInfo* particleTextureInfo;
	char* componentFileNamePtr;
	int index;
	char* fileBufferEnd;
	edBANK_ENTRY_INFO outChildFileData;
	char particleComponentFileName[76];
	int iStack4;
	char bCurrCharIsPeriod;

	fileBufferEnd = pFileBuffer + fileLength;
	this->nbManagers = 0;
	componentFileNamePtr = particleComponentFileName;
	this->nbMaterials = 0;
	this->pFileBuffer = (char*)0x0;
	fileNameSeekPos = strchr(g_ParticleReturnChars, *pFileBuffer);
	while ((fileNameSeekPos != (char*)0x0 && (pFileBuffer < fileBufferEnd))) {
		pFileBuffer = pFileBuffer + 1;
		fileNameSeekPos = strchr(g_ParticleReturnChars, *pFileBuffer);
	}

	for (; (fileNameSeekPos = strchr(g_ParticleReturnChars, *pFileBuffer), fileNameSeekPos == (char*)0x0 &&
		(pFileBuffer < fileBufferEnd)); pFileBuffer = pFileBuffer + 1) {
		*componentFileNamePtr = *pFileBuffer;
		componentFileNamePtr = componentFileNamePtr + 1;
	}

	*componentFileNamePtr = '\0';
	numChildFiles = atoi(particleComponentFileName);

	while (true) {
		if (numChildFiles == 0) {
			return true;
		}

		numChildFiles = numChildFiles + -1;
		componentFileNamePtr = particleComponentFileName;
		/* Search for the start of the file name */
		fileNameSeekPos = strchr(g_ParticleReturnChars, *pFileBuffer);

		while ((fileNameSeekPos != (char*)0x0 && (pFileBuffer < fileBufferEnd))) {
			pFileBuffer = pFileBuffer + 1;
			fileNameSeekPos = strchr(g_ParticleReturnChars, *pFileBuffer);
		}

		for (; (fileNameSeekPos = strchr(g_ParticleReturnChars, *pFileBuffer), fileNameSeekPos == (char*)0x0 &&
			(pFileBuffer < fileBufferEnd)); pFileBuffer = pFileBuffer + 1) {
			/* Search for the end of the file name, copying the file name into the buffer as we go */
			*componentFileNamePtr = *pFileBuffer;
			componentFileNamePtr = componentFileNamePtr + 1;
		}

		/* This loads other files, but if loading a particle will load in this order:
		   d:\projects\b-witch\resource\build\level_1\cinematiques\particle\a983538304.dsc
		   _grid_ground.g2d
		   A983538304.prt */
		*componentFileNamePtr = '\0';
		iVar2 = pBankBufferEntry->get_index(particleComponentFileName);

		NAME_NEXT_OBJECT(particleComponentFileName);

		if (iVar2 == -1) break;

		pBankBufferEntry->get_info(iVar2, &outChildFileData, (char*)0x0);
		/* Try find the file type:
		   Find the length of the file name, since we will search through it backwards */

		sVar4 = strlen(particleComponentFileName);
		fileNameSeekPos = particleComponentFileName + (int)sVar4;
		/* Travel back through the file looking for a '.' */
		bCurrCharIsPeriod = *fileNameSeekPos;

		while ((bCurrCharIsPeriod != '.' && (particleComponentFileName < fileNameSeekPos))) {
			fileNameSeekPos = fileNameSeekPos + -1;
			bCurrCharIsPeriod = *fileNameSeekPos;
		}

		/* Check if the file is a '.g2d' file */
		iVar2 = strcmp(fileNameSeekPos, ".g2d");
		if (iVar2 == 0) {
			pManager = ed3DInstallG2D(outChildFileData.fileBufferStart, outChildFileData.size, &iStack4, (ed_g2d_manager*)0x0, 1);
			if (pManager == (ed_g2d_manager*)0x0) {
				return false;
			}

			iVar2 = ed3DG2DGetG2DNbMaterials(pManager);
			index = 0;
			if (0 < iVar2) {
				do {
					this->aHashes[this->nbMaterials] = ed3DG2DGetMaterialFromIndex(pManager, index)->hash.number;
					edDListCreatMaterialFromIndex(this->aMaterials + this->nbMaterials, index, pManager, 2);
					index = index + 1;
					this->aMaterialPtrs[this->nbMaterials] = this->aMaterials + this->nbMaterials;
					this->nbMaterials = this->nbMaterials + 1;
				} while (index < iVar2);
			}

			uVar1 = this->nbManagers;
			this->nbManagers = uVar1 + 1;
			this->aManagers[uVar1] = pManager;
		}
		else {
			/* Reset the buffer and search through it again, looking for a period */
			sVar4 = strlen(particleComponentFileName);
			fileNameSeekPos = particleComponentFileName + (int)sVar4;
			bCurrCharIsPeriod = *fileNameSeekPos;

			while ((bCurrCharIsPeriod != '.' && (particleComponentFileName < fileNameSeekPos))) {
				fileNameSeekPos = fileNameSeekPos + -1;
				bCurrCharIsPeriod = *fileNameSeekPos;
			}

			/* Check if the file is a '.prt' file */
			iVar2 = strcmp(fileNameSeekPos, ".prt");
			if (iVar2 == 0) {
				this->pFileBuffer = outChildFileData.fileBufferStart;
				this->fileSize = outChildFileData.size;
			}
			else {
				/* Reset the buffer and search through it again, looking for a period */
				sVar4 = strlen(particleComponentFileName);
				fileNameSeekPos = particleComponentFileName + (int)sVar4;
				bCurrCharIsPeriod = *fileNameSeekPos;

				while ((bCurrCharIsPeriod != '.' && (particleComponentFileName < fileNameSeekPos))) {
					fileNameSeekPos = fileNameSeekPos + -1;
					bCurrCharIsPeriod = *fileNameSeekPos;
				}

				/* Check if the file is a '.g3d' file */
				strcmp(fileNameSeekPos, ".g3d");
			}
		}
	}

	return false;
}

bool S_STREAM_NTF_TARGET_SWITCH::PostSwitch(CActor* pActor)
{
	uint uVar1;
	bool bVar2;
	int iVar3;
	undefined uVar4;

	uVar1 = this->flags;
	uVar4 = 0;
	bVar2 = true;
	if (((uVar1 & 1) != 0) && ((uVar1 & 0x80000000) != 0)) {
		bVar2 = false;
	}

	if (bVar2) {
		this->flags = this->flags | 0x80000000;

		if (this->field_0x14 != 0) {
			g_CinematicManager_0048efc->NotifyCinematic(this->cutsceneId, pActor, this->field_0x14, this->flags);

			if (pActor == (CActor*)0x0) {
				pActor = LOAD_SECTION_CAST(CActor*, this->pRef);
			}

			if (pActor != (CActor*)0x0) {
				iVar3 = pActor->DoMessage(LOAD_SECTION_CAST(CActor*, this->pRef), (ACTOR_MESSAGE)this->field_0x14, (MSG_PARAM)this->flags);
				uVar4 = (undefined)iVar3;
			}
		}
	}

	return (bool)uVar4;
}

void CCinematic::Manage()
{
	uint soundInfo;
	bool bVar1;
	int iVar2;
	CActor* pTriggerActor;
	float fVar4;

	// Manage the zone based cutscene trigger
	ed_zone_3d* pZone = this->zoneRefA.Get();

	if (pZone != (ed_zone_3d*)0x0) {
		pTriggerActor = this->triggerActorRef.Get();
		if (this->triggerActorRef.Get() == (CActor*)0x0) {
			pTriggerActor = CActorHero::_gThis;
		}
	
		if ((pTriggerActor == (CActorHero*)0x0) || (edEventComputeZoneAgainstVertex((CScene::ptable.g_EventManager_006f5080)->activeChunkId, pZone, &pTriggerActor->currentLocation, 0) == 2)) {
			this->flags_0x8 = this->flags_0x8 & 0xfffffdff;
			bVar1 = this->state != CS_Stopped;

			if ((bVar1) && (((this->flags_0x4 & 8) != 0 && (bVar1)))) {
				this->flags_0x8 = this->flags_0x8 & 0xffffff7f;
				this->flags_0x8 = this->flags_0x8 | 0x100;
			}
		}
		else {
			if ((((this->flags_0x8 & 0x200) == 0) && (this->state == CS_Stopped)) && ((this->flags_0x8 & CINEMATIC_FLAG_ACTIVE) == 0)) {
				TryTriggerCutscene((CActor*)0x0, 0);
			}
		}
	}

	bVar1 = this->pSwitchListA != (S_NTF_TARGET_STREAM_REF*)0x0;
	if (bVar1) {
		bVar1 = this->pStreamEventCameraA != (S_STREAM_EVENT_CAMERA*)0x0;
	}

	if (bVar1) {
		this->pStreamEventCameraA->Manage((CActor*)0x0);
	}

	bVar1 = this->pSwitchListB != (S_NTF_TARGET_STREAM_REF*)0x0;
	if (bVar1) {
		bVar1 = this->pStreamEventCameraB != (S_STREAM_EVENT_CAMERA*)0x0;
	}

	if (bVar1) {
		this->pStreamEventCameraA->Manage((CActor*)0x0);
	}

	if ((this->state != CS_Stopped) && ((GameFlags & GAME_STATE_PAUSED) == 0)) {
		IncrementCutsceneDelta();

		CUTSCENE_LOG(LogLevel::Verbose, "CCinematic::Manage Updating cutscene time: {}", this->totalCutsceneDelta);
	}
	if (this->state == CS_Stopped) {
		iVar2 = this->cineBankLoadStage_0x2b4;
		if (iVar2 != 4) {
			if (iVar2 == 3) {
				if (this->pCineBankEntry->is_loaded() != false) {
					this->cineBankLoadStage_0x2b4 = 4;
				}
			}
			else {
				if (((iVar2 == 2) && (bVar1 = edSoundAreAllSoundDataLoaded(), bVar1 != false)) && (bVar1 = edMusicAreAllMusicDataLoaded(), bVar1 != false)) {
					this->pCineBankEntry->close();
					this->pCineBankEntry = (edCBankBufferEntry*)0x0;

					LoadCineBnk(true);
				}
			}
		}
	}
	else {
		if (this->state == CS_Playing) {
			ManageState_Playing();
		}
		else {
			if (this->state == CS_Interpolate) {
				IMPLEMENTATION_GUARD(
				ManageState_InterpolateBegin(this);)
			}
		}

		if ((this != (CCinematic*)0xffffff4c) && (this->field_0x2c8 != -1.0f)) {
			IMPLEMENTATION_GUARD_AUDIO(
			fVar4 = edFIntervalLERP((this->pActor->actorBase).data.adjustedMagnitude, this->field_0x2c8, this->field_0x2cc,
				this->field_0x2d4, this->field_0x2d0);
			soundInfo = (this->cinematicLoadObject).BWCinSourceAudio_Obj.intFieldA;
			if (soundInfo != 0) {
				edSoundInstanceSetVolume(fVar4, soundInfo);
			})
		}
	}

	this->flags_0x8 = this->flags_0x8 & 0xfffffffb;
	this->flags_0x8 = this->flags_0x8 & 0xffffdfff;
	return;
}

void CCinematic::ManageState_Playing()
{
	int* piVar1;
	bool bVar2;
	int iVar4;
	int iVar5;
	long lVar6;
	int iVar7;
	CBWCinSourceAudio* cinematicSoundObject;
	float fVar8;
	float fVar9;
	CCinematicManager* someGlobal;

	someGlobal = g_CinematicManager_0048efc;
	g_CinematicManager_0048efc->pCurCinematic = this;
	bVar2 = TimeSlice(this->totalCutsceneDelta);
	/* Check if the cutscene is over */
	someGlobal->pCurCinematic = (CCinematic*)0x0;
	if (bVar2 == false) {
		/* Cutscene has ended */
		if ((this->flags_0x4 & 8) == 0) {
			if (this->state != CS_Stopped) {
				this->flags_0x8 = this->flags_0x8 & 0xffffff7f;
				this->flags_0x8 = this->flags_0x8 | 0x100;
			}
		}
		else {
			bVar2 = (this->cinematicLoadObject).BWCinSourceAudio_Obj.intFieldA != 0;

			cinematicSoundObject = &(this->cinematicLoadObject).BWCinSourceAudio_Obj;
			if ((bVar2) && (bVar2 = true, (this->cinematicLoadObject).BWCinSourceAudio_Obj.field_0x8 == 0.0)) {
				bVar2 = false;
			}

			if (bVar2) {
				someGlobal->pCurCinematic = this;
				cinematicSoundObject->Stop();

				lVar6 = this->aAudioTrackIds[CMessageFile::get_default_language()];
				if (lVar6 == -1) {
					lVar6 = this->defaultAudioTrackId;
				}

				this->cinematicLoadObject.BWCinSourceAudio_Obj.SetAudioTrack(lVar6);

				cinematicSoundObject->Play();
				someGlobal->pCurCinematic = (CCinematic*)0x0;
				this->totalCutsceneDelta = this->field_0x30;
			}
			else {
				this->totalCutsceneDelta = this->field_0x30 + (this->totalCutsceneDelta - this->cinFileData.pCinTag->totalPlayTime);
				if ((this->flags_0x8 & 1) != 0) {
					bVar2 = (this->cinematicLoadObject).BWCinSourceAudio_Obj.intFieldA != 0;
					fVar9 = this->totalCutsceneDelta;
					if ((bVar2) && (bVar2 = true, (this->cinematicLoadObject).BWCinSourceAudio_Obj.field_0x8 == 0.0f)) {
						bVar2 = false;
					}
					if (bVar2) {
						IMPLEMENTATION_GUARD(
						fVar8 = CutsceneTypeTwoA((this->cinematicLoadObject).BWCinSourceAudio_Obj.intFieldA);
						(this->cinematicLoadObject).BWCinSourceAudio_Obj.field_0x8 = fVar8 - fVar9;
						CutsceneTypeTwoB(fVar9, (this->cinematicLoadObject).BWCinSourceAudio_Obj.intFieldA);)
					}
				}
			}
		}
	}
	else {
		/* Cutscene is still playing */
		if (((((this->flags_0x4 & 2) != 0) || ((this->flags_0x8 & 0x800) != 0)) &&
			(((this->flags_0x8 & 0x1400) != 0 || ((this->flags_0x4 & 4) == 0)))) &&
			(((GameFlags & 0x1c) == 0 && ((gPlayerInput.pressedBitfield & 0x1000000) != 0)))) {
			IMPLEMENTATION_GUARD(
			/* The tunnel cutscene doesn't ever go in here */
			piVar1 = this->field_0x25c;
			iVar4 = 0;
			if (piVar1 != (int*)0x0) {
				iVar4 = *piVar1;
			}
			iVar7 = 0;
			for (iVar5 = 0; (iVar7 < iVar4 && (*(float*)((int)piVar1 + iVar5 + 4) <= this->totalCutsceneDelta));
				iVar5 = iVar5 + 4) {
				iVar7 = iVar7 + 1;
			}
			if ((iVar4 <= iVar7) || (*(float*)&((this->cinFileData).pCinTag)->field_0x4 <= (float)piVar1[iVar7 + 1])) {
				if (this->state != CS_Stopped) {
					this->flags_0x8 = this->flags_0x8 & 0xffffff7f;
					this->flags_0x8 = this->flags_0x8 | 0x100;
				}
			}
			else {
				this->totalCutsceneDelta = (float)piVar1[iVar7 + 1];
				if ((this->flags_0x8 & 1) != 0) {
					fVar9 = this->totalCutsceneDelta;
					bVar2 = (this->cinematicLoadObject).BWCinSourceAudio_Obj.intFieldA != 0;
					if ((bVar2) && (bVar2 = true, (this->cinematicLoadObject).BWCinSourceAudio_Obj.field_0x8 == 0.0)) {
						bVar2 = false;
					}
					if (bVar2) {
						fVar8 = CutsceneTypeTwoA((this->cinematicLoadObject).BWCinSourceAudio_Obj.intFieldA);
						(this->cinematicLoadObject).BWCinSourceAudio_Obj.field_0x8 = fVar8 - fVar9;
						CutsceneTypeTwoB(fVar9, (this->cinematicLoadObject).BWCinSourceAudio_Obj.intFieldA);
					}
				}
			})
		}
	}
	return;
}

void CCinematic::Stop()
{
	bool bVar1;
	CActorHero* pActor;
	CCinematicManager* pCVar2;
	Timer* pTVar3;
	int iVar4;
	int* piVar5;
	int iVar6;
	float currentPlayTime;
	CCinematic* local_10;
	undefined4 local_c;
	undefined4 local_8;
	CCinematic** local_4;

	this->flags_0x8 = this->flags_0x8 | 0x1400;
	this->flags_0x8 = this->flags_0x8 & 0xfffffe7f;

	if (this->state != CS_Stopped) {
		if (this->pActor != (CActor*)0x0) {
			local_c = 3;
			local_4 = &local_10;
			local_10 = this;
			this->pActor->ReceiveMessage((CActor*)0x0, (ACTOR_MESSAGE)0x7c, local_4);
		}

		pCVar2 = g_CinematicManager_0048efc;
		g_CinematicManager_0048efc->pCurCinematic = this;
		this->flags_0x8 = this->flags_0x8 | 4;
		currentPlayTime = ((this->cinFileData).pCinTag)->totalPlayTime;

		if ((this->totalCutsceneDelta < currentPlayTime) && ((this->flags_0x4 & 0x80) != 0)) {
			this->TimeSlice(currentPlayTime);
		}

		if ((this->flags_0x8 & 2) != 0) {
			CCameraManager::_gThis->PopCamera(pCVar2->pCinematicCamera);
		}

		this->state = CS_Stopped;
		pTVar3 = GetTimer();
		this->time_0x88 = pTVar3->scaledTotalTime;
		if ((this->flags_0x8 & 0x10) != 0) {
			this->cinFileData.Shutdown();
			this->flags_0x8 = this->flags_0x8 & 0xffffffef;
		}

		if (this->pMeshInfo != (ed_g3d_manager*)0x0) {
			if (this->pMeshTransform != (edNODE*)0x0) {
				ed3DHierarchyRemoveFromScene(CScene::_scene_handleA, this->pMeshTransform);
				this->pMeshTransform = (edNODE*)0x0;
			}
			ed3DScenePopCluster(CScene::_scene_handleA, this->pMeshInfo);
		}

		if ((this->flags_0x4 & 0x280000) != 0) {
			IMPLEMENTATION_GUARD_AUDIO(
			FUN_00182da0(0x3f800000, (int)CScene::ptable.g_AudioManager_00451698);)
		}

		if ((this->flags_0x4 & 0x20000200) != 0) {
			GameFlags = GameFlags & ~GAME_CUTSCENE_80;
		}

		if ((this->flags_0x4 & 0x400) != 0) {
			GameFlags = GameFlags & ~GAME_CUTSCENE_100;
			CScene::ptable.g_FrontendManager_00451680->SetActive(true);
		}

		pActor = CActorHero::_gThis;
		if (CActorHero::_gThis != (CActorHero*)0x0) {
			if ((this->flags_0x4 & 0x10000) != 0) {
				IMPLEMENTATION_GUARD(
				CActorHero::_gThis->data.objectId =
					CActorHero::_gThis->data.objectId & 0xffbfffff;)
			}

			if ((this->flags_0x4 & 0x10) != 0) {
				local_8 = 0;
				pActor->DoMessage(pActor, (ACTOR_MESSAGE)0x83, (MSG_PARAM)0x0);
			}
		}

		if (this->prtBuffer == 1) {
			CScene::_pinstance->PopFogAndClippingSettings(&this->streamFogDef);
		}

		this->cinFileData.Destroy(this->cinematicLoadObject);

		if ((this->aActorCinematic != (CActorCinematic*)0x0) && (iVar4 = this->nbCinematicActors + -1, -1 < iVar4)) {
			do {
				this->aActorCinematic[iVar4].Term();
				iVar4 = iVar4 + -1;
			} while (-1 < iVar4);
		}

		iVar4 = 0;
		if (0 < this->nbInstalledParticles) {
			do {
				ParticleInstance* pParticleInstance = this->buffer_0x2e4 + iVar4;
				if (pParticleInstance->id != -1) {
					if (pParticleInstance->pParticle != (_ed_particle_manager*)0x0) {
						edParticlesUnInstall(pParticleInstance->pParticle, CScene::_scene_handleA);
					}

					if (pParticleInstance->pParticleFileData != (ParticleFileData*)0x0) {
						edMemFree(pParticleInstance->pParticleFileData);
					}

					pParticleInstance->id = -1;
					pParticleInstance->field_0x8 = 0.0f;
					pParticleInstance->field_0x4 = 0.0f;
					pParticleInstance->field_0xc = 0;
					pParticleInstance->pParticle = (_ed_particle_manager*)0x0;
					pParticleInstance->pParticleFileData = (ParticleFileData*)0x0;
					pParticleInstance->pActor = (CActor*)0x0;
				}

				iVar4 = iVar4 + 1;
			} while (iVar4 < this->nbInstalledParticles);
		}

		UninstallResources();

		pCVar2->pCurCinematic = (CCinematic*)0x0;
		if ((this->flags_0x8 & 0x800) == 0) {
			this->condArray_0x244.Perform();
			S_NTF_TARGET_STREAM_REF* pSwitchList = this->pSwitchListB;
			bVar1 = false;
			if ((pSwitchList != (S_NTF_TARGET_STREAM_REF*)0x0) && (this->pStreamEventCameraB != (S_STREAM_EVENT_CAMERA*)0x0)) {
				bVar1 = true;
			}

			if (bVar1) {
				pSwitchList->Switch((CActor*)0x0);
				this->pStreamEventCameraB->SwitchOn((CActor*)0x0);
				pSwitchList = this->pSwitchListB;
				pSwitchList->PostSwitch((CActor*)0x0);
			}

			if ((this == g_CinematicManager_0048efc->pCinematic) && (g_CinematicManager_0048efc->field_0x20 != -1)) {
				CLevelScheduler::gThis->FUN_002dc200(g_CinematicManager_0048efc->field_0x20, CLevelScheduler::gThis->currentLevelID, g_CinematicManager_0048efc->field_0x24);
			}
			else {
				CLevelScheduler::gThis->Level_Teleport((CActor*)0x0, this->endLevelId, this->endElevatorId, this->endCutsceneId, -1);
			}
		}

		this->flags_0x8 = this->flags_0x8 & 0xfffff7ff;
	}

	return;
}

bool CCinematic::TimeSlice(float currentPlayTime)
{
	CActorCinematic* pCinematicActor;
	int iVar1;
	int iVar2;
	edF32MATRIX4* peVar3;
	CCameraManager* pCVar4;
	CCinematicManager* pCVar5;
	CActorManager* pActorManager;
	//CLightManager* pCVar6;
	CBehaviourCinematic* cutsceneSubSubInfoObj;
	//BWCinSunLight* pBVar7;
	//BWCinSpotLight* pBVar8;
	undefined8 uVar9;
	int iVar10;
	int i;
	int iVar11;
	int* piVar12;
	int iVar13;
	int iVar14;
	undefined4 uVar15;
	float fVar16;
	undefined4 uVar17;
	float fVar18;
	float fVar19;
	undefined4 uVar20;
	FrameInfo local_30;
	CActorCinematic* cutsceneSubInfoObj;

	/* Iterate over the sub objects (named cutscene objects?) */
	for (i = 0; i < this->nbActorRefs; i = i + 1) {
		cutsceneSubInfoObj = this->ppActorCinematics[i];
		cutsceneSubInfoObj->flags = cutsceneSubInfoObj->flags | 0x40;
		cutsceneSubSubInfoObj = (CBehaviourCinematic*)cutsceneSubInfoObj->GetBehaviour(1);

		(cutsceneSubSubInfoObj->cinActor).nextPos = cutsceneSubInfoObj->currentLocation;
	}

	//pBVar7 = this->pCineSunHolderArray;
	//for (i = 0; i < this->count_0x224; i = i + 1) {
	//	*(undefined*)&pBVar7->field_0xb0 = 0;
	//	pBVar7 = pBVar7 + 1;
	//}
	//pBVar8 = this->pCineSpotHolderArray;
	//for (i = 0; i < this->count_0x22c; i = i + 1) {
	//	*(undefined*)&pBVar8->field_0xc0 = 0;
	//	pBVar8 = pBVar8 + 1;
	//}

	for (i = 0; i < this->nbInstalledParticles; i = i + 1) {
		this->buffer_0x2e4[i].field_0xc = 0;
	}

	this->cinFileData.Timeslice(currentPlayTime, &local_30);

	pCVar4 = CCameraManager::_gThis;
	if (((this->flags_0x8 & 2) != 0) && ((CCameraManager::_gThis->flags & 0x4000000) == 0)) {
		CCameraManager::_gThis->ApplyActiveCamera();
		pCVar4->BuildDisplayMatrix();
	}

	pCVar4 = CCameraManager::_gThis;
	peVar3 = &CCameraManager::_gThis->transformationMatrix;
	for (i = 0; pActorManager = CScene::ptable.g_ActorManager_004516a4, i < this->nbActorRefs; i = i + 1) {
		pCinematicActor = this->ppActorCinematics[i];
		fVar16 = pCinematicActor->sphereCentre.x - peVar3->rowT.x;
		fVar18 = pCinematicActor->sphereCentre.y - peVar3->rowT.y;
		fVar19 = pCinematicActor->sphereCentre.z - peVar3->rowT.z;
		pCinematicActor->distanceToCamera = sqrtf(fVar16 * fVar16 + fVar18 * fVar18 + fVar19 * fVar19) - pCinematicActor->sphereCentre.w;
		if ((pCinematicActor->flags & 4) == 0) {
			pCinematicActor->ChangeManageState(1);
		}
		pCinematicActor->EvaluateDisplayState();
		if ((((this->flags_0x4 & 0x800000) != 0) || ((pCinematicActor->flags & 0x4000) == 0)) ||
			(pCinematicActor->pHier == (ed_g3d_hierarchy*)0x0)) {
			pCinematicActor->flags = pCinematicActor->flags | 0x200000;
		}
		pCinematicActor->Manage();
	}

	if (CScene::ptable.g_ActorManager_004516a4->HasAnyLinkedActors()) {
		CScene::ptable.g_ActorManager_004516a4->UpdateLinkedActors();
	}

	//pCVar6 = CScene::ptable.g_LightManager_004516b0;
	//pBVar7 = this->pCineSunHolderArray;
	//for (i = 0; i < this->count_0x224; i = i + 1) {
	//	if (*(char*)&pBVar7->field_0xb0 == '\0') {
	//		if ((*(byte*)((int)&(pBVar7->lightSun).light.colour_0x4 + 3) & 2) != 0) {
	//			(*(code*)((pBVar7->lightSun).light.pVTable)->field_0x1c)();
	//		}
	//	}
	//	else {
	//		if ((pBVar7->lightSun).light.field_0xa == -1) {
	//			LightManager::Reference(pCVar6, (Light*)&pBVar7->lightSun, 0xff, 0, 0, -1);
	//		}
	//		if ((*(byte*)((int)&(pBVar7->lightSun).light.colour_0x4 + 3) & 2) == 0) {
	//			(*(code*)((pBVar7->lightSun).light.pVTable)->field_0x18)();
	//		}
	//	}
	//	pBVar7 = pBVar7 + 1;
	//}
	//pBVar8 = this->pCineSpotHolderArray;
	//for (i = 0; iVar13 = 0, i < this->count_0x22c; i = i + 1) {
	//	if (*(char*)&pBVar8->field_0xc0 == '\0') {
	//		if ((*(byte*)((int)&(pBVar8->lightSpot).light.colour_0x4 + 3) & 2) != 0) {
	//			(*(code*)((pBVar8->lightSpot).light.pVTable)->field_0x1c)();
	//		}
	//	}
	//	else {
	//		if ((pBVar8->lightSpot).light.field_0xa == -1) {
	//			LightManager::Reference(pCVar6, (Light*)&pBVar8->lightSpot, 0xff, 0, 0, -1);
	//		}
	//		if ((*(byte*)((int)&(pBVar8->lightSpot).light.colour_0x4 + 3) & 2) == 0) {
	//			(*(code*)((pBVar8->lightSpot).light.pVTable)->field_0x18)();
	//		}
	//	}
	//	pBVar8 = pBVar8 + 1;
	//}

	for (iVar10 = 0; pCVar5 = g_CinematicManager_0048efc, iVar10 < this->nbInstalledParticles; iVar10 = iVar10 + 1) {
		ParticleInstance* pParticleInstance = this->buffer_0x2e4 + iVar10;
		if (pParticleInstance->id != -1) {
			if (pParticleInstance->field_0xc == 0) {
				if (pParticleInstance->pParticle != (_ed_particle_manager*)0x0) {
					edParticlesUnInstall(pParticleInstance->pParticle, CScene::_scene_handleA);
				}

				if (pParticleInstance->pParticleFileData != (ParticleFileData*)0x0) {
					edMemFree(pParticleInstance->pParticleFileData);
				}

				pParticleInstance->id = -1;
				pParticleInstance->field_0x8 = 0.0f;
				pParticleInstance->field_0x4 = 0.0f;
				pParticleInstance->field_0xc = 0;
				pParticleInstance->pParticle = (_ed_particle_manager*)0x0;
				pParticleInstance->pParticleFileData = (ParticleFileData*)0x0;
				pParticleInstance->pActor = (CActor*)0x0;
			}
			else {
				edParticlesSetSystem(pParticleInstance->pParticle);

				for (int iVar9 = 0; iVar9 < pParticleInstance->pParticle->nbTotalGroups; iVar9 = iVar9 + 1) {
					_ed_particle_group* p_Var11 = pParticleInstance->pParticle->aGroups.pData + iVar9;
					for (int iVar8 = 0; iVar8 < p_Var11->field_0x20; iVar8 = iVar8 + 1) {
						CActor* pCVar1 = pParticleInstance->pActor;
						_ed_particle_generator_param* pParam = p_Var11->field_0x24.pData[iVar8].pData;

						pParam->position = pCVar1->currentLocation;
						pParam->rotationEuler = pCVar1->rotationEuler;
						pParam->scale = pCVar1->scale;
						edPartGeneratorComputeMatrices(pParam);
					}
				}

				float fVar12 = pParticleInstance->field_0x8 - pParticleInstance->field_0x4;
				if (fVar12 < 0.0f) {
					IMPLEMENTATION_GUARD(
					FUN_0027cd80(0);)
					fVar12 = pParticleInstance->field_0x8;
				}

				float fVar13;

				if (fVar12 < 0.0f) {
					fVar13 = 0.0f;
				}
				else {
					fVar13 = 1.0f;
					if (fVar12 <= 1.0f) {
						fVar13 = fVar12;
					}
				}

				for (; 0.1 < fVar13; fVar13 = fVar13 - 0.1f) {
					edParticlesUpdate(0.1f);
				}

				edParticlesUpdate(fVar13);
			}
		}
	}

	if ((local_30.field_0x0[5] == 0.0) || (local_30.field_0x0[6] < local_30.field_0x4)) {
		if (local_30.field_0x0[10] != 0.0f) {
			if ((*local_30.field_0x0 - local_30.field_0x0[0xb] <= local_30.field_0x4) &&
				(fVar16 = (local_30.field_0x4 - *local_30.field_0x0) / local_30.field_0x0[0xb] + 1.0f, fVar16 != 0.0f)) {
				g_CinematicManager_0048efc->fadeDuration = fVar16;
				fVar16 = local_30.field_0x0[0xd];
				fVar18 = local_30.field_0x0[0xe];
				pCVar5->fadeColor.x = local_30.field_0x0[0xc];
				pCVar5->fadeColor.y = fVar16;
				pCVar5->fadeColor.z = fVar18;
			}
		}
	}
	else {
		fVar16 = 1.0f - local_30.field_0x4 / local_30.field_0x0[6];
		if (fVar16 != 0.0f) {
			g_CinematicManager_0048efc->fadeDuration = fVar16;
			fVar16 = local_30.field_0x0[8];
			fVar18 = local_30.field_0x0[9];
			pCVar5->fadeColor.x = local_30.field_0x0[7];
			pCVar5->fadeColor.y = fVar16;
			pCVar5->fadeColor.z = fVar18;
		}
	}

	return currentPlayTime < ((this->cinFileData).pCinTag)->totalPlayTime;
}

void CCinematic::IncrementCutsceneDelta() 
{
	Timer* timeController;
	int iVar1;
	//BWCinSourceAudio* pBVar2;
	float fVar3;
	float deltaTime;

	/* This is true for cutscenes */
	if ((this->flags_0x8 & 0x40) == 0) {
		timeController = Timer::GetTimer();
		/* Returns 0.2 for cutscenes */
		deltaTime = timeController->cutsceneDeltaTime;
	}
	else {
		deltaTime = 0.0f;
	}

	/* This is true for cutscenes */
	if ((this->flags_0x8 & 1) == 0) {
		/* Increment float by 0.2 in cutscene manager  */
		deltaTime = this->totalCutsceneDelta + deltaTime;
		this->totalCutsceneDelta = deltaTime;
		if (deltaTime < 0.0f) {
			this->totalCutsceneDelta = 0.0f;
		}
	}
	else {
		iVar1 = this->aAudioTrackIds[CMessageFile::get_default_language()];
		if (iVar1 == -1) {
			iVar1 = this->defaultAudioTrackId;
		}

		fVar3 = (this->cinematicLoadObject).BWCinSourceAudio_Obj.Func_0x1c(iVar1);
		if (fVar3 == -1.0f) {
			this->totalCutsceneDelta = this->totalCutsceneDelta + deltaTime;
		}
		else {
			this->totalCutsceneDelta = fVar3;
		}
	}

	if (this->totalCutsceneDelta < 0.0f) {

		this->totalCutsceneDelta = 0.0f;
	}
	return;
}

void BWBankManagerGetFileNames(edCBankBufferEntry* pBankEntry, ushort type, ushort stype, int* pCount, char*** ppPtrArray, char** pszOutIopPath)
{
	bool bVar1;
	uint uVar2;
	int iVar3;
	char** pvVar4;
	char* szIopPath;
	int iVar5;
	int iVar6;
	uint uVar7;
	uint local_228;
	edBANK_ENTRY_INFO local_220;
	char acStack512[512];

	local_228 = (((uint)type) << 0x10) | (uint)stype;
	uVar2 = pBankEntry->get_element_count();
	iVar5 = 0;
	iVar6 = 0;
	uVar7 = 0;
	if (uVar2 != 0) {
		do {
			bVar1 = pBankEntry->get_info(uVar7, &local_220, acStack512);
			if (bVar1 == false) break;
			if ((local_220.type << 0x10 | local_220.stype) == local_228) {
				iVar3 = edStrLength(acStack512);
				iVar6 = iVar6 + 1;
				iVar5 = iVar5 + iVar3 + 1;
			}
			uVar7 = uVar7 + 1;
		} while (uVar7 < uVar2);
	}
	pvVar4 = (char**)edMemAlloc(TO_HEAP(H_MAIN), iVar6 << 2);
	*ppPtrArray = pvVar4;
	szIopPath = (char*)edMemAlloc(TO_HEAP(H_MAIN), iVar5);
	*pszOutIopPath = szIopPath;
	szIopPath = *pszOutIopPath;
	iVar5 = 0;
	uVar7 = 0;
	if (uVar2 != 0) {
		iVar6 = 0;
		do {
			bVar1 = pBankEntry->get_info(uVar7, &local_220, szIopPath);
			if (bVar1 == false) break;
			if ((local_220.type << 0x10 | local_220.stype) == local_228) {
				iVar5 = iVar5 + 1;
				(*ppPtrArray)[uVar7] = szIopPath;
				iVar6 = iVar6 + 4;
				iVar3 = edStrLength(szIopPath);
				szIopPath = szIopPath + iVar3 + 1;
			}
			uVar7 = uVar7 + 1;
		} while (uVar7 < uVar2);
	}
	*pCount = iVar5;
	return;
}

void CCinematic::InstallSounds()
{
	bool uVar1;
	bool bSuccess;
	ed_sound_sample* aSamples;
	uint elementCount;
	uint size;
	uint inFileIndex;
	int offset;
	edBANK_ENTRY_INFO bankEntry;

	if ((this->prtBuffer == 1) || ((this->flags_0x4 & 8) != 0)) {
		edMemSetFlags(TO_HEAP(H_MAIN), 0x100);
	}
	BWBankManagerGetFileNames(this->pCineBankEntry, 3, 1, &this->soundCount_0x2b8, &this->playingSounds_0x2c0, &this->field_0x2c4);
	this->cineBankLoadStage_0x2b4 = 2;
	if (this->soundCount_0x2b8 != 0) {
		aSamples = (ed_sound_sample*)edMemAlloc(TO_HEAP(H_MAIN), this->soundCount_0x2b8 * sizeof(ed_sound_sample));
		this->sound_0x2bc = aSamples;
		elementCount = this->pCineBankEntry->get_element_count();
		size = 0;
		inFileIndex = 0;
		if (elementCount != 0) {
			do {
				bSuccess = this->pCineBankEntry->get_info(inFileIndex, &bankEntry, (char*)0x0);
				if (bSuccess != false) {
					if (bankEntry.size == 0) {
#ifdef PLATFORM_PS2
						trap(7);
#else
						assert(false);
#endif
					}
					if (0x40 % (int)bankEntry.size == 0) {
						offset = 0x40;
					}
					else {
						if (bankEntry.size == 0) {
#ifdef PLATFORM_PS2
							trap(7);
#else
							assert(false);
#endif
						}
						offset = (0x40 / (int)bankEntry.size + 1) * bankEntry.size;
					}
					size = size + offset;
				}
				inFileIndex = inFileIndex + 1;
			} while (inFileIndex < elementCount);
		}
		//bSuccess = CanAllocate_00184640(Scene::ptable.g_GlobalSoundPtr_00451698, size, 0x1400);
		//if (bSuccess == false) {
		//	*(undefined4*)this->sound_0x2bc = 0;
		//}
		//else {
		//	size = 0;
		//	if (elementCount != 0) {
		//		offset = 0;
		//		do {
		//			bSuccess = edCBankBufferEntry::get_info(this->pCineBankEntry, size, &bankEntry, (char*)0x0);
		//			if ((bSuccess != false) && ((bankEntry.type << 0x10 | bankEntry.stype) == 0x30001)) {
		//				edSoundSampleLoad(bankEntry.fileBufferStart, (ed_sound_sample*)(&this->sound_0x2bc->field_0x0 + offset), 1);
		//				offset = offset + 0x18;
		//			}
		//			size = size + 1;
		//		} while (size < elementCount);
		//	}
		//}
	}
	if ((this->prtBuffer == 1) || ((this->flags_0x4 & 8) != 0)) {
		edMemClearFlags(TO_HEAP(H_MAIN), 0x100);
	}
	return;
}

CActor* CCinematic::GetActorByHashcode(uint hashCode)
{
	CActor* pFoundActor;
	int curActorIndex;

	CUTSCENE_LOG(LogLevel::Info, "CCinematic::GetActorByHashcode Looking for: {}", hashCode);

	if (((((this->flags_0x4 & 0x8000) != 0) && (this->actorRefB.Get() != (CActor*)0x0)) && (this->pActor != (CActor*)0x0)) &&
		(hashCode == actorRefB.Get()->subObjA->hashCode)) {
		hashCode = this->pActor->subObjA->hashCode;
	}

	CUTSCENE_LOG(LogLevel::Info, "CCinematic::GetActorByHashcode Adjusted code: {}", hashCode);

	pFoundActor = (CActor*)0x0;
	curActorIndex = 0;
	while ((curActorIndex < this->nbActorRefs && (pFoundActor == (CActor*)0x0))) {
		if (hashCode == this->ppActorCinematics[curActorIndex]->subObjA->hashCode) {
			pFoundActor = this->ppActorCinematics[curActorIndex];
			CUTSCENE_LOG(LogLevel::Info, "CCinematic::GetActorByHashcode Found actor existing in list at index: {}", curActorIndex);
		}

		curActorIndex = curActorIndex + 1;
	}

	if (pFoundActor == (CActor*)0x0) {
		pFoundActor = CScene::ptable.g_ActorManager_004516a4->GetActorByHashcode(hashCode);

		CUTSCENE_LOG(LogLevel::Info, "CCinematic::GetActorByHashcode Found actor in actor manager: {}", pFoundActor->name);

		this->ppActorCinematics[this->nbActorRefs] = (CActorCinematic*)pFoundActor;
		this->nbActorRefs = this->nbActorRefs + 1;
	}

	return pFoundActor;
}

int CCinematic::GetParticleInstance(int id)
{
	int iVar1;
	ParticleInstance* piVar2;

	iVar1 = 0;
	if (0 < this->nbInstalledParticles) {
		piVar2 = this->buffer_0x2e4;

		do {
			if (id == piVar2->id) {
				return iVar1;
			}

			iVar1 = iVar1 + 1;
			piVar2 = piVar2 + 1;
		} while (iVar1 < this->nbInstalledParticles);
	}

	return -1;
}

int CCinematic::CreateParticleInstance(float param_1, int index, int particleId, CActor* pActor)
{
	int iVar1;
	ParticleFileData* pFileData;
	_ed_particle_manager* p_Var2;
	int particleInstanceId;
	ParticleInstance* pPVar4;
	int iVar5;
	ParticleEntry* pPVar6;

	iVar5 = 0;
	iVar1 = this->nbInstalledParticles;
	if (0 < iVar1) {
		pPVar4 = this->buffer_0x2e4;
		do {
			if (pPVar4->id == -1) break;

			iVar5 = iVar5 + 1;
			pPVar4 = pPVar4 + 1;
		} while (iVar5 < iVar1);
	}

	particleInstanceId = -1;
	if (iVar5 != iVar1) {
		pPVar6 = this->particleSectionStart + index;
		pFileData = (ParticleFileData*)edMemAlloc(this->prtFileBufferPool, pPVar6->fileSize);
		if (pFileData == (ParticleFileData*)0x0) {
			particleInstanceId = -1;
		}
		else {
			memcpy(pFileData, pPVar6->pFileBuffer, pPVar6->fileSize);
			p_Var2 = edParticlesInstall(pFileData, CScene::_scene_handleA, (ed_g2d_manager*)0x0, pPVar6->aMaterialPtrs, pPVar6->aHashes, pPVar6->nbMaterials, (ed_g3d_manager*)0x0, true);
			if (p_Var2 == (_ed_particle_manager*)0x0) {
				particleInstanceId = -1;
			}
			else {
				pPVar4 = this->buffer_0x2e4 + iVar5;
				pPVar4->id = particleId;
				pPVar4->field_0x8 = param_1;
				pPVar4->field_0x4 = 0.0f;
				pPVar4->pParticle = p_Var2;
				pPVar4->pParticleFileData = pFileData;
				pPVar4->pActor = pActor;
				particleInstanceId = iVar5;
			}
		}
	}

	return particleInstanceId;
}



bool CCinematic::CanBePlayed()
{
	bool bVar1;

	bVar1 = (this->flags_0x4 & 1) != 0;

	if (bVar1) {
		bVar1 = (this->flags_0x8 & 0x400) != 0;
	}

	if (!bVar1) {
		bVar1 = (this->flags_0x8 & 0x28) != 0;
	}

	bVar1 = (bool)(bVar1 ^ 1);
	if (!bVar1) {
		bVar1 = (this->flags_0x8 & 0x800) != 0;
	}

	return bVar1;
}

CCinematic::~CCinematic()
{
	bool bVar1 = this->pCineBankEntry != (edCBankBufferEntry*)0x0;
	if (bVar1) {
		bVar1 = this->cineBankLoadStage_0x2b4 != 4;
	}

	if (bVar1) {
		Load(0);
	}

	if (this->cineBankLoadStage_0x2b4 == 4) {
		Flush(true);
	}

	FUN_001cbe40();

	if ((this != (CCinematic*)0xffffff64) && (this->aCineActorConfig != (CCineActorConfig*)0x0)) {
		delete[] this->aCineActorConfig;
	}
}

void CCinematic::FUN_001cbe40()
{
	if (this->aActorCinematic != (CActorCinematic*)0x0) {
		delete[] this->aActorCinematic;
		this->aActorCinematic = (CActorCinematic*)0x0;
		this->nbCinematicActors = 0;
	}

	if (this->ppActorCinematics != (CActorCinematic**)0x0) {
		delete(this->ppActorCinematics);
		this->ppActorCinematics = (CActorCinematic**)0x0;
		this->nbActorRefs = 0;
	}

	IMPLEMENTATION_GUARD_LOG();

	//if (this->pCineSunHolderArray != (BWCinSunLight*)0x0) {
	//	__destroy_new_array((undefined*)this->pCineSunHolderArray, BWCinSunLight::~BWCinSunLight);
	//	this->pCineSunHolderArray = (BWCinSunLight*)0x0;
	//	this->count_0x224 = 0;
	//}
	//
	//if (this->pCineSpotHolderArray != (BWCinSpotLight*)0x0) {
	//	__destroy_new_array((undefined*)this->pCineSpotHolderArray, BWCinSpotLight::~BWCinSpotLight);
	//	this->pCineSpotHolderArray = (BWCinSpotLight*)0x0;
	//	this->count_0x22c = 0;
	//}
	
	if (this->buffer_0x2e4 != (ParticleInstance*)0x0) {
		delete(this->buffer_0x2e4);
		this->buffer_0x2e4 = (ParticleInstance*)0x0;
	}

	if (this->particleSectionStart != (ParticleEntry*)0x0) {
		delete(this->particleSectionStart);
		this->particleSectionStart = (ParticleEntry*)0x0;
	}

	if (this->prtFileBufferPool != (void*)0x0) {
		edMemFree(this->prtFileBufferPool);
		this->prtFileBufferPool = (void*)0x0;
	}

	return;
}

void CCinematic::TryTriggerCutscene(CActor* pActor, int param_3)
{
	bool bVar1;
	S_NTF_TARGET_STREAM_REF* pSVar2;
	int iVar3;
	int iVar4;

	if ((this->state == CS_Stopped) &&
		((g_CinematicManager_0048efc->pCinematic == (CCinematic*)0x0 || ((this->flags_0x4 & 0x200) == 0)))) {
		if (param_3 != 0) {
			this->flags_0x8 = this->flags_0x8 | 0x800;
		}

		bVar1 = (this->flags_0x4 & 1) != 0;
		if (bVar1) {
			bVar1 = (this->flags_0x8 & 0x400) != 0;
		}

		if (!bVar1) {
			bVar1 = (this->flags_0x8 & 0x28) != 0;
		}

		bool bShouldTrigger = (bool)(bVar1 ^ 1);
		if (!bShouldTrigger) {
			bShouldTrigger = (this->flags_0x8 & 0x800) != 0;
		}

		if (bShouldTrigger) {
			this->pActor = pActor;
			this->flags_0x8 = this->flags_0x8 | CINEMATIC_FLAG_ACTIVE;

			if (this->cineBankLoadStage_0x2b4 != 4) {
				Load(1);
			}

			bVar1 = this->pCineBankEntry != (edCBankBufferEntry*)0x0;
			if (bVar1) {
				bVar1 = this->cineBankLoadStage_0x2b4 != 4;
			}

			if (((bVar1) || (this->cineBankLoadStage_0x2b4 == 4)) && ((this->flags_0x8 & 0x800) == 0)) {
				pSVar2 = this->pSwitchListA;
				bVar1 = false;
				if ((pSVar2 != (S_NTF_TARGET_STREAM_REF*)0x0) &&
					(this->pStreamEventCameraA != (S_STREAM_EVENT_CAMERA*)0x0)) {
					bVar1 = true;
				}

				if (bVar1) {
					pSVar2->Switch((CActor*)0x0);
					this->pStreamEventCameraA->SwitchOn((CActor*)0x0);
					pSVar2->PostSwitch((CActor*)0x0);
				}
			}
		}
	}

	return;
}

void CCinematic::Draw()
{
	CCinematicManager* pCinematicManager;
	bool bVar2;
	_ed_particle_manager* pManager;
	int iVar4;
	ParticleInstance* piVar6;
	edF32MATRIX4 local_40;
	CActorCinematic* pCinematicActor;

	pCinematicManager = g_CinematicManager_0048efc;
	if (this->state != CS_Stopped) {
		g_CinematicManager_0048efc->pCurCinematic = this;
		if (0 < this->nbActorRefs) {
			for (int i = 0; i < this->nbActorRefs; i++) {
				pCinematicActor = this->ppActorCinematics[i];

				if (((pCinematicActor->flags & 0x4000) != 0) &&
					(pCinematicActor->Draw(), pCinematicActor->p3DHierNode != (ed_3d_hierarchy_node*)0x0)) {
					pCinematicActor->ComputeLighting();
				}
			}
		}

		bVar2 = GameDList_BeginCurrent();
		if (bVar2 != false) {
			if (0 < this->nbInstalledParticles) {
				iVar4 = 0;
				do {
					piVar6 = this->buffer_0x2e4 + iVar4;
					if (((piVar6->id != -1) && (piVar6->field_0xc != 0)) && ((piVar6->pActor->flags & 0x2000060) == 0)) {
						edParticlesSetSystem(piVar6->pParticle);
						local_40 = gF32Matrix4Unit;
						pManager = piVar6->pParticle;
						edPartSetDisplayMatrix(pManager, &local_40);
						edParticlesDraw(pManager, 1.0f);
					}
					iVar4 = iVar4 + 1;
				} while (iVar4 < this->nbInstalledParticles);
			}
			GameDList_EndCurrent();
		}

		pCinematicManager->pCurCinematic = (CCinematic*)0x0;
	}
	return;
}

void CCinematic::Flush(bool param_2)
{
	CCinematicManager* pCVar1;
	long lVar2;
	int* piVar3;
	int iVar4;
	uint uVar5;
	CCinematic* local_10;
	undefined4 local_c;
	CCinematic** local_4;

	if (this->pActor == (CActor*)0x0) {
		lVar2 = 0;
	}
	else {
		local_c = 1;
		local_4 = &local_10;
		local_10 = this;
		lVar2 = this->pActor->ReceiveMessage(0, (ACTOR_MESSAGE)0x7c, local_4);
	}
	pCVar1 = g_CinematicManager_0048efc;
	if ((lVar2 == 0) || (param_2 != false)) {
		g_CinematicManager_0048efc->pCurCinematic = this;
		if (this->defaultAudioTrackId != -1) {
			this->cinematicLoadObject.BWCinSourceAudio_Obj.Stop();
			this->cinematicLoadObject.BWCinSourceAudio_Obj.pGlobalSoundFileData = (GlobalSound_FileData*)0x0;
		}

		if ((void*)this->field_0x2c4 != (void*)0x0) {
			edMemFree((void*)this->field_0x2c4);
			this->field_0x2c4 = 0;
		}

		if ((void*)this->playingSounds_0x2c0 != (void*)0x0) {
			edMemFree((void*)this->playingSounds_0x2c0);
			this->playingSounds_0x2c0 = 0;
		}

		if (this->fileInfoStart != (CinFileContainer*)0x0) {
			delete this->fileInfoStart;
			this->fileInfoStart = (CinFileContainer*)0x0;
			this->cinFileCount = 0;
		}

		if (this->sound_0x2bc != (ed_sound_sample*)0x0) {
			IMPLEMENTATION_GUARD_AUDIO(
			uVar5 = 0;
			if (this->soundCount_0x2b8 != 0) {
				iVar4 = 0;
				do {
					piVar3 = (int*)(&this->sound_0x2bc->field_0x0 + iVar4);
					if (*piVar3 != 0) {
						edSoundSampleFree(piVar3);
					}
					uVar5 = uVar5 + 1;
					iVar4 = iVar4 + 0x18;
				} while (uVar5 < (uint)this->soundCount_0x2b8);
			}
			edMemFree(this->sound_0x2bc);)
			this->sound_0x2bc = (ed_sound_sample*)0x0;
			this->soundCount_0x2b8 = 0;
		}
		this->pCineBankEntry->close();
		this->pCineBankEntry = (edCBankBufferEntry*)0x0;
		this->cineBank.terminate();
		this->cineBankLoadStage_0x2b4 = 0;
		pCVar1->pCurCinematic = (CCinematic*)0x0;
		this->flags_0x8 = this->flags_0x8 & 0xffffdfff;
	}
	return;
}

void CCinematic::PreReset()
{
	bool bVar1;
	CCinematicManager* pCVar2;
	S_NTF_TARGET_STREAM_REF* pSVar3;
	uint uVar4;
	int iVar5;
	int iVar6;

	pCVar2 = g_CinematicManager_0048efc;

	bVar1 = (this->cinematicLoadObject).BWCinSourceAudio_Obj.intFieldA != 0;
	if ((bVar1) && (bVar1 = true, (this->cinematicLoadObject).BWCinSourceAudio_Obj.field_0x8 != 0.0f)) {
		bVar1 = false;
	}

	if (bVar1) {
		g_CinematicManager_0048efc->pCurCinematic = this;
		this->cinematicLoadObject.BWCinSourceAudio_Obj.Stop();
		pCVar2->pCurCinematic = (CCinematic*)0x0;
	}

	bVar1 = this->pCineBankEntry != (edCBankBufferEntry*)0x0;
	if (bVar1) {
		bVar1 = this->cineBankLoadStage_0x2b4 != 4;
	}

	if (bVar1) {
		Load(0);
	}

	if (this->state != CS_Stopped) {
		Stop();
	}

	uVar4 = 0x27f4;
	if ((this->flags_0x4 & 0x10000000) != 0) {
		uVar4 = 0x27d4;
	}

	this->flags_0x8 = this->flags_0x8 & ~uVar4;
	this->count_0x2d8 = 0;
	this->totalCutsceneDelta = 0.0;
	pSVar3 = this->pSwitchListA;
	bVar1 = false;

	if ((pSVar3 != (S_NTF_TARGET_STREAM_REF*)0x0) && (this->pStreamEventCameraA != (S_STREAM_EVENT_CAMERA*)0x0))
	{
		bVar1 = true;
	}
	if (bVar1) {
		pSVar3->Reset();
		this->pStreamEventCameraA->Reset((CActor*)0x0);
	}

	pSVar3 = this->pSwitchListB;
	bVar1 = false;
	if ((pSVar3 != (S_NTF_TARGET_STREAM_REF*)0x0) && (this->pStreamEventCameraB != (S_STREAM_EVENT_CAMERA*)0x0))
	{
		bVar1 = true;
	}

	if (bVar1) {
		pSVar3->Reset();
		this->pStreamEventCameraB->Reset((CActor*)0x0);
	}

	if (((this->flags_0x4 & 0x800) != 0) &&
		((this->baseB == -1 ||
			(this->baseB == CLevelScheduler::gThis->aLevelInfo[CLevelScheduler::gThis->currentLevelID].sectorStartIndex)))) {
		TryTriggerCutscene((CActor*)0x0, 0);
	}

	if ((this->flags_0x4 & 0x2000000) != 0) {
		Load(0);
	}

	return;
}

void CCinematic::UninstallResources()
{
	void* pData;
	int iVar1;
	int iVar2;
	int resourceType;
	int iVar4;
	int iVar5;
	int iVar6;
	CinFileContainer* pFileContainer;

	iVar1 = this->textData.get_entry_count();
	if (iVar1 != 0) {
		this->textData.select_language((edCBankBufferEntry*)0x0, (char*)0x0, AUTO);
	}

	iVar1 = this->cinFileCount + -1;
	pFileContainer = this->fileInfoStart + iVar1;
	for (; iVar1 != -1; iVar1 = iVar1 + -1) {
		pData = pFileContainer->pData;
		if (pData != (void*)0x0) {
			resourceType = pFileContainer->type;

			if (resourceType == edResCollection::COT_Particle) {
				const int index = (int)pData;

				ParticleEntry* pParticleEntry = this->particleSectionStart + index - 1;
				iVar5 = 0;
				ed_g2d_manager** pManager = pParticleEntry->aManagers;
				if (0 < pParticleEntry->nbManagers) {
					do {
						iVar4 = 0;
						if (0 < pParticleEntry->nbMaterials) {
							do {
								edDListTermMaterial(pParticleEntry->aMaterials + iVar4);
								iVar4 = iVar4 + 1;
							} while (iVar4 < pParticleEntry->nbMaterials);
						}
						pParticleEntry->nbMaterials = 0;

						ed3DUnInstallG2D(*pManager);
						iVar5 = iVar5 + 1;
						pManager = pManager + 1;
					} while (iVar5 < pParticleEntry->nbManagers);
				}

				pParticleEntry->nbManagers = 0;
			}
			else {
				if (resourceType == edResCollection::COT_LipTrack) {
					IMPLEMENTATION_GUARD_LOG(
					CScene::ptable.g_LipTrackManager_00451694->Remove(pData);)
				}
				else {
					if (resourceType == edResCollection::COT_MeshModel) {
						ed3DUnInstallG3D((ed_g3d_manager*)pData);
					}
					else {
						if (resourceType == edResCollection::COT_MeshTexture) {
							ed3DUnInstallG2D((ed_g2d_manager*)pData);
						}
					}
				}
			}
			pFileContainer->pData = (char*)0x0;
		}
		pFileContainer = pFileContainer + -1;
	}
	return;
}

bool S_STREAM_NTF_TARGET_SWITCH::Switch(CActor* pActor)
{
	uint uVar1;
	bool bVar2;
	bool uVar3;

	uVar1 = this->flags;
	uVar3 = false;
	bVar2 = true;

	if (((uVar1 & 1) != 0) && ((uVar1 & 0x40000000) != 0)) {
		bVar2 = false;
	}

	if (bVar2) {
		this->flags = this->flags | 0x40000000;

		if (this->messageId != 0) {
			g_CinematicManager_0048efc->NotifyCinematic(this->cutsceneId, pActor, this->messageId, this->messageFlags);

			if (pActor == (CActor*)0x0) {
				pActor = (CActor*)LOAD_SECTION(this->pRef);
			}

			if (pActor != (CActor*)0x0) {
				uVar3 = pActor->DoMessage(LOAD_SECTION_CAST(CActor*, this->pRef), (ACTOR_MESSAGE)this->messageId, (MSG_PARAM)this->messageFlags);
			}
		}
	}

	return uVar3;
}

void S_STREAM_EVENT_CAMERA_CONTAINER::Create(ByteCode* pByteCode)
{
	this->pCameraStreamRef = reinterpret_cast<S_STREAM_EVENT_CAMERA*>(pByteCode->currentSeekPos);
	pByteCode->currentSeekPos = reinterpret_cast<char*>(this->pCameraStreamRef + 1);
	return;
}

bool S_STREAM_NTF_TARGET_SWITCH_EX::Switch(CActor* pActor, uint messageFlags)
{
	uint uVar1;
	bool bVar2;
	bool uVar3;

	uVar1 = this->flags;
	uVar3 = false;
	bVar2 = true;

	if (((uVar1 & 1) != 0) && ((uVar1 & 0x40000000) != 0)) {
		bVar2 = false;
	}

	if (bVar2) {
		this->flags = this->flags | 0x40000000;

		if (this->messageId != 0) {
			g_CinematicManager_0048efc->NotifyCinematic(this->cutsceneId, pActor, this->messageId, messageFlags);

			if (pActor == (CActor*)0x0) {
				pActor = (CActor*)LOAD_SECTION(this->pRef);
			}

			if (pActor != (CActor*)0x0) {
				uVar3 = pActor->DoMessage(LOAD_SECTION_CAST(CActor*, this->pRef), (ACTOR_MESSAGE)this->messageId, (MSG_PARAM)messageFlags);
			}
		}
	}

	return uVar3;
}

int S_STREAM_NTF_TARGET_ANALOG::NotifyAnalog(float param_1, float param_2, CActor* param_4, S_STREAM_EVENT_CAMERA* param_5)
{
	int iVar1;
	bool bVar2;
	int iVar3;
	CActor* pCVar4;
	CActor* pCVar5;
	float local_18;
	uint local_14;
	uint local_c;
	uint local_8;
	float* local_4;

	iVar3 = 0;

	iVar1 = this->field_0x1c;
	if (iVar1 == 2) {
		pCVar5 = (CActor*)0x0;
		if ((this->field_0x20 <= param_1) && (param_1 <= this->field_0x24)) {
			pCVar5 = (CActor*)1;
		}

		pCVar4 = (CActor*)(uint)(this->field_0x20 <= param_2);
		if (pCVar4 != (CActor*)0x0) {
			pCVar4 = (CActor*)(uint)(param_2 <= this->field_0x24);
		}
	}
	else {
		pCVar5 = (CActor*)1;

		if (iVar1 == 1) {
			if ((this->field_0x20 <= param_1) && (pCVar5 = (CActor*)1, param_1 <= this->field_0x24)) {
				pCVar5 = (CActor*)0x0;
			}

			pCVar4 = (CActor*)(uint)(param_2 < this->field_0x20);
			if (pCVar4 == (CActor*)0x0) {
				pCVar4 = (CActor*)(uint)(this->field_0x24 < param_2);
			}
		}
		else {
			pCVar4 = (CActor*)param_5;
			pCVar5 = param_4;
			if (iVar1 == 0) {
				local_14 = this->field_0x10;
				if (param_4 == (CActor*)0x0) {
					param_4 = LOAD_SECTION_CAST(CActor*, this->pRef);
				}

				local_18 = param_2;
				if (param_4 != (CActor*)0x0) {
					local_4 = &local_18;
					iVar3 = param_4->DoMessage(LOAD_SECTION_CAST(CActor*, this->pRef), (ACTOR_MESSAGE)0x11, (MSG_PARAM)local_4);
				}

				pCVar4 = (CActor*)1;
				pCVar5 = (CActor*)1;
			}
		}
	}

	if (pCVar5 != pCVar4) {
		if (pCVar4 == (CActor*)0x0) {
			iVar3 = 0;
			bVar2 = true;
			if (((this->flags & 1) != 0) && ((this->flags & 0x40000000) != 0)) {
				bVar2 = false;
			}
			if (bVar2) {
				this->flags = this->flags | 0x40000000;
				if (this->field_0x14 != 0) {
					g_CinematicManager_0048efc->NotifyCinematic(this->cutsceneId, param_4, this->field_0x14, this->field_0x18);
					if (param_4 == (CActor*)0x0) {
						param_4 = LOAD_SECTION_CAST(CActor*, this->pRef);
					}

					if (param_4 != (CActor*)0x0) {
						local_c = this->field_0x18;
						iVar3 = param_4->DoMessage(LOAD_SECTION_CAST(CActor*, this->pRef), (ACTOR_MESSAGE)this->field_0x14, (MSG_PARAM)local_c);
					}
				}
			}
		}
		else {
			param_5->SwitchOn(param_4);
			iVar3 = 0;
			bVar2 = true;
			if (((this->flags & 1) != 0) && ((this->flags & 0x40000000) != 0)) {
				bVar2 = false;
			}

			if (bVar2) {
				this->flags = this->flags | 0x40000000;
				if (this->field_0xc != 0) {
					g_CinematicManager_0048efc->NotifyCinematic(this->cutsceneId, param_4, this->field_0xc, this->field_0x10);
					if (param_4 == (CActor*)0x0) {
						param_4 = LOAD_SECTION_CAST(CActor*, this->pRef);
					}
					if (param_4 != (CActor*)0x0) {
						local_8 = this->field_0x10;
						iVar3 = param_4->DoMessage(LOAD_SECTION_CAST(CActor*, this->pRef), (ACTOR_MESSAGE)this->field_0xc, (MSG_PARAM)local_8);
					}
				}
			}
		}
	}

	return iVar3;
}

void S_STREAM_NTF_TARGET_SWITCH_EX::Reset()
{
	this->flags = this->flags & 1;

	return;
}

void S_STREAM_NTF_TARGET_ANALOG::Reset()
{
	this->flags = this->flags & 1;

	return;
}

void CCinematic::FUN_001c7390(bool param_2)
{
	ed_zone_3d* peVar1;
	bool bVar2;
	int iVar3;
	long lVar4;

	if ((this->state == CS_Stopped) && ((this->flags_0x8 & CINEMATIC_FLAG_ACTIVE) != 0)) {
		Start();
	}

	if ((param_2 != false) && (this->state == CS_Stopped)) {
		bVar2 = (this->flags_0x4 & 1) != 0;
		if (bVar2) {
			bVar2 = (this->flags_0x8 & 0x400) != 0;
		}
		if (!bVar2) {
			bVar2 = (this->flags_0x8 & 0x28) != 0;
		}
		bVar2 = (bool)(bVar2 ^ 1);
		if (!bVar2) {
			bVar2 = (this->flags_0x8 & 0x800) != 0;
		}
		if (bVar2) {
			bVar2 = this->cineBankLoadStage_0x2b4 != 4;
			if ((bVar2) && (this->pCineBankEntry == (edCBankBufferEntry*)0x0 || !bVar2)) {
				bVar2 = true;
				if (this->count_0x2d8 < 1) {
					peVar1 = (this->zoneRefB).Get();
					bVar2 = false;
		
					if ((peVar1 != (ed_zone_3d*)0x0 && CActorHero::_gThis != (CActorHero*)0x0) &&
						(iVar3 = edEventComputeZoneAgainstVertex((CScene::ptable.g_EventManager_006f5080)->activeChunkId, peVar1, &CActorHero::_gThis->currentLocation, 0), iVar3 != 2)) {
						bVar2 = true;
					}
				}

				if (bVar2) {
					Load(1);
				}
			}

			if (this->cineBankLoadStage_0x2b4 == 4) {
				bVar2 = true;
				if ((this->flags_0x4 & 0x4000000) == 0) {
					peVar1 = (this->zoneRefC).Get();
					bVar2 = false;
					if ((peVar1 != (ed_zone_3d*)0x0 && CActorHero::_gThis != (CActorHero*)0x0) &&
						(iVar3 = edEventComputeZoneAgainstVertex((CScene::ptable.g_EventManager_006f5080)->activeChunkId, peVar1, &CActorHero::_gThis->currentLocation, 0), iVar3 != 2)) {
						bVar2 = true;
					}
				}

				if ((bVar2) && (this->defaultAudioTrackId != -1)) {
					lVar4 = this->aAudioTrackIds[CMessageFile::get_default_language()];
					if (lVar4 == -1) {
						lVar4 = this->defaultAudioTrackId;
					}

					this->cinematicLoadObject.BWCinSourceAudio_Obj.SetAudioTrack(lVar4);
				}
			}
		}
	}
	return;
}

void CCinematic::FUN_001caeb0()
{
	this->fileName = "";
	this->pBankName_0x48 = "";
	this->pBankName_0x50 = "";
	(this->zoneRefA).index = 0x0;
	(this->triggerActorRef).index = 0x0;
	(this->zoneRefB).index = 0x0;
	(this->zoneRefC).index = 0x0;
	this->cond_0x248.Invalidate();
	this->field_0x25c = (int*)0x0;

	this->condArray_0x244.Invalidate();

	this->pSwitchListA = (S_NTF_TARGET_STREAM_REF*)0x0;
	this->pStreamEventCameraA = (S_STREAM_EVENT_CAMERA*)0x0;
	this->pSwitchListB = (S_NTF_TARGET_STREAM_REF*)0x0;
	this->pStreamEventCameraB = (S_STREAM_EVENT_CAMERA*)0x0;
	this->pActor = (CActor*)0x0;
	this->defaultAudioTrackId = -1;
	this->flags_0x4 = this->flags_0x4 & 0xffefffff;

	return;
}

void CCinematic::FUN_001c92b0()
{
	if (this->state != CS_Stopped) {
		this->flags_0x8 = this->flags_0x8 & 0xffffff7f;
		this->flags_0x8 = this->flags_0x8 | 0x100;
	}
	return;
}

bool CCinematic::Has_0x2d8()
{
	return 0 < this->count_0x2d8;
}

void CCinematic::Remove_0x2d8()
{
	if (0 < this->count_0x2d8) {
		this->count_0x2d8 = this->count_0x2d8 + -1;
	}

	return;
}

void CCinematic::Add_0x2d8()
{
	this->count_0x2d8 = this->count_0x2d8 + 1;

	return;
}

void CCinematic::Level_ClearAll()
{
	ed_zone_3d* pZone;
	bool bVar1;
	int iVar2;
	ulong uVar3;

	if (((this->flags_0x4 & 0x100000) != 0) && (this->field_0x4c != 0)) {
		uVar3 = this->cond_0x248.IsVerified();
		if (uVar3 == 0) {
			this->flags_0x8 = this->flags_0x8 | 8;
		}
		else {
			this->flags_0x8 = this->flags_0x8 & 0xfffffff7;
		}
	}

	if ((this->state != CS_Stopped) && ((this->flags_0x8 & 0x100) != 0)) {
		Stop();
	}

	if (((this->cineBankLoadStage_0x2b4 == 4) && (this->state == CS_Stopped)) && ((this->flags_0x8 & CINEMATIC_FLAG_ACTIVE) == 0)) {
		pZone = (this->zoneRefB).Get();
		bVar1 = false;

		if ((pZone != (ed_zone_3d*)0x0 && CActorHero::_gThis != (CActorHero*)0x0) &&
			(iVar2 = edEventComputeZoneAgainstVertex((CScene::ptable.g_EventManager_006f5080)->activeChunkId, pZone, &CActorHero::_gThis->currentLocation, 0), iVar2 != 2)) {
			bVar1 = true;
		};

		if ((bVar1) || (0 < this->count_0x2d8)) {
			bVar1 = (this->flags_0x4 & 1) != 0;
			if (bVar1) {
				bVar1 = (this->flags_0x8 & 0x400) != 0;
			}

			if (!bVar1) {
				bVar1 = (this->flags_0x8 & 0x28) != 0;
			}

			bVar1 = (bool)(bVar1 ^ 1);
			if (!bVar1) {
				bVar1 = (this->flags_0x8 & 0x800) != 0;
			}

			if (bVar1) {
				return;
			}
		}
		if ((this->flags_0x8 & 4) == 0) {
			Flush(false);
		}
		else {
			this->flags_0x8 = this->flags_0x8 | 0x2000;
		}
	}

	return;
}

bool edCinematic::ExtractVersions(int size, int* cinematicLibraryVersion, int* cinematicCompilerVersion)
{
	char* peVar1;
	char* local_4;

	local_4 = (char*)this;
	peVar1 = local_4 + size;
	if ((char*)this < peVar1) {
		do {
			if (*(int*)local_4 == 0x214e4943) {
				if (cinematicLibraryVersion != (int*)0x0) {
					*cinematicLibraryVersion = *(int*)(local_4 + 8);
				}
				if (cinematicCompilerVersion != (int*)0x0) {
					*cinematicCompilerVersion = *(int*)(local_4 + 0xc);
				}
				return true;
			}
			local_4 = local_4 + *(int*)(local_4 + 4) + 8;
		} while (local_4 < peVar1);
	}
	return false;
}

bool CBWitchCin::CreateActor(edCinActorInterface** ppActorInterface, edCinGameInterface::ACTORV_CREATIONtag* const pTag)
{
	ed_g2d_manager* textureObj;
	ed_g3d_manager* meshInfoObj;
	CActorCinematic* lVar1;
	edCinActorInterface* cutsceneObject;
	int resourceSize;
	CCinematic* pCinematic;

	meshInfoObj = (ed_g3d_manager*)0x0;
	textureObj = (ed_g2d_manager*)0x0;
	pCinematic = g_CinematicManager_0048efc->pCurCinematic;
	if ((pTag->meshName != 0) && ((char*)pTag->textureName != (char*)0x0)) {
		textureObj = (ed_g2d_manager*)pCinematic->InstallResource(edResCollection::COT_MeshTexture, pTag->bHasTexture, pTag->textureName, (ed_g2d_manager*)0x0, &resourceSize);
		if (textureObj == (ed_g2d_manager*)0x0) {
			textureObj = CScene::ptable.g_C3DFileManager_00451664->LoadDefaultTexture_001a65d0();
		}

		meshInfoObj = (ed_g3d_manager*)pCinematic->InstallResource(edResCollection::COT_MeshModel, pTag->bHasMesh, pTag->meshName, textureObj, &resourceSize);
	}
	/* Sound files can go in here */
	lVar1 = pCinematic->NewCinematicActor(pTag, meshInfoObj, textureObj);
	lVar1->behaviourCinematic.cinActor.SetupTransform(&pTag->position, &pTag->heading, &pTag->scale, 0);

	// HACK! Don't write 8 bytes here on win!
	int* pInt = (int*)ppActorInterface;
	*pInt = STORE_SECTION(&lVar1->behaviourCinematic.cinActor);

	CUTSCENE_LOG(LogLevel::Info, "CBWitchCin::CreateActor Created 0x{:x}", *pInt);

	return true;
}

bool CBWitchCin::GetActor(edCinActorInterface** ppActorInterface, uint hashCode, edCinGameInterface::ACTORV_CREATIONtag* const pTag)
{
	CActor* pActor;
	ed_g2d_manager* pTextureManager;
	ed_g3d_manager* pMeshManager;
	CBehaviourCinematic* pBehaviourCinematic;
	int iStack4;
	CCinematic* pCinematic;

	pCinematic = g_CinematicManager_0048efc->pCurCinematic;

	pActor = pCinematic->GetActorByHashcode(hashCode);
	pMeshManager = (ed_g3d_manager*)0x0;

	if ((((pTag->bHasMesh == false) || (pTag->bHasTexture == false)) && (pTag->meshName != (char*)0x0)) &&
		(pTag->textureName != (char*)0x0)) {
		pTextureManager = (ed_g2d_manager*)
			pCinematic->InstallResource(edResCollection::COT_MeshTexture, (ulong)pTag->bHasTexture, pTag->textureName, (ed_g2d_manager*)0x0, &iStack4);

		if (pTextureManager == (ed_g2d_manager*)0x0) {
			pTextureManager = CScene::ptable.g_C3DFileManager_00451664->LoadDefaultTexture_001a65d0();
		}

		pMeshManager = (ed_g3d_manager*)pCinematic->InstallResource(edResCollection::COT_MeshModel, (ulong)pTag->bHasMesh, pTag->meshName, pTextureManager, &iStack4);
	}

	pBehaviourCinematic = static_cast<CBehaviourCinematic*>(pActor->GetBehaviour(1));
	pBehaviourCinematic->cinActor.SetupTransform(&pTag->position, &pTag->heading, &pTag->scale, pMeshManager);

	// HACK! Don't write 8 bytes here on win!
	int* pInt = (int*)ppActorInterface;
	*pInt = STORE_SECTION(&pBehaviourCinematic->cinActor);

	CUTSCENE_LOG(LogLevel::Info, "CBWitchCin::GetActor Retrieved 0x{:x}", *pInt);

	//*ppActorInterface = (edCinActorInterface*)&pCVar1->cinActor;
	return true;
}

bool CBWitchCin::ReleaseActor(edCinActorInterface*)
{
	return true;
}

bool CBWitchCin::CreateScenery(edCinSceneryInterface** ppActorInterface, const edCinGameInterface::SCENERY_CREATIONtag* pTag)
{
	ed_g2d_manager* ret;
	ed_g3d_manager* outMeshInfo;
	int iStack4;
	CCinematic* pCinematic;

	pCinematic = g_CinematicManager_0048efc->pCurCinematic;

	CUTSCENE_LOG(LogLevel::Info, "CBWitchCin::CreateScenery Install", pTag->szTexturePath);
	ret = (ed_g2d_manager*)pCinematic->InstallResource(edResCollection::COT_MeshTexture, (ulong)pTag->textureType, pTag->szTexturePath, (ed_g2d_manager*)0x0, &iStack4);
	
	CUTSCENE_LOG(LogLevel::Info, "CBWitchCin::CreateScenery Install", pTag->szMeshPath);
	outMeshInfo = (ed_g3d_manager*)pCinematic->InstallResource(edResCollection::COT_MeshModel, (ulong)pTag->meshType, pTag->szMeshPath, ret, &iStack4);
	pCinematic->pMeshInfo = outMeshInfo;
	return true;
}

bool CBWitchCin::ReleaseScenery(edCinSceneryInterface* )
{
	g_CinematicManager_0048efc->pCurCinematic->pMeshInfo = (ed_g3d_manager*)0x0;
	return true;
}

bool CBWitchCin::ReleaseResource(byte, bool, void*)
{
	return true;
}

bool CBWitchCin::GetCamera(edCinCamInterface** pCinCamInterface, const edCinCamInterface::CAMERA_CREATIONtag*)
{
	*pCinCamInterface = &this->BWCinCam_Obj;
	g_CinematicManager_0048efc->pCurCinematic->flags_0x8 = g_CinematicManager_0048efc->pCurCinematic->flags_0x8 | 2;
	return true;
}

bool CBWitchCin::ReleaseCamera(edCinCamInterface*)
{
	g_CinematicManager_0048efc->pCurCinematic->flags_0x8 = g_CinematicManager_0048efc->pCurCinematic->flags_0x8 | 2;
	return true;
}

bool CBWitchCin::GetSourceAudioInterface(edCinSourceAudioI** ppSourceAudioInterface)
{
	*ppSourceAudioInterface = &this->BWCinSourceAudio_Obj;
	return true;
}

bool CBWitchCin::ReleaseSourceAudioInterface(edCinSourceAudioI*)
{
	return true;
}

bool CBWitchCin::GetSourceSubtitleInterface(edCinSourceSubtitleI** ppSourceSubtitleInterface)
{
	*ppSourceSubtitleInterface = &this->BWCinSourceSubtitle_Obj;
	return true;
}

bool CBWitchCin::ReleaseSourceSubtitleInterface(edCinSourceSubtitleI*)
{
	return false;
}

char* CBWitchCin::GetResource(edResCollection::RES_TYPE type1, bool type2, const char* fileName, int* bufferLengthOut)
{
	int* fileBufferStart;

	CUTSCENE_LOG(LogLevel::Info, "CBWitchCin::GetResource {}\n", fileName);

	if ((type1 == edResCollection::COT_MeshModel) || (type1 == edResCollection::COT_MeshTexture)) {
		*bufferLengthOut = 0;
		fileBufferStart = (int*)1;
	}
	else {
		fileBufferStart = g_CinematicManager_0048efc->pCurCinematic->InstallResource(type1, type2, fileName, (ed_g2d_manager*)0x0, bufferLengthOut);
	}
	return (char*)fileBufferStart;
}

bool CBWCinCam::Activate()
{
	CCinematic* pCVar1;
	CCameraCinematic* pCVar2;
	SWITCH_MODE iVar3;
	SWITCH_MODE SVar4;
	float fVar5;
	float fVar6;

	pCVar1 = g_CinematicManager_0048efc->pCurCinematic;
	pCVar2 = g_CinematicManager_0048efc->pCinematicCamera;
	if (((pCVar1->flags_0x4 & 0x800) == 0) && ((pCVar1->flags_0x8 & 0x800) == 0)) {
		iVar3 = pCVar1->field_0x64;
		fVar5 = pCVar1->field_0x68;
		SVar4 = pCVar1->field_0x5c;
		fVar6 = pCVar1->field_0x60;
		(pCVar2)->field_0x8c = pCVar1->field_0x58;
		(pCVar2)->switchMode = SVar4;
		(pCVar2)->field_0x98 = fVar6;
		(pCVar2)->field_0x94 = iVar3;
		(pCVar2)->field_0x9c = fVar5;
	}
	else {
		fVar5 = pCVar1->field_0x68;
		iVar3 = pCVar1->field_0x64;
		pCVar2->field_0x8c = 0.0;
		pCVar2->switchMode = SWITCH_MODE_B;
		pCVar2->field_0x98 = 0.0;
		pCVar2->field_0x94 = iVar3;
		pCVar2->field_0x9c = fVar5;
	}
	return true;
}

bool CBWCinCam::Initialize(bool param_2, uint* flags)
{
	if (param_2 != false) {
		*flags = *flags | 1;
	}
	return true;
}

bool CBWCinCam::SetFov(float fov)
{
	if (((g_CinematicManager_0048efc->pCurCinematic->flags_0x4 & 0x200) != 0) &&
		(CCameraManager::_gThis->aspectRatio == 1.777778f)) {
		fov = fov * 0.75f;
	}
	g_CinematicManager_0048efc->pCinematicCamera->fov = fov;
	return true;
}

bool CBWCinCam::SetPos(float x, float y, float z)
{
	CCinematicManager* pCVar1;
	edF32VECTOR4 outLocation;
	CCameraCinematic* pCinCam;

	pCVar1 = g_CinematicManager_0048efc;
	outLocation.w = 1.0f;
	outLocation.x = x;
	outLocation.y = y;
	outLocation.z = z;
	edF32Matrix4MulF32Vector4Hard(&outLocation, &g_CinematicManager_0048efc->pCurCinematic->matrix_0x120, &outLocation);
	pCinCam = pCVar1->pCinematicCamera;
	pCinCam->transformationMatrix.rowT = outLocation;
	return true;
}

bool CBWCinCam::SetHeadingQuat(float x, float y, float z, float w)
{
	CCinematic* pCVar1;
	CCinematicManager* pCVar2;
	edF32VECTOR4 local_50;
	edF32MATRIX4 eStack64;
	CCameraCinematic* pCinCam;

	pCVar2 = g_CinematicManager_0048efc;
	pCVar1 = g_CinematicManager_0048efc->pCurCinematic;
	local_50.x = x;
	local_50.y = y;
	local_50.z = z;
	local_50.w = w;
	edQuatToMatrix4Hard(&local_50, &eStack64);

	edF32Matrix4MulF32Matrix4Hard(&eStack64, &eStack64, &pCVar1->matrix_0x120);
	pCinCam = pCVar2->pCinematicCamera;
	eStack64.rowT = pCinCam->transformationMatrix.rowT;
	pCinCam->SetTransform(&eStack64);
	return true;
}

bool CBWCinCam::SetHeadingEuler(float x, float y, float z, bool param_5)
{
	CCinematic* pCVar1;
	CCameraCinematic* pCVar2;
	float* pfVar3;
	CCinematicManager* pCVar4;
	float* pfVar5;
	float* pfVar6;
	edF32MATRIX4 auStack96;
	edF32VECTOR4 newLookAt;
	float local_10;
	float local_c;
	float local_8;

	pCVar4 = g_CinematicManager_0048efc;
	pCVar1 = g_CinematicManager_0048efc->pCurCinematic;
	if (param_5 == false) {
		newLookAt.w = 1.0f;
		newLookAt.x = x;
		newLookAt.y = y;
		newLookAt.z = z;
		edF32Matrix4MulF32Vector4Hard(&newLookAt, &pCVar1->matrix_0x120, &newLookAt);
	}
	else {
		pfVar5 = (float*)0xc;
		pfVar6 = &local_10;
		pfVar3 = pfVar6;
		while (pfVar3 != (float*)0x0) {
			*(undefined*)pfVar6 = 0;
			pfVar6 = (float*)((int)pfVar6 + 1);
			pfVar5 = (float*)((int)pfVar5 + -1);
			pfVar3 = pfVar5;
		}
		local_10 = x;
		local_c = y;
		local_8 = z;
		edF32Matrix4FromEulerOrdSoft(&auStack96, "YXZ", &local_10);
		edF32Matrix4MulF32Matrix4Hard(&auStack96, &auStack96, &pCVar1->matrix_0x120);
	}
	pCVar2 = pCVar4->pCinematicCamera;
	if (param_5 != false) {
		edF32Vector4AddHard(&newLookAt, &pCVar2->transformationMatrix.rowT, &auStack96.rowZ);
	}
	pCVar2->lookAt = newLookAt;
	return true;
}

bool CBWCinActor::Initialize()
{
	CActor* pActor;
	CCinematic* pCinematic;

	pCinematic = g_CinematicManager_0048efc->GetCurCinematic();

	if ((pCinematic->flags_0x4 & 0x4000) == 0) {
		this->field_0x8 = 2;
	}
	else {
		this->field_0x8 = 0;
	}

	pActor = this->pParent;

	CUTSCENE_LOG(LogLevel::Info, "CBWCinActor::Initialize Adding CBWCinActor {} to {}", pActor->name, pCinematic->fileName);

	if ((pActor->flags & 0x800000) == 0) {
		pActor->CinematicMode_Enter(false);
	}
	else {
		pActor->SetState(3, -1);
	}
	return true;
}

bool CBWCinActor::OnFrameDirected()
{
	bool bVar1;
	CCinematic* pCVar2;
	CActor* pActor;

	pCVar2 = g_CinematicManager_0048efc->GetCurCinematic();
	bVar1 = pCVar2->state != CS_Interpolate;
	if (bVar1) {
		pActor = this->pParent;
		pActor->flags = pActor->flags & 0xffffffbf;

		pActor->EvaluateDisplayState();
	}
	return bVar1;
}

bool CBWCinActor::SetPos(float newX, float newY, float newZ)
{
	CCinematic* pCinematic;
	float fVar1;
	//CCollisionRay CStack80;
	edF32VECTOR4 local_30;
	edF32VECTOR4 transformedLocation;
	edF32VECTOR4 newLocation;
	CCollision* pCollision;

	pCinematic = g_CinematicManager_0048efc->GetCurCinematic();
	newLocation.w = 1.0f;
	newLocation.x = newX;
	newLocation.y = newY;
	newLocation.z = newZ;
	edF32Matrix4MulF32Vector4Hard(&newLocation, &pCinematic->matrix_0x120, &newLocation);
	if ((this->field_0x8 & 1U) != 0) {
		IMPLEMENTATION_GUARD(
		local_30.x = 0.0;
		local_30.y = -1.0f;
		local_30.z = 0.0;
		local_30.w = 0.0;
		transformedLocation.x = newLocation.x;
		transformedLocation.z = newLocation.z;
		transformedLocation.w = newLocation.w;
		pCollision = this->pParent->pCollisionData;
		if (pCollision == (CCollision*)0x0) {
			transformedLocation.y = newLocation.y + 0.2;
		}
		else {
			IMPLEMENTATION_GUARD(
			transformedLocation.y = newLocation.y + ((pCollision->vectorFieldB).y - this->pParent->currentLocation.y);)
		}
		CCollisionRay::CCollisionRay(10.0, &CStack80, &transformedLocation, &local_30);
		fVar1 = CCollisionRay::Intersect
		(&CStack80, 3, this->pParent, (CActor*)0x0, 0x40000001, (edF32VECTOR4*)0x0, (_ray_info_out*)0x0);
		if (fVar1 != 1e+30f) {
			newLocation.y = transformedLocation.y + fVar1 * local_30.y;
			newLocation.x = transformedLocation.x;
			newLocation.z = transformedLocation.z;
			newLocation.w = transformedLocation.w;
		})
	}
	(this->nextPos).x = newLocation.x;
	(this->nextPos).y = newLocation.y;
	(this->nextPos).z = newLocation.z;

	(this->nextPos).w = newLocation.w;
	this->pParent->flags = this->pParent->flags & 0xffdfffff;
	return true;
}

bool CBWCinActor::SetHeadingEuler(float x, float y, float z, bool param_5)
{
	CActor* pParent;
	CCinematic* pCinematic;
	edF32MATRIX4 transformationMatrix;
	edF32VECTOR3 newRotation;

	pCinematic = g_CinematicManager_0048efc->GetCurCinematic();
	if (param_5 != false) {
		newRotation.x = x * 0.01745329f;
		newRotation.y = y * 0.01745329f;
		newRotation.z = z * 0.01745329f;
		edF32Matrix4FromEulerSoft(&transformationMatrix, &newRotation, "XYZ");
		edF32Matrix4MulF32Matrix4Hard(&transformationMatrix, &transformationMatrix, &pCinematic->matrix_0x120);
		edF32Matrix4ToEulerSoft(&transformationMatrix, &newRotation, "XYZ");
		pParent = this->pParent;
		pParent->rotationEuler.x = newRotation.x;
		pParent->rotationEuler.y = newRotation.y;
		pParent->rotationEuler.z = newRotation.z;
		SetVectorFromAngles(&pParent->rotationQuat, &pParent->rotationEuler.xyz);
	}
	return true;
}

bool CBWCinActor::SetHeadingQuat(float x, float y, float z, float w)
{
	CCinematic* pCinematic;
	edF32VECTOR4 rotationVector;
	edF32VECTOR3 newRotation;
	edF32MATRIX4 rotationMatrix;
	CActor* pParent;

	pCinematic = g_CinematicManager_0048efc->GetCurCinematic();
	rotationVector.x = x;
	rotationVector.y = y;
	rotationVector.z = z;
	rotationVector.w = w;
	edQuatToMatrix4Hard(&rotationVector, &rotationMatrix);

	edF32Matrix4MulF32Matrix4Hard(&rotationMatrix, &rotationMatrix, &pCinematic->matrix_0x120);
	edF32Matrix4ToEulerSoft(&rotationMatrix, &newRotation, "XYZ");

	pParent = this->pParent;
	pParent->rotationEuler.xyz = newRotation;

	SetVectorFromAngles(&pParent->rotationQuat, &pParent->rotationEuler.xyz);
	return true;
}

bool CBWCinActor::SetScale(float x, float y, float z)
{
	this->pParent->SetScale(x, y, z);
	return true;
}

bool CBWCinActor::SetAnim(edCinActorInterface::ANIM_PARAMStag* pTag)
{
	char cVar1;
	char cVar2;
	CAnimation* pAnimationController;
	edANM_HDR* pDstAnimation;
	edANM_HDR* pSrcAnimation;
	edAnmLayer* peVar5;
	bool bVar6;
	CCinematic* pCinematic;
	CCineActorConfig* pCVar7;
	float fVar8;
	float fVar9;
	float fVar10;
	float local_30[4];
	float local_20[4];
	float local_10[4];

	pCinematic = g_CinematicManager_0048efc->GetCurCinematic();
	pCVar7 = pCinematic->GetActorConfig(this->pParent);
	bVar6 = (this->field_0x8 & 2U) == 0;
	if ((pCVar7 == (CCineActorConfig*)0x0) || ((pCVar7->flags & 8) == 0)) {
		pAnimationController = this->pParent->pAnimationController;
		if (pAnimationController != (CAnimation*)0x0) {
			pAnimationController->anmBinMetaAnimator.SetLayerTimeWarper(0.0f, 0);
			pDstAnimation = (edANM_HDR*)LOAD_SECTION(pTag->dstAnim.pHdr);
			pSrcAnimation = (edANM_HDR*)LOAD_SECTION(pTag->srcAnim.pHdr);
			if ((pDstAnimation == 0) || (bVar6)) {
				peVar5 = pAnimationController->anmBinMetaAnimator.aAnimData;
				cVar1 = pTag->srcAnim.field_0x8;
				fVar8 = pTag->srcAnim.field_0x4;
				peVar5->animPlayState = STATE_ANIM_NONE;
				peVar5->SetRawAnim(pSrcAnimation, cVar1 != '\0', 0xfffffffe);
				edAnmStage::ComputeAnimParams(fVar8, (peVar5->currentAnimDesc).state.keyStartTime_0x14, 0.0f, local_10, 0, (uint)(((peVar5->currentAnimDesc).state.currentAnimDataFlags & 1) != 0));
				(peVar5->currentAnimDesc).state.time_0x10 = local_10[0];
				(peVar5->currentAnimDesc).state.time_0xc = local_10[1];
			}
			else {
				peVar5 = pAnimationController->anmBinMetaAnimator.aAnimData;
				cVar1 = pTag->dstAnim.field_0x8;
				fVar9 = pTag->field_0x18;
				cVar2 = pTag->srcAnim.field_0x8;
				fVar8 = pTag->srcAnim.field_0x4;
				fVar10 = pTag->dstAnim.field_0x4;
				peVar5->animPlayState = STATE_ANIM_NONE;
				peVar5->SetRawAnim(pDstAnimation, cVar1 != '\0', (uint)(pDstAnimation + 1));
				peVar5->SetRawAnim(pSrcAnimation, cVar2 != '\0', (uint)(pSrcAnimation + 2));
				peVar5->field_0xbc = 1.0;
				peVar5->MorphingStartDT();
				(peVar5->currentAnimDesc).morphDuration = 1.0f;
				(peVar5->nextAnimDesc).morphDuration = fVar9;
				edAnmStage::ComputeAnimParams(fVar10, (peVar5->currentAnimDesc).state.keyStartTime_0x14, 0.0f, local_20, 0,(uint)(((peVar5->currentAnimDesc).state.currentAnimDataFlags & 1) != 0));
				(peVar5->currentAnimDesc).state.time_0x10 = local_20[0];
				(peVar5->currentAnimDesc).state.time_0xc = local_20[1];
				edAnmStage::ComputeAnimParams(fVar8, (peVar5->nextAnimDesc).state.keyStartTime_0x14, 0.0f, local_30, 0, (uint)(((peVar5->nextAnimDesc).state.currentAnimDataFlags & 1) != 0));
				(peVar5->nextAnimDesc).state.time_0x10 = local_30[0];
				(peVar5->nextAnimDesc).state.time_0xc = local_30[1];
			}
		}
	}
	else {
		pParent->CinematicMode_SetAnimation(pTag, bVar6);
	}
	return true;
}

void FUN_001c6ab0(float param_1, ParticleInstance* param_2)
{
	param_2->field_0x4 = param_2->field_0x8;
	param_2->field_0x8 = param_1;
	return;
}

bool CBWCinActor::SetParticles(float param_1, edCinActorInterface::PARTICLE_PARAMStag* pTag)
{
	bool bNeedToCreateInstance;
	ParticleInstance* pPVar2;
	CCinematic* pCinematic;
	int particleInstanceId;

	if (param_1 < pTag->field_0x8) {
		pCinematic = g_CinematicManager_0048efc->GetCurCinematic();
		particleInstanceId = pCinematic->GetParticleInstance(pTag->particleId);
	
		bNeedToCreateInstance = particleInstanceId == -1;
		if (bNeedToCreateInstance) {
			particleInstanceId = pCinematic->CreateParticleInstance(param_1, pTag->field_0x4 + -1, pTag->particleId, this->pParent);
		}

		if ((-1 < particleInstanceId) && (particleInstanceId < pCinematic->nbInstalledParticles)) {
			pPVar2 = pCinematic->buffer_0x2e4;
			if (!bNeedToCreateInstance) {
				FUN_001c6ab0(param_1, pPVar2 + particleInstanceId);
			}

			pPVar2[particleInstanceId].field_0xc = 1;
		}
	}

	return true;
}

bool CBWCinActor::SetSubtitle(float param_1, edCinSourceSubtitleI::SUBTITLE_PARAMStag* pParams)
{
	char* pText;
	ulong key;
	uint uVar1;
	undefined4 uVar2;

	if (param_1 < pParams->time) {
		key = ByteCode::BuildU64(pParams->keyB, pParams->keyA);
		pText = gMessageManager.get_message(key);
		uVar1 = pParams->flags & 3;
		uVar2 = 0;
		if (uVar1 == 2) {
			uVar2 = 2;
		}
		else {
			if (uVar1 == 1) {
				uVar2 = 1;
			}
		}

		g_CinematicManager_0048efc->SetSubtitle(param_1 / pParams->time, pText, &this->pParent->currentLocation, uVar2);
	}

	return true;
}

bool CBWCinActor::Shutdown()
{
	//CSound* pCVar1;
	int newBehaviourId;
	bool bVar3;
	uint uVar4;
	CCinematic* pCinematic;
	CCineActorConfig* pCVar5;
	CBehaviourCinematic* pCVar6;
	CBehaviour* pCVar7;

	if ((this->pParent->flags & 0x800000) != 0) {
		IMPLEMENTATION_GUARD_AUDIO(
		bVar3 = edSoundInstanceIsAlive(&this->soundInternalStruct);
		if (bVar3 != false) {
			if (DAT_00448ef0 == 0) {
				pCVar1 = (this->soundInternalStruct).SoundStructPtr;
				if (pCVar1 != (CSound*)0x0) {
					uVar4 = (**(code**)(pCVar1->vt + 0xc))(pCVar1, (this->soundInternalStruct).SoundID);
					(this->soundInternalStruct).SoundID = uVar4;
				}
			}
			CAudioManager::ReleaseSound3DData
			((int)CScene::ptable.g_AudioManager_00451698, (long)(this->soundInternalStruct).field_0x14);
		}
		if (this->field_0xc0 != 0) {
			if (this->pParent != (CActor*)0x0) {
				CActor::SV_ACT_LipsyncTerm(this->pParent);
			}
			this->field_0xc0 = 0;
		});

		pCinematic = g_CinematicManager_0048efc->GetCurCinematic();
		pCVar5 = pCinematic->GetActorConfig(this->pParent);
		pCVar6 = (CBehaviourCinematic*)this->pParent->GetBehaviour(1);
		if ((((pCVar5 == (CCineActorConfig*)0x0) || ((pCVar5->flags & 0x40) == 0)) ||
			(pCVar6->leaveCinematicBehaviourId == -1)) ||
			(pCVar7 = this->pParent->GetBehaviour(pCVar6->leaveCinematicBehaviourId), pCVar7 == (CBehaviour*)0x0)) {
			newBehaviourId = (this->pParent->subObjA)->defaultBehaviourId;
		}
		else {
			newBehaviourId = pCVar6->leaveCinematicBehaviourId;
		}
		this->pParent->CinematicMode_Leave(newBehaviourId);
	}
	return true;
}

bool CBWCinActor::SetVisibility(bool bVisible)
{
	CActor* pActor;

	if (bVisible == false) {
		pActor = this->pParent;
		pActor->flags = pActor->flags & 0xffffff7f;
		pActor->flags = pActor->flags | 0x20;
		pActor->EvaluateDisplayState();
	}
	else {
		pActor = this->pParent;
		pActor->flags = pActor->flags | 0x80;
		pActor->flags = pActor->flags & 0xffffffdf;
		pActor->EvaluateDisplayState();
	}
	return true;
}

void CBWCinActor::SetupTransform(edF32VECTOR4* position, edF32VECTOR4* heading, edF32VECTOR4* scale, ed_g3d_manager* pMeshManager)
{
	float z;
	float fVar1;
	float w;
	float fVar2;
	float y;
	float fVar3;

	this->position = *position;
	this->heading = *heading;
	this->scale = *scale;
	this->pAltModelManager = pMeshManager;
	return;
}

CCineActorConfig::CCineActorConfig()
{
	this->flags = 0;
}

void S_STREAM_EVENT_CAMERA::Init()
{
	ed_zone_3d* pEVar1;
	CActor* pCVar2;

	this->field_0x8 = this->field_0x8 * this->field_0x8;
	this->flags = (uint)((ulong)((long)this->flags << 0x23) >> 0x23);
	this->field_0x1c = 0;

	if (this->pActor == -1) {
		pCVar2 = (CActor*)0x0;
	}
	else {
		pCVar2 = (CScene::ptable.g_ActorManager_004516a4)->aActors[(int)this->pActor];
	}

	this->pActor = STORE_SECTION(pCVar2);

	pEVar1 = (ed_zone_3d*)0x0;
	if (this->pZone != -1) {
		pEVar1 = edEventGetChunkZone((CScene::ptable.g_EventManager_006f5080)->activeChunkId, this->pZone);
	}
	this->pZone = STORE_SECTION(pEVar1);

	return;
}

void S_STREAM_EVENT_CAMERA::Manage(CActor* pActor)
{
	CActor* puVar1;
	float fVar3;
	float fVar4;
	float fVar5;
	bool bVar6;
	Timer* pTVar7;
	uint uVar8;
	uint uVar9;
	float fVar10;
	ed_zone_3d* pEVar2;

	if ((this->flags & 0x40000000U) != 0) {
		pTVar7 = GetTimer();
		fVar10 = pTVar7->scaledTotalTime - this->field_0x1c;
		if (this->field_0xc <= fVar10) {
			bVar6 = false;
			if (((this->flags & 1U) != 0) &&
				((0.4f <= gPlayerInput.aAnalogSticks[PAD_STICK_LEFT].magnitude || ((gPlayerInput.pressedBitfield & 0x7f0) != 0)))) {
				bVar6 = true;
			}

			if ((this->field_0x8 != 0.0f) &&
				(fVar3 = CActorHero::_gThis->currentLocation.x - pActor->currentLocation.x,
					fVar4 = CActorHero::_gThis->currentLocation.y - pActor->currentLocation.y,
					fVar5 = CActorHero::_gThis->currentLocation.z - pActor->currentLocation.z,
					this->field_0x8 <= fVar3 * fVar3 + fVar4 * fVar4 + fVar5 * fVar5)) {
				bVar6 = true;
			}

			if (this->field_0x10 <= fVar10) {
				bVar6 = true;
			}

			puVar1 = LOAD_SECTION_CAST(CActor*, this->pActor);
			if ((puVar1 != (CActor*)0x0) && (pEVar2 = LOAD_SECTION_CAST(ed_zone_3d*, this->pZone), pEVar2 != (ed_zone_3d*)0x0)) {
				uVar9 = this->flags & 0x20000000;
				this->flags = this->flags & 0xdfffffff;
				if ((puVar1 != (CActor*)0x0) && ((pEVar2 != (ed_zone_3d*)0x0 &&
						(uVar8 = edEventComputeZoneAgainstVertex((CScene::ptable.g_EventManager_006f5080)->activeChunkId, pEVar2, &puVar1->currentLocation, 0), (uVar8 & 1) != 0)))) {
					this->flags = this->flags | 0x20000000;
				}

				uVar8 = this->flags;
				if ((uVar8 & 0x20000000) == 0) {
					if ((uVar8 & 0x20) != 0) {
						bVar6 = true;
					}

					if ((uVar9 != 0) && ((this->flags & 8U) != 0)) {
						bVar6 = true;
					}
				}
				else {
					if ((uVar8 & 0x10) != 0) {
						bVar6 = true;
					}

					if ((uVar9 == 0) && ((this->flags & 4U) != 0)) {
						bVar6 = true;
					}
				}
			}

			if ((bVar6) && ((this->flags & 0x40000000U) != 0)) {
				if ((this->cameraIndex != -1)) {
					CCameraManager::_gThis->PopCamera(this->cameraIndex);
					this->flags = this->flags & 0xbfffffff;
				}

				if (((this->flags & 0x40U) != 0) && (CActorHero::_gThis != (CActorHero*)0x0)) {
					if (pActor == (CActor*)0x0) {
						pActor = (CActor*)CActorHero::_gThis;
					}

					pActor->DoMessage(CActorHero::_gThis, MESSAGE_ENABLE_INPUT, (GetPositionMsgParams*)0x0);
				}
			}
		}
	}

	return;
}

void S_STREAM_EVENT_CAMERA::SwitchOn(CActor* pActor)
{
	int iVar1;
	uint uVar3;

	uVar3 = this->flags;
	if (((((uVar3 & 2) == 0) || ((uVar3 & 0x80000000) == 0)) && ((uVar3 & 0x40000000) == 0)) &&
		((this->cameraIndex != -1 && (iVar1 = CCameraManager::_gThis->PushCamera(this->cameraIndex, 0), iVar1 != 0)))) {
		this->flags = this->flags | 0xc0000000;

		if (((this->flags & 0x40U) != 0) && (CActorHero::_gThis != (CActorHero*)0x0)) {
			if (pActor == (CActor*)0x0) {
				pActor = (CActor*)CActorHero::_gThis;
			}
			pActor->DoMessage(CActorHero::_gThis, MESSAGE_DISABLE_INPUT, 0);
		}

		this->field_0x1c = GetTimer()->scaledTotalTime;
		this->flags = this->flags & 0xdfffffff;

		if (((this->pActor != 0x0) && (this->pZone != 0x0)) &&
			(uVar3 = edEventComputeZoneAgainstVertex((CScene::ptable.g_EventManager_006f5080)->activeChunkId, LOAD_SECTION_CAST(ed_zone_3d*, this->pZone),
				&LOAD_SECTION_CAST(CActor*, this->pActor)->currentLocation, 0), (uVar3 & 1) != 0)) {
			this->flags = this->flags | 0x20000000;
		}
	}

	return;
}


void S_STREAM_EVENT_CAMERA::SwitchOff(CActor* pActor)

{
	if ((this->flags & 0x40000000U) != 0) {
		if (this->cameraIndex != -1) {
			CCameraManager::_gThis->PopCamera(this->cameraIndex);
			this->flags = this->flags & 0xbfffffff;
		}

		if (((this->flags & 0x40U) != 0) && (CActorHero::_gThis != (CActorHero*)0x0)) {
			if (pActor == (CActor*)0x0) {
				pActor = (CActor*)CActorHero::_gThis;
			}

			pActor->DoMessage(CActorHero::_gThis, MESSAGE_ENABLE_INPUT, 0);
		}
	}

	return;
}



void S_STREAM_EVENT_CAMERA::Reset(CActor* pActor)
{
	if ((this->flags & 0x40000000U) != 0) {
		if (this->cameraIndex != -1) {
			CCameraManager::_gThis->PopCamera(this->cameraIndex);
			this->flags = this->flags & 0xbfffffff;
		}

		if (((this->flags & 0x40U) != 0) && (CActorHero::_gThis != (CActorHero*)0x0)) {
			if (pActor == (CActor*)0x0) {
				pActor = (CActor*)CActorHero::_gThis;
			}

			pActor->DoMessage(CActorHero::_gThis, MESSAGE_ENABLE_INPUT, 0);
		}
	}

	this->flags = (uint)((ulong)((ulong)this->flags << 0x23) >> 0x23);
	this->field_0x1c = 0;
	return;
}

void S_STREAM_EVENT_CAMERA::SaveContext(uint* pData)
{
	if ((this->flags & 0x80000000) == 0) {
		*pData = 0;
	}
	else {
		*pData = 1;
	}

	return;
}

bool CCinematicManagerB::LevelLoading_Manage()
{
	CCinematicManager* pCVar1;

	pCVar1 = g_CinematicManager_0048efc;
	if (g_CinematicManager_0048efc->pCinematic != (CCinematic*)0x0) {
		g_CinematicManager_0048efc->pCinematic->Level_ClearAll();
		pCVar1->pCinematic->FUN_001c7390(true);
	}

	return false;
}


void CCinematicManagerB::Level_Manage()
{
	CCinematic* pCinematic;
	bool bVar3;

	if (g_CinematicManager_0048efc->activeCinematicCount > 0) {
		bVar3 = true;

		for (int i = 0; i < g_CinematicManager_0048efc->activeCinematicCount; i++) {
			pCinematic = g_CinematicManager_0048efc->ppCinematicObjB_B[i];
			pCinematic->Level_ClearAll();
			if ((pCinematic->flags_0x8 & 0x2000) != 0) {
				bVar3 = false;
			}
		}
	}

	for (int i = 0; i < g_CinematicManager_0048efc->activeCinematicCount; i++) {
		pCinematic = g_CinematicManager_0048efc->ppCinematicObjB_B[i];
		pCinematic->FUN_001c7390(bVar3);
	}

	return;
}

void CCinematicManagerB::Level_ManagePaused()
{
	CCinematic* pCinematic;
	bool bVar3;

	if ((GameFlags & 0x200) == 0) {
		if (g_CinematicManager_0048efc->activeCinematicCount > 0) {
			bVar3 = true;

			for (int i = 0; i < g_CinematicManager_0048efc->activeCinematicCount; i++) {
				pCinematic = g_CinematicManager_0048efc->ppCinematicObjB_B[i];

				IMPLEMENTATION_GUARD(
					pCinematic->Level_ClearAll();)
					if ((pCinematic->flags_0x8 & 0x2000) != 0) {
						bVar3 = false;
					}
			}
		}

		for (int i = 0; i < g_CinematicManager_0048efc->activeCinematicCount; i++) {
			pCinematic = g_CinematicManager_0048efc->ppCinematicObjB_B[i];
			IMPLEMENTATION_GUARD(
				pCinematic->FUN_001c7390(bVar3);)
		}
	}

	return;
}

void CCinematicManagerB::Level_PreReset()
{
	CCinematicManager* pCutsceneManager;
	CCinematic** ppCinematic;
	CCinematic* pCinematic;
	int i;

	pCutsceneManager = g_CinematicManager_0048efc;
	pCutsceneManager->field_0x34 = 0;
	pCutsceneManager->pSubtitleText = (char*)0x0;
	pCutsceneManager->field_0x40 = 0.0f;
	pCutsceneManager->fadeDuration = 0.0f;
	ppCinematic = pCutsceneManager->ppCinematicObjB_A;

	for (i = pCutsceneManager->numCutscenes_0x8; i != 0; i = i + -1) {
		pCinematic = *ppCinematic;
		IMPLEMENTATION_GUARD(
		pCinematic->PreReset();)
		ppCinematic = ppCinematic + 1;
	}
	return;
}

void CCinematicManagerB::Level_PreCheckpointReset()
{
	CCinematicManager* pCutsceneManager;
	CCinematic** ppCinematic;
	CCinematic* pCinematic;
	int i;

	pCutsceneManager = g_CinematicManager_0048efc;
	pCutsceneManager->field_0x34 = 0;
	pCutsceneManager->pSubtitleText = (char*)0x0;
	pCutsceneManager->field_0x40 = 0.0f;
	pCutsceneManager->fadeDuration = 0.0f;
	ppCinematic = pCutsceneManager->ppCinematicObjB_A;

	for (i = pCutsceneManager->numCutscenes_0x8; i != 0; i = i + -1) {
		pCinematic = *ppCinematic;
		if ((pCinematic->flags_0x4 & 20) == 0)
			pCinematic->PreReset();
			ppCinematic = ppCinematic + 1;
	}

	return;
}

int CCinematicManager::GetNumCutscenes_001c50b0()
{
	return this->numCutscenes_0x8;
}

struct SaveDataChunk_BLCI
{
	int nbCutscenes;

	struct CutsceneData
	{
		uint bitFlags;
		uint flags;
		float totalCutsceneDelta;
	} data[];
};

void CCinematicManager::Level_SaveContext()
{
	SaveDataChunk_BLCI* piVar1;
	CCinematic** ppCinematic;
	int iVar2;
	int iVar3;
	CCinematic* pCinematic;
	CLevelScheduler* pLevelScheduler;

	pLevelScheduler = CLevelScheduler::gThis;
	ppCinematic = this->ppCinematicObjB_A;
	iVar3 = 0;
	for (iVar2 = this->numCutscenes_0x8; iVar2 != 0; iVar2 = iVar2 + -1) {
		if (((*ppCinematic)->flags_0x4 & 0x40) != 0) {
			iVar3 = iVar3 + 1;
		}
		ppCinematic = ppCinematic + 1;
	}
	if (iVar3 != 0) {
		piVar1 = reinterpret_cast<SaveDataChunk_BLCI*>(CLevelScheduler::gThis->SaveGame_BeginChunk(SAVEGAME_CHUNK_BLCI));
		piVar1->nbCutscenes = iVar3;
		SaveDataChunk_BLCI::CutsceneData* pData = reinterpret_cast<SaveDataChunk_BLCI::CutsceneData*>(piVar1 + 1);
		ppCinematic = this->ppCinematicObjB_A;
		for (iVar2 = this->numCutscenes_0x8; iVar2 != 0; iVar2 = iVar2 + -1) {
			if (((*ppCinematic)->flags_0x4 & 0x40) != 0) {
				pData->bitFlags = *reinterpret_cast<uint*>(&(*ppCinematic)->field_0x10);
				pCinematic = *ppCinematic;
				pData->flags = 0;
				pData->totalCutsceneDelta = pCinematic->totalCutsceneDelta;
				if ((pCinematic->flags_0x8 & 0x400) != 0) {
					pData->flags = pData->flags | 2;
				}

				if (pCinematic->state != CS_Stopped) {
					pData->flags = pData->flags | 1;
				}

				if ((pCinematic->flags_0x8 & 0x20) != 0) {
					pData->flags = pData->flags | 4;
				}

				if ((pCinematic->flags_0x8 & 0x40) != 0) {
					pData->flags = pData->flags | 8;
				}

				if ((pCinematic->flags_0x8 & 0x1000) != 0) {
					pData->flags = pData->flags | 0x10;
				}

				pData = pData + 1;
			}
			ppCinematic = ppCinematic + 1;
		}

		pLevelScheduler->SaveGame_EndChunk(pData);
	}

	return;
}

void CCinematicManager::Level_LoadContext()
{
	IMPLEMENTATION_GUARD();
}

void CCinematicManager::LevelLoading_Draw()
{
	int iVar1;

	if (this->pCinematic != (CCinematic*)0x0) {
		this->pCinematic->Draw();

		iVar1 = 0;
		if ((this->pCinematic->state != CS_Stopped) && ((this->pCinematic->flags_0x4 & 0x200) != 0)) {
			iVar1 = 1;
		}

		DrawBandsAndSubtitle(iVar1);
	}

	return;
}

bool CCinematicManager::IsCutsceneActive()
{
	bool bCutsecenActive;
	CCinematic** ppCVar1;
	int iVar2;

	bCutsecenActive = false;

	iVar2 = 0;
	if (0 < this->activeCinematicCount) {
		ppCVar1 = this->ppCinematicObjB_B;
		bCutsecenActive = false;
		do {
			if (((*ppCVar1)->state != CS_Stopped) && (((*ppCVar1)->flags_0x4 & 0x200) != 0)) {
				bCutsecenActive = true;
			}

			iVar2 = iVar2 + 1;
			ppCVar1 = ppCVar1 + 1;
		} while (iVar2 < this->activeCinematicCount);
	}

	return bCutsecenActive;
}

CCinematic* CCinematicManager::GetCinematic(int index)
{
	CCinematic* pCinematic;

	if ((index < 0) || (this->numCutscenes_0x8 <= index)) {
		pCinematic = (CCinematic*)0x0;
	}
	else {
		pCinematic = this->ppCinematicObjB_A[index];
	}
	return pCinematic;
}

void CCinematicManager::StopAllCutscenes()
{
	bool bVar1;
	CCinematic** ppCVar2;
	int iVar3;
	CCinematic* pCinematic;

	iVar3 = this->numCutscenes_0x8;
	if (iVar3 != 0) {
		ppCVar2 = this->ppCinematicObjB_A + iVar3;
		for (; iVar3 != 0; iVar3 = iVar3 + -1) {
			ppCVar2 = ppCVar2 + -1;
			pCinematic = *ppCVar2;

			bVar1 = (pCinematic)->pCineBankEntry != (edCBankBufferEntry*)0x0;
			if (bVar1) {
				bVar1 = pCinematic->cineBankLoadStage_0x2b4 != 4;
			}

			if (bVar1) {
				pCinematic->Load(0);
			}

			if ((this->pCinematic != pCinematic) && (pCinematic->state != CS_Stopped)) {
				pCinematic->Stop();
			}
		}
	}
	return;
}

bool CCinematicManager::PlayOutroCinematic(int index, CActor* param_3)
{
	bool bVar1;
	CActor* in_a2_lo;
	int iVar2;
	int iVar3;
	CCinematic* pCinematic;

	if ((index < 0) || (this->numCutscenes_0x8 <= index)) {
		pCinematic = (CCinematic*)0x0;
	}
	else {
		pCinematic = this->ppCinematicObjB_A[index];
	}

	if (pCinematic != (CCinematic*)0x0) {
		if (pCinematic->state == CS_Stopped) {
			bVar1 = (pCinematic->flags_0x4 & 1) != 0;
			if (bVar1) {
				bVar1 = (pCinematic->flags_0x8 & 0x400) != 0;
			}

			if (!bVar1) {
				bVar1 = (pCinematic->flags_0x8 & 0x28) != 0;
			}

			bVar1 = (bool)(bVar1 ^ 1);
			if (!bVar1) {
				bVar1 = (pCinematic->flags_0x8 & 0x800) != 0;
			}

			if (bVar1) {
				pCinematic->pActor = param_3;
				if ((pCinematic->flags_0x8 & 0x800) == 0) {
					bVar1 = false;
					if ((pCinematic->pSwitchListA != (S_NTF_TARGET_STREAM_REF*)0x0) &&
						(pCinematic->pStreamEventCameraA != (S_STREAM_EVENT_CAMERA*)0x0)) {
						bVar1 = true;
					}

					if (bVar1) {
						iVar3 = 0;
						pCinematic->pSwitchListA->Switch((CActor*)0x0);
						pCinematic->pStreamEventCameraA->SwitchOn((CActor*)0x0);
						pCinematic->pSwitchListA->PostSwitch((CActor*)0x0);
					}
				}

				pCinematic->Start();
			}
		}

		if (pCinematic->state != CS_Stopped) {
			this->pCinematic = pCinematic;
		}
	}

	return this->pCinematic != (CCinematic*)0x0;
}

void CCinematicManager::Level_ManagePaused()
{
	if ((GameFlags & 0x200) == 0) {
		Level_Manage();
	}

	return;
}

bool CCinematicManager::FUN_001c5c60()
{
	bool bVar1;
	CCinematic** ppCVar2;
	int iVar3;

	bVar1 = false;
	iVar3 = 0;
	if (0 < this->activeCinematicCount) {
		ppCVar2 = this->ppCinematicObjB_B;
		do {
			if (((*ppCVar2)->state != CS_Stopped) && (((*ppCVar2)->flags_0x4 & 0x200) != 0)) {
				bVar1 = true;
			}
			iVar3 = iVar3 + 1;
			ppCVar2 = ppCVar2 + 1;
		} while (iVar3 < this->activeCinematicCount);
	}

	if ((!bVar1) && (bVar1 = true, this->field_0x34 == 0.0f)) {
		bVar1 = false;
	}

	if ((!bVar1) && (bVar1 = this->pSubtitleText != (char*)0x0, bVar1)) {
		bVar1 = this->field_0x44 == 0;
	}

	return bVar1;
}

extern EFileLoadMode VERSION;

void CCinematicManager::DrawBandsAndSubtitle(int param_2)
{
	edF32VECTOR4* peVar1;
	bool bVar2;
	byte bVar3;
	edCTextStyle* pNewFont;
	int iVar4;
	edF32VECTOR4* peVar5;
	edF32VECTOR4* peVar7;
	uint uVar8;
	undefined8 uVar9;
	byte b;
	int iVar10;
	int iVar11;
	byte r;
	byte g;
	float fVar12;
	float fVar13;
	float fVar14;
	Rectangle local_15f0;
	edCTextStyle textStyle;
	edF32VECTOR2 screenCoords;

	if ((GameFlags & 0x1c) == 0) {
		if (GuiDList_BeginCurrent()) {
			textStyle.Reset();

			pNewFont = edTextStyleSetCurrent(&textStyle);
			textStyle.SetFont(BootDataFont, false);

			if ((this->pSubtitleText != (char*)0x0) && (this->field_0x44 != 0)) {
				textStyle.SetHorizontalAlignment(this->subtitleHorizontalAlignment);
				textStyle.SetVerticalAlignment(8);

				ed3DComputeSceneCoordinate(&screenCoords, &this->vector_0x50, CScene::_scene_handleA);

				screenCoords.x = (screenCoords.x * 0.5f + 0.5f) * (float)gVideoConfig.screenWidth;
				screenCoords.y = (0.5f - screenCoords.y * 0.5f) * (float)gVideoConfig.screenHeight;
				edTextDraw(screenCoords.x, screenCoords.y, this->pSubtitleText);
			}

			if ((CCameraManager::_gThis->aspectRatio != 1.777778f) && ((param_2 != 0 || (this->field_0x34 != 0.0f)))) {
				fVar13 = (float)gVideoConfig.screenHeight * 0.25f * 0.5f;
				fVar12 = fVar13 * GetTimer()->cutsceneDeltaTime * 2.0f;
				if (param_2 == 0) {
					fVar12 = this->field_0x34 - fVar12;
					this->field_0x34 = fVar12;
					if (fVar12 < 0.0f) {
						this->field_0x34 = 0.0f;
					}
				}
				else {
					fVar12 = this->field_0x34 + fVar12;
					this->field_0x34 = fVar12;
					if (fVar13 < fVar12) {
						this->field_0x34 = fVar13;
					}
				}

				if (0.0f < this->field_0x34) {
					edDListBlendSet(0);
					edDListLoadIdentity();
					edDListUseMaterial((edDList_material*)0x0);
					edDListColor4u8(0, 0, 0, 0x80);

					iVar4 = gVideoConfig.screenWidth;
					if (gVideoConfig.screenWidth < 0) {
						iVar4 = gVideoConfig.screenWidth + 0x1f;
					}

					fVar12 = this->field_0x34;
					fVar13 = 0.0f;
					iVar11 = 2;
					do {
						edDListBegin(0.0f, 0.0f, 0.0f, 4, ((iVar4 >> 5) + 1) * 2);
						fVar14 = 0.0f;
						for (iVar10 = iVar4 >> 5; iVar10 != 0; iVar10 = iVar10 + -1) {
							edDListVertex4f(fVar14, fVar13, 0.0f, 0.0f);
							edDListVertex4f(fVar14, fVar12, 0.0f, 0.0f);
							fVar14 = fVar14 + 32.0f;
						}

						fVar14 = (float)gVideoConfig.screenWidth;
						edDListVertex4f(fVar14, fVar13, 0.0f, 0.0f);
						edDListVertex4f(fVar14, fVar12, 0.0f, 0.0f);
						edDListEnd();

						iVar11 = iVar11 + -1;
						fVar12 = (float)gVideoConfig.screenHeight;
						fVar13 = fVar12 - this->field_0x34;
					} while (0 < iVar11);
				}
			}

			if (((this->pSubtitleText != (char*)0x0) && (this->field_0x44 == 0)) && (this->bInitialized != 0)) {
				fVar14 = (float)gVideoConfig.screenWidth - 40.0f;
				local_15f0.width = fVar14 - 40.0f;
				textStyle.SetShadow(0x100);
				textStyle.SetHorizontalAlignment(this->subtitleHorizontalAlignment);
				textStyle.SetVerticalAlignment(0);
				textStyle.SetEolAutomatic(0x80);
				textStyle.SetHorizontalJustification(0x10);
				textStyle.SetHorizontalSize(local_15f0.width);
				textStyle.SetScale(1.0f, 1.0f);
				textStyle.field_0x8c = textStyle.field_0x8c * 1.0f;

				edCTextFormat textFormat;
				textFormat.FormatString(this->pSubtitleText);

				fVar12 = (float)(uint)BootDataFont->field_0x16 + textStyle.field_0x8c;
				local_15f0.y = (float)(int)(this->field_0x40 * (float)(int)(textFormat.field_0xc / fVar12)) * fVar12;
				fVar13 = ((float)gVideoConfig.screenHeight - (float)gVideoConfig.screenHeight * 0.25f * 0.5f) + 12.0f;
				local_15f0.height = ((fVar13 + fVar12 * 1.0f) - fVar13) / 1.0f;
				fVar13 = fVar13 - local_15f0.y * 1.0f;

				//local_15f0 = {};

				iVar4 = this->subtitleHorizontalAlignment;
				if (iVar4 == 2) {
					textFormat.DisplayWindow((fVar14 + 40.0f) * 0.5f, fVar13, &local_15f0);
				}
				else {
					if (iVar4 == 1) {
						textFormat.DisplayWindow(fVar14, fVar13, &local_15f0);
					}
					else {
						if (iVar4 == 0) {
							textFormat.DisplayWindow(40.0f, fVar13, &local_15f0);
						}
					}
				}
			}

			edTextStyleSetCurrent(pNewFont);
			GuiDList_EndCurrent();
		}

		if (edVideoIsFadeActive() == 0) {
			if (this->fadeDuration != 0.0f) {
				r = this->fadeColor.x * 128.0f;
				g = this->fadeColor.y * 128.0f;
				b = this->fadeColor.z * 128.0f;
				edVideoSetFadeColor(r, g, b);
			}

			edVideoSetFade(this->fadeDuration);
		}

		// If we haven't started playing the cutscene in 15 seconds, display an error message.
		if ((VERSION != FLM_CD_DVD) && (this->field_0x28 != (CCinematic*)0x0)) {
			if ((Timer::GetTimer()->totalTime - this->startTime) < CUTSCENE_MAX_LOAD_TIME) {
				bVar2 = GuiDList_BeginCurrent();
				if (bVar2 != false) {
					IMPLEMENTATION_GUARD(
					edDebugMenu.DrawDebugTextLocationX_00468f34 = 0x14;
					edDebugMenu.DrawDebugTextLocationY_00468f38 = 0xdc;
					edDebugSetFontColor(7);
					edDebugPrintText("Cinematic %s\ncould not be loaded", this->field_0x28->fileName);
					GuiDList_EndCurrent();)
				}
			}
			else {
				this->field_0x28 = (CCinematic*)0x0;
				this->startTime = 0.0f;
			}
		}
	}

	return;
}

void CCinematicManager::SetSubtitle(float param_1, char* pText, edF32VECTOR4* param_4, int param_5)
{
	this->pSubtitleText = pText;
	this->subtitleHorizontalAlignment = param_5;
	this->field_0x40 = param_1;

	if (param_4 == (edF32VECTOR4*)0x0) {
		this->field_0x44 = 0;
	}
	else {
		this->vector_0x50 = *param_4;
		this->field_0x44 = 1;
	}

	return;
}

void CCinematicManager::Level_PauseChange(bool bPaused)
{
	int iVar1;
	int iVar2;

	if (this->activeCinematicCount == 0) {
		if (this->pCinematic != (CCinematic*)0x0) {
			IMPLEMENTATION_GUARD(
			PauseCutscene_001cac20(this->pCinematic, (long)bPaused);)
		}
	}
	else {
		iVar1 = 0;
		if (0 < this->activeCinematicCount) {
			iVar2 = 0;
			do {
				IMPLEMENTATION_GUARD_LOG(
				PauseCutscene_001cac20(*(CCinematic**)((int)this->ppCinematicObjB_B + iVar2), (long)bPaused);)
				iVar1 = iVar1 + 1;
				iVar2 = iVar2 + 4;
			} while (iVar1 < this->activeCinematicCount);
		}
	}

	return;
}

void CCinematicManager::Level_ClearAll()
{
	int iVar3;

	if (this->ppCinematicObjB_A != (CCinematic**)0x0) {
		iVar3 = 0;
		if (0 < this->numCutscenes_0x8) {
			do {
				CCinematic** pCin = this->ppCinematicObjB_A + iVar3;
				if (this->pCinematic != *pCin) {
					delete (*pCin);
				}
				iVar3 = iVar3 + 1;
			} while (iVar3 < this->numCutscenes_0x8);
		}

		delete this->ppCinematicObjB_A;
		delete this->ppCinematicObjB_B;
	}

	this->ppCinematicObjB_A = (CCinematic**)0x0;
	this->numCutscenes_0x8 = 0;
	this->ppCinematicObjB_B = (CCinematic**)0x0;
	this->activeCinematicCount = 0;
	this->pCurCinematic = (CCinematic*)0x0;
	this->field_0x28 = (CCinematic*)0x0;
	this->startTime = 0.0f;

	return;
}

void CCinematicManager::Level_Term()
{
	float fVar1;
	CCinematic* this_00;
	bool bVar2;
	CCinematicManager* pCVar3;
	int iVar4;
	edF32MATRIX4* peVar5;
	edF32MATRIX4* peVar6;
	CCinematic** ppCVar7;

	this->matrix_0x70 = this->pCinematicCamera->transformationMatrix;
	this->field_0xb0 = this->pCinematicCamera->fov;

	iVar4 = this->numCutscenes_0x8;
	if (iVar4 != 0) {
		ppCVar7 = this->ppCinematicObjB_A + iVar4;
		pCVar3 = g_CinematicManager_0048efc;
		for (; g_CinematicManager_0048efc = pCVar3, iVar4 != 0; iVar4 = iVar4 + -1) {
			ppCVar7 = ppCVar7 + -1;
			this_00 = *ppCVar7;

			if (this->pCinematic == this_00) {
				this_00->FUN_001caeb0();
			}
			else {
				bVar2 = (this_00->cinematicLoadObject).BWCinSourceAudio_Obj.intFieldA != 0;
				if ((bVar2) && (bVar2 = true, (this_00->cinematicLoadObject).BWCinSourceAudio_Obj.field_0x8 != 0.0)) {
					bVar2 = false;
				}

				if (bVar2) {
					pCVar3->pCurCinematic = this_00;
					this_00->cinematicLoadObject.BWCinSourceAudio_Obj.Stop();
					pCVar3->pCurCinematic = (CCinematic*)0x0;
				}

				bVar2 = (this_00)->pCineBankEntry != (edCBankBufferEntry*)0x0;
				if (bVar2) {
					bVar2 = this_00->cineBankLoadStage_0x2b4 != 4;
				}

				if (bVar2) {
					this_00->Load(0);
				}

				if (this_00->state != CS_Stopped) {
					this_00->Stop();
				}

				this_00->flags_0x8 = this_00->flags_0x8 & 0xfffffffb;
				this_00->pActor = (CActor*)0x0;
			}

			pCVar3 = g_CinematicManager_0048efc;
		}
	}

	this->pCinematicCamera = (CCameraCinematic*)0x0;

	return;
}

void CCinematicManager::Level_PreCheckpointReset()
{
	return;
}

void CCinematicManager::Level_CheckpointReset()
{
	return;
}

CBWCinSourceAudio::CBWCinSourceAudio()
{
	this->intFieldA = 0;
	this->field_0x8 = 0.0f;
	this->floatFieldA = 0.0f;
	this->field_0x38 = 0;
	this->field_0x39 = 0;
	this->pGlobalSoundFileData = (GlobalSound_FileData*)0x0;
	this->lastIntField = &this->field_0x14;
}

bool CBWCinSourceAudio::Create(char* pFileName)
{
	CUTSCENE_LOG(LogLevel::Info, "CBWCinSourceAudio::Create {}", pFileName);
	return true;
}

bool CBWCinSourceAudio::Play()
{
	bool bVar1;
	Timer* pTVar2;
	ulong uVar3;
	float fVar4;

	if (g_CinematicManager_0048efc->pCurCinematic->state == CS_Interpolate) {
		bVar1 = false;
	}
	else {
		this->field_0x38 = 0;
		this->field_0x39 = 1;
		if (this->intFieldA == 0) {
			bVar1 = false;
		}
		else {
			IMPLEMENTATION_GUARD_AUDIO(
			uVar3 = FUN_00284650(this->intFieldA);
			bVar1 = uVar3 != 0;)
		}

		if (((bVar1) && (this->intFieldA != 0)) && (this->field_0x8 == 0.0f)) {
			IMPLEMENTATION_GUARD_AUDIO(
			FUN_00283650(this->intFieldA);
			edSoundFlush();
			pTVar2 = Timer::GetTimer();
			fVar4 = pTVar2->cutsceneDeltaTime;
			pTVar2 = Timer::GetTimer();
			this->field_0x8 = pTVar2->totalPlayTime - fVar4;)
		}

		bVar1 = true;
	}

	return bVar1;
}

bool CBWCinSourceAudio::Stop()
{
	if (this->intFieldA != 0) {
		IMPLEMENTATION_GUARD_AUDIO(
		edSoundInstanceStop(this->intFieldA);
		this->intFieldA = 0;
		edSoundStreamFree(this->lastIntField);
		if (this->field_0x38 == 0) {
			FUN_00401f10((int)&StaticEdFileBase_004497f0);
		})
	}

	this->field_0x8 = 0.0f;
	this->floatFieldA = 0.0f;
	this->field_0x39 = 0;
	this->field_0x38 = 0;

	return true;
}

bool CBWCinSourceAudio::Destroy()
{
	return true;
}

void CBWCinSourceAudio::SetAudioTrack(int audioTrackId)
{
	IMPLEMENTATION_GUARD_AUDIO();
}

bool CBWCinSourceSubtitle::Create(char* pFileName, long param_3)
{
	int iStack4;

	g_CinematicManager_0048efc->pCurCinematic->InstallResource(edResCollection::COT_Text, param_3, pFileName, (ed_g2d_manager*)0x0, &iStack4);
	this->position = gF32Vertex4Zero;

	return true;
}

bool CBWCinSourceSubtitle::SetPos(float x, float y, float z)
{
	bool bVar1 = g_CinematicManager_0048efc->pCurCinematic->state != CS_Interpolate;

	if (bVar1) {
		(this->position).x = x;
		(this->position).y = y;
		(this->position).z = z;
	}

	return bVar1;
}

bool CBWCinSourceSubtitle::SetSubtitle(float keyTime, SUBTITLE_PARAMStag* pTag)
{
	CCinematicManager* pCVar1;
	bool bSuccess;
	char* pcVar3;
	ulong key;
	uint uVar4;
	int iVar5;
	float fVar6;
	float fVar7;
	float fVar8;

	if (g_CinematicManager_0048efc->pCurCinematic->state == CS_Interpolate) {
		bSuccess = false;
	}
	else {
		if (keyTime < pTag->time) {
			key = ByteCode::BuildU64(pTag->keyB, pTag->keyA);
			pcVar3 = gMessageManager.get_message(key);
			pCVar1 = g_CinematicManager_0048efc;
			uVar4 = pTag->flags & 3;
			iVar5 = 0;
			if (uVar4 == 2) {
				iVar5 = 2;
			}
			else {
				if (uVar4 == 1) {
					iVar5 = 1;
				}
			}

			if ((pTag->flags & 4) == 0) {
				fVar6 = pTag->time;
				g_CinematicManager_0048efc->pSubtitleText = pcVar3;
				pCVar1->subtitleHorizontalAlignment = iVar5;
				pCVar1->field_0x40 = keyTime / fVar6;
				pCVar1->field_0x44 = 0;
			}
			else {
				fVar6 = pTag->time;
				g_CinematicManager_0048efc->pSubtitleText = pcVar3;
				pCVar1->subtitleHorizontalAlignment = iVar5;
				pCVar1->field_0x40 = keyTime / fVar6;

				if (&this->position == (edF32VECTOR4*)0x0) {
					pCVar1->field_0x44 = 0;
				}
				else {
					pCVar1->vector_0x50 = this->position;
					pCVar1->field_0x44 = 1;
				}
			}
		}

		bSuccess = true;
	}

	return bSuccess;
}

void S_TARGET_ON_OFF_STREAM_REF::Create(S_TARGET_ON_OFF_STREAM_REF** pThis, ByteCode* pByteCode)
{
	*pThis = reinterpret_cast<S_TARGET_ON_OFF_STREAM_REF*>(pByteCode->currentSeekPos);
	pByteCode->currentSeekPos = pByteCode->currentSeekPos + 4;

	if ((*pThis)->entryCount != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + (*pThis)->entryCount * sizeof(S_STREAM_NTF_TARGET_ONOFF);
	}

	return;
}

void S_TARGET_ON_OFF_STREAM_REF::Init()
{
	int curSwitchIndex = 0;
	if (0 < this->entryCount) {
		do {
			this->aEntries[curSwitchIndex].Init();
			curSwitchIndex = curSwitchIndex + 1;
		} while (curSwitchIndex < this->entryCount);
	}

	return;
}

void S_TARGET_ON_OFF_STREAM_REF::Reset()
{
	int curSwitchIndex = 0;
	if (0 < this->entryCount) {
		do {
			this->aEntries[curSwitchIndex].Reset();
			curSwitchIndex = curSwitchIndex + 1;
		} while (curSwitchIndex < this->entryCount);
	}

	return;
}

void S_TARGET_ON_OFF_STREAM_REF::SwitchOn(CActor* pActor)
{
	int curSwitchIndex = 0;
	if (0 < this->entryCount) {
		do {
			this->aEntries[curSwitchIndex].SwitchOn(pActor);
			curSwitchIndex = curSwitchIndex + 1;
		} while (curSwitchIndex < this->entryCount);
	}

	return;
}

void S_TARGET_ON_OFF_STREAM_REF::SwitchOff(CActor* pActor)
{
	int curSwitchIndex = 0;
	if (0 < this->entryCount) {
		do {
			this->aEntries[curSwitchIndex].SwitchOff(pActor);
			curSwitchIndex = curSwitchIndex + 1;
		} while (curSwitchIndex < this->entryCount);
	}

	return;
}

void S_NTF_TARGET_STREAM_REF::Create(S_NTF_TARGET_STREAM_REF** pThis, ByteCode* pByteCode)
{
	*pThis = reinterpret_cast<S_NTF_TARGET_STREAM_REF*>(pByteCode->currentSeekPos);
	pByteCode->currentSeekPos = pByteCode->currentSeekPos + 4;

	if ((*pThis)->entryCount != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + (*pThis)->entryCount * sizeof(S_STREAM_NTF_TARGET_SWITCH);
	}

	return;
}

void S_NTF_TARGET_STREAM_REF::Init()
{
	int curSwitchIndex = 0;
	if (0 < this->entryCount) {
		do {
			this->aEntries[curSwitchIndex].Init();
			curSwitchIndex = curSwitchIndex + 1;
		} while (curSwitchIndex < this->entryCount);
	}

	return;
}

void S_NTF_TARGET_STREAM_REF::Reset()
{
	int curSwitchIndex = 0;
	if (0 < this->entryCount) {
		do {
			this->aEntries[curSwitchIndex].Reset();
			curSwitchIndex = curSwitchIndex + 1;
		} while (curSwitchIndex < this->entryCount);
	}

	return;
}

void S_NTF_TARGET_STREAM_REF::Switch(CActor* pActor)
{
	int curSwitchIndex = 0;
	if (0 < this->entryCount) {
		do {
			this->aEntries[curSwitchIndex].Switch(pActor);
			curSwitchIndex = curSwitchIndex + 1;
		} while (curSwitchIndex < this->entryCount);
	}

	return;
}

void S_NTF_TARGET_STREAM_REF::PostSwitch(CActor* pActor)
{
	int curSwitchIndex = 0;
	if (0 < this->entryCount) {
		do {
			this->aEntries[curSwitchIndex].PostSwitch(pActor);
			curSwitchIndex = curSwitchIndex + 1;
		} while (curSwitchIndex < this->entryCount);
	}

	return;
}

void S_NTF_SWITCH::Create(ByteCode* pByteCode)
{
	this->pTargetStreamRef = reinterpret_cast<S_NTF_TARGET_STREAM_REF*>(pByteCode->currentSeekPos);
	pByteCode->currentSeekPos = pByteCode->currentSeekPos + 4;

	if (this->pTargetStreamRef->entryCount != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + this->pTargetStreamRef->entryCount * sizeof(S_STREAM_NTF_TARGET_SWITCH);
	}

	this->pStreamEventCamera = reinterpret_cast<S_STREAM_EVENT_CAMERA*>(pByteCode->currentSeekPos);
	pByteCode->currentSeekPos = (char*)(this->pStreamEventCamera + 1);

	return;

}

void S_NTF_SWITCH::Init()
{
	this->pTargetStreamRef->Init();
	this->pStreamEventCamera->Init();

	return;
}

void S_NTF_SWITCH::Reset(CActor* pActor)
{
	this->pTargetStreamRef->Reset();
	this->pStreamEventCamera->Reset(pActor);

	return;
}

void S_NTF_SWITCH::Switch(CActor* pActor)
{
	this->pTargetStreamRef->Switch(pActor);
	this->pStreamEventCamera->SwitchOn(pActor);

	return;
}

void S_NTF_SWITCH::PostSwitch(CActor* pActor)
{
	this->pTargetStreamRef->PostSwitch(pActor);

	return;
}

void S_NTF_TARGET_STREAM_REF::SaveContext(S_SAVE_CLASS_NTF_SWITCH* pSaveData)
{
	uint shift = 1;
	int i = 0;

	pSaveData->mode = 2;
	pSaveData->switchBits = 0;

	if (0 < this->entryCount) {
		do {
			if ((this->aEntries[i].flags & 0x40000000) != 0) {
				pSaveData->switchBits = pSaveData->switchBits | shift;
			}

			i = i + 1;
			shift = shift << 1;
		} while (i < this->entryCount);
	}

	return;
}

void S_NTF_SWITCH::SaveContext(S_SAVE_CLASS_SWITCH_CAMERA* pSaveData)
{
	this->pTargetStreamRef->SaveContext(&pSaveData->switchData);
	this->pStreamEventCamera->SaveContext(&pSaveData->cameraData);

	return;
}

void S_NTF_SWITCH_ONOFF::Create(ByteCode* pByteCode)
{
	this->pTargetStreamRef = reinterpret_cast<S_TARGET_ON_OFF_STREAM_REF*>(pByteCode->currentSeekPos);
	pByteCode->currentSeekPos = pByteCode->currentSeekPos + 4;

	if (this->pTargetStreamRef->entryCount != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + this->pTargetStreamRef->entryCount * sizeof(S_STREAM_NTF_TARGET_ONOFF);
	}

	this->pStreamEventCamera = reinterpret_cast<S_STREAM_EVENT_CAMERA*>(pByteCode->currentSeekPos);
	pByteCode->currentSeekPos = (char*)(this->pStreamEventCamera + 1);

	return;
}

void S_NTF_SWITCH_ONOFF::Init()
{
	this->pTargetStreamRef->Init();
	this->pStreamEventCamera->Init();

	return;
}

void S_NTF_SWITCH_ONOFF::Reset(CActor* pActor)
{
	this->pTargetStreamRef->Reset();
	this->pStreamEventCamera->Reset(pActor);

	return;
}

void S_NTF_SWITCH_ONOFF::SwitchOn(CActor* pActor)
{
	this->pTargetStreamRef->SwitchOn(pActor);
	this->pStreamEventCamera->SwitchOn(pActor);

	return;
}

void S_NTF_SWITCH_ONOFF::SwitchOff(CActor* pActor)
{
	this->pTargetStreamRef->SwitchOff(pActor);
	this->pStreamEventCamera->SwitchOff(pActor);

	return;
}

void S_STREAM_NTF_TARGET_ANALOG_LIST::Create(S_STREAM_NTF_TARGET_ANALOG_LIST** pThis, ByteCode* pByteCode)
{
	*pThis = reinterpret_cast<S_STREAM_NTF_TARGET_ANALOG_LIST*>(pByteCode->currentSeekPos);
	pByteCode->currentSeekPos = pByteCode->currentSeekPos + 4;

	if ((*pThis)->entryCount != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + (*pThis)->entryCount * sizeof(S_STREAM_NTF_TARGET_ANALOG);
	}

	return;
}

void S_STREAM_NTF_TARGET_ANALOG_LIST::Init()
{
	int curSwitchIndex = 0;
	if (0 < this->entryCount) {
		do {
			this->aEntries[curSwitchIndex].Init();
			curSwitchIndex = curSwitchIndex + 1;
		} while (curSwitchIndex < this->entryCount);
	}

	return;
}

void S_STREAM_NTF_TARGET_ANALOG_LIST::NotifyAnalog(float param_1, float param_2, CActor* param_4, S_STREAM_EVENT_CAMERA* param_5)
{
	int curSwitchIndex = 0;
	if (0 < this->entryCount) {
		do {
			this->aEntries[curSwitchIndex].NotifyAnalog(param_1, param_2, param_4, param_5);
			curSwitchIndex = curSwitchIndex + 1;
		} while (curSwitchIndex < this->entryCount);
	}

	return;
}

void S_NTF_SWITCH_EX_LIST::Init()
{
	this->pTargetStreamRef->Init();
	this->pStreamEventCamera->Init();

	return;
}

void S_STREAM_NTF_TARGET_ANALOG_LIST::Reset()
{
	int curSwitchIndex = 0;
	if (0 < this->entryCount) {
		do {
			this->aEntries[curSwitchIndex].Reset();
			curSwitchIndex = curSwitchIndex + 1;
		} while (curSwitchIndex < this->entryCount);
	}

	return;
}

void S_NTF_SWITCH_EX_LIST::Create(ByteCode* pByteCode)
{
	this->pTargetStreamRef = reinterpret_cast<S_STREAM_NTF_TARGET_SWITCH_EX_LIST*>(pByteCode->currentSeekPos);
	pByteCode->currentSeekPos = pByteCode->currentSeekPos + 4;

	if (this->pTargetStreamRef->entryCount != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + this->pTargetStreamRef->entryCount * sizeof(S_STREAM_NTF_TARGET_SWITCH_EX);
	}

	this->pStreamEventCamera = reinterpret_cast<S_STREAM_EVENT_CAMERA*>(pByteCode->currentSeekPos);
	pByteCode->currentSeekPos = (char*)(this->pStreamEventCamera + 1);

	return;
}

void S_NTF_SWITCH_ANALOG::Create(ByteCode* pByteCode)
{
	S_STREAM_NTF_TARGET_ANALOG_LIST::Create(&pTargetStreamRef, pByteCode);

	this->pStreamEventCamera = reinterpret_cast<S_STREAM_EVENT_CAMERA*>(pByteCode->currentSeekPos);
	pByteCode->currentSeekPos = (char*)(this->pStreamEventCamera + 1);

	return;
}

void S_NTF_SWITCH_ANALOG::Init()
{
	this->pTargetStreamRef->Init();
	this->pStreamEventCamera->Init();

	return;
}

void S_NTF_SWITCH_ANALOG::NotifyAnalog(float param_1, float param_2, CActor* param_4)
{
	this->pTargetStreamRef->NotifyAnalog(param_1, param_2, param_4, this->pStreamEventCamera);

	return;
}

void S_NTF_SWITCH_ANALOG::Reset(CActor* pActor)
{
	this->pTargetStreamRef->Reset();
	this->pStreamEventCamera->Reset(pActor);

	return;
}

void S_NTF_SWITCH_EX_LIST::Switch(CActor* pActor, uint flags)
{
	this->pTargetStreamRef->Switch(pActor, flags);
	this->pStreamEventCamera->SwitchOn(pActor);

	return;
}

void S_STREAM_NTF_TARGET_SWITCH_EX_LIST::Init()
{
	int curSwitchIndex = 0;
	if (0 < this->entryCount) {
		do {
			this->aEntries[curSwitchIndex].Init();
			curSwitchIndex = curSwitchIndex + 1;
		} while (curSwitchIndex < this->entryCount);
	}

	return;
}

void S_NTF_SWITCH_EX_LIST::Reset(CActor* pActor)
{
	this->pTargetStreamRef->Reset();
	this->pStreamEventCamera->Reset(pActor);

	return;
}

void S_STREAM_NTF_TARGET_SWITCH_EX_LIST::Switch(CActor* pActor, uint flags)
{
	int curSwitchIndex = 0;
	if (0 < this->entryCount) {
		do {
			this->aEntries[curSwitchIndex].Switch(pActor, flags);
			curSwitchIndex = curSwitchIndex + 1;
		} while (curSwitchIndex < this->entryCount);
	}

	return;
}

void S_STREAM_NTF_TARGET_SWITCH_EX_LIST::Reset()
{
	int curSwitchIndex = 0;
	if (0 < this->entryCount) {
		do {
			this->aEntries[curSwitchIndex].Reset();
			curSwitchIndex = curSwitchIndex + 1;
		} while (curSwitchIndex < this->entryCount);
	}

	return;
}
