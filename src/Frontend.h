#ifndef FRONTEND_H
#define FRONTEND_H

#include "Types.h"
#include "LargeObject.h"

class CFrontend : public CObjectManager
{
public:
	static ed_3D_Scene* _scene_handle;

	// CObjectManager
	virtual void Level_ClearAll();

	static float GetTime();

	bool ComputeSceneCoordinate(float param_1, edF32VECTOR4* v0, edF32VECTOR2* pRect);
};

#endif // FRONTEND_H