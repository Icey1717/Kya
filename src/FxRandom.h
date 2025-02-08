#ifndef FX_RANDOM_H
#define FX_RANDOM_H

#include "Types.h"
#include "Fx.h"

struct ByteCode;

class CFxRandom : public CNewFx
{
	
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
	void Create(ByteCode* pByteCode);

	uint nbData;
	CFxRandomScenaricDataSubObj aSubObjs[8];
	float field_0x44;
};

class CFxRandomManager : public CFxPoolManager<CFxRandom, CFxRandomScenaricData>
{
public:
};

#endif //FX_RANDOM_H