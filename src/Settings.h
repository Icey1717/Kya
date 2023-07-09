#ifndef SETTINGS_H
#define SETTINGS_H

#include "Types.h"

class CSettings {
public:
	static int GetMaxSaveBufferSize();
	bool LoadFromBuffer(void* pDataV);
	void SetSettingsToGlobal();

	bool field_0x0;
	undefined field_0x1;
	undefined field_0x2;
	undefined field_0x3;
	LANGUAGE languageID;
	bool field_0x8;
	byte bWidescreen;
	undefined field_0xa;
	undefined field_0xb;
	int setOffsetX;
	int setOffsetY;
	undefined4 field_0x14;
	uint field_0x18;
	uint field_0x1c;
};

extern CSettings gSettings;

#endif // SETTINGS_H