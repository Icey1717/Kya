#ifndef FX_MANAGER_H
#define FX_MANAGER_H

#include "Types.h"
#include "LargeObject.h"

struct CFxManager : public CObjectManager {

	CFxManager();
	void AddPool(ByteCode* pMemoryStream);

	uint count_0x4;
	undefined* field_0x8;
	undefined* orderedCountArray;
	uint effectCountByType[7];
	int totalEffectCount_0x2c;
	int field_0x30;
	int field_0x34;
	int field_0x38;
	int field_0x3c;
	int field_0x40;
	int field_0x44;
	int field_0x48;
	struct EffectsManagerObjBase* aEffectCategory[7];
	undefined* field_0x68;
	int field_0x6c;
};


#endif // FX_MANAGER_H
