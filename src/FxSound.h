#ifndef FX_SOUND_H
#define FX_SOUND_H

#include "Types.h"
#include "Fx.h"

struct ByteCode;

class CFxSound : public CNewFx
{
public:
	void Manage() { IMPLEMENTATION_GUARD(); }
};

class CFxSoundScenaricData
{
public:
	void Init();
	void Create(ByteCode* pByteCode);

	int field_0x0;
	int field_0x4;
};

class CFxSoundManager : public CFxPoolManager<CFxSound, CFxSoundScenaricData>
{
public:
	virtual void* InstanciateFx(uint param_2, FX_MATERIAL_SELECTOR selector) { IMPLEMENTATION_GUARD(); }
};

#endif //FX_SOUND_H