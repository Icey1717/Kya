#ifndef FX_PARTICLE_H
#define FX_PARTICLE_H

#include "Types.h"
#include "Fx.h"

struct ed_g2d_manager;
struct ByteCode;

class CFxNewParticle : public CNewFx
{
	
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

	static void Level_Create(ByteCode* pByteCode);

	bool ReadPrtResChunk(ByteCode* pByteCode);

	uint nbParticleRes;
	CParticleRes* aParticleRes;
};

#endif //FX_PARTICLE_H