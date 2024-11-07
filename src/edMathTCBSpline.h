#ifndef ED_MATH_TCB_SPLINE_H
#define ED_MATH_TCB_SPLINE_H

#include "Types.h"

class CedMathTCBSplinePoint
{
public:
	float time;
	float tension;
	float bias;
	float continuity;

	union {
		struct {
			edF32VECTOR4 position;
			edF32VECTOR4 field_0x20;
			edF32VECTOR4 field_0x30;
			edF32VECTOR4 field_0x40;
		};

		edF32MATRIX4 matrix;
	};
};

class CedMathTCBSpline
{
public:
	CedMathTCBSpline(uint pointCount, int bDistributeWeights);
	void SetKeyTCB(float tension, float bias, float continuity, int keyIndex);
	void SetKeyPosition(int keyIndex, edF32VECTOR4* pPosition);

	void PreComputeSpline(int param_2);
	void ComputeKey(uint param_2, uint param_3, uint param_4, uint param_5);

	void GetPosition(float time, edF32VECTOR4* pOutPosition);
	uint GetKeyFromTime(float time, float* param_3);
	void GetFirstDerivative(float time, edF32VECTOR4* pDerivative);
	float GetRoughLength();

	uint field_0x0;
	float field_0x4;
	uint nbPoints;
	CedMathTCBSplinePoint* aPoints;
	undefined* field_0x10;
};

#endif // !ED_MATH_TCB_SPLINE_H