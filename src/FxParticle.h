#ifndef FX_PARTICLE_H
#define FX_PARTICLE_H

#include "Types.h"
#include "Fx.h"
#include "edParticles.h"

struct ed_g2d_manager;
struct ByteCode;

class CFxParticleScenaricData;

class CFxNewParticle : public CNewFx
{
public:
	virtual void Draw();
	virtual void Kill() { IMPLEMENTATION_GUARD(); }
	virtual int GetType();
	void Manage();
	void Instanciate(CFxParticleScenaricData* pData, FX_MATERIAL_SELECTOR selector);

	float field_0x80;
	uint field_0x84;
	ParticleFileData* pFileData;
	_ed_particle_manager* pManager;
};

class CFxParticleScenaricData
{
public:
	class CFxParticleScenaricDataSubObj
	{
	public:
		uint selectorType;
		int field_0x4;
	};

	void Init();
	void Create(ByteCode* pByteCode);

	uint field_0x0;
	CFxParticleScenaricDataSubObj aSubObjs[8];
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
	edDList_material* aMaterials[8];
};

class CFxParticleManager : public CFxPoolManager<CFxNewParticle, CFxParticleScenaricData>
{
public:
	static uint _prt_bank_first_index;

	CFxParticleManager();

	virtual void* InstanciateFx(uint param_2, FX_MATERIAL_SELECTOR selector);

	static void Level_Create(ByteCode* pByteCode);

	bool ReadPrtResChunk(ByteCode* pByteCode);

	uint nbParticleRes;
	CParticleRes* aParticleRes;
};

#endif //FX_PARTICLE_H