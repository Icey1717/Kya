#ifndef FX_LOD_H
#define FX_LOD_H

#include "Types.h"
#include "Fx.h"

struct ByteCode;

class CFxLod : public CNewFx
{
public:
	void Manage() { IMPLEMENTATION_GUARD(); }
};

class CFxLodScenaricData
{
public:
	void Init();
	void Create(ByteCode* pByteCode);
};

class CFxLodManager : public CFxPoolManager<CFxLod, CFxLodScenaricData>
{
public:
	virtual void* InstanciateFx(uint param_2, FX_MATERIAL_SELECTOR selector) { IMPLEMENTATION_GUARD(); }
};

#endif //FX_LOD_H