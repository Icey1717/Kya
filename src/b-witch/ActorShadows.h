#ifndef ACTOR_SHADOWS_H
#define ACTOR_SHADOWS_H

#include "Types.h"

class CGlobalDListPatch;

class CShadowShared : public CObject 
{
public:
	CShadowShared();

	virtual bool IsKindOfObject(ulong kind);

	virtual void SetDisplayable(int displayable);
	virtual void SetDisplayableAlt(int param_2);
	virtual byte Compute(edF32MATRIX4* param_2);

	void Init(int sectorId);
	void Draw(CGlobalDListPatch* pPatch, uint index);
	void SetIntensity(float newIntensity);
	void SetGlobalIntensity(float newGlobalIntensity);

	edF32VECTOR4 position;
	edF32VECTOR4 field_0x20;
	_rgba shadowColor;
	int displayable;
	int field_0x38;
	float field_0x3c;
	float globalIntensity;
	float field_0x44;
	float field_0x48;
	float field_0x4c;
};

class CShadow : public CShadowShared
{
public:
	CShadow();

	virtual bool IsKindOfObject(ulong kind);
	virtual bool InitDlistPatchable(int patchId);
	virtual void SetDisplayable(int displayable);
	virtual void SetDisplayableAlt(int param_2);
	virtual byte Compute(edF32MATRIX4* param_2);

	void Init(int sectorId);
	void Draw();
	void SetMaterialId(int newMaterialId);
	void Create(int newMaterialId);

	float field_0x50;
	int materialId;
	int dlistPatchId;
};

#endif //ACTOR_SHADOWS_H