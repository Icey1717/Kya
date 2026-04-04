#ifndef ACTOR_PATTERN_SERVICE_H
#define ACTOR_PATTERN_SERVICE_H

#include "Types.h"

class ByteCode;

class CPointPattern
{
public:
	edF32VECTOR4 field_0x0;
	edF32VECTOR4 field_0x10;
	float field_0x20;
	byte field_0x24;
	byte field_0x25;
};

class CPatternPart
{
public:
	CPatternPart();

	void Create(ByteCode* pByteCode);
	void Init(float param_1, CActor* pOwner);
	void Reset();
	void FUN_003a7cc0(float param_2);
	void Draw();

	void DrawWithOneStripAndADCFlags();
	float FUN_003a6320(int param_2, int param_3);
	float FUN_003a5e10(undefined8 param_2, int param_3, int param_4, edF32VECTOR4* param_5, edF32VECTOR4* param_6);

	void FireContinue(CActor* param_2, edF32VECTOR4* param_3, edF32VECTOR4* param_4);
	void FireContinueWithoutAddPoint(CActor* param_2, edF32VECTOR4* param_3);

	void FUN_003a78f0(float param_1, edF32VECTOR4* param_3, edF32VECTOR4* param_4, byte param_5);

	bool UpdatePatternPartLife();

	bool FUN_003a7190(int param_2);
	void FUN_003a6a60();
	void FUN_003a75b0(int param_2);

	float field_0x0;
	float field_0x4;
	float field_0x8;
	float field_0xc;
	float field_0x10;
	float field_0x14;

	int materialId;
	float field_0x1c;
	float field_0x20;
	float field_0x24;
	uint field_0x28;
	edF32VECTOR4 field_0x30;
	CActor* field_0x40;
	float field_0x48;
	int field_0x4c;
	undefined4 field_0x50;
	int field_0x5c;

	edF32VECTOR4 field_0x60;

	float field_0x70;
	float field_0x74;
	float field_0x78;

	CActor* pOwner;

	int nbPointPatterns;
	CPointPattern* aPointPatterns;
};

#endif //ACTOR_PATTERN_SERVICE_H