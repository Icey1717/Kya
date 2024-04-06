#ifndef _CAMERA_H
#define _CAMERA_H

#include "Types.h"
#include "Actor.h"

#define EDFCAMERA_LOG(level, format, ...) MY_LOG_CATEGORY("edFCamera", level, format, ##__VA_ARGS__)

class CCameraManager;

enum ECameraType {
	CT_AroundSpecial = 2,
	CT_Cinematic = 22,
	CT_Death = 25,
	CT_Frontend = 24,
	CT_IntView = 11,
	CT_Main = 3,
	CT_KyaJamgut = 7,
	CT_KyaWindWall = 5,
	CT_MainCamera = 21,
	CT_MouseAround = 1,
	CT_MouseQuake = 0,
	CT_ShadowSun = 12,
	CT_SilverBoomy = 10
};


struct SpecificCondition {
	int* pData;

	bool IsVerified();
};

struct CCamera : public CObject {
	CCamera();
	CCamera(struct ByteCode* pMemoryStream);

	// CObject
	virtual bool IsKindOfObject(ulong kind);

	virtual ECameraType GetMode();
	virtual void Init();
	virtual void Reset() { }
	virtual bool Manage();
	virtual void Draw() {}
	virtual bool AlertCamera(int param_2, int param_3, CCamera* param_4);
	virtual CActor* GetTarget() { return NULL; }
	virtual void SetTarget(CActor* pActor) {}
	virtual void SetOtherTarget(CActor* pNewTarget);
	virtual float GetDistance();
	virtual void SetDistance(float distance) {}
	virtual float GetAngleAlpha();
	virtual float GetAngleBeta();
	virtual float GetAngleGamma();
	virtual void SetAngleAlpha(float angle) { };
	virtual void SetAngleBeta(float angle) { };
	virtual void SetAngleGamma(float angle) { };
	virtual void SetMode(ECameraType type) {}

	static CCameraManager* _gpcam_man;

	int field_0x8;
	uint flags_0xc;
	edF32MATRIX4 transformationMatrix;
	edF32VECTOR4 field_0x50;
	edF32VECTOR4 lookAt;
	CActor* pOtherTarget;
	float fov;
	SpecificCondition specCondition;
	undefined4 field_0x7c;

	int field_0x80;
	ActorAndWaypoint* field_0x84;
	int* field_0x88;

	float field_0x8c;
	SWITCH_MODE switchMode;
	SWITCH_MODE field_0x94;
	float field_0x98;
	float field_0x9c;
	byte field_0xa0;
	undefined field_0xa1;
	undefined field_0xa2;
	undefined field_0xa3;
	struct CCamera* pNextCameraView_0xa4;
};

struct CCameraExt : public CCamera {
	CCameraExt(struct ByteCode* pMemoryStream);
	CCameraExt();


	// CObject
	virtual bool IsKindOfObject(ulong kind);

	// CCamera
	virtual void Init();
	virtual CActor* GetTarget();
	virtual void SetTarget(CActor* pActor);
	virtual float GetDistance();
	virtual void SetDistance(float distance);
	virtual float GetAngleAlpha();
	virtual float GetAngleBeta();
	virtual float GetAngleGamma();
	virtual void SetAngleAlpha(float angle);
	virtual void SetAngleBeta(float angle);
	virtual void SetAngleGamma(float angle);
	virtual edF32VECTOR3* GetAngles();
	// CCamera

	void ComputeTargetPosition(edF32VECTOR4* param_2);
	void ComputeTargetOffset(edF32VECTOR4* v0);

	undefined field_0xa8;
	undefined field_0xa9;
	undefined field_0xaa;
	undefined field_0xab;
	undefined field_0xac;
	undefined field_0xad;
	undefined field_0xae;
	undefined field_0xaf;
	struct CActor* pActorView;
	edF32VECTOR3 angles;
	float distance;
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
	float field_0xd0;
	int mousePortId;
	undefined field_0xd8;
	undefined field_0xd9;
	undefined field_0xda;
	undefined field_0xdb;
	undefined field_0xdc;
	undefined field_0xdd;
	undefined field_0xde;
	undefined field_0xdf;
};

#endif // _CAMERA_H
