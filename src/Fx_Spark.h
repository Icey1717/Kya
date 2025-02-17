#ifndef FX_SPARK_H
#define FX_SPARK_H

#include "Types.h"

class CFxSpark : public CObject
{
public:
	virtual bool IsKindOfObject(ulong kind);
	virtual bool InitDlistPatchable(int);

	void Create(int vectorCount, int floatCount, edF32VECTOR3* pVectorBuffer, float* pFloatBuffer, int particleID);
	void Init(int id);
	void SetParameters(float param_1, float param_2, float param_3, float param_4, float param_5, undefined4 param_7);

	edF32VECTOR3* pVector_0xc;
	float* pFloat_0x10;

	edF32VECTOR4 vector_0x80;

	int particleID;

	int dlistPatchId;

	uint field_0x90;
	uint field_0x94;

	int count_0x98;

	int count_0xa0;

	float field_0xb0;

	float field_0xc0;

	edF32VECTOR3* pVector_0xcc;
	edF32VECTOR4 field_0xd0;

	undefined4 field_0xe0;
	byte* field_0xe4;
};

template<int countA, int countB>
class CFxSparkNoAlloc : public CFxSpark
{
public:
	edF32VECTOR3 aVectorData[countA];
	float aFloatData[countB];
	byte aUnknown[countB];
};

#endif // FX_SPARK_H