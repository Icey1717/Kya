#ifndef FX_RANDOM_H
#define FX_RANDOM_H

#include "Types.h"
#include "Fx.h"

struct ByteCode;
class CFxRandomScenaricData;

class CRandomFx : public CNewFx
{
public:
	CRandomFx();

	virtual void Draw();
	virtual void Kill();
	virtual void Start(float param_1, float param_2);
	virtual void Stop(float param_1);
	virtual void Func_0x28();
	virtual int GetType();
	virtual void NotifySonIsDead(CNewFx* pSon, int param_2);
	virtual void SpatializeOnActor(uint flags, CActor* pActor, uint boneId);
	virtual void UpdateSpatializeActor(uint newFlags, edF32VECTOR4 *pNewPosition);

	virtual void Manage();

	CFxRandomScenaricData* pScenariacData;
	FX_MATERIAL_SELECTOR selector;
	CFxHandle randFxHandle;
	byte field_0x88;
	void* spatializeActor;
	uint spatializeFlags;
	uint spatializeBone;
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
	void Term();

	uint nbData;
	CFxRandomScenaricDataSubObj aSubObjs[8];
	float field_0x44;
};

class CFxRandomManager : public CFxPoolManager<CRandomFx, CFxRandomScenaricData>
{
public:
	virtual void* InstanciateFx(uint scenaricDataIndex, FX_MATERIAL_SELECTOR selector);
};

#endif //FX_RANDOM_H