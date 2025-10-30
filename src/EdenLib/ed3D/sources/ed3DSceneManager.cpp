#include "ed3D/ed3DSceneManager.h"

void ed3DSceneRemoveFlag(ed_3D_Scene* pScene, uint flag)
{
	pScene->flags = pScene->flags & ~flag;
	return;
}

void ed3DSceneSetFlag(ed_3D_Scene* pScene, uint flag)
{
	pScene->flags = pScene->flags | flag;
}

void ed3DSceneSetFogProperty(ed_3D_Scene* pScene, bool bValue)
{
	if (bValue == false) {
		pScene->flags = pScene->flags & ~SCENE_FLAG_FOG_PROPERTY;
	}
	else {
		pScene->flags = pScene->flags | SCENE_FLAG_FOG_PROPERTY;
	}
	return;
}

void SetFlag_002a5440(ed_3D_Scene* pScene, bool bValue)
{
	if (bValue == false) {
		pScene->flags = pScene->flags & 0xffffff7f;
	}
	else {
		pScene->flags = pScene->flags | 0x80;
	}
	return;
}