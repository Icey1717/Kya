#ifndef ED_3D_SCRATCH_PAD_GLOBAL_VAR_H
#define ED_3D_SCRATCH_PAD_GLOBAL_VAR_H

union edF32MATRIX4;
struct ed_g2d_material;
struct edF32VECTOR4;
struct edFCamera;
struct SceneConfig;
struct RenderInfo;
union edF32VECTOR3;
union AnimScratchpad;

extern edF32MATRIX4* WorldToCamera_Matrix;

extern edF32MATRIX4* CameraToScreen_Matrix;
extern edF32MATRIX4* CameraToCulling_Matrix;
extern edF32MATRIX4* CameraToClipping_Matrix;
extern edF32MATRIX4* gLightDirections_Matrix_Scratch;
extern edF32MATRIX4* gLightColor_Matrix_Scratch;
extern edF32VECTOR4* gLightAmbiant_Scratch;
extern edF32VECTOR4* gVU1_AnimST_NormalExtruder_Scratch;
extern edF32MATRIX4** g_pCurFlareMtx;
extern edF32MATRIX4** g_pCurFlareObj2WorldMtx;
extern ed_g2d_material** g_pCurFlareMaterial;
extern int* gShadowRenderMask;
extern edFCamera* gRenderCamera;
extern edF32MATRIX4* gF32Matrix4Unit_Scratch;
extern unsigned int* ed3DVU1Addr_Scratch;
extern int(*ed3DVU1AddrWithBufCur_Scratch)[3];
extern int* gMaterialNbPrimSend;
extern int* gbDoMipmap_SPR;
extern unsigned int* gMipmapL_SPR;
extern unsigned int* gMipmapK_SPR;
extern unsigned int* gbTrilinear_SPR;
extern char* gPad1;
extern char* gPad2;
extern char* gPad3;
extern SceneConfig* gRenderSceneConfig_SPR;
extern RenderInfo* gRender_info_SPR;
extern edF32VECTOR4* gBoundSphereCenter;
extern AnimScratchpad* PTR_AnimScratchpad_00449554;

#endif // ED_3D_SCRATCH_PAD_GLOBAL_VAR_H
