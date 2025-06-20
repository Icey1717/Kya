#ifndef FX_PARTICLE_H
#define FX_PARTICLE_H

#include "Types.h"
#include "Fx.h"

struct ed_g2d_manager;
struct ByteCode;

class CFxParticleScenaricData;

class CFxNewParticle : public CNewFx
{
public:
	void Manage() { IMPLEMENTATION_GUARD(); }
	void Instanciate(CFxParticleScenaricData* pData, FX_MATERIAL_SELECTOR selector);
};

class CFxParticleScenaricData
{
public:
	class CFxParticleScenaricDataSubObj
	{
	public:
		uint field_0x0;
		int field_0x4;
	};

	void Init();
	void Create(ByteCode* pByteCode);

	uint field_0x0;
	int field_0x8;
	CFxParticleScenaricDataSubObj aSubObjs[7];
	uint nbData;
};

class CParticleRes
{
public:
	bool Init();

	union
	{
		uint fileIndex;
		char* pFileStart;
	};

	uint fileSize;
	uint nbEntries;
	uint aEntries[8];
	int nbLoadedRes;
	ulong aHashes[8];
	ed_g2d_manager* aManagers[8];
};

class CFxParticleManager : public CFxPoolManager<CFxNewParticle, CFxParticleScenaricData>
{
public:
	static uint _prt_bank_first_index;

	virtual void* InstanciateFx(uint param_2, FX_MATERIAL_SELECTOR selector);

	static void Level_Create(ByteCode* pByteCode);

	bool ReadPrtResChunk(ByteCode* pByteCode);

	uint nbParticleRes;
	CParticleRes* aParticleRes;
};

#endif //FX_PARTICLE_H