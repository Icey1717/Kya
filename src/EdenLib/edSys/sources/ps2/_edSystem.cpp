#include "_edSystem.h"
#include <cassert>

#ifdef PLATFORM_PS2
void _edSystemPS2SpecificInit(void)
{
	// resets the entire VIF0 including VIF0-FIF0
	sceDevVif0Reset();

	// resets the entire VIF1 including VIF1-FIF0
	sceDevVif1Reset();

	// reset Vu0
	sceDevVu0Reset();

	// reset Vu1
	sceDevVu1Reset();

	// resets devices on the data path to GS (VIF1,Vu0,GIF)
	sceGsResetPath();

	// resets the DMAC
	sceDmaReset(1);

	return;
}

void _edSystemPS2SpecificAtMonInit(int argc, char** argv)
{
	assert(false);
}

#endif