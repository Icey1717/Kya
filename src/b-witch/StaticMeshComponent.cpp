#include "StaticMeshComponent.h"
#include "LargeObject.h"
#include "MathOps.h"
#include "FileManager3D.h"

StaticMeshComponent::StaticMeshComponent()
{
	this->pMeshTransformParent = (edNODE*)0x0;
	this->pMeshTransformData = (ed_3d_hierarchy_node*)0x0;
	this->meshIndex = -1;
	this->textureIndex = -1;
}

void StaticMeshComponent::Reset()
{
	this->pMeshTransformParent = (edNODE*)0x0;
	this->pMeshTransformData = (ed_3d_hierarchy_node*)0x0;

	return;
}

void StaticMeshComponent::SetHidden(ed_3D_Scene* pScene)
{
	if (pScene == (ed_3D_Scene*)0x0) {
		pScene = CScene::_scene_handleA;
	}

	if ((this->pMeshTransformParent != (edNODE*)0x0) && (this->pMeshTransformData != (ed_3d_hierarchy_node*)0x0)) {
		ed3DHierarchyNodeSetRenderOff(pScene, this->pMeshTransformParent);
	}

	return;
}

void StaticMeshComponent::SetVisible(ed_3D_Scene* pScene)
{
	if (pScene == (ed_3D_Scene*)0x0) {
		pScene = CScene::_scene_handleA;
	}

	if ((this->pMeshTransformParent != (edNODE*)0x0) && (this->pMeshTransformData != (ed_3d_hierarchy_node*)0x0)) {
		ed3DHierarchyNodeSetRenderOn(pScene, this->pMeshTransformParent);
	}

	return;
}

bool StaticMeshComponent::HasMesh()
{
	return this->pMeshTransformParent != (edNODE*)0x0;
}

void StaticMeshComponent::Term()
{
	this->pMeshTransformParent = (edNODE*)0x0;
	this->pMeshTransformData = (ed_3d_hierarchy_node*)0x0;
	this->meshIndex = -1;
	this->textureIndex = -1;

	return;
}

void StaticMeshComponent::Init(ed_3D_Scene* pScene, ed_g3d_manager* pMeshManager, ed_3d_hierarchy_setup* pHierarchySetup, char* szString)
{
	if (pScene == (ed_3D_Scene*)0x0) {
		pScene = CScene::_scene_handleA;
	}

	if (pMeshManager == (ed_g3d_manager*)0x0) {
		pMeshManager = CScene::ptable.g_C3DFileManager_00451664->GetG3DManager(this->meshIndex, this->textureIndex);
	}

	this->pMeshTransformParent = ed3DHierarchyAddToScene(pScene, pMeshManager, szString);

	if (this->pMeshTransformParent != (edNODE*)0x0) {
		this->pMeshTransformData = reinterpret_cast<ed_3d_hierarchy_node*>(this->pMeshTransformParent->pData);

		if (pHierarchySetup != (ed_3d_hierarchy_setup*)0x0) {
			ed3DHierarchySetSetup((ed_3d_hierarchy*)this->pMeshTransformData, pHierarchySetup);
		}

		if (this->pMeshTransformData != (ed_3d_hierarchy_node*)0x0) {
			edF32Matrix4CopyHard(&this->perspectiveMatrix, &this->pMeshTransformData->base.transformA);
		}

		SetHidden((ed_3D_Scene*)0x0);
	}

	return;
}

void StaticMeshComponent::Term(ed_3D_Scene* pScene)
{
	if (pScene == (ed_3D_Scene*)0x0) {
		pScene = CScene::_scene_handleA;
	}

	if (this->pMeshTransformParent != (edNODE*)0x0) {
		ed3DHierarchyRemoveFromScene(pScene, this->pMeshTransformParent);
	}

	this->pMeshTransformParent = (edNODE*)0x0;

	return;
}

void StaticMeshComponentHeroEx::Init(float param_1, float param_2, ed_3D_Scene* param_4, ed_g3d_manager* param_5, char* szString)
{
	StaticMeshComponent::Init(param_4, param_5, (ed_3d_hierarchy_setup*)0x0, szString);

	memset(&this->hierarchySetup, 0, sizeof(ed_3d_hierarchy_setup));

	if (param_1 != 0.0f) {
		(this->hierarchySetup).pBoundingSphere = (edF32VECTOR4*)this->bounds;
		this->bounds[0] = 0.0f;
		this->bounds[1] = 0.0f;
		this->bounds[2] = 0.0f;
		this->bounds[3] = param_1;
	}

	if (param_2 != 0.0f) {
		(this->hierarchySetup).clipping_0x0 = &this->field_0x60;
		this->field_0x60 = param_2;
	}

	ed3DHierarchySetSetup(&this->pMeshTransformData->base, &this->hierarchySetup);

	(this->hierarchySetup).pLightData = &this->lightConfig;
	(this->lightConfig).pLightAmbient = &this->lightAmbient;
	(this->lightConfig).pLightColorMatrix = (edF32MATRIX4*)0x0;
	(this->lightConfig).pLightDirections = (edF32MATRIX4*)0x0;

	ed3DHierarchySetSetup(&this->pMeshTransformData->base, &this->hierarchySetup);

	(this->lightAmbient).x = 255.0f;
	(this->lightAmbient).y = 255.0f;
	(this->lightAmbient).z = 255.0f;
	(this->lightAmbient).w = 0.0f;

	return;
}