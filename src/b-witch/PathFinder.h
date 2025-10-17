#ifndef PATH_FINDER_H
#define PATH_FINDER_H

#include "Types.h"
#include "PathDynamic.h"
#include "MemoryStream.h"
#include "NaviMesh.h"

class CPathFinder
{
	virtual void DeletePathDynamic(CPathDynamic* pPathDynamic) = 0;
};

class CAstarMemBank
{
public:
	CAstarMemBank();
	void DeletePathDynamic(CPathDynamic* pPathDynamic);
	CPathDynamic* NewPathDynamic(edF32VECTOR4* pStart, edF32VECTOR4* pDestination);

	void RecycleNodeFromNodeBank(CPathNode* pNewPathNode);

	CPathNode* GetFreeNodeFromNodeBank();

	int nbPathDynamicUsed;
	CPathDynamic* aDynamicPaths;
	CPathDynamic* pCurDynamicPath;

	int nbPathNodesUsed;
	CPathNode* aPathNodes;
	CPathNode* pCurPathNode;
};

class CPriorityQueue
{
public:
	CPriorityQueue();
	void Clear();
	void PushPriorityQueue(CPathNode* pPathNode);
	CPathNode* PopPriorityQueue();

	void UpdateNodeOnPriorityQueue(CPathNode* pNode);

	CPathNode* aEntries[256];
	int field_0x400;
};

class CBasicPathFinder : public CPathFinder
{
public:
	CBasicPathFinder();

	virtual bool IsValidPosition(edF32VECTOR4* pPosition, int id);

	virtual void DeletePathDynamic(CPathDynamic* pPathDynamic);

	virtual void Init();
	virtual void Create(ByteCode* pByteCode);
	virtual int FindPath(CPathDynamic* pPath, int id);
	virtual void FreeAllNodes();
	virtual bool ShouldAbortSearch();

	CBasicNaviMesh* GetArea(edF32VECTOR4* pLocation);

	CPathDynamic* NewPathDynamic(edF32VECTOR4* pStart, edF32VECTOR4* pDestination);

	void BuildPath(CPathDynamic* pPathDynamic, CPathNode* pNode);

	int CheckAndUpdatePath(CPathDynamic** ppPathDynamic, edF32VECTOR4* param_3, edF32VECTOR4* param_4);

	void ComputeSubTarget(float param_1, edF32VECTOR4* param_3, edF32VECTOR4* param_4, CPathDynamic* pPathDynamic);

	CAstarMemBank astarMemBank;

	CBasicNaviMesh* aNaviMeshes;
	uint nbNaviMeshes;

	int curNaviMeshIndex;

	CPriorityQueue priorityQueue;

	int nodeListCount;
	CPathNode* pNodeList;
};

#endif // PATH_FINDER_H