#ifndef EVENT_MANAGER_H
#define EVENT_MANAGER_H

#include "Types.h"
#include "LargeObject.h"

struct CEventManagerInternal {
	void* pGetActorPointer;
	void* pGetActorPositionVector;
	void* pGetActorIdent;
	void* pSendAllMessages;
};

struct EventQueue {
	void* aEntries;
	int totalCount;
};

struct CEventManager : public CObjectManager {
public:
	CEventManager();

	virtual void Level_Init() {};
	virtual void Level_Term();

	virtual void Level_ClearAll() {}
	virtual void Level_Manage();
	virtual void Level_ManagePaused() {};
	virtual void Level_Draw() {};

	virtual void Level_Reset();

	virtual void Level_CheckpointReset();

	int activeChunkId;
	int nbEvents;

	static CEventManagerInternal callbackFunctions;
};

void edEventInit(void);
uint edEventAddChunk(void* pFileData, uint mode);
uint edEventGetChunkNbEvents(int eventIndex);
ed_zone_3d* edEventGetChunkZone(uint chunkId, uint zoneId);

#endif // EVENT_MANAGER_H
