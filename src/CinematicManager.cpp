#include "CinematicManager.h"
#include "CameraViewManager.h"
#include "MemoryStream.h"
#include "FileManager3D.h"
#include "Rendering/CameraPanMasterHeader.h"
#include "SectorManager.h"
#include "TimeController.h"
#include "PauseManager.h"
#include "MathOps.h"
#include "LevelScheduleManager.h"
#include "edStr.h"
#include "port/pointer_conv.h"

#ifdef PLATFORM_PS2
#include <eekernel.h>
#endif

#include <string.h>
#include "kya.h"
#include "FrontendManager.h"
#include "InputManager.h"

CinematicManager* g_CinematicManager_0048efc;

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
	{ "BOOMY_CHANGED", 0xF, 0x62 },
	{ "DRAW_MOOD_OFF", 0xE, 0x0 },
	{ "DRAW_MOOD_ON", 0xE, 0x1 },
	{ "FIGHT_RING_CHANGED", 0x10, 0x6B },
	{ "LIPSYNC_OFF", 0xC, 0x0 },
	{ "LIPSYNC_ON", 0xB, 0x0 },
	{ "MOOD_ANGER", 0x8, 0x1 },
	{ "MOOD_DISGUST", 0x8, 0x02 },
	{ "MOOD_FEAR", 0x8, 0x03 },
	{ "MOOD_JOY", 0x8, 0x04 },
	{ "MOOD_NONE", 0x8, 0x00 },
	{ "MOOD_SORROW", 0x8, 0x05 },
	{ "MOOD_SURPRISE", 0x8, 0x06 },
	{ "RECEPTACLE_CHANGED", 0x11, 0x79 },
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


CinematicManager::CinematicManager()
{
	pCinematic = (CCinematic*)0x0;
	field_0x20 = -1;
	field_0x24 = -1;
	g_CinematicManager_0048efc = this;
	ppCinematicObjB_A = (CCinematic**)0x0;
	numCutscenes_0x8 = 0;
	ppCinematicObjB_B = (CCinematic**)0x0;
	activeCinematicCount = 0;
	pCinematicB_0x18 = (CCinematic*)0x0;
	field_0x28 = 0;
	field_0x2c = 0;
	return;
}

void CinematicManager::Game_Init()
{
	BWCinActor::InitCinMsgs();
	this->bInitialized = 1;
	return;
}

void CinematicManager::LevelLoading_Begin()
{
	CCameraCinematic* pCinematicCamera;
	CCameraCinematic* pCamera;
	CameraManager* pCameraManager;

	pCameraManager = CameraManager::_gThis;
	pCinematicCamera = (CCameraCinematic*)CameraManager::_gThis->GetDefGameCamera(CT_Cinematic);
	this->pCameraLocationObj = pCinematicCamera;
	if (this->pCinematic != (CCinematic*)0x0) {
		this->pCameraLocationObj->SetTransform(&this->matrix_0x70);
		pCameraLocationObj->fov = this->field_0xb0;
		pCamera = this->pCameraLocationObj;
		pCamera->field_0x8c = 0.0f;
		pCamera->switchMode = SWITCH_MODE_B;
		pCamera->field_0x98 = 0.0f;
		pCamera->field_0x94 = 1;
		pCamera->field_0x9c = 0.0f;
		IMPLEMENTATION_GUARD();
		//pCameraManager->PushCamera((Camera*)this->pCameraLocationObj, 1);
	}
	return;
}

void CinematicManager::LevelLoading_End()
{
	CCinematic* pCinematic;
	bool bVar1;
	CinematicManager* pCVar2;

	pCVar2 = g_CinematicManager_0048efc;
	pCinematic = this->pCinematic;
	if (pCinematic != (CCinematic*)0x0) {
		IMPLEMENTATION_GUARD(
			bVar1 = (pCinematic->cinematicLoadObject).soundObject.intFieldA != 0;
		if ((bVar1) && (bVar1 = true, (pCinematic->cinematicLoadObject).soundObject.field_0x8 != 0.0)) {
			bVar1 = false;
		}
		if (bVar1) {
			g_CinematicManager_0048efc->pCinematicB_0x18 = pCinematic;
			(**(code**)(*(int*)&(pCinematic->cinematicLoadObject).soundObject + 0x10))();
			pCVar2->pCinematicB_0x18 = (CCinematic*)0x0;
		}
		bVar1 = (pCinematic->cineBank).pBankFileAccessObject != (edCBankBufferEntry*)0x0;
		if (bVar1) {
			bVar1 = pCinematic->cineBankLoadStage_0x2b4 != 4;
		}
		if (bVar1) {
			CCinematic::Load(pCinematic, 0);
		}
		if (pCinematic->intOrPtrField != 0) {
			ActorGlobalFunc_001c84a0(pCinematic);
		}
		pCinematic->flags_0x8 = pCinematic->flags_0x8 & 0xfffffffb;
		pCinematic->pActor = (Actor*)0x0;
		GlobalDList_AddToView();
		edVideoFlip();
		CCinematic::Manage((long)(int)this->pCinematic, 1);
		this->pCinematic = (CCinematic*)0x0;);
	}
	this->pCameraLocationObj = (CCameraCinematic*)0x0;
	return;
}

bool CinematicManager::LevelLoading_Manage()
{
	bool bVar1;

	if (this->pCinematic == (CCinematic*)0x0) {
		bVar1 = false;
	}
	else {
		this->field_0x38 = 0;
		this->field_0x40 = 0;
		IMPLEMENTATION_GUARD(CCinematic::Manage(this->pCinematic));
		bVar1 = false;
		if ((this->pCinematic->flags_0x4 & 8) == 0) {
			bVar1 = this->pCinematic->intOrPtrField != 0;
		}
	}
	return bVar1;
}

void CinematicManager::Level_Init()
{
	float fVar1;
	float fVar2;
	float fVar3;
	CCameraCinematic* pCVar4;
	CCinematic** pCVar5;
	int iVar5;

	pCVar4 = (CCameraCinematic*)CameraManager::_gThis->GetDefGameCamera(CT_Cinematic);
	this->pCameraLocationObj = pCVar4;
	this->field_0x34 = 0;
	this->field_0x38 = 0;
	this->field_0x40 = 0;
	this->field_0x44 = 0;
	fVar3 = gF32Vertex4Zero.w;
	fVar2 = gF32Vertex4Zero.z;
	fVar1 = gF32Vertex4Zero.y;
	(this->vector_0x50).x = gF32Vertex4Zero.x;
	(this->vector_0x50).y = fVar1;
	(this->vector_0x50).z = fVar2;
	(this->vector_0x50).w = fVar3;
	this->field_0x6c = 0;
	fVar2 = gF32Vector4Zero.z;
	fVar1 = gF32Vector4Zero.y;
	this->x_0x60 = gF32Vector4Zero.x;
	this->y_0x64 = fVar1;
	this->z_0x68 = fVar2;
	pCVar5 = this->ppCinematicObjB_A;
	for (iVar5 = this->numCutscenes_0x8; iVar5 != 0; iVar5 = iVar5 + -1) {
		(*pCVar5)->Init();
		pCVar5 = pCVar5 + 1;
	}
	return;
}

void CinematicManager::Level_AddAll(ByteCode* pByteCode)
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

void CinematicManager::Level_Manage()
{
	int iVar1;

	this->field_0x38 = 0;
	this->field_0x40 = 0;
	this->field_0x6c = 0;
	iVar1 = 0;
	if (0 < this->activeCinematicCount) {
		do {
			this->ppCinematicObjB_B[iVar1]->Manage();
			iVar1 = iVar1 + 1;
		} while (iVar1 < this->activeCinematicCount);
	}
	if ((this->pCinematic != (CCinematic*)0x0) && (this->pCinematic->intOrPtrField == 0)) {
		this->pCinematic = (CCinematic*)0x0;
		this->field_0x20 = -1;
		this->field_0x24 = -1;
	}
	return;
}

void CinematicManager::Level_SectorChange(int param_2, int param_3)
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
				bVar2 = pCVar1->intOrPtrField != 0;
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
		((bVar2 = pCVar1->intOrPtrField != 0, bVar2 && (((pCVar1->flags_0x4 & 8) != 0 && (bVar2)))))) {
		pCVar1->flags_0x8 = pCVar1->flags_0x8 & 0xffffff7f;
		pCVar1->flags_0x8 = pCVar1->flags_0x8 | 0x100;
	}
	return;
}


