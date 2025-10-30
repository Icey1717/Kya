#ifndef ED3D_SCENE_MANAGER_H
#define ED3D_SCENE_MANAGER_H

#include "Types.h"

#include "edList.h"

struct FrustumData
{
	edF32MATRIX4 frustumMatrix;
	edF32VECTOR4 field_0x40;
	edF32VECTOR4 field_0x50;
};

struct ed_3D_Light_Config
{
	edF32VECTOR4* pLightAmbient;
	edF32MATRIX4* pLightDirections;
	edF32MATRIX4* pLightColorMatrix;
};

struct ed_3D_Shadow_Config
{
	undefined4 field_0x0;
	float field_0x4;
	float field_0x8;
	float field_0xc;
	struct ed_viewport* pCamera_0x10;
	struct ed_viewport* pViewport;
	int texWidth;
	int texHeight;
	undefined2 renderMask;
	byte field_0x22;
	byte field_0x23;
	byte field_0x24;
};

struct SceneConfig
{
	int field_0x0; // undefined* 
	float clipValue_0x4;
	float nearClip;
	float farClip;
	uint projectionScaleFactorA;
	uint projectionScaleFactorB;
	float clipValue_0x18;
	undefined field_0x1c;
	undefined field_0x1d;
	undefined field_0x1e;
	undefined field_0x1f;
	FrustumData frustumA;
	FrustumData frustumB;
	edF32VECTOR4 field_0xe0;
	ByteColor3 fogCol_0xf0;
	undefined field_0xf3;
	undefined field_0xf4;
	undefined field_0xf5;
	undefined field_0xf6;
	undefined field_0xf7;
	undefined field_0xf8;
	undefined field_0xf9;
	undefined field_0xfa;
	undefined field_0xfb;
	undefined field_0xfc;
	undefined field_0xfd;
	undefined field_0xfe;
	undefined field_0xff;
	ed_3D_Shadow_Config pShadowConfig;
	undefined field_0x125;
	undefined field_0x126;
	undefined field_0x127;
	int field_0x128;
	int field_0x12c;
	ed_3D_Light_Config pLightConfig;
	undefined field_0x13c;
	undefined field_0x13d;
	undefined field_0x13e;
	undefined field_0x13f;
	undefined field_0x140;
	undefined field_0x141;
	undefined field_0x142;
	undefined field_0x143;
	undefined field_0x144;
	undefined field_0x145;
	undefined field_0x146;
	undefined field_0x147;
	undefined field_0x148;
	undefined field_0x149;
	undefined field_0x14a;
	undefined field_0x14b;
	undefined field_0x14c;
	undefined field_0x14d;
	undefined field_0x14e;
	undefined field_0x14f;
};

struct ed_3D_Scene
{
	int bShadowScene;
	uint flags;
	struct edFCamera* pCamera;
	struct ed_viewport* pViewport;
	edLIST meshClusterShadowList;
	edLIST meshClusterList;
	SceneConfig sceneConfig;
	undefined field_0x18c;
	undefined field_0x18d;
	undefined field_0x18e;
	undefined field_0x18f;
	undefined field_0x190;
	undefined field_0x191;
	undefined field_0x192;
	undefined field_0x193;
	undefined field_0x194;
	undefined field_0x195;
	undefined field_0x196;
	undefined field_0x197;
	undefined field_0x198;
	undefined field_0x199;
	undefined field_0x19a;
	undefined field_0x19b;
	undefined field_0x19c;
	undefined field_0x19d;
	undefined field_0x19e;
	undefined field_0x19f;
	struct edLIST* field_0x1a0;
	struct edLIST* pHierListA;
	struct edLIST* pHierListB;
	int* field_0x1ac;
	uint heirarchyListCount;
	undefined field_0x1b4;
	undefined field_0x1b5;
	undefined field_0x1b6;
	undefined field_0x1b7;
	undefined field_0x1b8;
	undefined field_0x1b9;
	undefined field_0x1ba;
	undefined field_0x1bb;
	undefined field_0x1bc;
	undefined field_0x1bd;
	undefined field_0x1be;
	undefined field_0x1bf;
};

#define SCENE_FLAG_IN_USE		0x1
#define SCENE_FLAG_CAST_SHADOW	0x2
#define SCENE_FLAG_FOG_PROPERTY	0x40

void ed3DSceneRemoveFlag(ed_3D_Scene* pScene, uint flag);
void ed3DSceneSetFlag(ed_3D_Scene* pScene, uint flag);
void ed3DSceneSetFogProperty(ed_3D_Scene* pScene, bool bValue);
void SetFlag_002a5440(ed_3D_Scene* pScene, bool bValue);

#endif // ED3D_SCENE_MANAGER_H
