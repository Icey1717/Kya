#ifndef _ANM_SKELETON_H
#define _ANM_SKELETON_H

#include "Types.h"

struct edANM_SKELETON {
	edANM_SKELETON() {}
	edANM_SKELETON(ushort inBoneCount, ushort inFlags) : boneCount(inBoneCount), flags(inFlags) {}

	ushort boneCount;
	ushort flags;
};

struct edAnmSkeleton {
	edAnmSkeleton() {}

	edAnmSkeleton(edANM_SKELETON* pInTag) : pTag(pInTag) {}

	edANM_SKELETON* pTag;

	static edAnmSkeleton TheNullOne;

	int NodeIndexFromID(uint key);
};

#endif //_ANM_SKELETON_H
