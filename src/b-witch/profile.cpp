#include "profile.h"

byte GameProfile = 0;

void ProfileInit(byte bEnabled)
{
	undefined4* puVar1;

	GameProfile = bEnabled;

	if (bEnabled != 0) {
		IMPLEMENTATION_GUARD_PROFILE(
		puVar1 = static_cast<undefined4*>(edProfileGetConfig());
		*puVar1 = 0x80;
		puVar1[2] = 8;
		edProfileInit();
		DAT_00391911 = 1;)
	}

	return;
}

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
