#include "CinematicManager.h"

CinematicManager* g_CinematicManager_0048efc;

CinematicManager::CinematicManager()
{
	pSubObj_0x1c = (CinematicObjectB*)0x0;
	field_0x20 = -1;
	field_0x24 = -1;
	g_CinematicManager_0048efc = this;
	ppCinematicObjB_A = (CinematicObjectB**)0x0;
	numCutscenes_0x8 = 0;
	ppCinematicObjB_B = (CinematicObjectB**)0x0;
	count_0x14 = 0;
	pCinematicObjB_0x18 = (CinematicObjectB*)0x0;
	field_0x28 = 0;
	field_0x2c = 0;
	return;
}
