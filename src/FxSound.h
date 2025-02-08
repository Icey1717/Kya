#ifndef FX_SOUND_H
#define FX_SOUND_H

#include "Types.h"
#include "Fx.h"

struct ByteCode;

class CFxSound : public CNewFx
{

};

class CFxSoundScenaricData
{
public:
	void Create(ByteCode* pByteCode);

	int field_0x0;
	int field_0x4;
};

class CFxSoundManager : public CFxPoolManager<CFxSound, CFxSoundScenaricData>
{
public:
};

#endif //FX_SOUND_H