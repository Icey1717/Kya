#ifndef FX_SOUND_H
#define FX_SOUND_H

#include "Types.h"
#include "Fx.h"

struct ByteCode;

class CFxSoundScenaricData;

class CFxNewSound : public CNewFx
{
public:
	virtual void Draw();
	virtual void Kill();
	virtual void Start(float param_1, float param_2);
	virtual void Pause() { IMPLEMENTATION_GUARD(); }
	virtual void Resume() { IMPLEMENTATION_GUARD(); }
	virtual void Stop(float param_1);
	virtual void Func_0x28() { IMPLEMENTATION_GUARD(); }
	virtual int GetType();
	virtual void Func_0x30(float param_1) { IMPLEMENTATION_GUARD(); }
	virtual void SetTimeScaler() { IMPLEMENTATION_GUARD(); }

	void Manage() { IMPLEMENTATION_GUARD(); }

	void Instanciate(CFxSoundScenaricData* pData, FX_MATERIAL_SELECTOR selector);

	FX_MATERIAL_SELECTOR field_0x80;
	CFxSoundScenaricData* field_0x84;
};

class CFxSoundScenaricData
{
public:
	void Init();
	void Create(ByteCode* pByteCode);
	void Term();

	int field_0x0;
	int field_0x4;
};

class CFxSoundManager : public CFxPoolManager<CFxNewSound, CFxSoundScenaricData>
{
public:
	virtual void* InstanciateFx(uint scenaricDataIndex, FX_MATERIAL_SELECTOR selector);
};

#endif //FX_SOUND_H