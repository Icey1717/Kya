#ifndef _MATH_OPS_H
#define _MATH_OPS_H

#include "Types.h"

#ifdef PLATFORM_PS2
#include <libvu0.h>
#endif

extern sceVu0FMATRIX g_ZeroMatrix_00431690;
extern Matrix g_IdentityMatrix;

#ifdef PLATFORM_WIN
void sceVu0Normalize(float* v0, float* v1);
void sceVu0TransposeMatrix(sceVu0FMATRIX m0, sceVu0FMATRIX m1);
#endif
void sceVu0InverseMatrix(Matrix* m0, Matrix* m1);

void ps2_vu0_sub_vector(Vector* v0, Vector* v1, Vector* v2);
void ps2_vu0_outer_product(Vector* v0, Vector* v1, Vector* v2);
void ps2_vu0_sqr_vector(Vector* v0, Vector* v1);

void CalculatePitchAngles2D_00193c70(Vector* pitchAngles, Vector* v0);

void RotateMatrixByAngle(float angle, Matrix* m0, Matrix* m1);
void ApplyRotationToMatrix(float angle, Matrix* outputMatrix, Matrix* inputMatrix);

void MatrixSetPosition_00267750(sceVu0FMATRIX m0, sceVu0FMATRIX m1, sceVu0FVECTOR v0);
void CalculateRotationMatrix_002673f0(float t0, Matrix* m0, Matrix* m1);

void ComputeMapRectangleMatrix_0029b9e0
(float startX, float endX, float startY, float endY, float startZ, float endZ, float startW, float endW, Matrix* m0);
void CalculateYAxisTransformMatrix_0029ba70(float x, float y, float yMin, float yMax, Matrix* m0);
uint ToPowerOfTwo_0029e3f0(uint value);

#endif // _MATH_OPS_H