void CinematicManager::WillLoadCinematic()
{
	bool bVar1;
	int* piVar2;
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
			if ((bVar1) &&
				((pCinematic->baseB == -1 ||
					(pCinematic->baseB == ((Scene::ptable.g_SectorManager_00451670)->baseSector).field_0x0)))) {
				pCinematic->pActor = (Actor*)0x0;
				if ((pCinematic->flags_0x8 & 0x800) == 0) {
					piVar2 = pCinematic->field_0x24c;
					bVar1 = false;
					if ((piVar2 != (int*)0x0) && (pCinematic->field_0x250 != 0)) {
						bVar1 = true;
					}
					if (bVar1) {
						iVar4 = 0;
						if (0 < *piVar2) {
							iVar3 = 0;
							do {
								IMPLEMENTATION_GUARD(ActorContainerFunc_00119780((MagicalSwitch_1C*)((int)piVar2 + iVar3 + 4), (Actor*)0x0));
								piVar2 = pCinematic->field_0x24c;
								iVar4 = iVar4 + 1;
								iVar3 = iVar3 + 0x1c;
							} while (iVar4 < *piVar2);
						}
						//IMPLEMENTATION_GUARD(ActorGlobalFunc_00118b50((MagicalSwitch_20*)pCinematic->field_0x250, (Actor*)0x0));
						piVar2 = pCinematic->field_0x24c;
						iVar4 = 0;
						if (0 < *piVar2) {
							iVar3 = 0;
							do {
								IMPLEMENTATION_GUARD(ActorContainerFunc_00119690((MagicalSwitch_1C*)((int)piVar2 + iVar3 + 4), (AActorBase*)0x0));
								piVar2 = pCinematic->field_0x24c;
								iVar4 = iVar4 + 1;
								iVar3 = iVar3 + 0x1c;
							} while (iVar4 < *piVar2);
						}
					}
				}
				pCinematic->Start();
			}
		}
		pCVar6 = pCVar6 + 1;
	}
	return;
}



struct CineActorConfig {
	int field_0x0;
	uint field_0x4;
	float field_0x8;
	float field_0xc;
	float field_0x10;
	float field_0x14;
	float field_0x18;
	float field_0x1c;
	float field_0x20;
	float field_0x24;
	float field_0x28;
	float field_0x2c;
};

void CCinematic::InitInternalData()
{
	this->baseB = -1;
	this->flags_0x4 = 0;
	*(undefined4*)&this->field_0x10 = 0;
	this->subObjBasePtr = (int*)0x0;
	this->count_0x220 = 0;
	//this->pCutsceneSubInfo = (ACutsceneActor**)0x0;
	this->numObjects_0x218 = 0;
	this->fileInfoStart = (CinFileContainer*)0x0;
	this->cinFileCount = 0;
	(this->cineBank).pBankFileAccessObject = (edCBankBufferEntry*)0x0;
	this->totalCutsceneDelta = 0.0;
	this->pActor = (Actor*)0x0;
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
	this->count_0x2e8 = 0;
	this->particleSectionStart = 0;
	this->buffer_0x2e4 = 0;
	this->field_0x2ec = 0;
	return;
}

CCinematic::CCinematic()
{
	this->objCount_0x9c = 0;
	this->field_0xa0 = (CineActorConfig*)0x0;
	//(this->cinematicLoadObject).BWitchCin_Obj = (BWitchCin*)&edCinGameInterface_VTable_0043dd70;
	//(this->cinematicLoadObject).BWitchCin_Obj = (BWitchCin*)&BWitchCin_VTable_0043dea0;
	//(this->cinematicLoadObject).BWCinSourceAudio_Obj.vt = &BWCinSourceAudio_VTable_0043dd50;
	//(this->cinematicLoadObject).BWCinSourceAudio_Obj.vt = &BWCinSourceAudio_VTable_0043df70;
	//(this->cinematicLoadObject).BWCinSourceAudio_Obj.intFieldA = 0;
	//(this->cinematicLoadObject).BWCinSourceAudio_Obj.field_0x8 = 0.0;
	//(this->cinematicLoadObject).BWCinSourceAudio_Obj.floatFieldA = 0.0;
	//*(undefined*)&(this->cinematicLoadObject).field_0x3c = 0;
	//*(undefined*)&(this->cinematicLoadObject).field_0x3d = 0;
	//(this->cinematicLoadObject).BWCinSourceAudio_Obj.pGlobalSoundFileData = (GlobalSound_FileData*)0x0;
	//(this->cinematicLoadObject).BWCinSourceAudio_Obj.lastIntField =
	//	&(this->cinematicLoadObject).BWCinSourceAudio_Obj.field_0x14;
	//(this->cinematicLoadObject).BWCinCam_Obj.vt = &BWCinCam_VTable_0043dd20;
	//(this->cinematicLoadObject).BWCinCam_Obj.vt = &g_CinematicUpdateCameraFuncData_0043df90;
	//this->BWCinSourceSubtitle_Obj = (BWCinSourceSubtitle*)&BWCinSourceSubtitle_VTable_0043dd00;
	//this->BWCinSourceSubtitle_Obj = (BWCinSourceSubtitle*)&BWCinSourceSubtitle_VTable_0043def0;
	//(this->cinFileData).headerPtr = (CinematicFileDataHeader*)0x0;
	this->textData = CMessageFile();
	(this->condArray_0x244).field_0x0 = (uint*)0x0;
	(this->cond_0x248).field_0x0 = (char*)0x0;
	InitInternalData();
	return;
}

void ConditionedOperationArray::Create(ByteCode* pByteCode)
{
	uint* puVar1;
	int* piVar2;
	int iVar3;
	uint uVar4;

	puVar1 = (uint*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)(puVar1 + 1);
	this->field_0x0 = puVar1;
	uVar4 = 0;
	if (*this->field_0x0 != 0) {
		do {
			piVar2 = (int*)pByteCode->currentSeekPos;
			pByteCode->currentSeekPos = (char*)(piVar2 + 1);
			iVar3 = *piVar2;
			if (iVar3 != 0) {
				pByteCode->currentSeekPos = pByteCode->currentSeekPos + iVar3 * 0xc;
			}
			piVar2 = (int*)pByteCode->currentSeekPos;
			pByteCode->currentSeekPos = (char*)(piVar2 + 1);
			iVar3 = *piVar2;
			if (iVar3 != 0) {
				pByteCode->currentSeekPos = pByteCode->currentSeekPos + iVar3 * 0x10;
			}
			uVar4 = uVar4 + 1;
		} while (uVar4 < *this->field_0x0);
	}
	return;
}

void ConditionedOperationArray::Perform()
{
	int iVar1;
	uint* puVar2;
	ulong uVar3;
	uint uVar4;
	uint* puVar5;
	S_STREAM_SIMPLE_COND* pCond;
	uint* puVar6;
	uint uVar7;
	int iVar8;

	puVar2 = this->field_0x0;
	if (puVar2 != (uint*)0x0) {
		uVar4 = 0;
		puVar5 = puVar2 + 1;
		if (*puVar2 != 0) {
			IMPLEMENTATION_GUARD(
			do {
				uVar3 = 1;
				puVar2 = puVar5 + *puVar5 * 3;
				uVar7 = puVar2[1];
				pCond = (S_STREAM_SIMPLE_COND*)(puVar2 + 2);
				iVar1 = uVar7 * 4;
				for (; 0 < (int)uVar7; uVar7 = uVar7 - 1) {
					uVar3 = ScenaricCondition::IsVerified(pCond, uVar3);
					pCond = pCond + 1;
				}
				if ((uVar3 != 0) && (iVar8 = 0, puVar6 = puVar5, 0 < (int)*puVar5)) {
					do {
						FUN_00117510((int*)(puVar6 + 1));
						iVar8 = iVar8 + 1;
						puVar6 = puVar6 + 3;
					} while (iVar8 < (int)*puVar5);
				}
				uVar4 = uVar4 + 1;
				puVar5 = puVar2 + iVar1 + 2;
			} while (uVar4 < *this->field_0x0);)
		}
	}
	return;
}

