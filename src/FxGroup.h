#ifndef FX_GROUP_H
#define FX_GROUP_H

#include "Types.h"
#include "Fx.h"

struct ByteCode;

class CFxGroup : public CFx
{
public:
	virtual void AddAll(ByteCode* pByteCode, CFx* aFx, int count);
	virtual void ManageAll(uint count, CFx* aFx);
	virtual void Draw();
	virtual void CheckpointResetAll(uint count, CFx* aFx);
};

class CFxGroupScenaricData
{

};

class CFxGroupManager : public CFxPoolManager<CFxGroup, CFxGroupScenaricData>
{
public:
};

#endif //FX_GROUP_H