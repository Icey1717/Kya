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

	int nbPathDynamicUsed;
	CPathDynamic* aDynamicPaths;
	CPathDynamic* pCurDynamicPath;

	int nbPathNodesUsed;
	CPathNode* aPathNodes;
	CPathNode* pCurPathNode;
};

class CBasicPathFinder : public CPathFinder
{
public:
	CBasicPathFinder();

	virtual bool IsValidPosition(edF32VECTOR4* pPosition, int id);

	virtual void DeletePathDynamic(CPathDynamic* pPathDynamic);

	virtual void Init();
	virtual void Create(ByteCode* pByteCode);

	CBasicNaviMesh* GetArea(edF32VECTOR4* pLocation);

	CAstarMemBank astarMemBank;

	CBasicNaviMesh* aNaviMeshes;
	uint nbNaviMeshes;

	int curNaviMeshIndex;
};

#endif // PATH_FINDER_H