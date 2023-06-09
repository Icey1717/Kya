#ifndef _MATH_OPS_H
#define _MATH_OPS_H

#include "Types.h"

#ifdef PLATFORM_PS2
#include <libvu0.h>
#endif

extern sceVu0FMATRIX g_ZeroMatrix_00431690;
extern edF32MATRIX4 g_IdentityMatrix;

#ifdef PLATFORM_WIN
void sceVu0Normalize(float* v0, float* v1);
#endif
void sceVu0ApplyMatrix(Vector* v0, edF32MATRIX4* m0, Vector* v1);
void sceVu0TransposeMatrixFixed(sceVu0FMATRIX m0, sceVu0FMATRIX m1);
void sceVu0InverseMatrix(edF32MATRIX4* m0, edF32MATRIX4* m1);

void ps2_vu0_sub_vector(Vector* v0, Vector* v1, Vector* v2);
void ps2_vu0_outer_product(Vector* v0, Vector* v1, Vector* v2);
void ps2_vu0_sqr_vector(Vector* v0, Vector* v1);

void CalculatePitchAngles2D_00193c70(Vector* pitchAngles, Vector* v0);

void RotateMatrixByAngle(float angle, edF32MATRIX4* m0, edF32MATRIX4* m1);
void ApplyRotationToMatrix(float angle, edF32MATRIX4* outputMatrix, edF32MATRIX4* inputMatrix);

void edF32Matrix4ScaleHard(sceVu0FMATRIX m0, sceVu0FMATRIX m1, sceVu0FVECTOR v0);
void edF32Matrix4RotateZHard(float t0, edF32MATRIX4* m0, edF32MATRIX4* m1);

void ComputeMapRectangleMatrix_0029b9e0
(float startX, float endX, float startY, float endY, float startZ, float endZ, float startW, float endW, edF32MATRIX4* m0);
void CalculateYAxisTransformMatrix_0029ba70(float x, float y, float yMin, float yMax, edF32MATRIX4* m0);
uint GetGreaterPower2Val(uint value);

#endif // _MATH_OPS_H
