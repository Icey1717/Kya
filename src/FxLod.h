#ifndef FX_LOD_H
#define FX_LOD_H

#include "Types.h"
#include "Fx.h"

struct ByteCode;

class CLodFx : public CNewFx
{
public:
	virtual void Draw();
	virtual void Kill() { IMPLEMENTATION_GUARD(); }
	virtual void Start(float param_1, float param_2) { IMPLEMENTATION_GUARD(); }
	virtual int GetType();
	virtual void NotifySonIsDead(CNewFx* pSon, int) { IMPLEMENTATION_GUARD(); }
	void Manage() { IMPLEMENTATION_GUARD(); }
};

class CFxLodScenaricData
{
public:
	void Init();
	void Create(ByteCode* pByteCode);
};

class CFxLodManager : public CFxPoolManager<CLodFx, CFxLodScenaricData>
{
public:
	virtual void* InstanciateFx(uint scenaricDataIndex, FX_MATERIAL_SELECTOR selector) { IMPLEMENTATION_GUARD(); }
};

#endif //FX_LOD_H