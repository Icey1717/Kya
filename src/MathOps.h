#ifndef _MATH_OPS_H
#define _MATH_OPS_H

#include "Types.h"

#ifdef PLATFORM_PS2
#include <libvu0.h>
#endif

extern float g_DefaultNearClip_0044851c;

extern edF32MATRIX4 gF32Matrix4Zero;
extern edF32VECTOR3 gF32Vector3Zero;
extern edF32MATRIX4 gF32Matrix4Unit;
extern edF32VECTOR4 gF32Vertex4Zero;
extern edF32VECTOR4 gF32Vector4Zero;
extern edF32VECTOR4 gF32Vector4UnitX;
extern edF32VECTOR4 gF32Vector4UnitZ;
extern edF32VECTOR4 g_xVector;

float edDistPointToPlane(edF32VECTOR4* subvector2, edF32VECTOR4* innerProductVector, edF32VECTOR4* subVector1);

void edQuatToMatrix4Hard(edF32VECTOR4* v0, edF32MATRIX4* m0);
void edQuatInverse(edF32VECTOR4* param_1, edF32VECTOR4* param_2);
void edQuatMul(edF32VECTOR4* param_1, edF32VECTOR4* param_2, edF32VECTOR4* param_3);
void edQuatNormalize(edF32VECTOR4* v0, edF32VECTOR4* v1);

void edQuatShortestSLERPAccurate(float param_1, edF32VECTOR4* param_2, edF32VECTOR4* param_3, edF32VECTOR4* param_4);

float edF32Vector4NormalizeHard(edF32VECTOR4* v0, edF32VECTOR4* v1);

void edF32Matrix4GetTransposeHard(edF32MATRIX4* m0, edF32MATRIX4* m1);

void edF32Vector4SubHard(edF32VECTOR4* v0, edF32VECTOR4* v1, edF32VECTOR4* v2);
void edF32Vector4CrossProductHard(edF32VECTOR4* v0, edF32VECTOR4* v1, edF32VECTOR4* v2);

void GetAnglesFromVector(edF32VECTOR4* pitchAngles, edF32VECTOR4* v0);

void edF32Matrix4RotateXHard(float angle, edF32MATRIX4* m0, edF32MATRIX4* m1);
void edF32Matrix4RotateYHard(float angle, edF32MATRIX4* outputMatrix, edF32MATRIX4* inputMatrix);

void edF32Matrix4CopyHard(edF32MATRIX4* dst, edF32MATRIX4* src);
void edF32Matrix4ScaleHard(edF32MATRIX4* m0, edF32MATRIX4* m1, edF32VECTOR4* v0);
void edF32Matrix4RotateZHard(float t0, edF32MATRIX4* m0, edF32MATRIX4* m1);

void ed3DComputeProjectionToScreenMatrix(float startX, float endX, float startY, float endY, float startZ, float endZ, float startW, float endW, edF32MATRIX4* m0);
void ed3DComputeLocalToProjectionMatrix(float x, float y, float yMin, float yMax, edF32MATRIX4* m0);
uint GetGreaterPower2Val(uint value);

float edF32ATan2Soft(float a, float b);
void SetVectorFromAngles(edF32VECTOR4* rotQuat, edF32VECTOR3* rotEuler);
void edF32Matrix4ToEulerSoft(edF32MATRIX4* m0, edF32VECTOR3* v0, char* rotationOrder);
void MatrixRotationFromVectorAndAngle(float angle, edF32MATRIX4* v0, edF32VECTOR4* v1);
edF32MATRIX4* edF32Matrix4FromEulerSoft(edF32MATRIX4* m0, edF32VECTOR3* v0, char* order);
void edQuatFromMatrix4(edF32VECTOR4* v0, edF32MATRIX4* m0);
void edF32Matrix4FromEulerOrdSoft(edF32MATRIX4* rotatedMatrix, char* rotationOrder, float* rotationAngles);
void edQuatShortestSLERPHard(float alpha, edF32VECTOR4* outRotation, edF32VECTOR4* currentRotation, edF32VECTOR4* targetRotation);
void edF32Vector3LERPSoft(float alpha, edF32VECTOR3* outWorldLocation, edF32VECTOR3* currentLocation, edF32VECTOR3* targetLocation);
void edF32Vector4AddHard(edF32VECTOR4* v0, edF32VECTOR4* v1, edF32VECTOR4* v2);

