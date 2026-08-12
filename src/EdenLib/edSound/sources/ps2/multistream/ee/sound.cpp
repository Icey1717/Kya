// ACtuall sound.c

#include "Types.h"

int gMaxStreamLimit_00449118;

int SOUND_SetMaxStreamLimit(uint nbStreams)
{
	int iVar1;

	iVar1 = -1;
	if (nbStreams < 0x30) {
		gMaxStreamLimit_00449118 = nbStreams;

#ifdef PLATFORM_PS2
		if (SOUND_Command != -1) {
			/* >>>>> ERROR: SOUND_StartCommand
			   Previous command (%d) not completed\n
			   To complete a command, it must end with SOUND_EndCommand\n */
			printf(s_>>>> > _ERROR:_SOUND_StartCommand_00431e00);
			printf(s_Previous_command_(% d)_not_comple_00431e30, SOUND_Command);
			printf(s_To_complete_a_command, _it_must_e_00431e60);
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
