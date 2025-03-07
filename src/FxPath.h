#ifndef FX_PATH_H
#define FX_PATH_H

#include "Types.h"
#include "Fx.h"

struct ByteCode;

class CFxPath : public CFx
{
public:
	CFxPath();
	virtual void AddAll(ByteCode* pByteCode, CFx* aFx, int count);
	virtual void ManageAll(uint count, CFx* aFx);
	virtual void Draw();
	virtual void CheckpointResetAll(uint count, CFx* aFx);

	edF32VECTOR4 vector_0x60;
};

class CFxPathScenaricData
{

};

class CFxPathManager : public CFxPoolManager<CFxPath, CFxPathScenaricData>
{
public:

};

#endif //FX_PATH_H