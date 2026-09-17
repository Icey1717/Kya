#ifndef LIGHT_BASE_H
#define LIGHT_BASE_H

#include "Types.h"

#define LIGHT_MANAGER_LOG(level, format, ...) MY_LOG_CATEGORY("LightManager", level, format, ##__VA_ARGS__)

struct ByteCode;

struct BaseShapeA
{
	edF32VECTOR4 position;
};

struct BaseShapeB : public BaseShapeA
{
	float fallout0;
	float fallout1;
	undefined field_0x18;
	undefined field_0x19;
	undefined field_0x1a;
	undefined field_0x1b;
	undefined field_0x1c;
	undefined field_0x1d;
	undefined field_0x1e;
	undefined field_0x1f;
};

struct BaseShapeC : public BaseShapeB
{
	edF32VECTOR4 direction;
};

struct BaseShape : public BaseShapeC
{
	float fov0;
	float fov1;
	undefined field_0x58;
	undefined field_0x59;
	undefined field_0x5a;
	undefined field_0x5b;
	undefined field_0x5c;
	undefined field_0x5d;
	undefined field_0x5e;
	undefined field_0x5f;
};

struct SunShape : public BaseShape
{
	void Read(ByteCode* pByteCode);
};

struct LightShape : public BaseShape
{
	void Read(ByteCode* pByteCode);
};

struct SpotShape : public BaseShape
{
	void Read(ByteCode* pByteCode);
};

struct SimplestColorModel
{
	edF32VECTOR4 ambientColor;
};

struct SimpleColorModel : public SimplestColorModel
{
	edF32VECTOR4 color;
};

struct FullColorModel : public SimpleColorModel
{
	edF32VECTOR4 field_0x20;
	void Read(ByteCode* pByteCode);
};

#endif // LIGHT_BASE_H
