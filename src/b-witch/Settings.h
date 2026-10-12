#ifndef SETTINGS_H
#define SETTINGS_H

#include "Types.h"
#include "edSound/edSoundPlay.h"

struct SettingsSaveData
{
	int field_0x0;
	int field_0x4;
	int languageID;
	int audioMode;
	int musicVolume;
	int sfxVolume;
	int field_0x18;
	int setOffsetX;
	int setOffsetY;
	bool bEnableVibration;
	bool bEnableSubtitles;
	bool field_0x26;
	bool field_0x27;
};

class CSettings
{
public:
	//undefined fields seem to be unused (previously used? The May 12 prototype did have some extra settings)
	static int GetMaxSaveBufferSize();
	bool LoadFromBuffer(void* pDataV);
	void SetSettingsToGlobal();
	void StoreGlobalSettings();
	uint FUN_003c87f0(SettingsSaveData* param_2);

	bool bEnableVibration;
	undefined field_0x1;
	undefined field_0x2;
	undefined field_0x3;
	LANGUAGE languageID;
	bool bEnableSubtitles;
	byte bWidescreen;
	undefined field_0xa;
	undefined field_0xb;
	int setOffsetX;
	int setOffsetY;
	AUDIO_MODE audioMode;
	uint musicVolume;
	uint sfxVolume;
};

extern CSettings gSettings;

#endif // SETTINGS_H