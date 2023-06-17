#include "CinematicManager.h"
#include "CameraViewManager.h"

CinematicManager* g_CinematicManager_0048efc;

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
	pSubObj_0x1c = (CinematicObjectB*)0x0;
	field_0x20 = -1;
	field_0x24 = -1;
	g_CinematicManager_0048efc = this;
	ppCinematicObjB_A = (CinematicObjectB**)0x0;
	numCutscenes_0x8 = 0;
	ppCinematicObjB_B = (CinematicObjectB**)0x0;
	count_0x14 = 0;
	pCinematicObjB_0x18 = (CinematicObjectB*)0x0;
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
	if (this->pSubObj_0x1c != (CinematicObjectB*)0x0) {
		this->pCameraLocationObj->SetTransform(&this->matrix_0x70);
		pCameraLocationObj->field_0x74 = this->field_0xb0;
		pCamera = this->pCameraLocationObj;
		pCamera->field_0x8c = 0.0f;
		pCamera->field_0x90 = 1;
		pCamera->field_0x98 = 0.0f;
		pCamera->field_0x94 = 1;
		pCamera->field_0x9c = 0.0f;
		//pCameraManager->PushCamera((Camera*)this->pCameraLocationObj, 1);
	}
	return;
}