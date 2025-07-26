#ifndef _SECTORMANAGER_H
#define _SECTORMANAGER_H

#include "Types.h"
#include "LargeObject.h"
#include "FileManager3D.h"
#include "ed3D.h"
#include "EdenLib/edCollision/sources/edCollisions.h"
#include "EdenLib/edBank/include/edBankBuffer.h"

namespace ed3D
{
	struct MeshData_ANHR;
}

struct ByteCode;

struct S_HIERANM_HIER
{
	int pNode; // edNODE*
	int p3dHierarchy; // ed_3d_hierarchy*
	int field_0x8;
	undefined4 field_0xc;
	edF32VECTOR4 field_0x10;
	uint flags_0x20;
};

static_assert(sizeof(S_HIERANM_HIER) == 0x24);

struct S_COMPANION_INFO;

class CSectorHierarchy {
public:
	ulong hash;
	CSectorHierarchy* pNext;
	edNODE* pNode;
	S_HIERANM_HIER* pHier;
	uint flags;
	byte alpha;
	byte field_0x19;

	void Create(ByteCode* pByteCode);
	void Init(int param_2);
	void Term(int index);
	void SetHiddenOn();
	void SetHiddenOff();
	void SetAlpha(byte alpha);

	void Sector_Enter(edNODE* pNode, S_HIERANM_HIER* pHier);
};

struct SectorManagerSubObj {
	uint flags;
	undefined* pFileData;
	CSectorHierarchy* aSectorHierarchies;
	byte field_0xc;
};

struct SectorManagerSubObjOther {
	uint flags;
	int bankSize;
	int nbSectorConditions;
	S_COMPANION_INFO* aCompanionInfo;
};

struct CSector {
public:
	CSector();

	void InstallCallback();
	void Load(int param_2, int param_3, bool bFileFlag);
	void Level_Manage(int sectID, int param_3);
	void Flush();
	int desiredSectorID;
	int sectorIndex;
	int loadStage_0x8;
	edCBankBuffer bankObject;
	edCBankBufferEntry* pBankBufferEntry;
	ed_g3d_manager sectorMesh;
	ed_g2d_manager sectorTexture;
	edObbTREE_DYN* pObbTree;
	ed_g3d_manager backgroundMesh;
	ed_g2d_manager backgroundTexture;
	struct edNODE* pBackgroundNode;
	struct edNODE* pMeshTransformParent_0x130;
	float field_0x134;
	CHierarchyAnm pANHR;
	int sectID;
	int currentSectorID;
};

struct CSectorManager : public CObjectManager {
	
public:
	CSectorManager();

	void Level_ClearInternalData();
	void InstallEnterSector();

	virtual void LevelLoading_Begin();
	virtual bool LevelLoading_Manage();

	virtual void Level_Install();
	virtual void Level_Init();
	virtual void Level_Term();
	virtual void Level_AddAll(struct ByteCode* pByteCode);
	virtual void Level_ClearAll();

	virtual void Level_Manage();
	virtual void Level_ManagePaused();
	virtual void Level_Draw();

	virtual void Level_Reset();

	virtual void Level_SectorChange(int oldSectorId, int newSectorId);

	void SetupCompanionSectors(uint flags);

	void Level_Create(ByteCode* pMemoryStream);

	void SwitchToSector(int sectID, bool param_3);

	void Flush();

	edNODE* RegisterDynamicHierarchy(ulong hash, S_HIERANM_HIER** pOutHier);

	char szSectorFileRoot[32];
	undefined field_0x24;
	undefined field_0x25;
	undefined field_0x26;
	undefined field_0x27;
	undefined field_0x28;
	undefined field_0x29;
	undefined field_0x2a;
	undefined field_0x2b;
	undefined field_0x2c;
	undefined field_0x2d;
	undefined field_0x2e;
	undefined field_0x2f;
	undefined field_0x30;
	undefined field_0x31;
	undefined field_0x32;
	undefined field_0x33;
	undefined field_0x34;
	undefined field_0x35;
	undefined field_0x36;
	undefined field_0x37;
	undefined field_0x38;
	undefined field_0x39;
	undefined field_0x3a;
	undefined field_0x3b;
	undefined field_0x3c;
	undefined field_0x3d;
	undefined field_0x3e;
	undefined field_0x3f;
	undefined field_0x40;
	undefined field_0x41;
	undefined field_0x42;
	undefined field_0x43;
	SectorManagerSubObj subObjArray[30];
	CSector baseSector;
	int count_0x368;
	int field_0x36c;
	undefined4 field_0x370;
	int nbSectors;
	CSector* aSectors;
	undefined4 field_0x37c;
	CSectorHierarchy* pSectorHierarchy;
	undefined4 field_0x384;
};

#endif // _SECTORMANAGER_H
