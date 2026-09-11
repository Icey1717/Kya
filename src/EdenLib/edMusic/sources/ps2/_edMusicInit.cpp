#include "edMusic/edMusic.h"
#include "edMem.h"
#ifdef PLATFORM_WIN
#include "edMusicService.h"
#endif

void _edMusicInit(ed_music_config* pConfig)
{
#ifdef PLATFORM_WIN
	Audio::InitializeMusic(pConfig->nbStreams, pConfig->nbVoices);
#endif
	bool bVar1;
	int* piVar2;
	int iVar3;
	void** ppvVar4;
	int* piVar5;
	uint uVar6;
	int iVar7;

	IMPLEMENTATION_GUARD_PS2(
		_edMusicCommandsAllocBuffer = edMemAlloc(TO_HEAP(H_MAIN), pConfig->nbStreams * 0x16);

	uVar6 = 0;
	ppvVar4 = (void**)&_edMusicCommands;
	do {
		*ppvVar4 = (void*)((int)_edMusicCommandsAllocBuffer + uVar6 * pConfig->nbStreams);
		memset(*ppvVar4, 0, pConfig->nbStreams);
		uVar6 = uVar6 + 1;
		ppvVar4 = ppvVar4 + 1;
	} while (uVar6 < 0x16);

	memset(_edMusicCommandsUsed, 0, 0x16);
	memset(&_edMusicReturnQueue, 0, 0x400);
	_edMusicReturnQueue = 0xffffffff;)

		IMPLEMENTATION_GUARD_PS2(
			piVar2 = (int*)operator.new(0xac);
	if (piVar2 != (int*)0x0) {
		*piVar2 = 0x454d5553;
		piVar2[1] = 4;
		iVar7 = piVar2[1] + -1;
		if (piVar2[1] != 0) {
			piVar5 = piVar2 + iVar7 * 10;
			do {
				do {
					sceSifBindRpc((sceSifClientData*)(piVar5 + 3), iVar7 + *piVar2, 0);
					do {
						iVar3 = sceSifCheckStatRpc((sceSifClientData*)(piVar5 + 3));
					} while (iVar3 != 0);
				} while (piVar5[0xc] == 0);
				piVar5 = piVar5 + -10;
				bVar1 = iVar7 != 0;
				iVar7 = iVar7 + -1;
			} while (bVar1);
		}
		piVar2[2] = piVar2[1];
	}

	UINT_00482270 = edMusicConfig.nbVoices;
	UINT_00482274 = edMusicConfig.nbStreams;
	UINT_00482278 = edMusicConfig.nbSongs;
	UINT_0048227c = edMusicConfig.nbBanks;
	_pedMusicRPCClient = piVar2;
	_edSysCallRPC(0, 0, 0, 0x482270, 0x40, 0, 0, 0, (uint*)0x0, piVar2[1], (sceSifClientData*)(piVar2 + 3), piVar2 + 2);)

		return;
}

void _edMusicSetOutputMode(AUDIO_MODE newMode)
{
	IMPLEMENTATION_GUARD_PS2(
		AUDIO_MODE local_4;

	local_4 = newMode;
	_edMusicQueueCommand(7, 0, &local_4, 4);)
		return;
}
