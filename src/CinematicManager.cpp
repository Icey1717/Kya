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
	pCinematic = (Cinematic*)0x0;
	field_0x20 = -1;
	field_0x24 = -1;
	g_CinematicManager_0048efc = this;
	ppCinematicObjB_A = (Cinematic**)0x0;
	numCutscenes_0x8 = 0;
	ppCinematicObjB_B = (Cinematic**)0x0;
	count_0x14 = 0;
	pCinematicObjB_0x18 = (Cinematic*)0x0;
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
	CameraCinematic* pCinematicCamera;
	CameraCinematic* pCamera;
	CameraManager* pCameraManager;

	pCameraManager = CameraManager::_gThis;
	pCinematicCamera = (CameraCinematic*)CameraManager::_gThis->GetDefGameCamera(CT_Cinematic);
	this->pCameraLocationObj = pCinematicCamera;
	if (this->pCinematic != (Cinematic*)0x0) {
		this->pCameraLocationObj->SetTransform(&this->matrix_0x70);
		pCameraLocationObj->field_0x74 = this->field_0xb0;
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
	Cinematic* pCinematic;
	bool bVar1;
	CinematicManager* pCVar2;

	pCVar2 = g_CinematicManager_0048efc;
	pCinematic = this->pCinematic;
	if (pCinematic != (Cinematic*)0x0) {
		IMPLEMENTATION_GUARD(
			bVar1 = (pCinematic->cinematicLoadObject).soundObject.intFieldA != 0;
		if ((bVar1) && (bVar1 = true, (pCinematic->cinematicLoadObject).soundObject.field_0x8 != 0.0)) {
			bVar1 = false;
		}
		if (bVar1) {
			g_CinematicManager_0048efc->pCinematicObjB_0x18 = pCinematic;
			(**(code**)(*(int*)&(pCinematic->cinematicLoadObject).soundObject + 0x10))();
			pCVar2->pCinematicObjB_0x18 = (Cinematic*)0x0;
		}
		bVar1 = (pCinematic->cineBank).pBankFileAccessObject != (edCBankBufferEntry*)0x0;
		if (bVar1) {
			bVar1 = pCinematic->cineBankLoadStage_0x2b4 != 4;
		}
		if (bVar1) {
			Cinematic::Load(pCinematic, 0);
		}
		if (pCinematic->intOrPtrField != 0) {
			ActorGlobalFunc_001c84a0(pCinematic);
		}
		pCinematic->flags_0x8 = pCinematic->flags_0x8 & 0xfffffffb;
		pCinematic->pActor = (Actor*)0x0;
		GlobalDList_AddToView();
		edVideoFlip();
		Cinematic::Manage((long)(int)this->pCinematic, 1);
		this->pCinematic = (Cinematic*)0x0;);
	}
	this->pCameraLocationObj = (CameraCinematic*)0x0;
	return;
}

