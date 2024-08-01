#include "AnmSkeleton.h"
#include "MathOps.h"

edANM_SKELETON _AnmSkeleton = edANM_SKELETON( 0, 0 );
edAnmSkeleton edAnmSkeleton::TheNullOne = edAnmSkeleton(&_AnmSkeleton);

int edAnmSkeleton::NodeIndexFromID(uint key)
{
	ushort uVar1;
	uint* aIndexes;
	int iVar3;
	uint uVar4;
	int iVar5;
	int iVar6;
	uint* aIds;

	uVar1 = this->pTag->boneCount;
	if (uVar1 == 0) {
		iVar3 = -1;
	}
	else {
		uVar4 = (uint)uVar1;
		iVar6 = uVar1 - 1;
		aIds = (uint*)(this->pTag + uVar4 + 1);
		aIndexes = aIds + uVar4;
		iVar5 = 0;
		iVar3 = iVar6;
		if (0 < iVar6) {
			do {
				iVar6 = (int)uVar4 >> 1;
				if (aIds[iVar6] == key) {
					return aIndexes[iVar6];
				}
				if (aIds[iVar6] <= key) {
					iVar5 = iVar6 + 1;
					iVar6 = iVar3;
				}
				uVar4 = iVar6 + iVar5;
				iVar3 = iVar6;
			} while (iVar5 < iVar6);
		}
		iVar3 = -1;
		if (key == aIds[iVar6]) {
			iVar3 = aIndexes[iVar6];
		}
	}
	return iVar3;
}

void edAnmSkeleton::UnskinNMatrices(edF32MATRIX4* m0, edF32MATRIX4* m1, int index, int count)
{
	bool bVar1;
	ushort uVar2;
	edANM_SKELETON* pSkeleton;
	float fVar4;
	int iVar5;
	edF32MATRIX4* peVar6;
	edF32MATRIX4* peVar7;
	edF32MATRIX4* m2;
	int iVar8;
	edF32VECTOR4* v1;
	edF32VECTOR4 eStack16;

	pSkeleton = this->pTag;
	m2 = m1 + index;

	if ((pSkeleton->flags & 2) == 0) {
		uVar2 = pSkeleton->boneCount;
		iVar8 = count + -1;

		const uint offsetA = (((uint)uVar2 * 2 + (uint)uVar2) * 4 + index * 0x10) * 4;
		const uint offsetB = (uint)uVar2 * 0xc + 0x13 & 0xfffffff0;
		const uint totalOffset = offsetA + offsetB;
		peVar6 = reinterpret_cast<edF32MATRIX4*>(reinterpret_cast<char*>(pSkeleton) + totalOffset);
		
		if (0 < count) {
			do {
				edF32Matrix4GetInverseOrthoHard(m0, peVar6);
				edF32Matrix4MulF32Matrix4Hard(m0, m0, m2);
				m0 = m0 + 1;
				m2 = m2 + 1;
				peVar6 = peVar6 + 1;
				bVar1 = 0 < iVar8;
				iVar8 = iVar8 + -1;
			} while (bVar1);
		}
	}
	else {
		uVar2 = pSkeleton->boneCount;
		iVar8 = count + -1;
		const uint offsetA = (((uint)uVar2 * 2 + (uint)uVar2) * 4 + index * 4) * 4;
		const uint offsetB = (uint)uVar2 * 0xc + 0x13 & 0xfffffff0;
		const uint totalOffset = offsetA + offsetB;
		v1 = reinterpret_cast<edF32VECTOR4*>(reinterpret_cast<char*>(pSkeleton) + totalOffset);

		if (0 < count) {
			do {
				*m0 = *m2;

				edF32Vector4ScaleHard(-1.0f, &eStack16, v1);
				eStack16.w = 1.0f;
				edF32Matrix4MulF32Vector4Hard(&m0->rowT, m2, &eStack16);
				m0 = m0 + 1;
				m2 = m2 + 1;
				v1 = v1 + 1;
				bVar1 = 0 < iVar8;
				iVar8 = iVar8 + -1;
			} while (bVar1);
		}
	}
	return;
}
