#ifndef ED3D_G2D_H
#define ED3D_G2D_H

#include "Types.h"

struct ed_hash_code;
struct ed_g2d_manager;
struct ed_g2d_material;
struct ed_g2d_bitmap;
struct ed_g2d_texture;
struct ed_g2d_layer;

ed_hash_code* ed3DG2DGetMaterialFromIndex(ed_g2d_manager* pManager, int index);

ed_g2d_material* ed3DG2DGetG2DMaterialFromIndex(ed_g2d_manager* pManager, int index);
ed_g2d_material* ed3DG2DGetG2DMaterialFromIndex(ed_hash_code* pMBNK, int index);

ed_g2d_bitmap* ed3DG2DGetBitmapFromMaterial(ed_g2d_material* pMaterial, int index);

int ed3DG2DGetNeededSizeForDuplicateMaterial(ed_hash_code* pHashCode);
ed_g2d_texture* ed3DG2DGetTextureFromMaterial(ed_g2d_material* pMaterial, int index);

ed_g2d_material* ed3DG2DGetG2DMaterial(ed_g2d_manager* pManager, ulong hashCode);
ed_g2d_layer* ed3DG2DMaterialGetLayer(ed_g2d_material* pMaterial, uint index);

ed_hash_code* ed3DG2DGetMaterial(ed_g2d_manager* pTextureInfo, ulong hash);

ed_g2d_manager* ed3DG2DDuplicateMaterial(ed_hash_code* pHashCode, char* pData, ed_g2d_manager* pG2d);

#endif // ED3D_G2D_H