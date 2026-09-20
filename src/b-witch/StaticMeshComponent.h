#ifndef STATIC_MESH_COMPONENT_H
#define STATIC_MESH_COMPONENT_H

#include "Types.h"
#include "ed3D.h"

struct ed_3D_Scene;
struct ed_g3d_manager;
struct ed_3d_hierarchy_setup;
struct edNODE;
struct ed_3d_hierarchy_node;

class StaticMeshComponent
{
public:
	StaticMeshComponent();

	virtual void Reset();
	virtual void SetHidden(ed_3D_Scene* pScene);
	virtual void SetVisible(ed_3D_Scene* pScene);
	virtual bool HasMesh();
	virtual void Term();

	void Init(ed_3D_Scene* pScene, ed_g3d_manager* pMeshManager, ed_3d_hierarchy_setup* pHierarchySetup, char* szString);
	void Term(ed_3D_Scene* pScene);

	edNODE* pMeshTransformParent;
	ed_3d_hierarchy_node* pMeshTransformData;

	int meshIndex;
	int textureIndex;

	edF32MATRIX4 perspectiveMatrix;
};

class StaticMeshComponentHeroEx : public StaticMeshComponent
{
public:
	void Init(float param_1, float param_2, ed_3D_Scene* param_4, ed_g3d_manager* param_5, char* szString);

	float field_0x60;
	float bounds[4];
	ed_3d_hierarchy_setup hierarchySetup;
	ed_3D_Light_Config lightConfig;
	edF32VECTOR4 lightAmbient;
};

#endif // !STATIC_MESH_COMPONENT_H
