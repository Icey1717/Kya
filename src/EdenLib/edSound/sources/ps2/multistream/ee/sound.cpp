// ACtuall sound.c

#include "Types.h"
#include "edSound/edSoundPlay.h"

int gMaxStreamLimit_00449118;
byte SOUND_STREAM_STATUS[48];
byte BYTE_ARRAY_00488990[48];
byte BYTE_ARRAY_00488930[48];
byte BYTE_ARRAY_00488960[48];
byte BYTE_ARRAY_004889c0[48];
int SOUND_STREAM_POSITION_BLOCK[48];
int SOUND_STREAM_POSITION_OFFSET[48];
int INT_ARRAY_00488d50[48];
int INT_ARRAY_00488840[48];
int INT_ARRAY_00488a50[48];
ushort USHORT_ARRAY_004889f0[48];

int SOUND_Command = 0;

int SOUND_InitIOP(void)
{
	for (int i = 0; i < 48; i++) {
		SOUND_STREAM_STATUS[i] = 0;
		BYTE_ARRAY_00488990[i] = 0;
		BYTE_ARRAY_00488930[i] = 0;
		BYTE_ARRAY_00488960[i] = 0;
		BYTE_ARRAY_004889c0[i] = 0;
		SOUND_STREAM_POSITION_BLOCK[i] = 0;
		SOUND_STREAM_POSITION_OFFSET[i] = 0;
		INT_ARRAY_00488d50[i] = 0;
		INT_ARRAY_00488840[i] = 0;
		INT_ARRAY_00488a50[i] = 0;
		USHORT_ARRAY_004889f0[i] = 0;
	}

	IMPLEMENTATION_GUARD_PS2();

	return 1;
}

int SOUND_SetMaxStreamLimit(uint nbStreams)
{
	int iVar1;

	iVar1 = -1;
	if (nbStreams < 0x30) {
		gMaxStreamLimit_00449118 = nbStreams;

#ifdef PLATFORM_PS2
		if (SOUND_Command != -1) {
			printf(">>>>> ERROR: SOUND_StartCommand");
			printf("Previous command (%d) not completed\n", SOUND_Command);
			printf("To complete a command, it must end with SOUND_EndCommand\n");
		}
		SOUND_Command = 0x10;
		IOPCommandData.IOPCommandData = (undefined2)nbStreams;
		SOUND_DataCounter = 1;
		SOUND_EndCommand();
#endif
		iVar1 = 0;
	}
	return iVar1;
}

int SOUND_FindFreeStream(void)
{
	int iVar1;
	byte* pbVar2;

	iVar1 = 0;
	if (0 < (int)gMaxStreamLimit_00449118) {
		pbVar2 = SOUND_STREAM_STATUS;
		do {
			if (*pbVar2 == 0) {
				return iVar1;
			}

			if (*pbVar2 == 3) {
				return iVar1;
			}

			iVar1 = iVar1 + 1;
			pbVar2 = pbVar2 + 1;
		} while (iVar1 < (int)gMaxStreamLimit_00449118);
	}

	printf("SOUND_FindFreeStream\n");
	printf("No Spare streams free\n");

	return -1;
}

void SOUND_AllocateStreamBuffer(int streamBufferId, void* pMem, uint size)
{
	SOUND_STREAM_POSITION_BLOCK[streamBufferId] = 0;
	SOUND_STREAM_POSITION_OFFSET[streamBufferId] = 0;

#ifdef PLATFORM_PS2
	if (SOUND_Command != -1) {
		printf(">>>>> ERROR: SOUND_StartCommand");
		printf("Previous command (%d) not completed\n", SOUND_Command);
		printf("To complete a command, it must end with SOUND_EndCommand\n");
	}
	SOUND_Command = 0xd;
	IOPCommandData.IOPCommandData = (undefined2)streamBufferId;
	SOUND_DataCounter = 5;
	SOUND_EndCommand();
	IOPCommandData.field_0x6 = (undefined2)(size >> 0x10);
	IOPCommandData.field_0x8 = (undefined2)size;
	IOPCommandData.field_0x2 = (undefined2)((uint)pMem >> 0x10);
	IOPCommandData.field_0x4 = SUB42(pMem, 0);
#endif

	return;
}

