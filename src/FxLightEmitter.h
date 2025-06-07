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
	void ResetRays();
	void ChangeMaterialId(int mainMaterialId, int param_3);
	void ChangeRadius(float rMin, float rMax);
	void ChangeLengthAndWidth(float param_1, float param_2, float param_3, float param_4);
	void ChangeSpeedAndAccel(float param_1, float param_2, float param_3, float param_4);
	void ChangeColors(uint newColor);
	void ChangeStallTime(float stalltime);
	void Manage(edF32VECTOR4*, undefined4);
	void GenerateNewOne(edF32VECTOR4* param_2);

	RAY_DEF* pRayDef;
	uint flags;

	edF32VECTOR4 field_0x20;

	int stallTime;

	uint nbRayDefs;

	uint aColors[4];

	float field_0x40;
	float field_0x44;

	float field_0x60;

	// Length or width
	float field_0x64;

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
};

#endif // FX_LIGHT_EMITTER_H