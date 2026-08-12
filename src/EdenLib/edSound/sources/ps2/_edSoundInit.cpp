#include "edSound/ps2/_edSoundInit.h"
#include "edSound/edSoundInit.h"
#include "edSound/edSoundPlay.h"
#include "edMem.h"

ed_sound_voice_position* pedSoundVoicePosition;
void* edComBuffer_IOP;

void _edSoundInit(undefined4 param_1)
{
	bool bVar1;
	undefined4* puVar2;
	int* piVar3;
	int iVar4;
	//RPC_Client* pRVar5;
	int iVar6;
	ulong uVar7;
	uint uVar8;
	undefined8 uVar9;
	long lVar10;
	undefined8 uVar11;
	undefined8 uVar12;
	undefined8 uVar13;
	undefined8 uVar14;
	int* piVar15;
	//RPC_Client* pRVar16;

	IMPLEMENTATION_GUARD_AUDIO(
	edComDoubleBuffers = edMemAllocAlign(H_MAIN, 0x1004, 0x10);
	edComBuffer = static_cast<undefined4*>(edMemAllocAlign(H_MAIN, 0x1004, 0x10));
	edComCurrentBufferIndex = 1;
	DAT_004490ac = edComBuffer;
	*edComBuffer = 0;)
	pedSoundVoicePosition = static_cast<ed_sound_voice_position*>(edMemAllocAlign(TO_HEAP(H_MAIN), sizeof(ed_sound_voice_position) * 0x30, 0x10));
	uVar8 = 0;
	iVar6 = 0;

	edComBuffer_IOP = pedSoundVoicePosition;
	for (uint i = 0; i < 48; i++) {
		pedSoundVoicePosition[i].position = -2;
	}

	IMPLEMENTATION_GUARD_AUDIO(
	FlushCache(0);
	pedSoundVoicePosition = static_cast<undefined*>((uint)pedSoundVoicePosition | 0x20000000);
	_pedSoundRPCSendBufferUncached = static_cast<undefined4*>(0x20486800);
	_pedSoundRPCReceiveBufferUncached = 0x20487000;
	piVar3 = static_cast<int*>(operator).new(0x34);
	if (piVar3 != (int*)0x0) {
		*piVar3 = 0x4553494e;
		piVar3[1] = 1;
		iVar6 = piVar3[1] + -1;
		if (piVar3[1] != 0) {
			piVar15 = piVar3 + iVar6 * 10;
			do {
				do {
					sceSifBindRpc(static_cast<sceSifClientData*>(piVar15 + 3), iVar6 + *piVar3, 0);
					do {
						iVar4 = sceSifCheckStatRpc(static_cast<sceSifClientData*>(piVar15 + 3));
					} while (iVar4 != 0);
				} while (piVar15[0xc] == 0);
				piVar15 = piVar15 + -10;
				bVar1 = iVar6 != 0;
				iVar6 = iVar6 + -1;
			} while (bVar1);
		}
		piVar3[2] = piVar3[1];
	}
	uVar8 = soundConfig.field_0x14;
	PTR_DAT_00449094 = static_cast<undefined*>(piVar3);
	FUN_00289e00(22000.0f, 50.0f, 0.1f, 0x1400, 8);
	puVar2 = _pedSoundRPCSendBufferUncached;
	uVar7 = 0;
	uVar9 = 0;
	uVar11 = 0x1c;
	uVar12 = 0;
	uVar13 = 0;
	*_pedSoundRPCSendBufferUncached = param_1;
	puVar2[1] = 0x1004;
	puVar2[2] = 0x7764e28c;
	puVar2[3] = 1;
	puVar2[4] = 0;
	puVar2[5] = 0x1400;
	puVar2[6] = uVar8;
	lVar10 = static_cast<long>(int)_pedSoundRPCSendBufferUncached;
	uVar14 = 0;
	_edSysCallRPC(0, 0, 0, lVar10, 0x1c, 0, 0, 0, (uint*)0x0, *static_cast<int*>(PTR_DAT_00449094 + 4), static_cast<sceSifClientData*>(PTR_DAT_00449094 + 0xc), static_cast<int*>(PTR_DAT_00449094 + 8));
	pRVar5 = static_cast<RPC_Client*>(operator).new(0xd4);
	if (pRVar5 != (RPC_Client*)0x0) {
		*static_cast<int*>(pRVar5) = 0x45534e44;
		pRVar5->field_0x4 = 5;
		iVar6 = pRVar5->field_0x4 + -1;
		if (pRVar5->field_0x4 != 0) {
			pRVar16 = static_cast<RPC_Client*>((int)pRVar5 + iVar6 * 0x28);
			do {
				do {
					uVar9 = 0;
					uVar7 = SEXT48(iVar6 + *static_cast<int*>(pRVar5));
					sceSifBindRpc(&pRVar16->field_0xc, iVar6 + *static_cast<int*>(pRVar5), 0);
					do {
						iVar4 = sceSifCheckStatRpc(&pRVar16->field_0xc);
					} while (iVar4 != 0);
				} while ((pRVar16->field_0xc).serve == (_sif_serve_data*)0x0);
				pRVar16 = (RPC_Client*)&pRVar16[-1].field_0xc;
				bVar1 = iVar6 != 0;
				iVar6 = iVar6 + -1;
			} while (bVar1);
		}
		pRVar5->field_0x8 = pRVar5->field_0x4;
	}
	_pedSoundRPCClient = pRVar5;

	SOUND_InitIOP();
	SOUND_InitCD(1, uVar7, uVar9, lVar10, uVar11, uVar12, uVar13, uVar14);
	SOUND_InitStreamData(0, 1, 0, lVar10, uVar11, uVar12, uVar13, uVar14);
	FlushIOPCommand(0, 0);
	USHORT_00449140 = 0;
	_edSoundApplyCommandMSLastCount = 0;)

	memset(_edSoundAllocatedVoices, 0, sizeof(_edSoundAllocatedVoices));
	_edSoundSetVoiceRange(soundConfig.nbVoices, soundConfig.field_0x10);

	return;
}