void CCinematic::Create(ByteCode* pByteCode)
{
	float* pfVar1;
	int* piVar2;
	float fVar3;
	float fVar4;
	uint uVar5;
	char* pcVar6;
	int iVar7;
	CineActorConfig* piVar7;
	CineActorConfig* ppCVar8;
	int iVar8;
	uint uVar9;
	uint uVar10;
	ulong uVar11;
	int iVar12;
	CineActorConfig* piVar13;
	float fVar13;

	this->flags_0x8 = 0;
	uVar5 = pByteCode->GetU32();
	this->prtBuffer = uVar5;
	pcVar6 = pByteCode->GetString();
	this->pBankName_0x48 = pcVar6;
	pcVar6 = pByteCode->GetString();
	this->pBankName_0x50 = pcVar6;
	pcVar6 = pByteCode->GetString();
	this->fileName = pcVar6;

	CUTSCENE_LOG(LogLevel::Info, "Cinematic::Create Bank A %s", this->pBankName_0x48);
	CUTSCENE_LOG(LogLevel::Info, "Cinematic::Create Bank B %s", this->pBankName_0x50);
	CUTSCENE_LOG(LogLevel::Info, "Cinematic::Create File %s", this->fileName);

	uVar5 = pByteCode->GetS32();
	this->field_0x18 = uVar5;
	iVar7 = pByteCode->GetS32();
	this->field_0x1c = iVar7;
	uVar5 = pByteCode->GetS32();
	this->count_0x20 = uVar5;
	uVar5 = pByteCode->GetS32();
	this->field_0x24 = uVar5;
	iVar7 = pByteCode->GetS32();
	this->field_0x28 = iVar7;
	iVar7 = pByteCode->GetS32();
	this->field_0x2c = iVar7;
	uVar5 = pByteCode->GetU32();
	this->field_0x4c = uVar5;
	uVar5 = pByteCode->GetU32();
	this->field_0x54 = uVar5;
	iVar7 = pByteCode->GetS32();
	this->baseB = iVar7;
	uVar5 = pByteCode->GetU32();
	*(uint*)&this->field_0x10 = uVar5;
	uVar5 = pByteCode->GetU32();
	this->flags_0x4 = uVar5;
	fVar13 = pByteCode->GetF32();
	this->field_0x30 = fVar13;
	pfVar1 = (float*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)(pfVar1 + 4);
	this->field_0x34 = *pfVar1;
	this->field_0x38 = pfVar1[1];
	this->field_0x3c = pfVar1[2];
	this->field_0x40 = pfVar1[3];
	iVar7 = pByteCode->GetS32();
	this->intFieldC = iVar7;
	iVar7 = pByteCode->GetS32();
	this->field_0x90 = iVar7;
	iVar7 = pByteCode->GetS32();
	this->field_0x94 = iVar7;
	iVar7 = pByteCode->GetS32();
	this->field_0x98 = iVar7;
	iVar7 = pByteCode->GetS32();
	this->field_0x44 = iVar7;
	fVar13 = pByteCode->GetF32();
	this->field_0x58 = fVar13;
	iVar7 = pByteCode->GetS32();
	this->field_0x5c = iVar7;
	fVar13 = pByteCode->GetF32();
	this->field_0x60 = fVar13;
	iVar7 = pByteCode->GetS32();
	this->field_0x64 = iVar7;
	fVar13 = pByteCode->GetF32();
	this->field_0x68 = fVar13;
	uVar5 = pByteCode->GetS32();
	this->objCount_0x9c = uVar5;
	uVar5 = this->objCount_0x9c;
	if (uVar5 != 0) {
		this->field_0xa0 = new CineActorConfig[uVar5 + 0x10];
		iVar7 = 0;
		if (0 < (int)this->objCount_0x9c) {
			iVar12 = 0;
			do {
				piVar13 = (CineActorConfig*)((int)&this->field_0xa0->field_0x0 + iVar12);
				iVar8 = pByteCode->GetS32();
				piVar13->field_0x0 = iVar8;
				uVar5 = pByteCode->GetU32();
				piVar13->field_0x4 = uVar5;
				fVar13 = pByteCode->GetF32();
				piVar13->field_0x8 = fVar13;
				fVar13 = pByteCode->GetF32();
				piVar13->field_0xc = fVar13;
				iVar7 = iVar7 + 1;
				piVar13->field_0xc = piVar13->field_0xc * 0.01745329;
				fVar4 = gF32Vertex4Zero.w;
				fVar3 = gF32Vertex4Zero.z;
				fVar13 = gF32Vertex4Zero.y;
				piVar13->field_0x10 = gF32Vertex4Zero.x;
				piVar13->field_0x14 = fVar13;
				piVar13->field_0x18 = fVar3;
				piVar13->field_0x1c = fVar4;
				fVar4 = gF32Vector4Zero.w;
				fVar3 = gF32Vector4Zero.z;
				fVar13 = gF32Vector4Zero.y;
				piVar13->field_0x20 = gF32Vector4Zero.x;
				piVar13->field_0x24 = fVar13;
				piVar13->field_0x28 = fVar3;
				piVar13->field_0x2c = fVar4;
				iVar12 = iVar12 + 0x30;
			} while (iVar7 < (int)this->objCount_0x9c);
		}
	}
	this->cond_0x248.Create(pByteCode);
	uVar11 = this->cond_0x248.IsVerified();
	if (uVar11 == 0) {
		this->flags_0x8 = this->flags_0x8 | 8;
	}
	if (2.16f <= Scene::_pinstance->field_0x1c) {
		uVar5 = pByteCode->GetU32();
		this->intFieldD = uVar5;
	}
	else {
		this->intFieldD = -1;
	}
	uVar5 = pByteCode->GetU32();
	memset(&this->intFieldE, 0xff, 0x14);
	for (; uVar5 != 0; uVar5 = uVar5 - 1) {
		uVar9 = pByteCode->GetU32();
		uVar10 = pByteCode->GetU32();
		(&this->intFieldE)[uVar9] = uVar10;
	}
	if (this->intFieldD == -1) {
		this->intFieldD = this->intFieldE;
	}
	iVar7 = pByteCode->GetS32();
	this->field_0x6c = iVar7;
	iVar7 = pByteCode->GetS32();
	this->field_0x70 = iVar7;
	iVar7 = pByteCode->GetS32();
	this->field_0x74 = iVar7;
	uVar5 = pByteCode->GetU32();
	this->field_0x78 = uVar5;
	uVar5 = pByteCode->GetU32();
	this->field_0x7c = uVar5;
	fVar13 = pByteCode->GetF32();
	this->field_0x80 = fVar13;
	piVar2 = (int*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)(piVar2 + 1);
	if (*piVar2 != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + *piVar2 * 4;
	}
	this->field_0x25c = piVar2;
	this->condArray_0x244.Create(pByteCode);
	piVar2 = (int*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)(piVar2 + 1);
	if (*piVar2 != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + *piVar2 * 0x1c;
	}
	this->field_0x24c = piVar2;
	pcVar6 = pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = pcVar6 + 0x20;
	this->field_0x250 = (int)pcVar6;
	piVar2 = (int*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)(piVar2 + 1);
	if (*piVar2 != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + *piVar2 * 0x1c;
	}
	this->field_0x254 = piVar2;
	pcVar6 = pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = pcVar6 + 0x20;
	this->field_0x258 = (int)pcVar6;
	if (this->field_0x4c == 0) {
		this->flags_0x8 = this->flags_0x8 | 8;
	}
	//SetupInternalData(this);
	return;
}

void CCinematic::Init()
{
	Timer* pTVar1;
	int* piVar2;
	undefined4* puVar3;
	uint uVar4;
	int iVar5;
	int iVar6;

	//SetupCheckpointEventData_00119990((CheckpointManagerSubObjA*)&this->intFieldC);
	//LinkCheckpointActor_00119a00((CheckpointManagerSubObjB*)&this->field_0x90);
	//SetupCheckpointEventData_00119990((CheckpointManagerSubObjA*)&this->field_0x94);
	//SetupCheckpointEventData_00119990((CheckpointManagerSubObjA*)&this->field_0x98);
	//LinkCheckpointActor_00119a00((CheckpointManagerSubObjB*)&this->field_0x44);
	iVar6 = 0;
	//if (0 < (int)this->objCount_0x9c) {
	//	iVar5 = 0;
	//	do {
	//		LinkCheckpointActor_00119a00((CheckpointManagerSubObjB*)((int)&this->field_0xa0->field_0x0 + iVar5));
	//		iVar6 = iVar6 + 1;
	//		iVar5 = iVar5 + 0x30;
	//	} while (iVar6 < (int)this->objCount_0x9c);
	//}
	//for (iVar6 = *this->field_0x25c; iVar6 != 0; iVar6 = iVar6 + -1) {
	//}
	piVar2 = this->field_0x24c;
	iVar6 = 0;
	//if (0 < *piVar2) {
	//	iVar5 = 0;
	//	do {
	//		FUN_00119880((MagicalSwitch_1C*)((int)piVar2 + iVar5 + 4));
	//		piVar2 = this->field_0x24c;
	//		iVar6 = iVar6 + 1;
	//		iVar5 = iVar5 + 0x1c;
	//	} while (iVar6 < *piVar2);
	//}
	//FUN_00118db0((MagicalSwitch_20*)this->field_0x250);
	piVar2 = this->field_0x254;
	iVar6 = 0;
	//if (0 < *piVar2) {
	//	iVar5 = 0;
	//	do {
	//		FUN_00119880((MagicalSwitch_1C*)((int)piVar2 + iVar5 + 4));
	//		piVar2 = this->field_0x254;
	//		iVar6 = iVar6 + 1;
	//		iVar5 = iVar5 + 0x1c;
	//	} while (iVar6 < *piVar2);
	//}
	//FUN_00118db0((MagicalSwitch_20*)this->field_0x258);
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
	this->intOrPtrField = 0;
	pTVar1 = GetTimer();
	this->field_0x88 = pTVar1->scaledTotalTime;
	if ((this->flags_0x8 & 0x10) != 0) {
		IMPLEMENTATION_GUARD(this->cinFileData.Shutdown());
		this->flags_0x8 = this->flags_0x8 & 0xffffffef;
	}
	this->count_0x224 = 0;
	this->count_0x22c = 0;
	iVar6 = 0;
	if (0 < this->count_0x2e8) {
		iVar5 = 0;
		do {
			iVar6 = iVar6 + 1;
			puVar3 = (undefined4*)(this->buffer_0x2e4 + iVar5);
			*puVar3 = 0xffffffff;
			puVar3[2] = 0;
			puVar3[1] = 0;
			*(undefined*)(puVar3 + 3) = 0;
			puVar3[4] = 0;
			puVar3[5] = 0;
			puVar3[6] = 0;
			iVar5 = iVar5 + 0x1c;
		} while (iVar6 < this->count_0x2e8);
	}
	if ((this->flags_0x4 & 0x2000000) != 0) {
		this->count_0x2d8 = this->count_0x2d8 + 1;
		this->Load(1);
	}
	return;
}

