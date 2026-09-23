#ifndef FX_TAIL_H
#define FX_TAIL_H

#include "Types.h"

class CGlobalDListPatch;

class CFxTail : public CObject
{
public:
	CFxTail();
	~CFxTail();

	virtual bool IsKindOfObject(ulong kind);
	virtual bool InitDlistPatchable(int);

	void Create(float param_1, int count, int param_4, int materialId);
	void Init(float param_1, int id);

	void Reset();

	void ChangeOrder(char* pNewOrder);

	bool SetPatchActive(int bActive);

	bool Manage(edF32VECTOR4* param_2, edF32VECTOR4* param_3, int param_4);
	bool Manage(edF32VECTOR4* param_2, int param_3, int param_4);

	void _ManageLife();
	void _PatchListAlpha();
	void _CreateSegment(edF32VECTOR4* param_2, edF32VECTOR4* param_3);
	void _CreateSegment(edF32VECTOR4* param_2, int param_3);
	void _PatchSegmentST(float param_1, uint param_3);

	int dlistPatchId;
	CGlobalDListPatch* pDlistPatch;

	uint flags;

	int materialId;

	float* pSegmentAlpha;

	// Distance from the center to either edge.
	float halfWidth;
	float previousHalfWidth;

	_rgba color;
	_rgba previousColor;

	// Number of newest segments over which alpha ramps up from zero.
	int headFadeSegmentCount;

	float alphaDecayPerUpdate;
	int nbSegments;
	int nbUsedSegments;
	int nextSegmentIndex;
	int pendingBreakSegmentIndex;

	edF32VECTOR4 rotationEuler;
	edF32VECTOR4 previousRotationEuler;
	edF32MATRIX4 transformMatrix;

	uint widthAxis;
	char szOrder[4];

	// S-coordinate increment, divided across interpolated segments.
	float textureSStep;

	// Running S coordinate used by _PatchSegmentST.
	float nextTextureS;

	float negativeEdgeTextureT;
	float positiveEdgeTextureT;
};

#endif // FX_TAIL_H
