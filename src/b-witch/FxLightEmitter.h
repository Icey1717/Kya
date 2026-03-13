#ifndef FX_LIGHT_EMITTER_H
#define FX_LIGHT_EMITTER_H

#include "Types.h"

struct RAY_DEF
{

};

class CFxLightEmitter
{
public:
	void Create(float param_1, uint param_3, uint param_4);
	void InitRays(int nbRays);
	void ResetRays();
	void ChangeUvMapping(edF32VECTOR4* pUvMapping);
	void ChangeUvSpeed(float param_1, short param_3, short param_4);
	void ChangeColor(int index, uint newColor);
	void SetSharedRays(RAY_DEF* pSharedRays);
	void ChangeMaterialId(int mainMaterialId, int param_3);
	void ChangeRadius(float rMin, float rMax);
	void ChangeLengthAndWidth(float param_1, float param_2, float param_3, float param_4);
	void ChangeSpeedAndAccel(float param_1, float param_2, float param_3, float param_4);
	void ChangeColors(uint newColor);
	void ChangeStallTime(float stalltime);
	void ChangeDirection(edF32VECTOR4* pDirection);
	void ChangeAlphaFactor(float alphaFactor);
	bool Manage(edF32VECTOR4*, undefined4);
	void GenerateNewOne(edF32VECTOR4* param_2);
	void Enable(int bEnable);
	void ReleaseRays(int nbRays);

	RAY_DEF* pRayDef;
	float alphaFactor;
	uint flags;

	edF32VECTOR4 field_0x10;
	edF32VECTOR4 field_0x20;

	edF32VECTOR4 direction;

	int stallTime;

	uint nbRayDefs;
	edF32VECTOR4 uvMapping;
	uint aColors[4];

	float field_0x40;
	float field_0x44;

	float instanceIndex;

	float field_0x60;

	// Length or width
	float field_0x64;

	float field_0x68;

	// Speed and Accel
	float field_0x78;
	float field_0x7c;
	float field_0x80;
	float field_0x84;

	// Length or width
	float field_0x88;
	float field_0x8c;
	float field_0x90;

	int materialId;
	int nbMaterials;
	int field_0x98;

	float radiusMin;
	float radiusMax;

	float field_0xb8;
	short field_0xbc;
	short field_0xbe;
};

#endif // FX_LIGHT_EMITTER_H