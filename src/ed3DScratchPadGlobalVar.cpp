#include "ed3DScratchPadGlobalVar.h"

#ifdef PLATFORM_PS2
edF32MATRIX4* WorldToCamera_Matrix = (edF32MATRIX4*)0x70000000;
#else
char gwScratchPad[0x4000] = {};
edF32MATRIX4* WorldToCamera_Matrix = (edF32MATRIX4*)gwScratchPad;
#endif

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
unsigned int* ed3DVU1Addr_Scratch = (unsigned int*)((char*)WorldToCamera_Matrix + 800);
int(*ed3DVU1AddrWithBufCur_Scratch)[3] = (int(*)[3])((char*)WorldToCamera_Matrix + 0x32c);
int* gMaterialNbPrimSend = (int*)((char*)WorldToCamera_Matrix + 0x350);
int* gbDoMipmap_SPR = (int*)((char*)WorldToCamera_Matrix + 0x354);
unsigned int* gMipmapL_SPR = (unsigned int*)WorldToCamera_Matrix + 0x358;
unsigned int* gMipmapK_SPR = (unsigned int*)WorldToCamera_Matrix + 0x35c;
unsigned int* gbTrilinear_SPR = (unsigned int*)WorldToCamera_Matrix + 0x360;
char* gPad1 = (char*)WorldToCamera_Matrix + 0x364;
char* gPad2 = (char*)WorldToCamera_Matrix + 0x368;
char* gPad3 = (char*)WorldToCamera_Matrix + 0x36c;
SceneConfig* gRenderSceneConfig_SPR = (SceneConfig*)((char*)WorldToCamera_Matrix + 0x370);
RenderInfo* gRender_info_SPR = (RenderInfo*)((char*)WorldToCamera_Matrix + 0x4c0);
edF32VECTOR4* gBoundSphereCenter = (edF32VECTOR4*)((char*)WorldToCamera_Matrix + 0x4e0);
AnimScratchpad* PTR_AnimScratchpad_00449554 = (AnimScratchpad*)((char*)WorldToCamera_Matrix + 0x4f0);
