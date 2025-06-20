#ifndef FRONTEND_H
#define FRONTEND_H

#include "Types.h"
#include "LargeObject.h"

class CFrontendSamplePlayer
{
public:
	static long PlaySample(float, float*, int, int);
};

class CFrontend : public CObjectManager
{
public:
	static ed_3D_Scene* _scene_handle;

	CFrontend();

	// CObjectManager
	virtual void Level_ClearAll();

	static float GetTime();

	bool ComputeSceneCoordinate(float param_1, edF32VECTOR4* v0, edF32VECTOR2* pRect);

	struct ed_viewport* pViewport;
};

#endif // FRONTEND_H