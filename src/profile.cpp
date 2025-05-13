#include "profile.h"

uint edProfileNew(uint, uint, uint, uint)
{
	IMPLEMENTATION_GUARD_PROFILE();
	return 0;
}

uint edProfileNew(uint, uint, uint, uint, char*)
{
	IMPLEMENTATION_GUARD_PROFILE();
	return 0;
}

void edProfileBegin(uint profileId)
{
	IMPLEMENTATION_GUARD_PROFILE();
}

void edProfileEnd(uint profileId)
{
	IMPLEMENTATION_GUARD_PROFILE();
}

void edProfileDel(uint profileId)
{
	IMPLEMENTATION_GUARD_PROFILE();
}
