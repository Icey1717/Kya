#ifndef _CAMERA_GAME_H
#define _CAMERA_GAME_H

#include "CameraViewManager.h"
#include "Dynamic.h"

class CActorHeroPrivate;
class CCamFigData;

struct CCameraGame : public CCameraExt
{
	CCameraGame(ECameraType type, struct ByteCode* pMemoryStream);
	virtual void Init();
	virtual bool Manage();
	virtual void Reset();
	virtual bool AlertCamera(int param_2, int param_3, CCamera* param_4);
	virtual void SetMode(ECameraType type);
	virtual ECameraType GetMode();

	void UpdateTarget(edF32VECTOR4* v0, bool doSomething);
	void InitFromConfig(CAMERA_CONFIG* pConfig);

	void _ResetDataForCut(int param_2);

	void CameraFunc_002c5b50();
	void _TestRay_Sphere();
	void FUN_002c8160();
	void UpdateMode();
	void FUN_00199c20(edF32VECTOR4* param_2, edF32VECTOR4* param_3);
	void FUN_002bff30(edF32VECTOR4* v0, edF32VECTOR4* v1, edF32VECTOR4* v2);
	void FUN_002bee80();

	void ClampFunc(uint* puVar5);
	void ClampFuncA(uint* puVar5);

	void SlowHorizontalCameraPan();
	void SlowVerticalCameraPan();

	float _Manage_TargetPos(edF32VECTOR4* v0);
	void _UpdateCameraData(edF32VECTOR4* translation);
	void _Toboggan_UpdateCameraData();

	void _Behind_UpdateAngleBetaData();

	void _UpdateAngleAlphaData();
	void _UpdateAngleBetaData();

	void _Normal_ManageIdle();

	void _Manage_Leash();
	void _After_Manage_Beta();
	void _After_Manage_Alpha();
	void CameraGetWorldTranslation(edF32VECTOR4* outTranslation);

	void FillThisFrameExpectedDifferentialMatrix(edF32MATRIX4* pMatrix);

	void Fight_SetConfig(CCamFigData* pConfig);

	static int _b_use_fig_data;
	static CCamFigData* _pfig_data;

