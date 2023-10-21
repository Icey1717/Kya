#ifndef _MATH_OPS_H
#define _MATH_OPS_H

#include "Types.h"

#ifdef PLATFORM_PS2
#include <libvu0.h>
#endif

extern edF32MATRIX4 gF32Matrix4Zero;
extern edF32VECTOR3 gF32Vector3Zero;
extern edF32MATRIX4 gF32Matrix4Unit;
extern edF32VECTOR4 gF32Vertex4Zero;
extern edF32VECTOR4 gF32Vector4Zero;

void edQuatToMatrix4Hard(edF32VECTOR4* v0, edF32MATRIX4* m0);

void edF32Vector4NormalizeHard(float* v0, float* v1);
void sceVu0ApplyMatrix(edF32VECTOR4* v0, edF32MATRIX4* m0, edF32VECTOR4* v1);
void edF32Matrix4GetTransposeHard(edF32MATRIX4* m0, edF32MATRIX4* m1);
void sceVu0InverseMatrix(edF32MATRIX4* m0, edF32MATRIX4* m1);

void edF32Vector4SubHard(edF32VECTOR4* v0, edF32VECTOR4* v1, edF32VECTOR4* v2);
void edF32Vector4CrossProductHard(edF32VECTOR4* v0, edF32VECTOR4* v1, edF32VECTOR4* v2);
void ps2_vu0_sqr_vector(edF32VECTOR4* v0, edF32VECTOR4* v1);

void GetAnglesFromVector(edF32VECTOR4* pitchAngles, edF32VECTOR4* v0);

void edF32Matrix4RotateXHard(float angle, edF32MATRIX4* m0, edF32MATRIX4* m1);
void edF32Matrix4RotateYHard(float angle, edF32MATRIX4* outputMatrix, edF32MATRIX4* inputMatrix);

void edF32Matrix4CopyHard(edF32MATRIX4* m0, edF32MATRIX4* m1);
void edF32Matrix4ScaleHard(edF32MATRIX4* m0, edF32MATRIX4* m1, edF32VECTOR4* v0);
void edF32Matrix4RotateZHard(float t0, edF32MATRIX4* m0, edF32MATRIX4* m1);

void ed3DComputeProjectionToScreenMatrix(float startX, float endX, float startY, float endY, float startZ, float endZ, float startW, float endW, edF32MATRIX4* m0);
void ed3DComputeLocalToProjectionMatrix(float x, float y, float yMin, float yMax, edF32MATRIX4* m0);
uint GetGreaterPower2Val(uint value);

float edF32ATan2Soft(float a, float b);
void SetVectorFromAngles(edF32VECTOR4* rotQuat, edF32VECTOR3* rotEuler);
void edF32Matrix4ToEulerSoft(edF32MATRIX4* m0, edF32VECTOR3* v0, char* rotationOrder);
edF32MATRIX4* edF32Matrix4FromEulerSoft(edF32MATRIX4* m0, edF32VECTOR3* v0, char* order);
void edF32Matrix4FromEulerOrdSoft(edF32MATRIX4* rotatedMatrix, char* rotationOrder, float* rotationAngles);
void edQuatShortestSLERPHard(float delta, edF32VECTOR4* outRotation, edF32VECTOR4* currentRotation, edF32VECTOR4* targetRotation);
void edF32Vector3LERPSoft(float delta, edF32VECTOR3* outWorldLocation, edF32VECTOR3* currentLocation, edF32VECTOR3* targetLocation);
void edF32Vector4AddHard(edF32VECTOR4* v0, edF32VECTOR4* v1, edF32VECTOR4* v2);

void edF32Matrix4GetInverseOrthoHard(edF32MATRIX4* m0, edF32MATRIX4* m1);
void edF32Vector4ScaleHard(float t, edF32VECTOR4* v0, edF32VECTOR4* v1);

void edF32Matrix4MulF32Matrix4Hard(edF32MATRIX4* m0, edF32MATRIX4* m1, edF32MATRIX4* m2);

void edF32Matrix4SetIdentityHard(edF32MATRIX4* m0);

void edF32Matrix4TranslateHard(edF32MATRIX4* m0, edF32MATRIX4* m1, edF32VECTOR4* v0);

void edF32Matrix4MulF32Vector4Hard(edF32VECTOR4* v0, edF32MATRIX4* m0, edF32VECTOR4* v1);

#endif // _MATH_OPS_H
