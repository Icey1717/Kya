#ifndef FX_COMPOSITE_H
#define FX_COMPOSITE_H

#include "Types.h"
#include "Fx.h"

struct ByteCode;

class CFxComposite : public CNewFx
{

};

class CFxCompositeScenaricData
{
public:
	CFxCompositeScenaricData();
	void Create(ByteCode* pByteCode);

	uint nbData;
	uint* field_0x4;
	float* field_0x8;
};

class CFxCompositeManager : public CFxPoolManager<CFxComposite, CFxCompositeScenaricData>
{
public:
};

#endif //FX_COMPOSITE_H