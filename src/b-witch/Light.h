#ifndef LIGHT_H
#define LIGHT_H

#include "Types.h"
#include "LightBase.h"

struct ByteCode;

struct LightingContext
{
	edF32VECTOR4 position;
	edF32VECTOR4* pLightDirection;
	edF32VECTOR4* pLightAmbient;
	edF32VECTOR4* pLightColor;
	undefined4 field_0x1c;
	float colorMultiplier;
};

class CLight
{
public:
	CLight();
	virtual ~CLight() = default;

	virtual bool IsInCluster(ed_zone_3d* pZone);
	virtual void Init();
	virtual void Manage();
	virtual void Term();
	virtual void Activate();
	virtual void Inactivate();
	virtual bool DoLighting(LightingContext* pContext);
	virtual int GetBaseShape(BaseShape** ppBaseShape);
	virtual edF32VECTOR4* GetPosition() = 0;
	virtual void SetPosition(edF32VECTOR4* pPosition);
	virtual edF32VECTOR4* GetColour() = 0;
	virtual void SetColour(edF32VECTOR4* pColour);

	bool TestIlluminationZones(edF32VECTOR4* pLocation, int id);

	_rgba colour_0x4;
	ushort field_0x8;
	short referencedLightIndex;
	S_ZONE_STREAM_REF* pZoneHolderA;
	S_ZONE_STREAM_REF* pZoneHolderB;
	undefined field_0x14;
	undefined field_0x15;
	undefined field_0x16;
	undefined field_0x17;
	undefined field_0x18;
	undefined field_0x19;
	undefined field_0x1a;
	undefined field_0x1b;
	undefined field_0x1c;
	undefined field_0x1d;
	undefined field_0x1e;
	undefined field_0x1f;
};

class CLightTorch : public CLight
{
public:
	CLightTorch(ByteCode* pByteCode);

	virtual void Manage();
	virtual void Activate();
	virtual bool DoLighting(LightingContext* pContext);
	virtual int GetBaseShape(BaseShape** ppBaseShape);
	virtual edF32VECTOR4* GetPosition();
	virtual edF32VECTOR4* GetColour();

	virtual void Create(ByteCode* pByteCode);

	FullColorModel colorModel;
	int lightType;
	BaseShape baseShape;

	edF32VECTOR4 angleRotY;
	edF32VECTOR4 field_0xb0;
	edF32VECTOR4 field_0xc0;
};

class CLightSun : public CLight
{
public:
	CLightSun();

	virtual void Manage();
	virtual void Activate();
	virtual bool DoLighting(LightingContext* pContext);
	virtual int GetBaseShape(BaseShape** ppBaseShape);
	virtual edF32VECTOR4* GetPosition();
	virtual edF32VECTOR4* GetColour();

	BaseShapeC baseShape;
	FullColorModel colorModel;
};

class CLightSpot : public CLight
{
public:
	CLightSpot();

	virtual void Manage();
	virtual void Activate();
	virtual bool DoLighting(LightingContext* pContext);
	virtual int GetBaseShape(BaseShape** ppBaseShape);
	virtual edF32VECTOR4* GetPosition();
	virtual edF32VECTOR4* GetColour();

	BaseShape baseShape;
	FullColorModel colorModel;
};

class CLightDirectional : public CLight
{
public:
	CLightDirectional();

	virtual void Manage();
	virtual void Activate();
	virtual void Inactivate();
	virtual bool DoLighting(LightingContext* pContext);
	virtual int GetBaseShape(BaseShape** ppBaseShape);
	virtual edF32VECTOR4* GetPosition();
	virtual edF32VECTOR4* GetColour();

	BaseShape baseShape;
	FullColorModel colorModel;
};

class CLightAmbient : public CLight
{
public:
	CLightAmbient();

	virtual void Manage();
	virtual bool DoLighting(LightingContext* pContext);
	virtual edF32VECTOR4* GetPosition();
	virtual edF32VECTOR4* GetColour();

	BaseShapeA baseShape;
	SimplestColorModel colorModel;
};

class CLightOmni : public CLight
{
public:
	virtual void Manage();
	virtual void Activate();
	virtual bool DoLighting(LightingContext* pContext);
	virtual int GetBaseShape(BaseShape** ppBaseShape);
	virtual edF32VECTOR4* GetPosition();
	virtual edF32VECTOR4* GetColour();

	virtual void Create();

	BaseShapeB baseShape;
	FullColorModel colorModel;
};

#endif // LIGHT_H