class CActorHero
{
public:
	static CActorHero* _gThis;
};

CActorHero* CActorHero::_gThis = NULL;

void CCinematic::Start()
{
	Actor* pAVar1;
	float fVar2;
	uint uVar3;
	uint uVar4;
	CActorHero* pAVar5;
	CinematicManager* pCVar6;
	bool bVar7;
	Timer* pTVar8;
	edNODE* pMVar9;
	int* piVar10;
	edF32MATRIX4* peVar11;
	long lVar12;
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

	this->flags_0x8 = this->flags_0x8 & 0xffffff7f;
	if (this->intOrPtrField == 0) {
		bVar7 = (this->flags_0x4 & 1) != 0;
		if (bVar7) {
			bVar7 = (this->flags_0x8 & 0x400) != 0;
		}
		if (!bVar7) {
			bVar7 = (this->flags_0x8 & 0x28) != 0;
		}
		bVar7 = (bool)(bVar7 ^ 1);
		if (!bVar7) {
			bVar7 = (this->flags_0x8 & 0x800) != 0;
		}
		if (bVar7) {
			if (this->cineBankLoadStage_0x2b4 != 4) {
				Load(0);
			}
			pCVar6 = g_CinematicManager_0048efc;
			if (this->cineBankLoadStage_0x2b4 == 4) {
				if (this->intFieldC != 0) {
					this->flags_0x8 = this->flags_0x8 | 0x200;
				}
				pCVar6->pCinematicB_0x18 = this;
				/* Takes in a bank header and file name, and returns index of file in bank
				   Example: Start_ff_Montage_SceneMontage.cin returns 59. */
				iVar16 = (this->cineBank).pBankFileAccessObject->get_index(this->fileName);
				if ((((iVar16 != -1) &&
					(bVar7 = (this->cineBank).pBankFileAccessObject->get_info(iVar16, &eStack48, (char*)0x0), bVar7 != false)) &&
					(bVar7 = ((edCinematic*)eStack48.fileBufferStart)->ExtractVersions
					(eStack48.size, &cinematicLibraryVersion,
						&cinematicCompilerVersion), bVar7 != false)) &&
					((cinematicLibraryVersion != 0x81 || (cinematicCompilerVersion != 0xa0)))) {
					/* Cinematic library version: %d (should be %d)\n */
					edDebugPrintf("Cinematic library version: %d (should be %d)\n", cinematicLibraryVersion, 0x81);
					/* Cinematic compiler version: %d (should be %d)\n */
					edDebugPrintf("Cinematic compiler version: %d (should be %d)\n", cinematicCompilerVersion, 0xa0);
				}
				if (this->intFieldD != -1) {
					iVar16 = CMessageFile::get_default_language();
					lVar12 = (long)(&this->intFieldE)[iVar16];
					if (lVar12 == -1) {
						lVar12 = (long)this->intFieldD;
					}
					//BWCinSourceAudio::Create(&(this->cinematicLoadObject).BWCinSourceAudio_Obj, lVar12);
				}
				if ((this->prtBuffer == 1) || ((this->flags_0x4 & 8) != 0)) {
					edMemSetFlags(TO_HEAP(H_MAIN), 0x100);
				}
				this->numObjects_0x218 = 0;
				this->count_0x220 = 0;
				this->count_0x224 = 0;
				this->count_0x22c = 0;
				this->numberOfParticles = 0;
				/* This will take a cinematic file and load all of its assets
				   Example: AIRLIFT_TO_NATIV_Cin_Montage_SceneMontage.cin */
				this->cinFileData.Create(this->cinematicLoadObject, this->fileName);

				local_70 = gF32Matrix4Unit;

				if ((this->flags_0x4 & 0x2000) != 0) {
					IMPLEMENTATION_GUARD(
					iVar16 = this->field_0x44;
					if (iVar16 != 0) {
						edF32Matrix4FromEulerSoft(&local_70, *(int*)(iVar16 + 0x18) + 0x18, s_XYZ_0042b950);
						local_70.da = *(float*)(iVar16 + 0x70);
						local_70.db = *(float*)(iVar16 + 0x74);
						local_70.dc = *(float*)(iVar16 + 0x78);
						local_70.dd = *(float*)(iVar16 + 0x7c);
						edF32Matrix4GetInverseOrthoHard(&local_70, &local_70);
					}
					if (this->pActor != (Actor*)0x0) {
						edF32Matrix4FromEulerSoft(&eStack176, (int)&(this->pActor->actorBase).data.rotationVector, s_XYZ_0042b950);
						pAVar1 = this->pActor;
						eStack176.da = (pAVar1->actorBase).data.currentLocation.x;
						eStack176.db = (pAVar1->actorBase).data.currentLocation.y;
						eStack176.dc = (pAVar1->actorBase).data.currentLocation.z;
						eStack176.dd = (pAVar1->actorBase).data.currentLocation.w;
						edF32Matrix4MulF32Matrix4Hard(&local_70, &local_70, &eStack176);
					})
				}
				this->matrix_0x120 = local_70;
				
				if (this->prtBuffer == 1) {
					IMPLEMENTATION_GUARD(
					FUN_001b9b70(0, (int)Scene::_pinstance, (int)&this->field_0x34);)
				}
				if (this->pMeshInfo != (ed_g3d_manager*)0x0) {
					IMPLEMENTATION_GUARD(
					ed3DScenePushCluster(scene::_scene_handle, this->pMeshInfo);
					iVar16 = FUN_00295c40((int)this->pMeshInfo, 0x43494d414e5944);
					if (iVar16 != 0) {
						FUN_0029f330(iVar16, 0xffff);
					}
					pMVar9 = ed3DHierarchyAddToSceneByHashcode(scene::_scene_handle, this->pMeshInfo, 0x43494d414e5944);
					this->pMeshTransform = pMVar9;
					if (this->pMeshTransform != (MeshTransformParent*)0x0) {
						SetMeshTransformFlag_002abd80(this->pMeshTransform, 0xffff);
						ClearMeshTransformFlag_002abff0(this->pMeshTransform, 0xffff);
					})
				}
				pAVar5 = CActorHero::_gThis;
				if (CActorHero::_gThis != (CActorHero*)0x0) {
					IMPLEMENTATION_GUARD(
					if ((this->flags_0x4 & 0x10) != 0) {
						local_c = 0;
						ActorCompareFunc_00103f00
						((AActorBase*)ActorHero::_gThis, (AActorBase*)ActorHero::_gThis, 0x82, (ActorCompareStruct*)0x0);
					}
					if ((this->flags_0x4 & 0x10000) != 0) {
						(pAVar5->character).characterBase.actorBase.flags =
							(pAVar5->character).characterBase.actorBase.flags | 0x400000;
					})
				}
				if ((this->flags_0x4 & 0x400) != 0) {
					Scene::ptable.g_FrontendManager_00451680->SetActive(false);
					GameFlags = GameFlags | 0x100;
				}
				if ((this->flags_0x4 & 0x20000200) != 0) {
					GameFlags = GameFlags | 0x80;
				}
				uVar3 = this->flags_0x4;
				if ((uVar3 & 0x280000) != 0) {
					IMPLEMENTATION_GUARD(
					FUN_00182db0(0x3f800000, (int)Scene::ptable.g_GlobalSoundPtr_00451698, uVar3 & 0x80000, uVar3 & 0x200000);)
				}
				if ((this->flags_0x4 & 0x100) == 0) {
					this->totalCutsceneDelta = 0.0;
				}
				if ((this->flags_0x4 & 0x1000000) != 0) {
					this->flags_0x8 = this->flags_0x8 | 0x40;
				}
				bVar7 = false;
				if ((this->flags_0x8 & 0x800) == 0) {
					uVar3 = this->objCount_0x9c;
					iVar16 = 0;
					if (0 < (int)uVar3) {
						iVar15 = 0;
						do {
							IMPLEMENTATION_GUARD(
							ppiVar14 = (int**)((int)&this->field_0xa0->field_0x0 + iVar15);
							piVar10 = *ppiVar14;
							if (piVar10 != (int*)0x0) {
								if (((uint)ppiVar14[1] & 0x10) != 0) {
									uVar4 = piVar10[2];
									piVar19 = (int*)piVar10[0xd];
									piVar17 = (int*)piVar10[0xe];
									piVar18 = (int*)piVar10[0xf];
									ppiVar14[4] = (int*)piVar10[0xc];
									ppiVar14[5] = piVar19;
									ppiVar14[6] = piVar17;
									ppiVar14[7] = piVar18;
									piVar19 = (int*)piVar10[0x15];
									piVar17 = (int*)piVar10[0x16];
									piVar18 = (int*)piVar10[0x17];
									ppiVar14[8] = (int*)piVar10[0x14];
									ppiVar14[9] = piVar19;
									ppiVar14[10] = piVar17;
									ppiVar14[0xb] = piVar18;
									if ((uVar4 & 0x1000) == 0) {
										ppiVar14[1] = (int*)((ulong)((long)(int)ppiVar14[1] << 0x21) >> 0x21);
									}
									else {
										ppiVar14[1] = (int*)((uint)ppiVar14[1] | 0x80000000);
									}
								}
								if (((uint)ppiVar14[1] & 1) != 0) {
									(**(code**)(*piVar10 + 0xa8))();
									bVar7 = true;
								}
							}
							iVar16 = iVar16 + 1;
							iVar15 = iVar15 + 0x30;
							)
						} while (iVar16 < (int)uVar3);
					}
					if ((this->flags_0x4 & 0x1000) != 0) {
						bVar7 = true;
					}
				}
				if (bVar7) {
					this->intOrPtrField = 1;
					pTVar8 = GetTimer();
					this->field_0x88 = pTVar8->scaledTotalTime;
				}
				else {
					this->intOrPtrField = 2;
					pTVar8 = GetTimer();
					this->field_0x88 = pTVar8->scaledTotalTime;
					this->cinFileData.Initialize();
					this->flags_0x8 = this->flags_0x8 | 0x10;
				}
				if ((this->flags_0x8 & 2) != 0) {
					this->cinematicLoadObject.BWCinCam_Obj.Activate();
					this->cinFileData.Timeslice(0.0f, &FStack224);
					CameraManager::_gThis->PushCamera((Camera*)pCVar6->pCameraLocationObj, 1);
				}
				if ((this->prtBuffer == 1) || ((this->flags_0x4 & 8) != 0)) {
					edMemClearFlags(TO_HEAP(H_MAIN), 0x100);
				}
				pCVar6->pCinematicB_0x18 = (CCinematic*)0x0;
				if ((this->flags_0x4 & 2) == 0) {
					this->flags_0x8 = this->flags_0x8 | 0x1400;
				}
				if (this->pActor != (Actor*)0x0) {
					IMPLEMENTATION_GUARD(
					local_14 = 2;
					local_10 = &local_18;
					local_18 = this;
					(*(code*)((this->pActor->actorBase).pVTable)->interactWith)(this->pActor, 0, 0x7c, local_10);)
				}
			}
			else {
				g_CinematicManager_0048efc->field_0x28 = this;
				pTVar8 = GetTimer();
				pCVar6->field_0x2c = pTVar8->totalTime;
				IMPLEMENTATION_GUARD();
				//if ((this->flags_0x8 & 0x800) == 0) {
				//	piVar10 = this->field_0x24c;
				//	bVar7 = false;
				//	if ((piVar10 != (int*)0x0) && (this->field_0x250 != 0)) {
				//		bVar7 = true;
				//	}
				//	if (bVar7) {
				//		iVar16 = 0;
				//		if (0 < *piVar10) {
				//			iVar15 = 0;
				//			do {
				//				ActorContainerFunc_00119780((MagicalSwitch_1C*)((int)piVar10 + iVar15 + 4), (Actor*)0x0);
				//				piVar10 = this->field_0x24c;
				//				iVar16 = iVar16 + 1;
				//				iVar15 = iVar15 + 0x1c;
				//			} while (iVar16 < *piVar10);
				//		}
				//		ActorGlobalFunc_00118b50((MagicalSwitch_20*)this->field_0x250, (Actor*)0x0);
				//		piVar10 = this->field_0x24c;
				//		iVar16 = 0;
				//		if (0 < *piVar10) {
				//			iVar15 = 0;
				//			do {
				//				ActorContainerFunc_00119690((MagicalSwitch_1C*)((int)piVar10 + iVar15 + 4), (AActorBase*)0x0);
				//				piVar10 = this->field_0x24c;
				//				iVar16 = iVar16 + 1;
				//				iVar15 = iVar15 + 0x1c;
				//			} while (iVar16 < *piVar10);
				//		}
				//	}
				//}
				//if ((this->flags_0x8 & 0x800) == 0) {
				//	FUN_00117210((uint**)&this->condArray_0x244);
				//	piVar10 = this->field_0x254;
				//	bVar7 = false;
				//	if ((piVar10 != (int*)0x0) && (this->field_0x258 != 0)) {
				//		bVar7 = true;
				//	}
				//	if (bVar7) {
				//		iVar16 = 0;
				//		if (0 < *piVar10) {
				//			iVar15 = 0;
				//			do {
				//				ActorContainerFunc_00119780((MagicalSwitch_1C*)((int)piVar10 + iVar15 + 4), (Actor*)0x0);
				//				piVar10 = this->field_0x254;
				//				iVar16 = iVar16 + 1;
				//				iVar15 = iVar15 + 0x1c;
				//			} while (iVar16 < *piVar10);
				//		}
				//		ActorGlobalFunc_00118b50((MagicalSwitch_20*)this->field_0x258, (Actor*)0x0);
				//		piVar10 = this->field_0x254;
				//		iVar16 = 0;
				//		if (0 < *piVar10) {
				//			iVar15 = 0;
				//			do {
				//				ActorContainerFunc_00119690((MagicalSwitch_1C*)((int)piVar10 + iVar15 + 4), (AActorBase*)0x0);
				//				piVar10 = this->field_0x254;
				//				iVar16 = iVar16 + 1;
				//				iVar15 = iVar15 + 0x1c;
				//			} while (iVar16 < *piVar10);
				//		}
				//	}
				//	if ((this == g_CinematicManager_0048efc->pCinematic) && (g_CinematicManager_0048efc->field_0x20 != -1)) {
				//		LevelScheduleManager::HoldsGetActorFunc_002dc200
				//		(LevelScheduleManager::gThis, g_CinematicManager_0048efc->field_0x20,
				//			LevelScheduleManager::gThis->currentLevelID, g_CinematicManager_0048efc->field_0x24);
				//	}
				//	else {
				//		LevelScheduleManager::SetLevelLoadInfo_002dc3a0
				//		(LevelScheduleManager::gThis, (undefined*)0x0, (long)this->field_0x6c, this->field_0x70,
				//			(long)this->field_0x74, 0xffffffff);
				//	}
				//}
				this->flags_0x8 = this->flags_0x8 & 0xfffff7ff;
			}
		}
	}
	return;
}