bool CinematicManager::LevelLoading_Manage()
{
	bool bVar1;

	if (this->pCinematic == (Cinematic*)0x0) {
		bVar1 = false;
	}
	else {
		this->field_0x38 = 0;
		this->field_0x40 = 0;
		IMPLEMENTATION_GUARD(Cinematic::Manage(this->pCinematic));
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
	CameraCinematic* pCVar4;
	Cinematic** pCVar5;
	int iVar5;

	pCVar4 = (CameraCinematic*)CameraManager::_gThis->GetDefGameCamera(CT_Cinematic);
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
	Cinematic** ppCVar2;
	Cinematic* this_00;
	int iVar4;
	long lVar3;

	edMemSetFlags(TO_HEAP(H_MAIN), 0x100);
	pByteCode->GetChunk();
	iVar1 = pByteCode->GetS32();
	this->numCutscenes_0x8 = iVar1;
	this->count_0x14 = 0;
	iVar1 = this->numCutscenes_0x8;
	if (iVar1 != 0) {
		ppCVar2 = new Cinematic*[iVar1];
		this->ppCinematicObjB_A = ppCVar2;
		ppCVar2 = new Cinematic*[this->numCutscenes_0x8];
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
				this_00 = new Cinematic();
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

void CinematicManager::Level_SectorChange(int param_2, int param_3)
{
	Cinematic* pCVar1;
	bool bVar2;
	Cinematic** ppCVar3;
	int iVar4;

	iVar4 = this->numCutscenes_0x8;
	ppCVar3 = this->ppCinematicObjB_A;
	this->count_0x14 = 0;
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
			this->ppCinematicObjB_B[this->count_0x14] = pCVar1;
			this->count_0x14 = this->count_0x14 + 1;
		}
		ppCVar3 = ppCVar3 + 1;
	}
	if (((param_2 == -1) && (pCVar1 = this->pCinematic, pCVar1 != (Cinematic*)0x0)) &&
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
	Cinematic** pCVar6;
	int cutsceneID;
	Cinematic* pCinematic;

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

void Cinematic::InitInternalData()
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

Cinematic::Cinematic()
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

void Cinematic::Create(ByteCode* pByteCode)
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

void Cinematic::Init()
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

void Cinematic::Start()
{
	return;
	Actor* pAVar1;
	float fVar2;
	uint uVar3;
	uint uVar4;
	//APlayer* pAVar5;
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
	//CutsceneTimeObject CStack224;
	edF32MATRIX4 eStack176;
	edF32MATRIX4 local_70;
	edBANK_ENTRY_INFO eStack48;
	Cinematic* local_18;
	undefined4 local_14;
	Cinematic** local_10;
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
				pCVar6->pCinematicObjB_0x18 = this;
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
				this->cinFileData.Create(&this->cinematicLoadObject, this->fileName);
				local_70.aa = gF32Matrix4Unit.aa;
				local_70.ab = gF32Matrix4Unit.ab;
				local_70.ac = gF32Matrix4Unit.ac;
				local_70.ad = gF32Matrix4Unit.ad;
				local_70.ba = gF32Matrix4Unit.ba;
				local_70.bb = gF32Matrix4Unit.bb;
				local_70.bc = gF32Matrix4Unit.bc;
				local_70.bd = gF32Matrix4Unit.bd;
				local_70.ca = gF32Matrix4Unit.ca;
				local_70.cb = gF32Matrix4Unit.cb;
				local_70.cc = gF32Matrix4Unit.cc;
				local_70.cd = gF32Matrix4Unit.cd;
				local_70.da = gF32Matrix4Unit.da;
				local_70.db = gF32Matrix4Unit.db;
				local_70.dc = gF32Matrix4Unit.dc;
				local_70.dd = gF32Matrix4Unit.dd;
				if ((this->flags_0x4 & 0x2000) != 0) {
					//iVar16 = this->field_0x44;
					//if (iVar16 != 0) {
					//	edF32Matrix4FromEulerSoft(&local_70, *(int*)(iVar16 + 0x18) + 0x18, s_XYZ_0042b950);
					//	local_70.da = *(float*)(iVar16 + 0x70);
					//	local_70.db = *(float*)(iVar16 + 0x74);
					//	local_70.dc = *(float*)(iVar16 + 0x78);
					//	local_70.dd = *(float*)(iVar16 + 0x7c);
					//	edF32Matrix4GetInverseOrthoHard(&local_70, &local_70);
					//}
					//if (this->pActor != (Actor*)0x0) {
					//	edF32Matrix4FromEulerSoft(&eStack176, (int)&(this->pActor->actorBase).data.rotationVector, s_XYZ_0042b950);
					//	pAVar1 = this->pActor;
					//	eStack176.da = (pAVar1->actorBase).data.currentLocation.x;
					//	eStack176.db = (pAVar1->actorBase).data.currentLocation.y;
					//	eStack176.dc = (pAVar1->actorBase).data.currentLocation.z;
					//	eStack176.dd = (pAVar1->actorBase).data.currentLocation.w;
					//	edF32Matrix4MulF32Matrix4Hard(&local_70, &local_70, &eStack176);
					//}
				}
				//peVar13 = &local_70;
				//peVar11 = &this->matrix_0x120;
				//iVar16 = 8;
				//do {
				//	iVar16 = iVar16 + -1;
				//	fVar2 = peVar13->ab;
				//	peVar11->aa = peVar13->aa;
				//	peVar13 = (edF32MATRIX4*)&peVar13->ac;
				//	peVar11->ab = fVar2;
				//	peVar11 = (edF32MATRIX4*)&peVar11->ac;
				//} while (0 < iVar16);
				//if (this->prtBuffer == 1) {
				//	FUN_001b9b70(0, (int)Scene::_pinstance, (int)&this->field_0x34);
				//}
				//if (this->pMeshInfo != (ed_g3d_manager*)0x0) {
				//	ed3DScenePushCluster(scene::_scene_handle, this->pMeshInfo);
				//	iVar16 = FUN_00295c40((int)this->pMeshInfo, 0x43494d414e5944);
				//	if (iVar16 != 0) {
				//		FUN_0029f330(iVar16, 0xffff);
				//	}
				//	pMVar9 = ed3DHierarchyAddToSceneByHashcode(scene::_scene_handle, this->pMeshInfo, 0x43494d414e5944);
				//	this->pMeshTransform = pMVar9;
				//	if (this->pMeshTransform != (MeshTransformParent*)0x0) {
				//		SetMeshTransformFlag_002abd80(this->pMeshTransform, 0xffff);
				//		ClearMeshTransformFlag_002abff0(this->pMeshTransform, 0xffff);
				//	}
				//}
				//pAVar5 = ActorHero::_gThis;
				//if (ActorHero::_gThis != (APlayer*)0x0) {
				//	if ((this->flags_0x4 & 0x10) != 0) {
				//		local_c = 0;
				//		ActorCompareFunc_00103f00
				//		((AActorBase*)ActorHero::_gThis, (AActorBase*)ActorHero::_gThis, 0x82, (ActorCompareStruct*)0x0);
				//	}
				//	if ((this->flags_0x4 & 0x10000) != 0) {
				//		(pAVar5->character).characterBase.actorBase.flags =
				//			(pAVar5->character).characterBase.actorBase.flags | 0x400000;
				//	}
				//}
				//if ((this->flags_0x4 & 0x400) != 0) {
				//	FrontendManager::Func_001cf740(Scene::ptable.g_FrontendManager_00451680, 0);
				//	GameFlags = GameFlags | 0x100;
				//}
				//if ((this->flags_0x4 & 0x20000200) != 0) {
				//	GameFlags = GameFlags | 0x80;
				//}
				//uVar3 = this->flags_0x4;
				//if ((uVar3 & 0x280000) != 0) {
				//	FUN_00182db0(0x3f800000, (int)Scene::ptable.g_GlobalSoundPtr_00451698, uVar3 & 0x80000, uVar3 & 0x200000);
				//}
				//if ((this->flags_0x4 & 0x100) == 0) {
				//	this->totalCutsceneDelta = 0.0;
				//}
				//if ((this->flags_0x4 & 0x1000000) != 0) {
				//	this->flags_0x8 = this->flags_0x8 | 0x40;
				//}
				//bVar7 = false;
				//if ((this->flags_0x8 & 0x800) == 0) {
				//	uVar3 = this->objCount_0x9c;
				//	iVar16 = 0;
				//	if (0 < (int)uVar3) {
				//		iVar15 = 0;
				//		do {
				//			ppiVar14 = (int**)((int)&this->field_0xa0->field_0x0 + iVar15);
				//			piVar10 = *ppiVar14;
				//			if (piVar10 != (int*)0x0) {
				//				if (((uint)ppiVar14[1] & 0x10) != 0) {
				//					uVar4 = piVar10[2];
				//					piVar19 = (int*)piVar10[0xd];
				//					piVar17 = (int*)piVar10[0xe];
				//					piVar18 = (int*)piVar10[0xf];
				//					ppiVar14[4] = (int*)piVar10[0xc];
				//					ppiVar14[5] = piVar19;
				//					ppiVar14[6] = piVar17;
				//					ppiVar14[7] = piVar18;
				//					piVar19 = (int*)piVar10[0x15];
				//					piVar17 = (int*)piVar10[0x16];
				//					piVar18 = (int*)piVar10[0x17];
				//					ppiVar14[8] = (int*)piVar10[0x14];
				//					ppiVar14[9] = piVar19;
				//					ppiVar14[10] = piVar17;
				//					ppiVar14[0xb] = piVar18;
				//					if ((uVar4 & 0x1000) == 0) {
				//						ppiVar14[1] = (int*)((ulong)((long)(int)ppiVar14[1] << 0x21) >> 0x21);
				//					}
				//					else {
				//						ppiVar14[1] = (int*)((uint)ppiVar14[1] | 0x80000000);
				//					}
				//				}
				//				if (((uint)ppiVar14[1] & 1) != 0) {
				//					(**(code**)(*piVar10 + 0xa8))();
				//					bVar7 = true;
				//				}
				//			}
				//			iVar16 = iVar16 + 1;
				//			iVar15 = iVar15 + 0x30;
				//		} while (iVar16 < (int)uVar3);
				//	}
				//	if ((this->flags_0x4 & 0x1000) != 0) {
				//		bVar7 = true;
				//	}
				//}
				//if (bVar7) {
				//	this->intOrPtrField = 1;
				//	pTVar8 = GetTimer();
				//	*(float*)&this->field_0x88 = pTVar8->scaledTotalTime;
				//}
				//else {
				//	this->intOrPtrField = 2;
				//	pTVar8 = GetTimer();
				//	*(float*)&this->field_0x88 = pTVar8->scaledTotalTime;
				//	FUN_0031f3c0(&this->cinFileData);
				//	this->flags_0x8 = this->flags_0x8 | 0x10;
				//}
				//if ((this->flags_0x8 & 2) != 0) {
				//	(*(code*)((this->cinematicLoadObject).BWCinCam_Obj.vt)->Activate)();
				//	SetupCutscene(0.0, &this->cinFileData, &CStack224);
				//	CameraManager::PushCamera(CameraManager::_gThis, (Camera*)pCVar6->pCameraLocationObj, 1);
				//}
				//if ((this->prtBuffer == 1) || ((this->flags_0x4 & 8) != 0)) {
				//	edMemClearFlags(H_MAIN, 0x100);
				//}
				//pCVar6->pCinematicObjB_0x18 = (Cinematic*)0x0;
				//if ((this->flags_0x4 & 2) == 0) {
				//	this->flags_0x8 = this->flags_0x8 | 0x1400;
				//}
				//if (this->pActor != (Actor*)0x0) {
				//	local_14 = 2;
				//	local_10 = &local_18;
				//	local_18 = this;
				//	(*(code*)((this->pActor->actorBase).pVTable)->interactWith)(this->pActor, 0, 0x7c, local_10);
				//}
			}
			else {
				g_CinematicManager_0048efc->field_0x28 = this;
				pTVar8 = GetTimer();
				pCVar6->field_0x2c = pTVar8->totalTime;
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

void Cinematic::Load(long mode)
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


bool Cinematic::LoadInternal(long mode)
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
		((Cinematic*)pObj)->Install();
	}
	return;
}

void Cinematic::Install()
{
	float fVar1;
	undefined4 uVar2;
	void* pvVar3;
	uint uVar4;
	int iVar5;
	edF32MATRIX4* peVar6;
	edF32MATRIX4* peVar7;
	Cinematic* local_10;
	undefined4 local_c;
	Cinematic** local_4;
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

bool Cinematic::LoadCineBnk(bool mode)
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

int* Cinematic::InstallResource(edResCollectionEnum objectType, long type2, char* fileName, ed_g2d_manager* textureObj,	int* bufferLengthOut)
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
	if (objectType == (edResCollectionEnum)10) {
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
		if (objectType == COT_Sound) {
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
						if (objectType == COT_Particle) {
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
							if (objectType == COT_Text) {
								this->textData.select_language(bankObj, fileName, AUTO);
							}
							else {
								if (objectType == (edResCollectionEnum)6) {
									IMPLEMENTATION_GUARD(
									iVar4 = FUN_001ad730((int)Scene::ptable.g_Manager208_00451694, (int)outFileData.fileBufferStart);
									piVar7->pData = *(int*)(&pMVar1->field_0x8 + iVar4 * 4);)
								}
								else {
									if (objectType == COT_Animation) {
										outAnimationInfo = edAnmAnim::LoadFromMem(outFileData.fileBufferStart, outFileData.size);
										piVar7->pData = (char*)outAnimationInfo;
									}
									else {
										if (objectType == COT_MeshModel) {
											outMeshInfo = ed3DInstallG3D(outFileData.fileBufferStart, outFileData.size, 0, &iStack4,
												(TextureInfo*)textureObj, 0xc, (ed_g3d_manager*)0x0);
											piVar7->pData = (char*)outMeshInfo;
										}
										else {
											if (objectType == COT_MeshTexture) {
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
					if (objectType == COT_Animation) {
						outMeshInfo = (ed_g3d_manager*)edAnmAnim::LoadFromMem(outFileData.fileBufferStart, outFileData.size);
					}
					else {
						if (objectType == COT_MeshModel) {
							IMPLEMENTATION_GUARD(
							outMeshInfo = 3DFileManager::GetMeshInfoAtAddress_001a6dd0
							(Scene::ptable.g_FileManager3D_00451664, (int)outFileData.fileBufferStart);)
						}
						else {
							outMeshInfo = (ed_g3d_manager*)outFileData.fileBufferStart;
							if (objectType == COT_MeshTexture) {
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

bool edCinematic::ExtractVersions(int size, int* cinematicLibraryVersion, int* cinematicCompilerVersion)
{
	edCinematic* peVar1;
	edCinematic* local_4;

	peVar1 = (edCinematic*)((ulong)&this->headerPtr + size);
	local_4 = this;
	if (this < peVar1) {
		do {
			if (local_4->headerPtr == (char*)0x214e4943) {
				if (cinematicLibraryVersion != (int*)0x0) {
					*cinematicLibraryVersion = (int)local_4->pRes;
				}
				if (cinematicCompilerVersion != (int*)0x0) {
					*cinematicCompilerVersion = (int)local_4[1].headerPtr;
				}
				return true;
			}
			local_4 = (edCinematic*)((ulong)(local_4->fileStart + 2) + (ulong)((ulong)&local_4->pRes + -8));
		} while (local_4 < peVar1);
	}
	return false;
}

PACK(
struct ResourceHeader {
	uint type;
	int pData; // char*
	int size;
});

char* LoadResource(edResCollection** resPtr, BWitchCin* loadObj, int offset, int* outSize)
{
	char* fileBuffer;
	edResCollection* resSeekPos;
	int iVar1;
	char* resNameStart;
	ResourceHeader* resInfo;

	resSeekPos = *resPtr + 1;
	resInfo = (ResourceHeader*)(resSeekPos + offset * 3);
	if (resInfo->pData == 0x0) {
		resNameStart = (char*)((ulong)&resSeekPos[(int)(*resPtr)->pData * 3].pData + 1);
		for (iVar1 = offset + -1; -1 < iVar1; iVar1 = iVar1 + -1) {
			resNameStart = resNameStart + (byte)resNameStart[-1];
		}
		/* This will call LoadCutsceneFile and return a pointer to the start of the file buffer */
		fileBuffer = loadObj->GetResource((edResCollectionEnum)(resInfo->type & 0x7fffffff), (resInfo->type & 0x80000000) != 0, resNameStart, &resInfo->size);
		resInfo->pData = (int)STORE_SECTION(fileBuffer);
	}
	if (outSize != (int*)0x0) {
		*outSize = resInfo->size;
	}
	return (char*)LOAD_SECTION(resInfo->pData);
}

PACK(
struct edCinematicSourceInternal {
	int type;
	int offset;
	int pFileData; // char*
});

struct edCinematicSource {
	edCinematicSourceInternal* pInternal;
};

struct edScene {
	char* field_0x0;
	char* Create(char* inFileBuffer, int fileLength, BWitchCin* loadObj);
};

struct CameraInfo {

};

PACK(
struct CineCreatureObjectHeader {
	undefined field_0x0;
	undefined field_0x1;
	undefined field_0x2;
	undefined field_0x3;
	undefined field_0x4;
	undefined field_0x5;
	undefined field_0x6;
	undefined field_0x7;
	undefined field_0x8;
	undefined field_0x9;
	undefined field_0xa;
	undefined field_0xb;
	undefined field_0xc;
	undefined field_0xd;
	undefined field_0xe;
	undefined field_0xf;
	undefined field_0x10;
	undefined field_0x11;
	undefined field_0x12;
	undefined field_0x13;
	int cutNameActor; // CutsceneNameActorB*
});

PACK(
struct CineCreatureObject {
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
	bool Create(BWitchCin* loadObj, edResCollection** param_3);
};

char* edResCollection::GetResFilename(edResCollection** resBufferPtr, int offset)
{
	char* resFilePath;
	int iVar1;

	/* Dereference the pointer to the res buffer */
	/* Dereference the res buffer again and load into v1 */
	resFilePath = (char*)((ulong) & (*resBufferPtr)[(int)(*resBufferPtr)->pData * 3 + 1].pData + 1);
	/* Reduce param 2 by 1 */
	/* Shift the bits left by 1 (4 will become 8) */
	/* Add v0 to v1 */
	/* Shift v0 another 2 bits to the left */
	/* Move v0 bits along the res buffer */
	/* Check if num < 0 */
	for (iVar1 = offset + -1; -1 < iVar1; iVar1 = iVar1 + -1) {
		resFilePath = resFilePath + (byte)resFilePath[-1];
		/* Reduce num by 1 */
		/* Repeat if num is still > 0 */
	}
	return resFilePath;
}

bool BWitchCin::CreateActor(int* ppActorInterface, edCinGameInterface* ppGameInterface)
{
	TextureInfo* textureObj;
	ed_g3d_manager* meshInfoObj;
	long lVar1;
	edCinActorInterface* cutsceneObject;
	int iStack4;
	Cinematic* pCinematic;

	meshInfoObj = (ed_g3d_manager*)0x0;
	textureObj = (TextureInfo*)0x0;
	pCinematic = g_CinematicManager_0048efc->pCinematicObjB_0x18;
	if ((ppGameInterface->meshName != 0) && ((char*)ppGameInterface->textureName != (char*)0x0)) {
		textureObj = (TextureInfo*)
			pCinematic->InstallResource(COT_MeshTexture, (ulong)ppGameInterface->bHasTexture,
				(char*)ppGameInterface->textureName, (ed_g2d_manager*)0x0, &iStack4);
		if (textureObj == (TextureInfo*)0x0) {
			IMPLEMENTATION_GUARD();
			//textureObj = 3DFileManager::LoadDefaultTexture_001a65d0(Scene::ptable.g_FileManager3D_00451664);
		}
		meshInfoObj = (ed_g3d_manager*)
			pCinematic->InstallResource(COT_MeshModel, (ulong)ppGameInterface->bHasMesh, (char*)ppGameInterface->meshName
				, (ed_g2d_manager*)textureObj, &iStack4);
	}
	/* Sound files can go in here */
	//lVar1 = Cinematic::NewCinematicActor(pCinematic, (char*)ppGameInterface, meshInfoObj, (ed_g2d_manager*)textureObj);
	//cutsceneObject = (edCinActorInterface*)((int)lVar1 + 0x180);
	//Cinematic::Func(cutsceneObject, &ppGameInterface->vectorFieldA, &ppGameInterface->vectorFieldB,
	//	&ppGameInterface->vectorFieldC, (ed_g3d_manager*)0x0);
	*ppActorInterface = STORE_SECTION(cutsceneObject);
	return true;
}

bool edSceneActorVirtual::Create(BWitchCin* loadObj, edResCollection** param_3)
{
	bool bVar1;
	edCinGameInterface playerName;
	CineCreatureObject* cineCreatureObject;

	/* This sets up objects for a cutscene, and gives them a name
	   Example: SC_SOUND_EMITTER */
	cineCreatureObject = this->pObj;
	playerName.vectorFieldA.x = (cineCreatureObject->field_0x50).x;
	playerName.vectorFieldA.y = (cineCreatureObject->field_0x50).y;
	playerName.vectorFieldA.z = (cineCreatureObject->field_0x50).z;
	playerName.vectorFieldA.w = 1.0;
	playerName.vectorFieldB.x = (cineCreatureObject->field_0x50).w;
	playerName.vectorFieldB.y = (cineCreatureObject->field_0x60).x;
	playerName.vectorFieldB.z = (cineCreatureObject->field_0x60).y;
	playerName.vectorFieldB.w = (cineCreatureObject->field_0x60).z;
	playerName.vectorFieldC.x = (cineCreatureObject->field_0x60).w;
	playerName.vectorFieldC.y = *(float*)&cineCreatureObject->field_0x70;
	playerName.vectorFieldC.z = *(float*)&cineCreatureObject->field_0x74;
	playerName.vectorFieldC.w = 1.0;
	playerName.field_0x40 = (cineCreatureObject->field_0x38).x;
	playerName.field_0x44 = (cineCreatureObject->field_0x38).y;
	playerName.field_0x48 = (cineCreatureObject->field_0x38).z;
	playerName.field_0x4c = (cineCreatureObject->field_0x38).w;
	/* Loads the name of the player into this buffer.
	   Example: ARAIGNOSBLACK_TOONPLAYER_L0 */
	strcpy((char*)&playerName, cineCreatureObject->name);
	if (cineCreatureObject->textureID == -1) {
		playerName.textureName = 0;
		playerName.bHasTexture = false;
	}
	else {
		playerName.textureName = edResCollection::GetResFilename(param_3, cineCreatureObject->textureID);
		playerName.bHasTexture = ((uint)(*param_3)[cineCreatureObject->textureID * 3 + 1].pData & 0x80000000) != 0;
	}
	if (cineCreatureObject->meshID == -1) {
		playerName.meshName = 0;
		playerName.bHasMesh = false;
	}
	else {
		playerName.meshName = edResCollection::GetResFilename(param_3, cineCreatureObject->meshID);
		playerName.bHasMesh = ((uint)(*param_3)[cineCreatureObject->meshID * 3 + 1].pData & 0x80000000) != 0;
	}
	playerName.field_0x30 = (cineCreatureObject->field_0x38).w;
	bVar1 = loadObj->CreateActor(&(this->pObj->header).cutNameActor, &playerName);
	return bVar1 != false;
}

char* edScene::Create(char* inFileBuffer, int fileLength, BWitchCin* loadObj)
{
	int iVar1;
	char* dataPtr;
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
	edResCollection* resPtr;
	char* fileBuffer;

	if (inFileBuffer == (char*)0x0) {
		dataPtr = (char*)0x0;
	}
	else {
		resPtr = (edResCollection*)0x0;
		dataPtr = (char*)0x0;
		fileBuffer = inFileBuffer;
		if (inFileBuffer < inFileBuffer + fileLength) {
			do {
				seekCounter = *(int*)fileBuffer;
				/* If the data we read from the buffer == 'DATA' */
				if (seekCounter == 0x21534552) {
					resPtr = (edResCollection*)(fileBuffer + 8);
				}
				else {
					/* If the data we read from the buffer == 'RES!' */
					if (seekCounter == 0x41544144) {
						dataPtr = fileBuffer + 8;
					}
					else {
						/* If the data we read from the buffer == 'SCE!' */
						if ((seekCounter == 0x21454353) && (*(int*)(fileBuffer + 8) != 100)) {
							return (char*)0x0;
						}
					}
				}
				fileBuffer = fileBuffer + *(int*)(fileBuffer + 4) + 8;
			} while (fileBuffer < inFileBuffer + fileLength);
		}
		/* Seems to have searched through the file looking for Data4?
		   Also potentially found the RES section */
		if (dataPtr == (char*)0x0) {
			dataPtr = (char*)0x0;
		}
		else {
			this->field_0x0 = dataPtr;
			seekCounter = 0;
			*(int*)(this->field_0x0 + 0x1c) = STORE_SECTION(resPtr);
			dataPtr = (char*)resPtr->pData;
			filePath = (char*)((ulong)&resPtr[(ulong)dataPtr * 3 + 1].pData + 1);
			if (0 < (ulong)dataPtr) {
				seekIncrement = 0;
				do {
					internalSeekPos = (char*)((ulong)&resPtr->pData + seekIncrement);
					uVar2 = *(uint*)(internalSeekPos + 4) & 0x7fffffff;
					if (uVar2 == 7) {
						/* Set some value along the res ptr */
						*(char**)(internalSeekPos + 8) = filePath;
					}
					else {
						if ((uVar2 != 1) && (uVar2 != 4)) {
							/* Will call Load Cutscene File */
							returnPtr = loadObj->GetResource((edResCollectionEnum)uVar2,
									(ulong)((*(uint*)(internalSeekPos + 4) & 0x80000000) != 0), filePath,
									(int*)(internalSeekPos + 0xc));
							/* Store the pointer we got back */
							*(char**)(internalSeekPos + 8) = returnPtr;
							/* If the pointer we got back was null, then exit */
							if (*(int*)(internalSeekPos + 8) == 0) {
								return (char*)0x0;
							}
						}
					}
					seekCounter = seekCounter + 1;
					seekIncrement = seekIncrement + 0xc;
					filePath = filePath + (byte)filePath[-1];
				} while (seekCounter < (ulong)dataPtr);
			}
			/* Load the rest of the cutscene assets */
			seekIncrement = 0;
			seekCounter = *(int*)(this->field_0x0 + 4);
			pCVar3 = this->field_0x0 + 0x20;
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
									virtualActor.Create(loadObj, &resPtr);
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
											CUTSCENE_LOG(LogLevel::Warning, "Skipping Camera\n");
											//cineCamera = (CameraInfo*)pCVar3;
											//edSceneCamera::Create(&cineCamera, loadObj);
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
			dataPtr = this->field_0x0;
		}
	}
	return dataPtr;
}

bool Create(edCinematicSourceInternal** ppSource, BWitchCin* loadObj, edResCollection** resPtr)
{
	char* pcVar1;
	int* local_10;
	int* local_c;
	int sceFileLength;
	edScene local_4;
	int dataValue;

	dataValue = (*ppSource)->type;
	if (dataValue == 1) {
		local_4.field_0x0 = (char*)0x0;
		/* Loads the SCE asset for the cutscene
		   Example: default.sce */
		pcVar1 = LoadResource(resPtr, loadObj, (*ppSource)->offset, &sceFileLength);
		/* This will load all the assets from the SCE file */
		pcVar1 = local_4.Create(pcVar1, sceFileLength, loadObj);
		(*ppSource)->pFileData = STORE_SECTION(pcVar1);
	}
	else {
		if (dataValue == 2) {
			//local_c = (int*)0x0;
			//(*(code*)loadObj->vt->GetSourceAudioInterface)(loadObj, &local_c);
			//pcVar1 = edResCollection::GetResFilename(resPtr, (*ppSource)->offset);
			//(**(code**)(*local_c + 8))(local_c, pcVar1);
			//(*ppSource)->pFileData = (char*)local_c;
		}
		else {
			if (dataValue == 3) {
				//local_10 = (int*)0x0;
				//(*(code*)loadObj->vt->GetSourceSubtitleInterface)(loadObj, &local_10);
				//dataValue = (*ppSource)->offset;
				//pcVar1 = edResCollection::GetResFilename(resPtr, dataValue);
				//(**(code**)(*local_10 + 8))(local_10, pcVar1, ((uint)(*resPtr)[dataValue * 3 + 1].pData & 0x80000000) != 0);
				//(*ppSource)->pFileData = (char*)local_10;
			}
		}
	}
	return true;
}


bool edCinematic::Create(BWitchCin* pInterface, char* cinFileBuffer, int bufferLength)
{
	bool bVar1;
	char* dataPtr;
	char* pSeekPos;
	edCinematicSourceInternal* currentDataPtr;
	edResCollection* resPtr;
	char* seekPos;
	int readData;

	resPtr = (edResCollection*)0x0;
	dataPtr = (char*)0x0;
	seekPos = cinFileBuffer;
	/* This statement is always true? */
	if (cinFileBuffer < cinFileBuffer + bufferLength) {
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
		} while (seekPos < cinFileBuffer + bufferLength);
	}
	bVar1 = false;
	if (dataPtr != (char*)0x0) {
		/* Assign some values to our cin file object */
		this->fileStart = (int*)cinFileBuffer;
		this->pRes = resPtr;
		this->headerPtr = dataPtr;
		/* Set the read and data start points */
		pSeekPos = this->headerPtr + 0x2c;
		for (readData = *(int*)(this->headerPtr + 0xc); readData != 0; readData = readData + -1) {
			currentDataPtr = (edCinematicSourceInternal*)pSeekPos;
			::Create(&currentDataPtr, pInterface, &resPtr);
			pSeekPos = (char*)((ulong)pSeekPos + 0xc);
		}
		bVar1 = true;
	}
	return bVar1;
}

bool edCinematic::Create(BWitchCin* pInterface, char* fileName)
{
	bool uVar1;
	char* returnPtr;
	int bufferLength;

	bufferLength = 0;
	/* Load the CIN file for the cutscene
	   Example: Airlift_To_Nativ_Cin_Montage_SceneMontage.cin */
	returnPtr = (char*)pInterface->GetResource((edResCollectionEnum)0xff, 0, fileName, &bufferLength);
	if (bufferLength < 1) {
		uVar1 = false;
	}
	else {
		uVar1 = Create(pInterface, returnPtr, bufferLength);
	}
	return uVar1;
}


char* BWitchCin::GetResource(edResCollectionEnum type1, long type2, char* fileName, int* bufferLengthOut)
{
	int* fileBufferStart;

	CUTSCENE_LOG(LogLevel::Info, "BWitchCin::GetResource %s\n", fileName);

	if ((type1 == COT_MeshModel) || (type1 == COT_MeshTexture)) {
		*bufferLengthOut = 0;
		fileBufferStart = (int*)1;
	}
	else {
		fileBufferStart = g_CinematicManager_0048efc->pCinematicObjB_0x18->InstallResource(type1, type2, fileName, (ed_g2d_manager*)0x0, bufferLengthOut);
	}
	return (char*)fileBufferStart;
}
