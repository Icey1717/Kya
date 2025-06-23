#ifndef FX_SOUND_H
#define FX_SOUND_H

#include "Types.h"
#include "Fx.h"

struct ByteCode;

class CFxNewSound : public CNewFx
{
public:
	virtual void Draw();
	virtual void Kill() { IMPLEMENTATION_GUARD(); }
	virtual void Start(float param_1, float param_2) { IMPLEMENTATION_GUARD(); }
	virtual int GetType();
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

class CFxSoundManager : public CFxPoolManager<CFxNewSound, CFxSoundScenaricData>
{
public:
	virtual void* InstanciateFx(uint param_2, FX_MATERIAL_SELECTOR selector) { IMPLEMENTATION_GUARD(); }
};

#endif //FX_SOUND_H