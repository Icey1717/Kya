#ifndef NAVI_MESH_H
#define NAVI_MESH_H

#include "Types.h"
#include "MemoryStream.h"
#include "NaviCell.h"

bool IsInBoundingBox(edF32MATRIX4* pBox, edF32VECTOR4* pPoint);

class CNaviMeshPoint
{
public:
	float field_0x0;
	float field_0x4;
	uint field_0x8;
};

class CAstarBasicTransition
{
public:
	void Create(ByteCode* pByteCode);

	ushort field_0x0;
	ushort field_0x2;

	float field_0x4;

	ushort field_0x8;
	ushort field_0xa;
};

class CAstarSubGraph
{
public:
	void Create(ByteCode* pByteCode);

	uint nbCells;
	uint cellStart;

	edF32MATRIX4 boundingBox;
};

class CNaviMesh
{
public:
	uint GetEdgeId(ushort param_2, ushort param_3);

	CNaviMeshPoint* aMeshPoints;
	uint nbMeshPoints;

	CBasicNaviCell* aNaviCells;
	uint nbCells;

	CAstarBasicTransition* aAstarBasicTransitions;
	uint nbTransitions;

	CAstarSubGraph* aSubGraphs;
	uint nbSubGraphs;

	edF32MATRIX4 field_0x30;
};

class CBasicNaviMesh : public CNaviMesh
{
public:
	virtual void Init();
	virtual void Create(ByteCode* pByteCode);

	uint SearchCellId(edF32VECTOR4* pPosition);

	bool IsInCell(int param_2, edF32VECTOR2* param_3);

	ushort* pDataStreamA;
	ushort* pDataStreamB;
};

#endif // NAVI_MESH_H