void edF32Matrix4GetInverseOrthoHard(edF32MATRIX4* m0, edF32MATRIX4* m1);
void edF32Vector4ScaleHard(float t, edF32VECTOR4* v0, edF32VECTOR4* v1);

void edF32Matrix4MulF32Matrix4Hard(edF32MATRIX4* dst, edF32MATRIX4* m1, edF32MATRIX4* m2);

void edF32Matrix4SetIdentityHard(edF32MATRIX4* m0);

void edF32Matrix4TranslateHard(edF32MATRIX4* m0, edF32MATRIX4* m1, edF32VECTOR4* v0);

void edF32Matrix4MulF32Vector4Hard(edF32VECTOR4* v0, edF32MATRIX4* m0, edF32VECTOR4* v1);

void BuildMatrixFromNormalAndSpeed(edF32MATRIX4* m0, edF32VECTOR4* v0, edF32VECTOR4* v1);
void edF32Matrix4BuildFromVectorAndAngle(float t0, edF32MATRIX4* m0, edF32VECTOR4* v0);

void edF32Matrix4InverseSoft(edF32MATRIX4* m0);
void edF32Matrix4InverseOrthoSoft(edF32MATRIX4* m0);
bool edF32Matrix4GetInverseGaussSoft(edF32MATRIX4* param_1, edF32MATRIX4* param_2);

void edF32Vector4SquareHard(edF32VECTOR4* v0, edF32VECTOR4* v1);

void edF32Matrix4BuildFromVectorUnitSoft(edF32MATRIX4* m0, edF32VECTOR4* v0);

const float g_TinyFloat_00448548 = 1.0E-6f;

float edF32Vector4SafeNormalize1Hard(edF32VECTOR4* v0, edF32VECTOR4* v1);
float edF32Vector4GetDistHard(edF32VECTOR4* v0);
float edF32Vector4SafeNormalize0Hard(edF32VECTOR4* v0, edF32VECTOR4* v1);
float edF32Vector4DotProductHard(edF32VECTOR4* v0, edF32VECTOR4* v1);
float edFIntervalUnitSrcLERP(float start, float end, float alpha);
bool edProjectVectorOnPlane(float projectionFactor, edF32VECTOR4* pResult, edF32VECTOR4* pInput, edF32VECTOR4* pPlaneNormal, int optionFlag);
bool edReflectVectorOnPlane(float reflectionFactor, edF32VECTOR4* pResult, edF32VECTOR4* pInput, edF32VECTOR4* pPlaneNormal, int mode);
void FUN_00193060(edF32MATRIX4* param_1, edF32VECTOR4* param_2, edF32VECTOR4* pLookAt);

float edFIntervalDotSrcLERP(float param_1, float param_2, float param_3);
float edFIntervalLERP(float param_1, float param_2, float param_3, float param_4, float param_5);
float edFIntervalUnitDstLERP(float param_1, float param_2, float param_3);

void edF32Vector4LERPHard(float t, edF32VECTOR4* v0, edF32VECTOR4* v1, edF32VECTOR4* v2);

float GetAngleXFromVector(edF32VECTOR4* v0);
float GetAngleYFromVector(edF32VECTOR4* v0);
float edF32GetAnglesDelta(float t0, float t1);
float edF32Between_2Pi(float param_1);
float edF32Between_0_2Pi(float param_1);
float edF32Between_Pi(float param_1);

void edF32Vector4GetNegHard(edF32VECTOR4* v0, edF32VECTOR4* v1);

float edF32Vector4GetLengthSoft(edF32VECTOR4* v0);

void edF32Vector2Sub(edF32VECTOR2* v0, edF32VECTOR2* v1, edF32VECTOR2* v2);

float edF32Vector4DotProductHard_I(edF32VECTOR4* v0, edF32VECTOR4* v1);
void edF32Vector4SubHard_I(edF32VECTOR4* v0, edF32VECTOR4* v1, edF32VECTOR4* v2);

float edF32ACosHard(float param_1);

#endif // _MATH_OPS_H
