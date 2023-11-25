#include "AnmSkeleton.h"

edANM_SKELETON _AnmSkeleton = { 0, 0 };
edAnmSkeleton edAnmSkeleton::TheNullOne = { &_AnmSkeleton };

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