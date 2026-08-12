#include "edSound/edSoundInit.h"
#include "edSound/edSoundPlay.h"
#include "edSound/edSoundInstance.h"
#include "EdenLib/edSys/sources/EdHandlers.h"
#include "edMem.h"
#include "edVideo/VideoD.h"
#include "MathOps.h"

edSoundConfig soundConfig;

edSoundConfig* edSoundGetConfig(void)
{
	return &soundConfig;
}

uint edSoundVoicesNumberGet(void)
{
	return 0x30;
}

struct ED_SOUND_3D_DATA
{
	edF32VECTOR3 field_0x0;
	edF32VECTOR3 field_0xc;
	float field_0x18;
	float field_0x1c;
	float field_0x20;
	byte field_0x24;
};

ED_SOUND_3D_DATA edSound3DDataDefault;

void _edSoundAcousticInit(void)
{}

void edSoundAcousticInit(void)
{
	float local_10;
	float fStack12;

	edSound3DDataDefault.field_0x0 = gF32Vector3Zero;
	edSound3DDataDefault.field_0xc = gF32Vector3Zero;
	edSound3DDataDefault.field_0x1c = 1.0f;
	edSound3DDataDefault.field_0x18 = -1.0f;
	_edSoundAcousticInit();

	return;
}

void edSoundFlushHandler(int, int, char*)
{
	if (edSoundGlobalParams.field_0x58 != 0) {
		edSoundFlush();
	}

	return;
}

void edSoundInit(void)
{
	uint uVar2;

	edDebugPrintf("edSound v1.2\ncompiled on Oct  3 2003 at 15:00:14");
	edDebugPrintf("edSound sync number [EE] : %d\n", 0);

	edSoundInitInstances(soundConfig.nbMaxInstances);

	edSoundInstanceCom = (edSoundInstanceComType*)edMemAlloc(TO_HEAP(H_MAIN), soundConfig.nbMaxInstances * sizeof(edSoundInstanceComType));
	uVar2 = 0;
	if (soundConfig.nbMaxInstances != 0) {
		do {
			edSoundInstanceCom[uVar2].flags = 0;
			edSoundInstanceCom[uVar2].soundInstanceId = 0;
			uVar2 = uVar2 + 1;
		} while (uVar2 < soundConfig.nbMaxInstances);
	}

	pedSoundInstancesToDelete = (uint*)edMemAlloc(TO_HEAP(H_MAIN), soundConfig.nbMaxInstances * sizeof(uint));
	edSoundInstancesToDeleteNb = 0;

#ifdef PLATFORM_PS2
	_edSoundInit(soundConfig.nbMaxInstances);
#endif
	edSoundAcousticInit();
	edSoundGlobalParams.field_0x20 = 0;
	edSoundGlobalParams.field_0x6c = 0;
	edSoundGlobalParams.g_DesiredFrameTime_00483824 = 0.02f;
	edSoundGlobalParams.outputMode = STEREO;
	edSoundGlobalParams.finishedInstancesCallback = (edSoundFinishedInstancesCallback)0x0;
	edSoundGlobalParams.field_0x5c = 343.5f;
	edSoundGlobalParams.field_0x64 = 1.0f;
	edSoundGlobalParams.field_0x60 = 1.0f;
	edSoundGlobalParams.field_0x68 = 1.0f;
	edSoundGlobalParams.volume = 1.0f;
	edSoundGlobalParams.field_0x58 = soundConfig.field_0x8;
	edSysHandlersAdd(edVideoHandlers.nodeParent, edVideoHandlers.entries, edVideoHandlers.maxEventID, 7, edSoundFlushHandler, 1, 1);
	edSoundFlush();

	return;
}

void edSoundInitFunc(float param_1)
{
	edSoundGlobalParams.field_0x60 = param_1;
	edSoundGlobalParams.field_0x64 = param_1 * param_1;
	return;
}
