#ifndef FX_RANDOM_H
#define FX_RANDOM_H

#include "Types.h"
#include "Fx.h"

struct ByteCode;

class CRandomFx : public CNewFx
{
public:
	virtual void Draw();
	virtual void Kill() { IMPLEMENTATION_GUARD(); }
	virtual void Start(float param_1, float param_2) { IMPLEMENTATION_GUARD(); }
	virtual int GetType();
	virtual void NotifySonIsDead(CNewFx* pSon, int) { IMPLEMENTATION_GUARD(); }
	virtual void SpatializeOnActor(uint flags, CActor* pActor, uint boneId) { IMPLEMENTATION_GUARD(); }
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

class CFxRandomManager : public CFxPoolManager<CRandomFx, CFxRandomScenaricData>
{
public:
	virtual void* InstanciateFx(uint param_2, FX_MATERIAL_SELECTOR selector) { IMPLEMENTATION_GUARD(); }
};

#endif //FX_RANDOM_H