#ifndef SETTINGS_H
#define SETTINGS_H

#include "Types.h"

class CSettings {
public:
	//undefined fields seem to be unused (previously used? The May 12 prototype did have some extra settings)
	static int GetMaxSaveBufferSize();
	bool LoadFromBuffer(void* pDataV);
	void SetSettingsToGlobal();
	void StoreGlobalSettings();
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