#include "ed3DScratchPadGlobalVar.h"
#include "DlistManager.h"
#include "Types.h"
#include "ed3D.h"
#include "CameraViewManager.h"

PACK(
struct edScratchPadData
{
	edF32MATRIX4 WorldToCamera_Matrix;
	edF32MATRIX4 CameraToScreen_Matrix;
	edF32MATRIX4 CameraToCulling_Matrix;
	edF32MATRIX4 CameraToClipping_Matrix;

	edF32MATRIX4 gLightDirections_Matrix_Scratch;
	edF32MATRIX4 gLightColor_Matrix_Scratch;
	edF32VECTOR4 gLightAmbiant_Scratch;

	edF32VECTOR4 gVU1_AnimST_NormalExtruder_Scratch;

	edF32MATRIX4* g_pCurFlareMtx;
	edF32MATRIX4* g_pCurFlareObj2WorldMtx;
	ed_g2d_material* g_pCurFlareMaterial;

	int gShadowRenderMask;
	edFCamera gRenderCamera;
	edF32MATRIX4 gF32Matrix4Unit_Scratch;

	uint ed3DVU1Addr_Scratch[3];
	int ed3DVU1AddrWithBufCur_Scratch[3][3];

	int gMaterialNbPrimSend;
	int gbDoMipmap_SPR;
	uint gMipmapL_SPR;
	uint gMipmapK_SPR;
	uint gbTrilinear_SPR;

	int gPad1;
	int gPad2;
	int gPad3;

	SceneConfig gRenderSceneConfig_SPR;
	ScratchPadRenderInfo gRender_info_SPR;

	edF32VECTOR4 gBoundSphereCenter;

	AnimScratchpad PTR_AnimScratchpad_00449554;

	byte pad[0x10000];
});

#ifdef PLATFORM_PS2
edScratchPadData* pScratchData = (edScratchPadData*)0x70000000;
#else
namespace ScratchData {
	edScratchPadData data;
}
edScratchPadData* pScratchData = &ScratchData::data;
#endif

#ifdef PLATFORM_WIN
edF32MATRIX4* WorldToCamera_Matrix = &pScratchData->WorldToCamera_Matrix;
edF32MATRIX4* CameraToScreen_Matrix = &pScratchData->CameraToScreen_Matrix;
edF32MATRIX4* CameraToCulling_Matrix = &pScratchData->CameraToCulling_Matrix;
edF32MATRIX4* CameraToClipping_Matrix = &pScratchData->CameraToClipping_Matrix;

edF32MATRIX4* gLightDirections_Matrix_Scratch = &pScratchData->gLightDirections_Matrix_Scratch;
edF32MATRIX4* gLightColor_Matrix_Scratch = &pScratchData->gLightColor_Matrix_Scratch;
edF32VECTOR4* gLightAmbiant_Scratch = &pScratchData->gLightAmbiant_Scratch;

edF32VECTOR4* gVU1_AnimST_NormalExtruder_Scratch = &pScratchData->gVU1_AnimST_NormalExtruder_Scratch;

edF32MATRIX4** g_pCurFlareMtx = &pScratchData->g_pCurFlareMtx;
edF32MATRIX4** g_pCurFlareObj2WorldMtx = &pScratchData->g_pCurFlareObj2WorldMtx;
ed_g2d_material** g_pCurFlareMaterial = &pScratchData->g_pCurFlareMaterial;

int* gShadowRenderMask = &pScratchData->gShadowRenderMask;
edFCamera* gRenderCamera = &pScratchData->gRenderCamera;
edF32MATRIX4* gF32Matrix4Unit_Scratch = &pScratchData->gF32Matrix4Unit_Scratch;
uint (*ed3DVU1Addr_Scratch)[3] = &pScratchData->ed3DVU1Addr_Scratch;
int(*ed3DVU1AddrWithBufCur_Scratch)[3][3] = &pScratchData->ed3DVU1AddrWithBufCur_Scratch;
int* gMaterialNbPrimSend = &pScratchData->gMaterialNbPrimSend;
int* gbDoMipmap_SPR = &pScratchData->gbDoMipmap_SPR;
uint* gMipmapL_SPR = &pScratchData->gMipmapL_SPR;
uint* gMipmapK_SPR = &pScratchData->gMipmapK_SPR;
uint* gbTrilinear_SPR = &pScratchData->gbTrilinear_SPR;
int* gPad1 = &pScratchData->gPad1;
int* gPad2 = &pScratchData->gPad2;
int* gPad3 = &pScratchData->gPad3;
SceneConfig* gRenderSceneConfig_SPR = &pScratchData->gRenderSceneConfig_SPR;

