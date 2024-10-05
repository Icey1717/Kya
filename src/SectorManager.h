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
struct S_HIERANM_HIER;

class CSectorHierarchy {
public:
	ulong field_0x0;
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
};

struct SectorManagerSubObj {
	uint flags;
	undefined* pFileData;
	CSectorHierarchy* aSectorHierarchies;
	byte field_0xc;
};

struct SectorManagerSubObjOther {
	uint flags;
	undefined* pFileData;
	int aSectorHierarchies;
	uint* field_0xc;
};

struct StaticEdFileBase {
	struct edCFiler* pEdFileBase;
	undefined4 field_0x4;
};

extern StaticEdFileBase StaticEdFileBase_004497f0;

bool CheckFunc_00401fd0(StaticEdFileBase* param_1);

struct CSector {
public:
	CSector() {
		bankObject.pBankFileAccessObject = 0;
	}
	void InstallCallback();
	void Load(int param_2, int param_3, bool bFileFlag);
	void Level_Manage(int sectID, int param_3);
	void Flush();
	int desiredSectorID;
	int sectorIndex;
	int loadStage_0x8;
	edCBankBuffer bankObject;
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

	// Manager
	virtual void LevelLoading_Begin();
	virtual bool LevelLoading_Manage();
	virtual void Level_Install();
	virtual void Level_Init();
	virtual void Level_AddAll(struct ByteCode* pMemoryStream);
	virtual void Level_Manage();
	virtual void Level_SectorChange(int oldSectorId, int newSectorId);
	// End Manager

	void SetupCompanionSectors(uint flags);

	void Level_Create(ByteCode* pMemoryStream);

	void SwitchToSector(int sectID, bool param_3);

	void Flush();

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
	int sectDataCount;
	CSector* sectorArray;
	undefined4 field_0x37c;
	CSectorHierarchy* pSectorHierarchy;
	undefined4 field_0x384;
};

#endif // _SECTORMANAGER_H
