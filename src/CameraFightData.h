#ifndef _CAMERA_FIGHT_DATA_H_
#define _CAMERA_FIGHT_DATA_H_

#include "Types.h"
#include "Dynamic.h"

class CActor;
class CCameraGame;

struct ByteCode;

struct cam_temp_struct
{
	edF32VECTOR4* pVector;
	edF32VECTOR4* field_0x4;
	float field_0x8;
	float field_0xc;
	float field_0x10;
};

struct CamUpdateSubObjA
{
	short field_0x0;
	short field_0x2;
	ushort field_0x4;
	ushort field_0x6;
};

struct CamUpdateSubObjB
{
	float intersectionDistance;
	float field_0x4;
};

struct CamUpdate
{
	inline CamUpdate();

	void FUN_003c4a40(edF32VECTOR4* param_2);
	void CalculateIntersection(int index);
	uint FUN_Count(float param_1);
	float FUN_003c4e20(float param_1, float param_2);

	edF32VECTOR4 position;
	float field_0x10;
	float field_0x14;
	float field_0x18;
	float maxDistance;
	uint field_0x20;

	CamUpdateSubObjA aSubObjA[0x23];
	int activeIndex;
	int field_0x140;
	CamUpdateSubObjB aSubObjB[0x23];
};

struct ParCamFigLAB
{
	undefined field_0x0;
	float field_0x4;
	float field_0x8;
	float field_0xc;
};

class CCamFigData
{
public:
	CCamFigData();

	void Create(ByteCode* pByteCode);
	void IsValid(undefined4 param_2);
	void SetBoundingSphere(ed_Bound_Sphere* pBoundSphere);
	void SetAdversaryPos(edF32VECTOR4* pPosition);
	void SetPosParam(edF32VECTOR4* pPosition);
	void Cam_SetDefault(float param_1, float param_2, CCameraGame* pCamera);
	void Reset() {}
	cam_temp_struct* Cam_Update(edF32VECTOR4* param_2, int param_3);
	void ComputeData();
	void DetermineBetaAndWay();
	float SV_TestCollisionSphere(ParCamFigLAB* param_2, edF32VECTOR4* param_3);
	void _UpdateCameraData();

	void CancelCurrentMovements();
	void FUN_003c5ba0(float param_1, int param_3, long param_4);
	bool FUN_003c79f0(float param_1, float param_2);

	float field_0x0;
	float field_0x4;
	float field_0x8;
	float field_0xc;
	float field_0x10;
	float field_0x14;
	float field_0x18;
	float field_0x1c;
	float field_0x20;
	float field_0x24;
	float field_0x28;

	float field_0x4c;

	edF32VECTOR4 adversaryPos;

	CActor* pCommanderRef_0x230;
	CCameraGame* pCamera;
	cam_temp_struct camTemp;
	edF32VECTOR4 field_0x250;
	undefined4 field_0x260;
	float field_0x264;
	float field_0x268;
	float field_0x26c;
	float field_0x270;
	float field_0x280;
	float field_0x284;
	float field_0x288;
	float field_0x28c;
	edF32VECTOR4 field_0x290;
	undefined4 field_0x2a0;
	int field_0x2a4;
	undefined4 field_0x2a8;
	int field_0x2ac;
	int field_0x2b0;
	int field_0x2b4;
	int field_0x2b8;
	int field_0x2bc;
	float field_0x2c0;
	undefined4 field_0x2c8;
	float field_0x2cc;
	int field_0x2d0;

	float field_0x2c4;
	edF32VECTOR4 field_0x2e0;
	float field_0x2f0;
	undefined4 field_0x2f8;

	float field_0x300;
	int field_0x304;
	int field_0x308;
	float field_0x30c;

	CAngleDyn angleDynA;
	CAngleDyn angleBetaDyn;

	CValueDyn distanceDyn;
	CValueDyn boundingSphereDyn;
	CVertexDyn vertexDynExtA;
	CVertexDyn vertexDynExtB;
	CVertexDyn vertexDynExtC;
	CVertexDyn vertexDynExtD;
	CVertexDyn vertexDynExtE;
	SPEED_DYN speedDyn;
};

#endif // _CAMERA_FIGHT_DATA_H_