TypePairData _gCinTableBankCallback[1] = {-1, -1, 0x0, 0, 0, 0, 0, 0};

void CCinematic::Load(long mode)
{
	int iVar1;
	bool bVar2;
	uint uVar3;
	uint size;
	edCBankInstall eStack32;

	bVar2 = (this->cineBank).pBankFileAccessObject != (edCBankBufferEntry*)0x0;
	if (bVar2) {
		bVar2 = this->cineBankLoadStage_0x2b4 != 4;
	}
	if (bVar2) {
		if (mode == 0) {
			iVar1 = this->cineBankLoadStage_0x2b4;
			while (iVar1 != 4) {
				iVar1 = this->cineBankLoadStage_0x2b4;
				if (iVar1 != 4) {
					if (iVar1 == 3) {
						bVar2 = (this->cineBank).pBankFileAccessObject->is_loaded();
						if (bVar2 != false) {
							this->cineBankLoadStage_0x2b4 = 4;
						}
					}
					else {
						if (((iVar1 == 2) && (bVar2 = edSoundAreAllSoundDataLoaded(), bVar2 != false)) &&
							(bVar2 = edMusicAreAllMusicDataLoaded(), bVar2 != false)) {
							(this->cineBank).pBankFileAccessObject->close();
							(this->cineBank).pBankFileAccessObject = (edCBankBufferEntry*)0x0;
							LoadCineBnk(1);
						}
					}
				}
				edFileNoWaitStackFlush();
				iVar1 = this->cineBankLoadStage_0x2b4;
			}
		}
	}
	else {
		size = this->field_0x4c;
		if ((uint)this->field_0x4c <= (uint)this->field_0x54) {
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
		if (((bVar2) && (bVar2 = CheckFunc_00401fd0(&StaticEdFileBase_004497f0), bVar2 != false)) &&
			(uVar3 = edMemGetMemoryAvailable(TO_HEAP(H_MAIN)), size + 0x2800 < uVar3)) {
			if ((this->prtBuffer == 1) || ((this->flags_0x4 & 8) != 0)) {
				edMemSetFlags(TO_HEAP(H_MAIN), 0x100);
			}
			memset(&eStack32, 0, sizeof(edCBankInstall));
			this->cineBank.initialize(size, 1, &eStack32);
			this->cineBank.bank_buffer_setcb(_gCinTableBankCallback);
			if ((this->prtBuffer == 1) || ((this->flags_0x4 & 8) != 0)) {
				edMemClearFlags(TO_HEAP(H_MAIN), 0x100);
			}
			if (this->field_0x54 == 0) {
				this->LoadCineBnk(mode);
			}
			else {
				bVar2 = LoadInternal(mode);
				if ((bVar2 != false) && (mode == 0)) {
					(this->cineBank).pBankFileAccessObject->close();
					(this->cineBank).pBankFileAccessObject = (edCBankBufferEntry*)0x0;
					this->LoadCineBnk(0);
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
	MY_LOG("Ignoring sound: \n");
}


bool CCinematic::LoadInternal(long mode)
{
	char cVar1;
	LevelScheduleManager* pLVar2;
	bool bVar3;
	int iVar4;
	edFILEH* pDebugBank;
	edCBankBufferEntry* peVar5;
	char* pcVar6;
	edCBankInstall local_420;
	char acStack1024[512];
	char acStack512[512];

	pLVar2 = LevelScheduleManager::gThis;
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
	edStrCatMulti(acStack512, pLVar2->levelPath, pLVar2->aLevelInfo[pLVar2->currentLevelID].levelName, 0x42b960, acStack1024, 0
	);
	bVar3 = true;
	if (((gVideoConfig.omode != 2) && ((this->flags_0x4 & 0x8000000) != 0)) &&
		(pDebugBank = edFileOpen(acStack512, 9), pDebugBank != (edFILEH*)0x0)) {
		edFileClose(pDebugBank);
		bVar3 = false;
	}
	if (bVar3) {
		edStrCatMulti(acStack512, pLVar2->levelPath, pLVar2->aLevelInfo[pLVar2->currentLevelID].levelName, 0x42b960,
			this->pBankName_0x50, 0);
	}
	memset(&local_420, 0, sizeof(edCBankInstall));
	local_420.filePath = acStack512;
	local_420.fileFunc = _gCinSoundCallback;
	local_420.pObjectReference = this;
	peVar5 = this->cineBank.get_free_entry();
	(this->cineBank).pBankFileAccessObject = peVar5;
	local_420.fileFlagA = 4;
	this->cineBankLoadStage_0x2b4 = 1;
	bVar3 = (this->cineBank).pBankFileAccessObject->load(&local_420);
	if (bVar3 == false) {
		(this->cineBank).pBankFileAccessObject = (edCBankBufferEntry*)0x0;
		this->cineBank.terminate();
		this->cineBankLoadStage_0x2b4 = 0;
	}
	else {
		if (mode == 0) {
			(this->cineBank).pBankFileAccessObject->wait();
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
	float fVar1;
	undefined4 uVar2;
	void* pvVar3;
	uint uVar4;
	int iVar5;
	edF32MATRIX4* peVar6;
	edF32MATRIX4* peVar7;
	CCinematic* local_10;
	undefined4 local_c;
	CCinematic** local_4;
	CinFileContainer* pCVar3;

	uVar2 = (this->cineBank).pBankFileAccessObject->get_element_count();
	this->cinFileCount = uVar2;
	if ((this->prtBuffer == 1) || ((this->flags_0x4 & 8) != 0)) {
		edMemSetFlags(TO_HEAP(H_MAIN), 0x100);
	}
	if (this->cinFileCount != 0) {
		pCVar3 = new CinFileContainer[this->cinFileCount];
		this->fileInfoStart = pCVar3;
		pCVar3 = this->fileInfoStart;
		uVar4 = 0;
		if (this->cinFileCount != 0) {
			do {
				pCVar3->pData = (char*)0x0;
				uVar4 = uVar4 + 1;
				pCVar3 = pCVar3 + 1;
			} while (uVar4 < (uint)this->cinFileCount);
		}
	}
	if ((this->prtBuffer == 1) || ((this->flags_0x4 & 8) != 0)) {
		edMemClearFlags(TO_HEAP(H_MAIN), 0x100);
	}
	peVar7 = &gF32Matrix4Unit;
	peVar6 = &this->matrix_0x120;
	iVar5 = 8;
	do {
		iVar5 = iVar5 + -1;
		fVar1 = peVar7->ab;
		peVar6->aa = peVar7->aa;
		peVar7 = (edF32MATRIX4*)&peVar7->ac;
		peVar6->ab = fVar1;
		peVar6 = (edF32MATRIX4*)&peVar6->ac;
	} while (0 < iVar5);
	if (this->intFieldD != -1) {
		this->flags_0x8 = this->flags_0x8 | 1;
	}
	this->cineBankLoadStage_0x2b4 = 4;
	if (this->pActor != (Actor*)0x0) {
		local_4 = &local_10;
		local_c = 0;
		local_10 = this;
		//(*(code*)((this->pActor->actorBase).pVTable)->interactWith)(this->pActor, 0, 0x7c, local_4);
	}
	return;
}

bool CCinematic::LoadCineBnk(bool mode)
{
	bool bVar1;
	edCBankBufferEntry* peVar2;
	edCBankInstall filePathContainer;
	char outFilePath[512];

	/* Loads a bank file from the cine folder
	   Example: CDEURO/LEVEL/PREINTRO/CINE/1c9de79b.bnk */
	edStrCatMulti(outFilePath, LevelScheduleManager::gThis->levelPath,
		LevelScheduleManager::gThis->aLevelInfo[LevelScheduleManager::gThis->currentLevelID].levelName, "\\Cine\\",
		this->pBankName_0x48, NULL);
	memset(&filePathContainer, 0, sizeof(edCBankInstall));
	filePathContainer.filePath = outFilePath;
	filePathContainer.fileFunc = _gCinCallback;
	filePathContainer.pObjectReference = this;
	peVar2 = this->cineBank.get_free_entry();
	(this->cineBank).pBankFileAccessObject = peVar2;
	filePathContainer.fileFlagA = 4;
	this->cineBankLoadStage_0x2b4 = 3;
	bVar1 = (this->cineBank).pBankFileAccessObject->load(&filePathContainer);
	if (bVar1 == false) {
		(this->cineBank).pBankFileAccessObject = (edCBankBufferEntry*)0x0;
		this->cineBank.terminate();
		this->cineBankLoadStage_0x2b4 = 0;
	}
	else {
		if (mode == false) {
			(this->cineBank).pBankFileAccessObject->wait();
		}
	}
	return this->cineBankLoadStage_0x2b4 == 3;
}

PACK(
struct AnimHeaderPacked {
	int hash;
	int offset;
	float field_0x8;
});


struct edAnmAnim {
	static edAnmAnim* LoadFromMem(char* pFileData, int size);
};

edAnmAnim* edAnmAnim::LoadFromMem(char* pInFileData, int size)
{
	int* pFileEnd;
	AnimHeaderPacked* pFileData = (AnimHeaderPacked*)pInFileData;

	pFileEnd = &pFileData->hash;
	if (pFileData != (AnimHeaderPacked*)0x0) {
		for (; pFileData < (AnimHeaderPacked*)((ulong)pFileEnd + size);
			pFileData = (AnimHeaderPacked*)((ulong)&pFileData->field_0x8 + pFileData->offset)) {
			if (pFileData->hash == 0x41544144) {
				return (edAnmAnim*)&pFileData->field_0x8;
			}
			if ((pFileData->hash == 0x214d4e41) && (pFileData->field_0x8 != 1.1f)) {
				return (edAnmAnim*)0x0;
			}
		}
	}
	return (edAnmAnim*)0x0;
}

struct Manager_208;

int* CCinematic::InstallResource(edResCollection::RES_TYPE objectType, bool type2, const char* fileName, ed_g2d_manager* textureObj, int* bufferLengthOut)
{
	Manager_208* pMVar1;
	bool bVar2;
	int result;
	ed_g2d_manager* outTextureInfo;
	ed_g3d_manager* outMeshInfo;
	edAnmAnim* outAnimationInfo;
	ulong uVar3;
	int iVar4;
	char* pcVar5;
	char* pcVar6;
	CinFileContainer* piVar7;
	uint counter;
	edBANK_ENTRY_INFO outFileData;
	byte localisedAudioFileName[524];
	int iStack4;
	char* suffix;
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
#if 0
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
					edStrCopy((char*)localisedAudioFileName, fileName);
					localisedAudioFileName[(int)(pcVar6 + -(int)fileName)] = 0;
					iVar4 = GetLanguageID_00336b30();
					edStrCat((char*)localisedAudioFileName, g_languageSuffixPtr[iVar4]);
					edStrCat((char*)localisedAudioFileName, "\\");
					edStrCat((char*)localisedAudioFileName, suffix);
					fileName = (char*)localisedAudioFileName;
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
						result = edStrICmp(*(byte**)(this->playingSounds_0x2c0 + iVar4), (byte*)fileName);
						if (result == 0) {
							/* We already have the sound file loaded, return it here */
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
			/* First cutscene object load falls into here
			   Get the appropriate bank file */
			if (type2 == 0) {
				bankObj = (this->cineBank).pBankFileAccessObject;
			}
			else {
				bankObj = (LevelScheduleManager::gThis->levelBank).pBankFileAccessObject;
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
				CUTSCENE_LOG(LogLevel::Info, "File Size: 0x%X (%d)\n", outFileData.size, outFileData.size);
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
						pMVar1 = Scene::ptable.g_Manager208_00451694;
						if (objectType == edResCollection::COT_Particle) {
							//IMPLEMENTATION_GUARD(
							//uVar3 = LoadCutsceneParticle
							//((ParticleData*)(this->particleSectionStart + this->numberOfParticles * 0x218),
							//	(byte*)outFileData.fileBufferStart, outFileData.size, bankObj);
							//iVar4 = -1;
							//if (uVar3 != 0) {
							//	iVar4 = this->numberOfParticles;
							//	this->numberOfParticles = iVar4 + 1;
							//}
							//)
							/* For particles we store the number of particle objects. */
							piVar7->pData = (char*)(iVar4 + 1);
						}
						else {
							if (objectType == edResCollection::COT_Text) {
								this->textData.select_language(bankObj, fileName, AUTO);
							}
							else {
								if (objectType == (edResCollection::RES_TYPE)6) {
									IMPLEMENTATION_GUARD(
									iVar4 = FUN_001ad730((int)Scene::ptable.g_Manager208_00451694, (int)outFileData.fileBufferStart);
									piVar7->pData = *(int*)(&pMVar1->field_0x8 + iVar4 * 4);)
								}
								else {
									if (objectType == edResCollection::COT_Animation) {
										outAnimationInfo = edAnmAnim::LoadFromMem(outFileData.fileBufferStart, outFileData.size);
										piVar7->pData = (char*)outAnimationInfo;
									}
									else {
										if (objectType == edResCollection::COT_MeshModel) {
											outMeshInfo = ed3DInstallG3D(outFileData.fileBufferStart, outFileData.size, 0, &iStack4,
												(TextureInfo*)textureObj, 0xc, (ed_g3d_manager*)0x0);
											piVar7->pData = (char*)outMeshInfo;
										}
										else {
											if (objectType == edResCollection::COT_MeshTexture) {
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
							IMPLEMENTATION_GUARD(
							outMeshInfo = 3DFileManager::GetMeshInfoAtAddress_001a6dd0
							(Scene::ptable.g_FileManager3D_00451664, (int)outFileData.fileBufferStart);)
						}
						else {
							outMeshInfo = (ed_g3d_manager*)outFileData.fileBufferStart;
							if (objectType == edResCollection::COT_MeshTexture) {
								IMPLEMENTATION_GUARD(
								outMeshInfo = (ed_g3d_manager*)
									3DFileManager::GetTextureInfoAtAddress_001a6d60
									(Scene::ptable.g_FileManager3D_00451664, (int)outFileData.fileBufferStart);)
							}
						}
					}
				}
			}
		}
	}
	return (int*)outMeshInfo;
}

void CCinematic::Manage()
{
	uint soundInfo;
	bool bVar1;
	int iVar2;
	CActorHero* pCVar3;
	float fVar4;

	if (this->intFieldC != 0x0) {
		//pCVar3 = (CActorHero*)this->field_0x90;
		//if ((CActorHero*)this->field_0x90 == (CActorHero*)0x0) {
		//	pCVar3 = CActorHero::_gThis;
		//}
		//if ((pCVar3 == (CActorHero*)0x0) ||
		//	(iVar2 = edEventComputeZoneAgainstVertex
		//	((Scene::ptable.g_EventManager_006f5080)->activeEventChunkID_0x8,
		//		(EventChunk_24*)this->intFieldC, &(pCVar3->character).characterBase.actorBase.currentLocation
		//		, 0), iVar2 == 2)) {
		//	this->flags_0x8 = this->flags_0x8 & 0xfffffdff;
		//	bVar1 = this->intOrPtrField != 0;
		//	if ((bVar1) && (((this->flags_0x4 & 8) != 0 && (bVar1)))) {
		//		this->flags_0x8 = this->flags_0x8 & 0xffffff7f;
		//		this->flags_0x8 = this->flags_0x8 | 0x100;
		//	}
		//}
		//else {
		//	if ((((this->flags_0x8 & 0x200) == 0) && (this->intOrPtrField == 0)) && ((this->flags_0x8 & 0x80) == 0)) {
		//		UsedInCutsceneManagerUpdateB(this, (Actor*)0x0, 0);
		//	}
		//}
	}
	bVar1 = this->field_0x24c != (int*)0x0;
	if (bVar1) {
		bVar1 = this->field_0x250 != 0;
	}
	if (bVar1) {
		//IMPLEMENTATION_GUARD(
		//UsedInCutsceneManagerUpdateC((MagicalSwitch_20*)this->field_0x250, (Actor*)0x0);)
	}
	bVar1 = this->field_0x254 != (int*)0x0;
	if (bVar1) {
		bVar1 = this->field_0x258 != 0;
	}
	if (bVar1) {
		//IMPLEMENTATION_GUARD(
		//UsedInCutsceneManagerUpdateC((MagicalSwitch_20*)this->field_0x258, (Actor*)0x0);)
	}
	if ((this->intOrPtrField != 0) && ((GameFlags & 0x20) == 0)) {
		// #HACK
		if (this->totalCutsceneDelta < 1.0f) {
			//IncrementCutsceneDelta();
		}
		else {
			this->totalCutsceneDelta = 1.0f;
		}
	}
	if (this->intOrPtrField == 0) {
		iVar2 = this->cineBankLoadStage_0x2b4;
		if (iVar2 != 4) {
			if (iVar2 == 3) {
				bVar1 = (this->cineBank).pBankFileAccessObject->is_loaded();
				if (bVar1 != false) {
					this->cineBankLoadStage_0x2b4 = 4;
				}
			}
			else {
				if (((iVar2 == 2) && (bVar1 = edSoundAreAllSoundDataLoaded(), bVar1 != false)) &&
					(bVar1 = edMusicAreAllMusicDataLoaded(), bVar1 != false)) {
					(this->cineBank).pBankFileAccessObject->close();
					(this->cineBank).pBankFileAccessObject = (edCBankBufferEntry*)0x0;
					LoadCineBnk(true);
				}
			}
		}
	}
	else {
		if (this->intOrPtrField == 2) {
			ManageState_Playing();
		}
		else {
			if (this->intOrPtrField == 1) {
				IMPLEMENTATION_GUARD(
				ManageState_InterpolateBegin(this);)
			}
		}
		if ((this != (CCinematic*)0xffffff4c) && (this->field_0x2c8 != -1.0f)) {
			IMPLEMENTATION_GUARD(
			fVar4 = UsedInCutsceneManagerUpdateH
			((this->pActor->actorBase).data.adjustedMagnitude, this->field_0x2c8, this->field_0x2cc,
				this->field_0x2d4, this->field_0x2d0);
			soundInfo = (this->cinematicLoadObject).BWCinSourceAudio_Obj.intFieldA;
			if (soundInfo != 0) {
				UsedInCutsceneManagerUpdateI(fVar4, soundInfo);
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
	//BWCinSourceAudio* cinematicSoundObject;
	float fVar8;
	float fVar9;
	CinematicManager* someGlobal;

	someGlobal = g_CinematicManager_0048efc;
	g_CinematicManager_0048efc->pCinematicB_0x18 = this;
	bVar2 = TimeSlice(this->totalCutsceneDelta);
	/* Check if the cutscene is over */
	someGlobal->pCinematicB_0x18 = (CCinematic*)0x0;
	if (bVar2 == false) {
		/* Cutscene has ended */
		if ((this->flags_0x4 & 8) == 0) {
			if (this->intOrPtrField != 0) {
				this->flags_0x8 = this->flags_0x8 & 0xffffff7f;
				this->flags_0x8 = this->flags_0x8 | 0x100;
			}
		}
		else {
			IMPLEMENTATION_GUARD(
			bVar2 = (this->cinematicLoadObject).BWCinSourceAudio_Obj.intFieldA != 0;
			cinematicSoundObject = &(this->cinematicLoadObject).BWCinSourceAudio_Obj;
			if ((bVar2) && (bVar2 = true, (this->cinematicLoadObject).BWCinSourceAudio_Obj.field_0x8 == 0.0)) {
				bVar2 = false;
			}
			if (bVar2) {
				someGlobal->pCinematicB_0x18 = this;
				(*(code*)cinematicSoundObject->vt->Stop)(cinematicSoundObject);
				iVar4 = CMessageFile::get_default_language();
				lVar6 = (long)(&this->intFieldE)[iVar4];
				if (lVar6 == -1) {
					lVar6 = (long)this->intFieldD;
				}
				BWCinSourceAudio::Create(cinematicSoundObject, lVar6);
				(*(code*)cinematicSoundObject->vt->Play)(cinematicSoundObject);
				someGlobal->pCinematicB_0x18 = (CCinematic*)0x0;
				this->totalCutsceneDelta = this->field_0x30;
			}
			else {
				this->totalCutsceneDelta =
					this->field_0x30 + (this->totalCutsceneDelta - *(float*)&((this->cinFileData).pCinTag)->field_0x4);
				if ((this->flags_0x8 & 1) != 0) {
					bVar2 = (this->cinematicLoadObject).BWCinSourceAudio_Obj.intFieldA != 0;
					fVar9 = this->totalCutsceneDelta;
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
	else {
		/* Cutscene is still playing */
		if (((((this->flags_0x4 & 2) != 0) || ((this->flags_0x8 & 0x800) != 0)) &&
			(((this->flags_0x8 & 0x1400) != 0 || ((this->flags_0x4 & 4) == 0)))) &&
			(((GameFlags & 0x1c) == 0 && (((uint)g_InputManager_00450960.pressedBitfield & 0x1000000) != 0)))) {
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
				if (this->intOrPtrField != 0) {
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

bool CCinematic::TimeSlice(float currentPlayTime)
{
	FrameInfo local_30;
	this->cinFileData.Timeslice(currentPlayTime, &local_30);
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
		deltaTime = 0.0;
	}
	/* This is true for cutscenes */
	//pBVar2 = &(this->cinematicLoadObject).BWCinSourceAudio_Obj;
	if ((this->flags_0x8 & 1) == 0) {
		/* Increment float by 0.2 in cutscene manager  */
		deltaTime = this->totalCutsceneDelta + deltaTime;
		this->totalCutsceneDelta = deltaTime;
		if (deltaTime < 0.0) {
			this->totalCutsceneDelta = 0.0;
		}
	}
	else {
		iVar1 = CMessageFile::get_default_language();
		iVar1 = (&this->intFieldE)[iVar1];
		if (iVar1 == -1) {
			iVar1 = this->intFieldD;
		}
		fVar3 = -1.0; // (float)(*(code*)pBVar2->vt->field_0x1c)(pBVar2, iVar1);
		if (fVar3 == -1.0) {
			this->totalCutsceneDelta = this->totalCutsceneDelta + deltaTime;
		}
		else {
			this->totalCutsceneDelta = fVar3;
		}
	}
	if (this->totalCutsceneDelta < 0.0) {
		this->totalCutsceneDelta = 0.0;
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

//char* LoadResource(edResCollection** resPtr, BWitchCin* loadObj, int offset, int* outSize)
//{
//	char* fileBuffer;
//	edResCollection* resSeekPos;
//	int iVar1;
//	char* resNameStart;
//	ResourceHeader* resInfo;
//
//	resSeekPos = *resPtr + 1;
//	resInfo = (ResourceHeader*)(resSeekPos + offset * 3);
//	if (resInfo->pData == 0x0) {
//		resNameStart = (char*)((ulong)&resSeekPos[(int)(*resPtr)->pData * 3].pData + 1);
//		for (iVar1 = offset + -1; -1 < iVar1; iVar1 = iVar1 + -1) {
//			resNameStart = resNameStart + (byte)resNameStart[-1];
//		}
//		/* This will call LoadCutsceneFile and return a pointer to the start of the file buffer */
//		fileBuffer = loadObj->GetResource((edResCollectionEnum)(resInfo->type & 0x7fffffff), (resInfo->type & 0x80000000) != 0, resNameStart, &resInfo->size);
//		resInfo->pData = (int)STORE_SECTION(fileBuffer);
//	}
//	if (outSize != (int*)0x0) {
//		*outSize = resInfo->size;
//	}
//	return (char*)LOAD_SECTION(resInfo->pData);
//}

bool CBWitchCin::CreateActor(edCinActorInterface** ppActorInterface, const edCinGameInterface::ACTORV_CREATIONtag* pTag)
{
	TextureInfo* textureObj;
	ed_g3d_manager* meshInfoObj;
	long lVar1;
	edCinActorInterface* cutsceneObject;
	int iStack4;
	CCinematic* pCinematic;

	meshInfoObj = (ed_g3d_manager*)0x0;
	textureObj = (TextureInfo*)0x0;
	pCinematic = g_CinematicManager_0048efc->pCinematicB_0x18;
	if ((pTag->meshName != 0) && ((char*)pTag->textureName != (char*)0x0)) {
		textureObj = (TextureInfo*)
			pCinematic->InstallResource(edResCollection::COT_MeshTexture, (ulong)pTag->bHasTexture,
				(char*)pTag->textureName, (ed_g2d_manager*)0x0, &iStack4);
		if (textureObj == (TextureInfo*)0x0) {
			IMPLEMENTATION_GUARD();
			//textureObj = 3DFileManager::LoadDefaultTexture_001a65d0(Scene::ptable.g_FileManager3D_00451664);
		}
		meshInfoObj = (ed_g3d_manager*)
			pCinematic->InstallResource(edResCollection::COT_MeshModel, (ulong)pTag->bHasMesh, (char*)pTag->meshName
				, (ed_g2d_manager*)textureObj, &iStack4);
	}
	/* Sound files can go in here */
	//lVar1 = Cinematic::NewCinematicActor(pCinematic, (char*)pTag, meshInfoObj, (ed_g2d_manager*)textureObj);
	//cutsceneObject = (edCinActorInterface*)((int)lVar1 + 0x180);
	//Cinematic::Func(cutsceneObject, &pTag->vectorFieldA, &pTag->vectorFieldB,

	//	&pTag->vectorFieldC, (ed_g3d_manager*)0x0);
	*ppActorInterface = (edCinActorInterface*)STORE_SECTION(cutsceneObject);
	return true;
}

bool CBWitchCin::ReleaseResource(void*, bool)
{
	return true;
}

bool CBWitchCin::GetCamera(edCinCamInterface** pCinCam, const edCinCamInterface::CAMERA_CREATIONtag*)
{
	*pCinCam = &this->BWCinCam_Obj;
	g_CinematicManager_0048efc->pCinematicB_0x18->flags_0x8 = g_CinematicManager_0048efc->pCinematicB_0x18->flags_0x8 | 2;
	return true;
}

char* CBWitchCin::GetResource(edResCollection::RES_TYPE type1, long type2, const char* fileName, int* bufferLengthOut)
{
	int* fileBufferStart;

	CUTSCENE_LOG(LogLevel::Info, "BWitchCin::GetResource %s\n", fileName);

	if ((type1 == edResCollection::COT_MeshModel) || (type1 == edResCollection::COT_MeshTexture)) {
		*bufferLengthOut = 0;
		fileBufferStart = (int*)1;
	}
	else {
		fileBufferStart = g_CinematicManager_0048efc->pCinematicB_0x18->InstallResource(type1, type2, fileName, (ed_g2d_manager*)0x0, bufferLengthOut);
	}
	return (char*)fileBufferStart;
}

bool CBWCinCam::Activate()
{
	CCinematic* pCVar1;
	CCameraCinematic* pCVar2;
	int iVar3;
	SWITCH_MODE SVar4;
	float fVar5;
	float fVar6;

	pCVar1 = g_CinematicManager_0048efc->pCinematicB_0x18;
	pCVar2 = g_CinematicManager_0048efc->pCameraLocationObj;
	if (((pCVar1->flags_0x4 & 0x800) == 0) && ((pCVar1->flags_0x8 & 0x800) == 0)) {
		iVar3 = pCVar1->field_0x64;
		fVar5 = pCVar1->field_0x68;
		SVar4 = (SWITCH_MODE)pCVar1->field_0x5c;
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
	if (((g_CinematicManager_0048efc->pCinematicB_0x18->flags_0x4 & 0x200) != 0) &&
		(CameraManager::_gThis->aspectRatio == 1.777778f)) {
		fov = fov * 0.75f;
	}
	g_CinematicManager_0048efc->pCameraLocationObj->fov = fov;
	return true;
}

bool CBWCinCam::SetPos(float x, float y, float z)
{
	CinematicManager* pCVar1;
	edF32VECTOR4 outLocation;
	CCameraCinematic* pCinCam;

	pCVar1 = g_CinematicManager_0048efc;
	outLocation.w = 1.0f;;
	outLocation.x = x;
	outLocation.y = y;
	outLocation.z = z;
	sceVu0ApplyMatrix(&outLocation, &g_CinematicManager_0048efc->pCinematicB_0x18->matrix_0x120, &outLocation);
	pCinCam = pCVar1->pCameraLocationObj;
	pCinCam->transformationMatrix.rowT = outLocation;
	return true;
}

bool CBWCinCam::SetHeadingQuat(float x, float y, float z, float w)
{
	CCinematic* pCVar1;
	CinematicManager* pCVar2;
	edF32VECTOR4 local_50;
	edF32MATRIX4 eStack64;
	CCameraCinematic* pCinCam;

	pCVar2 = g_CinematicManager_0048efc;
	pCVar1 = g_CinematicManager_0048efc->pCinematicB_0x18;
	local_50.x = x;
	local_50.y = y;
	local_50.z = z;
	local_50.w = w;
	edQuatToMatrix4Hard(&local_50, &eStack64);
	edF32Matrix4MulF32Matrix4Hard(&eStack64, &eStack64, &pCVar1->matrix_0x120);
	pCinCam = pCVar2->pCameraLocationObj;
	eStack64.rowT = pCinCam->transformationMatrix.rowT;
	pCinCam->SetTransform(&eStack64);
	return true;
}

bool CBWCinCam::SetHeadingEuler(float x, float y, float z, bool param_5)

{
	CCinematic* pCVar1;
	CCameraCinematic* pCVar2;
	float* pfVar3;
	CinematicManager* pCVar4;
	float* pfVar5;
	float* pfVar6;
	edF32MATRIX4 auStack96;
	edF32VECTOR4 newLookAt;
	float local_10;
	float local_c;
	float local_8;

	pCVar4 = g_CinematicManager_0048efc;
	pCVar1 = g_CinematicManager_0048efc->pCinematicB_0x18;
	if (param_5 == false) {
		newLookAt.w = 1.0;
		newLookAt.x = x;
		newLookAt.y = y;
		newLookAt.z = z;
		sceVu0ApplyMatrix(&newLookAt, &pCVar1->matrix_0x120, &newLookAt);
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
	pCVar2 = pCVar4->pCameraLocationObj;
	if (param_5 != false) {
		edF32Vector4AddHard(&newLookAt, &pCVar2->transformationMatrix.rowT, &auStack96.rowZ);
	}
	pCVar2->lookAt = newLookAt;
	return true;
}
