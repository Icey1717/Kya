#ifndef FX_COMPOSITE_H
#define FX_COMPOSITE_H

#include "Types.h"
#include "Fx.h"

struct ByteCode;

class CFxCompositeScenaricData;

class CFxNewComposite : public CNewFx
{
public:
	void Manage() { IMPLEMENTATION_GUARD(); }
	void Instanciate(CFxCompositeScenaricData* pData, FX_MATERIAL_SELECTOR selector);

	uint nbComponentParticles;

	CFxHandle aFxHandles[8];

	float* field_0xc8;
};

class CFxCompositeScenaricData
{
public:
	CFxCompositeScenaricData();
	void Init();
	void Create(ByteCode* pByteCode);

	uint nbData;
	uint* aComponentParticles;
	float* field_0x8;
};

class CFxCompositeManager : public CFxPoolManager<CFxNewComposite, CFxCompositeScenaricData>
{
public:
	virtual void* InstanciateFx(uint param_2, FX_MATERIAL_SELECTOR selector);
};

#endif //FX_COMPOSITE_H