#ifndef FX_RANDOM_H
#define FX_RANDOM_H

#include "Types.h"
#include "Fx.h"

struct ByteCode;

class CFxRandom : public CNewFx
{
public:
	void Manage() { IMPLEMENTATION_GUARD(); }
};

class CFxRandomScenaricData
{
public:
	class CFxRandomScenaricDataSubObj
	{
	public:
		uint field_0x0;
		float field_0x4;
	};

	CFxRandomScenaricData();
	void Init();
	void Create(ByteCode* pByteCode);

	uint nbData;
	CFxRandomScenaricDataSubObj aSubObjs[8];
	float field_0x44;
};

class CFxRandomManager : public CFxPoolManager<CFxRandom, CFxRandomScenaricData>
{
public:
	virtual void* InstanciateFx(uint param_2, FX_MATERIAL_SELECTOR selector) { IMPLEMENTATION_GUARD(); }
};

#endif //FX_RANDOM_H