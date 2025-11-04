#ifndef ED3D_G3D_H
#define ED3D_G3D_H

#include "Types.h"

struct ed_hash_code;
struct ed_g2d_manager;
struct ed_3d_hierarchy;
struct ed_g3d_Anim_def;
struct ed_g2d_material;
struct ed_Chunck;
struct ed_g3d_manager;
struct ed_g3d_hierarchy;

void ed3DResetTime(void);
void ed3DSetDeltaTime(int newTime);

int ed3DG2DGetG2DNbMaterials(ed_hash_code* pHashCode);
int ed3DG2DGetG2DNbMaterials(ed_g2d_manager* pManager);
ed_hash_code* ed3DHierarchyGetMaterialBank(ed_3d_hierarchy* pHier);
void ed3DManageAnim(ed_g3d_Anim_def* pAnimDef);
ed_hash_code* ed3DG2DGetHashCode(ed_g2d_manager* pManager, ed_g2d_material* pMaterial);
int ed3DG2DGetG2DNbMaterials(ed_Chunck* pChunck);
ed_g3d_hierarchy* ed3DG3DHierarchyGetFromIndex(ed_g3d_manager* pMeshInfo, int count);
void ed3DG3DHierarchySetStripShadowCastFlag(ed_g3d_hierarchy* pHier, ushort flag);
void ed3DG3DHierarchySetStripShadowReceiveFlag(ed_g3d_hierarchy* pHier, ushort flag);

extern int gCurTime;
extern int gCurFrame;
extern int gStepTime;

#endif // _ED3D_G3D_H