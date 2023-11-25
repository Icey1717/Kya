#ifndef _ANM_SKELETON_H
#define _ANM_SKELETON_H

#include "Types.h"

struct edANM_SKELETON {
	ushort boneCount;
	ushort flags;
};

struct edAnmSkeleton {
	edANM_SKELETON* pTag;

	static edAnmSkeleton TheNullOne;

	int NodeIndexFromID(uint key);
};

#endif //_ANM_SKELETON_H
