#ifndef PATH_FOLLOW_H
#define PATH_FOLLOW_H

#include "Types.h"
#include "MemoryStream.h"

class CPath {
	virtual void Create(ByteCode* pByteCode) = 0;
};

class CPathFollow : public CPath {
public:
	CPathFollow();

	static edF32VECTOR4 gPathDefQuat;

	char* field_0x0;
	uint mode;
	uint type;
	uint field_0xc;
	int splinePointCount;
	int field_0x14;
	uint field_0x18;
	edF32VECTOR4* aSplinePoints;
	edF32VECTOR4* aSplineRotationsEuler;
	edF32VECTOR4* aSplineRotationsQuat;
	char* field_0x28;
	float* aDelays;
	char* field_0x30;
	char* field_0x34;
	char* field_0x38;

	virtual void Create(ByteCode* pByteCode);
};

struct S_PATHREADER_POS_INFO {
	int field_0x0;
	int field_0x4;
	float field_0x8;
};

class CPathFollowReaderAbsolute {
public:
	CPathFollowReaderAbsolute();
	void Create(ByteCode* pByteCode);
	void Create(float param_1, float param_2, CPathFollow* pPathFollow, int type, int mode, int timing, int param_8);

	int ComputeSegment(float param_1, int* param_3, int* param_4, float* param_5);
	int ComputePosition(float param_1, edF32VECTOR4* param_3, edF32VECTOR4* param_4, S_PATHREADER_POS_INFO* param_5);
	int ComputeMatrix(float param_1, edF32MATRIX4* pMatrix, edF32VECTOR4* param_4, S_PATHREADER_POS_INFO* param_5);

	CPathFollow* pActor3C_0x0;
	float barFullAmount_0x4;
	float field_0x8;
	float field_0xc;
	float* field_0x10;

	int field_0x1c;
	int type;
	int mode;
};

class CPathFollowReader {
public:
	CPathFollowReader();
	void Create(ByteCode* pByteCode);
	void Init();
	void Reset();

	void NextWayPoint();
	bool AtGoal(int param_2, int param_3);
	int GetPrevPlace(int param_2, int param_3);

	float GetDelay();

	edF32VECTOR4* GetWayPoint(int index);
	edF32VECTOR4* GetWayPoint();
	edF32VECTOR4* GetWayPointAngles();

	int GetNextPlace(int param_2, int param_3);

	union {
		int index;
		CPathFollow* pPathFollow;
	};

	int splinePointIndex;
	int field_0x8;
	int field_0xc;
};

#endif // !PATH_FOLLOW_H