ScratchPadRenderInfo* gRender_info_SPR = &pScratchData->gRender_info_SPR;
edF32VECTOR4* gBoundSphereCenter = &pScratchData->gBoundSphereCenter;
AnimScratchpad* PTR_AnimScratchpad_00449554 = &pScratchData->PTR_AnimScratchpad_00449554;

#else

edF32MATRIX4* WorldToCamera_Matrix = (edF32MATRIX4*)((char*)pScratchData + 0x0);
edF32MATRIX4* CameraToScreen_Matrix = (edF32MATRIX4*)((char*)WorldToCamera_Matrix + 0x40);
edF32MATRIX4* CameraToCulling_Matrix = (edF32MATRIX4*)((char*)WorldToCamera_Matrix + 0x80);
edF32MATRIX4* CameraToClipping_Matrix = (edF32MATRIX4*)((char*)WorldToCamera_Matrix + 0xc0);

edF32MATRIX4* gLightDirections_Matrix_Scratch = (edF32MATRIX4*)((char*)WorldToCamera_Matrix + 0x100);
edF32MATRIX4* gLightColor_Matrix_Scratch = (edF32MATRIX4*)((char*)WorldToCamera_Matrix + 0x140);
edF32VECTOR4* gLightAmbiant_Scratch = (edF32VECTOR4*)((char*)WorldToCamera_Matrix + 0x180);

edF32VECTOR4* gVU1_AnimST_NormalExtruder_Scratch = (edF32VECTOR4*)((char*)WorldToCamera_Matrix + 400);

edF32MATRIX4** g_pCurFlareMtx = (edF32MATRIX4**)((char*)WorldToCamera_Matrix + 0x1a0);
edF32MATRIX4** g_pCurFlareObj2WorldMtx = (edF32MATRIX4**)((char*)WorldToCamera_Matrix + 0x1a4);
ed_g2d_material** g_pCurFlareMaterial = (ed_g2d_material**)((char*)WorldToCamera_Matrix + 0x1a8);

int* gShadowRenderMask = (int*)((char*)WorldToCamera_Matrix + 0x1ac);
edFCamera* gRenderCamera = (edFCamera*)((char*)WorldToCamera_Matrix + 0x1b0);
edF32MATRIX4* gF32Matrix4Unit_Scratch = (edF32MATRIX4*)((char*)WorldToCamera_Matrix + 0x2e0);
uint(*ed3DVU1Addr_Scratch)[3] = (unsigned int(*)[3])((char*)WorldToCamera_Matrix + 800);
int(*ed3DVU1AddrWithBufCur_Scratch)[3][3] = (int(*)[3][3])((char*)WorldToCamera_Matrix + 0x32c);
int* gMaterialNbPrimSend = (int*)((char*)WorldToCamera_Matrix + 0x350);
int* gbDoMipmap_SPR = (int*)((char*)WorldToCamera_Matrix + 0x354);
uint* gMipmapL_SPR = (unsigned int*)WorldToCamera_Matrix + 0x358;
uint* gMipmapK_SPR = (unsigned int*)WorldToCamera_Matrix + 0x35c;
uint* gbTrilinear_SPR = (unsigned int*)WorldToCamera_Matrix + 0x360;
int* gPad1 = (int*)WorldToCamera_Matrix + 0x364;
int* gPad2 = (int*)WorldToCamera_Matrix + 0x368;
int* gPad3 = (int*)WorldToCamera_Matrix + 0x36c;
SceneConfig* gRenderSceneConfig_SPR = (SceneConfig*)((char*)WorldToCamera_Matrix + 0x370);
ScratchPadRenderInfo* gRender_info_SPR = (ScratchPadRenderInfo*)((char*)WorldToCamera_Matrix + 0x4c0);
edF32VECTOR4* gBoundSphereCenter = (edF32VECTOR4*)((char*)WorldToCamera_Matrix + 0x4e0);
AnimScratchpad* PTR_AnimScratchpad_00449554 = (AnimScratchpad*)((char*)WorldToCamera_Matrix + 0x4f0);
#endif