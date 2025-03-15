#ifndef FX_LOD_H
#define FX_LOD_H

#include "Types.h"
#include "Fx.h"

struct ByteCode;

class CFxLod : public CNewFx
{

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

};

#endif //FX_LOD_H