	CActorHeroPrivate* field_0xd0;
	ECameraType cameraType_0xd4;
	CCamConfig cameraConfig;
	undefined4 field_0x1ac;
	uint field_0x1b0;
	undefined4 field_0x1b4;
	int field_0x1b8;
	int mode_0x1bc;
	edF32VECTOR4 field_0x1c0;
	undefined4 field_0x1d0;
	int field_0x1d4;
	float field_0x1d8;
	float field_0x1dc;
	float field_0x1e0;
	float field_0x1e4;
	float angleBeta_0x1e8;
	CActorHeroPrivate* field_0x1ec;
	edF32VECTOR4 gameLookAt;
	float targetPitch;
	float field_0x204;
	float field_0x208;
	float field_0x20c;
	uint cameraRayFlags;
	float field_0x214;
	undefined field_0x218;
	undefined field_0x219;
	undefined field_0x21a;
	undefined field_0x21b;
	undefined field_0x21c;
	undefined field_0x21d;
	undefined field_0x21e;
	undefined field_0x21f;
	float field_0x220;
	float field_0x224;
	float field_0x228;
	float field_0x22c;
	float field_0x230;
	float field_0x234;
	float field_0x238;
	float field_0x23c;
	float field_0x240;
	float field_0x244;
	float pitchAlpha;
	float field_0x24c;
	float field_0x250;
	undefined field_0x254;
	undefined field_0x255;
	undefined field_0x256;
	undefined field_0x257;
	undefined field_0x258;
	undefined field_0x259;
	undefined field_0x25a;
	undefined field_0x25b;
	undefined field_0x25c;
	undefined field_0x25d;
	undefined field_0x25e;
	undefined field_0x25f;
	edF32VECTOR4 field_0x260;
	edF32VECTOR4 field_0x270;
	undefined4 field_0x280;
	undefined field_0x284;
	undefined field_0x285;
	undefined field_0x286;
	undefined field_0x287;
	undefined field_0x288;
	undefined field_0x289;
	undefined field_0x28a;
	undefined field_0x28b;
	undefined field_0x28c;
	undefined field_0x28d;
	undefined field_0x28e;
	undefined field_0x28f;
	SPEED_DYN pitchDyn;
	undefined field_0x298;
	undefined field_0x299;
	undefined field_0x29a;
	undefined field_0x29b;
	undefined field_0x29c;
	undefined field_0x29d;
	undefined field_0x29e;
	undefined field_0x29f;
	edF32VECTOR4 field_0x2a0;
	float field_0x2b0;
	float field_0x2b4;
	struct astruct_12 subObj_12;
	float field_0x2dc;
	undefined4 flags_0x2e0;
	int mode_0x2e4;
	float field_0x2e8;
	uint modeFlags;
	float field_0x2f0;
	undefined field_0x2f4;
	undefined field_0x2f5;
	undefined field_0x2f6;
	undefined field_0x2f7;
	undefined field_0x2f8;
	undefined field_0x2f9;
	undefined field_0x2fa;
	undefined field_0x2fb;
	undefined field_0x2fc;
	undefined field_0x2fd;
	undefined field_0x2fe;
	undefined field_0x2ff;
	edF32VECTOR4 field_0x300;
	undefined field_0x310;
	undefined field_0x311;
	undefined field_0x312;
	undefined field_0x313;
	undefined field_0x314;
	undefined field_0x315;
	undefined field_0x316;
	undefined field_0x317;
	undefined field_0x318;
	undefined field_0x319;
	undefined field_0x31a;
	undefined field_0x31b;
	undefined field_0x31c;
	undefined field_0x31d;
	undefined field_0x31e;
	undefined field_0x31f;
	edF32VECTOR4 field_0x320;
	edF32VECTOR4 field_0x330;
	CActorsTable actorsTable;
	undefined field_0x430;
	undefined field_0x431;
	undefined field_0x432;
	undefined field_0x433;
	undefined field_0x434;
	undefined field_0x435;
	undefined field_0x436;
	undefined field_0x437;
	undefined field_0x438;
	undefined field_0x439;
	undefined field_0x43a;
	undefined field_0x43b;
	undefined field_0x43c;
	undefined field_0x43d;
	undefined field_0x43e;
	undefined field_0x43f;
	undefined field_0x440;
	undefined field_0x441;
	undefined field_0x442;
	undefined field_0x443;
	float field_0x444;
	undefined field_0x448;
	undefined field_0x449;
	undefined field_0x44a;
	undefined field_0x44b;
	undefined field_0x44c;
	undefined field_0x44d;
	undefined field_0x44e;
	undefined field_0x44f;
	edF32VECTOR4 field_0x450;
	uint field_0x460;
	float field_0x464;
	float field_0x468;
	float field_0x46c;
	edF32VECTOR4 field_0x470;
	float field_0x480;
	undefined field_0x484;
	undefined field_0x485;
	undefined field_0x486;
	undefined field_0x487;
	undefined field_0x488;
	undefined field_0x489;
	undefined field_0x48a;
	undefined field_0x48b;
	undefined field_0x48c;
	undefined field_0x48d;
	undefined field_0x48e;
	undefined field_0x48f;
	undefined field_0x490;
	undefined field_0x491;
	undefined field_0x492;
	undefined field_0x493;
	undefined field_0x494;
	undefined field_0x495;
	undefined field_0x496;
	undefined field_0x497;
	undefined field_0x498;
	undefined field_0x499;
	undefined field_0x49a;
	undefined field_0x49b;
	undefined field_0x49c;
	undefined field_0x49d;
	undefined field_0x49e;
	undefined field_0x49f;
	undefined4 field_0x4a0;
	undefined4 field_0x4a4;
	undefined field_0x4a8;
	undefined field_0x4a9;
	undefined field_0x4aa;
	undefined field_0x4ab;
	undefined field_0x4ac;
	undefined field_0x4ad;
	undefined field_0x4ae;
	undefined field_0x4af;
};

class CSP_Manager
{
public:
	void* GetFreeBuffer(size_t size) {
#ifdef PLATFORM_PS2
		IMPLEMENTATION_GUARD();
		return (void*)0x0;
#else
		return malloc(size);
#endif
	}

	void ReleaseBuffer(void* ptr) {
#ifdef PLATFORM_PS2
		IMPLEMENTATION_GUARD();
		return;
#else
		return free(ptr);
#endif
	}
};

extern CSP_Manager gSP_Manager;

#endif