void SOUND_CreateFileInfoFromLsn(ushort streamFileId, uint lsn, uint fileSize, undefined8 param_4, undefined8 param_5)
{
#ifdef PLATFORM_PS2
	if (SOUND_Command != -1) {
		printf(">>>>> ERROR: SOUND_StartCommand");
		printf("Previous command (%d) not completed\n", SOUND_Command);
		printf("To complete a command, it must end with SOUND_EndCommand\n");
	}

	SOUND_Command = 0x1e;
	IOPCommandData.field_0x2 = (undefined2)(lsn >> 0x10);
	IOPCommandData.field_0x4 = (undefined2)lsn;
	IOPCommandData.field_0x6 = (undefined2)((ulong)fileSize >> 0x10);
	IOPCommandData.field_0x8 = (undefined2)fileSize;
	IOPCommandData.field_0xc = (undefined2)((ulong)param_5 >> 0x10);
	IOPCommandData.field_0xe = (undefined2)param_5;
	SOUND_DataCounter = 8;
	IOPCommandData.IOPCommandData = streamFileId;
	IOPCommandData.field_0xa = (undefined2)param_4;
	SOUND_EndCommand();
#endif

	return;
}

int SOUND_GetStreamInfo(uint streamID, SOUND_StreamInfo* outStreamInfo)
{
	int iVar1;
	int iVar2;

	IMPLEMENTATION_GUARD_PS2(
	outStreamInfo->field_0x0 = DAT_00449110;
	outStreamInfo->b10 = (byte)DAT_004490d4;
	outStreamInfo->b8 = (byte)DAT_004490d0;
	outStreamInfo->b9 = (byte)SOUND_CD_STATUS;
	outStreamInfo->b5 = SOUND_CD_ERROR;
	outStreamInfo->b6 = DAT_00449100;
	outStreamInfo->b7 = SOUND_CD_IERROR;)
	if (streamID < gMaxStreamLimit_00449118) {
		if ((SOUND_STREAM_STATUS[streamID] == 0) || (SOUND_STREAM_STATUS[streamID] == 3)) {
			outStreamInfo->playing = 0;
		}
		else {
			outStreamInfo->playing = 1;
		}
		iVar2 = streamID * 4;
		outStreamInfo->int1 = INT_ARRAY_00488d50[streamID];
		outStreamInfo->b17 = BYTE_ARRAY_00488930[streamID];
		outStreamInfo->b18 = BYTE_ARRAY_00488990[streamID];
		iVar1 = 0;
		outStreamInfo->b19 = BYTE_ARRAY_00488960[streamID];
		outStreamInfo->field_0x18 = INT_ARRAY_00488840[streamID];
		outStreamInfo->field_0x1c = INT_ARRAY_00488a50[streamID];
		outStreamInfo->b20 = BYTE_ARRAY_004889c0[streamID];
		outStreamInfo->field_0x16 = USHORT_ARRAY_004889f0[streamID];
		outStreamInfo->field_0x20 = SOUND_STREAM_POSITION_BLOCK[streamID];
		outStreamInfo->int4 = SOUND_STREAM_POSITION_OFFSET[streamID];
	}
	else {
		printf(">>>>>ERROR: SOUND_GetStreamInfo\n");
		printf("Stream (%d) out of range\n", streamID);
		printf("Call SOUND_SetMaxStreamLimit to allow more\n");
		printf("Only NON-STREAM related info is valid\n");
		printf("(such as CD status etc..)\n");
		iVar1 = -1;
	}

	return iVar1;
}

void SOUND_CloseStreamBuffer(int streamBufferId)
{
#ifdef PLATFORM_PS2
	if (SOUND_Command != -1) {
		printf(">>>>> ERROR: SOUND_StartCommand");
		printf("Previous command (%d) not completed\n", SOUND_Command);
		printf("To complete a command, it must end with SOUND_EndCommand\n");
	}

	SOUND_Command = 0xf;
	IOPCommandData.IOPCommandData = (undefined2)streamBufferId;
	SOUND_DataCounter = 1;
	SOUND_EndCommand();
#endif
	return;
}

void SOUND_FreeFileID(int streamFileId)
{
#ifdef PLATFORM_PS2
	if (SOUND_Command != -1) {
		printf(">>>>> ERROR: SOUND_StartCommand");
		printf("Previous command (%d) not completed\n", SOUND_Command);
		printf("To complete a command, it must end with SOUND_EndCommand\n");
	}

	SOUND_Command = 0x20;
	IOPCommandData.IOPCommandData = (undefined2)streamFileId;
	SOUND_DataCounter = 1;
	SOUND_EndCommand();
#endif

	return;
}

