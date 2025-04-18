#ifndef _CAMERA_GAME_H
#define _CAMERA_GAME_H

#include "CameraViewManager.h"

class CActorHeroPrivate;

struct CCameraGame : public CCameraExt {
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
	void FUN_002c7ee0();
	void FUN_00199c20(edF32VECTOR4* param_2, edF32VECTOR4* param_3);
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

	static int _b_use_fig_data;
	static CCamFigData* _pfig_data;

	CActorHeroPrivate* field_0xd0;
	ECameraType cameraType_0xd4;
	CCamConfig cameraConfig;
	undefined4 field_0x1ac;
	uint field_0x1b0;
	undefined4 field_0x1b4;
	int field_0x1b8;
	int field_0x1bc;
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
	int field_0x2e4;
	float field_0x2e8;
	uint field_0x2ec;
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
	undefined field_0x320;
	undefined field_0x321;
	undefined field_0x322;
	undefined field_0x323;
	undefined field_0x324;
	undefined field_0x325;
	undefined field_0x326;
	undefined field_0x327;
	undefined field_0x328;
	undefined field_0x329;
	undefined field_0x32a;
	undefined field_0x32b;
	undefined field_0x32c;
	undefined field_0x32d;
	undefined field_0x32e;
	undefined field_0x32f;
	edF32VECTOR4 field_0x330;
	int field_0x340;
	undefined field_0x344;
	undefined field_0x345;
	undefined field_0x346;
	undefined field_0x347;
	undefined field_0x348;
	undefined field_0x349;
	undefined field_0x34a;
	undefined field_0x34b;
	undefined field_0x34c;
	undefined field_0x34d;
	undefined field_0x34e;
	undefined field_0x34f;
	undefined field_0x350;
	undefined field_0x351;
	undefined field_0x352;
	undefined field_0x353;
	undefined field_0x354;
	undefined field_0x355;
	undefined field_0x356;
	undefined field_0x357;
	undefined field_0x358;
	undefined field_0x359;
	undefined field_0x35a;
	undefined field_0x35b;
	undefined field_0x35c;
	undefined field_0x35d;
	undefined field_0x35e;
	undefined field_0x35f;
	undefined field_0x360;
	undefined field_0x361;
	undefined field_0x362;
	undefined field_0x363;
	undefined field_0x364;
	undefined field_0x365;
	undefined field_0x366;
	undefined field_0x367;
	undefined field_0x368;
	undefined field_0x369;
	undefined field_0x36a;
	undefined field_0x36b;
	undefined field_0x36c;
	undefined field_0x36d;
	undefined field_0x36e;
	undefined field_0x36f;
	undefined field_0x370;
	undefined field_0x371;
	undefined field_0x372;
	undefined field_0x373;
	undefined field_0x374;
	undefined field_0x375;
	undefined field_0x376;
	undefined field_0x377;
	undefined field_0x378;
	undefined field_0x379;
	undefined field_0x37a;
	undefined field_0x37b;
	undefined field_0x37c;
	undefined field_0x37d;
	undefined field_0x37e;
	undefined field_0x37f;
	undefined field_0x380;
	undefined field_0x381;
	undefined field_0x382;
	undefined field_0x383;
	undefined field_0x384;
	undefined field_0x385;
	undefined field_0x386;
	undefined field_0x387;
	undefined field_0x388;
	undefined field_0x389;
	undefined field_0x38a;
	undefined field_0x38b;
	undefined field_0x38c;
	undefined field_0x38d;
	undefined field_0x38e;
	undefined field_0x38f;
	undefined field_0x390;
	undefined field_0x391;
	undefined field_0x392;
	undefined field_0x393;
	undefined field_0x394;
	undefined field_0x395;
	undefined field_0x396;
	undefined field_0x397;
	undefined field_0x398;
	undefined field_0x399;
	undefined field_0x39a;
	undefined field_0x39b;
	undefined field_0x39c;
	undefined field_0x39d;
	undefined field_0x39e;
	undefined field_0x39f;
	undefined field_0x3a0;
	undefined field_0x3a1;
	undefined field_0x3a2;
	undefined field_0x3a3;
	undefined field_0x3a4;
	undefined field_0x3a5;
	undefined field_0x3a6;
	undefined field_0x3a7;
	undefined field_0x3a8;
	undefined field_0x3a9;
	undefined field_0x3aa;
	undefined field_0x3ab;
	undefined field_0x3ac;
	undefined field_0x3ad;
	undefined field_0x3ae;
	undefined field_0x3af;
	undefined field_0x3b0;
	undefined field_0x3b1;
	undefined field_0x3b2;
	undefined field_0x3b3;
	undefined field_0x3b4;
	undefined field_0x3b5;
	undefined field_0x3b6;
	undefined field_0x3b7;
	undefined field_0x3b8;
	undefined field_0x3b9;
	undefined field_0x3ba;
	undefined field_0x3bb;
	undefined field_0x3bc;
	undefined field_0x3bd;
	undefined field_0x3be;
	undefined field_0x3bf;
	undefined field_0x3c0;
	undefined field_0x3c1;
	undefined field_0x3c2;
	undefined field_0x3c3;
	undefined field_0x3c4;
	undefined field_0x3c5;
	undefined field_0x3c6;
	undefined field_0x3c7;
	undefined field_0x3c8;
	undefined field_0x3c9;
	undefined field_0x3ca;
	undefined field_0x3cb;
	undefined field_0x3cc;
	undefined field_0x3cd;
	undefined field_0x3ce;
	undefined field_0x3cf;
	undefined field_0x3d0;
	undefined field_0x3d1;
	undefined field_0x3d2;
	undefined field_0x3d3;
	undefined field_0x3d4;
	undefined field_0x3d5;
	undefined field_0x3d6;
	undefined field_0x3d7;
	undefined field_0x3d8;
	undefined field_0x3d9;
	undefined field_0x3da;
	undefined field_0x3db;
	undefined field_0x3dc;
	undefined field_0x3dd;
	undefined field_0x3de;
	undefined field_0x3df;
	undefined field_0x3e0;
	undefined field_0x3e1;
	undefined field_0x3e2;
	undefined field_0x3e3;
	undefined field_0x3e4;
	undefined field_0x3e5;
	undefined field_0x3e6;
	undefined field_0x3e7;
	undefined field_0x3e8;
	undefined field_0x3e9;
	undefined field_0x3ea;
	undefined field_0x3eb;
	undefined field_0x3ec;
	undefined field_0x3ed;
	undefined field_0x3ee;
	undefined field_0x3ef;
	undefined field_0x3f0;
	undefined field_0x3f1;
	undefined field_0x3f2;
	undefined field_0x3f3;
	undefined field_0x3f4;
	undefined field_0x3f5;
	undefined field_0x3f6;
	undefined field_0x3f7;
	undefined field_0x3f8;
	undefined field_0x3f9;
	undefined field_0x3fa;
	undefined field_0x3fb;
	undefined field_0x3fc;
	undefined field_0x3fd;
	undefined field_0x3fe;
	undefined field_0x3ff;
	undefined field_0x400;
	undefined field_0x401;
	undefined field_0x402;
	undefined field_0x403;
	undefined field_0x404;
	undefined field_0x405;
	undefined field_0x406;
	undefined field_0x407;
	undefined field_0x408;
	undefined field_0x409;
	undefined field_0x40a;
	undefined field_0x40b;
	undefined field_0x40c;
	undefined field_0x40d;
	undefined field_0x40e;
	undefined field_0x40f;
	undefined field_0x410;
	undefined field_0x411;
	undefined field_0x412;
	undefined field_0x413;
	undefined field_0x414;
	undefined field_0x415;
	undefined field_0x416;
	undefined field_0x417;
	undefined field_0x418;
	undefined field_0x419;
	undefined field_0x41a;
	undefined field_0x41b;
	undefined field_0x41c;
	undefined field_0x41d;
	undefined field_0x41e;
	undefined field_0x41f;
	undefined field_0x420;
	undefined field_0x421;
	undefined field_0x422;
	undefined field_0x423;
	undefined field_0x424;
	undefined field_0x425;
	undefined field_0x426;
	undefined field_0x427;
	undefined field_0x428;
	undefined field_0x429;
	undefined field_0x42a;
	undefined field_0x42b;
	undefined field_0x42c;
	undefined field_0x42d;
	undefined field_0x42e;
	undefined field_0x42f;
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
	undefined4 field_0x444;
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

#endif