#ifndef FX_COMPOSITE_H
#define FX_COMPOSITE_H

#include "Types.h"
#include "Fx.h"

struct ByteCode;

class CFxCompositeScenaricData;

class CFxNewComposite : public CNewFx
{
public:
	virtual void Draw();
	virtual void Kill();
	virtual void Start(float param_1, float param_2);
	virtual void Stop(float param_1);
	virtual void Func_0x28() { IMPLEMENTATION_GUARD(); }
	virtual int GetType();
	virtual void Func_0x30(float param_1) { IMPLEMENTATION_GUARD(); }
	virtual void NotifySonIsDead(CNewFx* pSon, int index);
	virtual void SpatializeOnActor(uint flags, CActor* pActor, uint boneId);
	virtual void Func_0x3c() { IMPLEMENTATION_GUARD(); }

	void Manage();
	void Instanciate(CFxCompositeScenaricData* pData, FX_MATERIAL_SELECTOR selector);

	uint nbComponentParticles;

	CFxHandle aFxHandles[8];

	float field_0x80;

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
	virtual void* InstanciateFx(uint scenaricDataIndex, FX_MATERIAL_SELECTOR selector);
};

#endif //FX_COMPOSITE_H