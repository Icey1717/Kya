#include "ed3D/ed3DG2D.h"

#include "ed3D/ed3DG3D.h"
#include "ed3D.h"

ed_hash_code* ed3DG2DGetMaterialFromIndex(ed_g2d_manager* pManager, int index)
{
	ed_Chunck* pMATA_HASH;
	ed_Chunck* pNextChunk;
	uint materialIndex;
	ed_hash_code* pHashCode;
	char* pChunkEnd;

	ED3D_LOG(LogLevel::Verbose, "ed3DG2DGetMaterialFromIndex Looking for index: {}", index);

	pMATA_HASH = pManager->pMATA_HASH;
	ed_Chunck* pMATA = pManager->pMATA_HASH - 1;

	/* Ensure we are trying to read something from the 'MATA' section */
	if (pMATA->hash == HASH_CODE_MATA) {
		/* Work out the section end address */
		pChunkEnd = (char*)pMATA + pMATA->size;
		for (pNextChunk = edChunckGetFirst(pMATA_HASH, pChunkEnd); pNextChunk != (ed_Chunck*)0x0; pNextChunk = edChunckGetNext(pNextChunk, pChunkEnd)) {
			ED3D_LOG(LogLevel::Verbose, "ed3DG2DGetMaterialFromIndex Chunk: {}", pNextChunk->GetHeaderString());

			/* Check the first value in the buffer is *MAT.* */
			if (pNextChunk->hash == HASH_CODE_MAT) {
				ED3D_LOG(LogLevel::Verbose, "ed3DG2DGetMaterialFromIndex Found MAT. chunk - size: 0x{:x}", pNextChunk->size);

				if ((index == 0) && (pMATA_HASH->hash == HASH_CODE_HASH)) {
					ED3D_LOG(LogLevel::Verbose, "ed3DG2DGetMaterialFromIndex Found HASH chunk - size: 0x{:x}", pNextChunk->size);

					/* Check the first value in the buffer is *HASH* */
					pHashCode = (ed_hash_code*)(pMATA_HASH + 1);

					const int materialCount = ed3DG2DGetG2DNbMaterials(pMATA_HASH);

					ED3D_LOG(LogLevel::Verbose, "ed3DG2DGetMaterialFromIndex Searching through {} materials for hash: {}", materialCount, pHashCode->hash.ToString());

					for (materialIndex = materialCount; materialIndex != 0; materialIndex = materialIndex - 1) {

						ed_Chunck* pMAT = LOAD_POINTER_CAST(ed_Chunck*, pHashCode->pData);

						assert(pMAT->hash == HASH_CODE_MAT);

						if (pMAT == pNextChunk) {
							ED3D_LOG(LogLevel::Verbose, "ed3DG2DGetMaterialFromIndex Found material at index {}", materialCount - materialIndex);
							return pHashCode;
						}

						pHashCode = pHashCode + 1;
					}
				}

				index = index + -1;
			}
			/* Jump to the next section */
		}
	}

	return (ed_hash_code*)0x0;
}

ed_g2d_material* ed3DG2DGetG2DMaterialFromIndex(ed_g2d_manager* pManager, int index)
{
	ed_hash_code* pMAT_HASH;
	ed_g2d_material* pMAT_Internal;

	ED3D_LOG(LogLevel::Verbose, "ed3DG2DGetG2DMaterialFromIndex Looking for index: {}", index);

	pMAT_HASH = ed3DG2DGetMaterialFromIndex(pManager, index);

	if (pMAT_HASH == (ed_hash_code*)0x0) {
		pMAT_Internal = (ed_g2d_material*)0x0;
	}
	else {
		ed_Chunck* pMAT = LOAD_POINTER_CAST(ed_Chunck*, pMAT_HASH->pData);

		assert(pMAT->hash == HASH_CODE_MAT);

		pMAT_Internal = reinterpret_cast<ed_g2d_material*>(pMAT + 1);
	}

	return pMAT_Internal;
}

ed_g2d_material* ed3DG2DGetG2DMaterialFromIndex(ed_hash_code* pMBNK, int index)
{
	ed_Chunck* pChunk;
	ed_g2d_material* pMaterial;
	pChunk = LOAD_POINTER_CAST(ed_Chunck*, pMBNK[index].pData);

	pMaterial = (ed_g2d_material*)0x0;
	if ((pChunk != (ed_Chunck*)0x0) && (pMaterial = reinterpret_cast<ed_g2d_material*>(pChunk + 1), pChunk->hash != HASH_CODE_MAT)) {
		// Header didn't match MAT, so must actually be a hash code.
		ed_hash_code* pHashCode = reinterpret_cast<ed_hash_code*>(pChunk);
		pChunk = LOAD_POINTER_CAST(ed_Chunck*, pHashCode->pData);

		// Check we definitely found a material.
		assert(pChunk->hash == HASH_CODE_MAT);

		pMaterial = reinterpret_cast<ed_g2d_material*>(pChunk + 1);
	}

	return pMaterial;
}

ed_g2d_bitmap* ed3DG2DGetBitmapFromMaterial(ed_g2d_material* pMaterial, int index)
{
	ed_g2d_bitmap* pBitmap;
	ed_g2d_texture* pTexture;
	ed_Chunck* pLAY;

	pTexture = (ed_g2d_texture*)0x0;
	pLAY = LOAD_POINTER_CAST(ed_Chunck*, pMaterial->aLayers[index]);
	ed_g2d_layer* pLayer = reinterpret_cast<ed_g2d_layer*>(pLAY + 1);

	pBitmap = (ed_g2d_bitmap*)0x0;

	if ((pLAY != (ed_Chunck*)0xfffffff0) && (pLayer->bHasTexture != 0)) {
		ed_Chunck* pTEX = LOAD_POINTER_CAST(ed_Chunck*, pLayer->pTex);
		pTexture = reinterpret_cast<ed_g2d_texture*>(pTEX + 1);
	}

	if (pTexture == (ed_g2d_texture*)0x0) {
		pBitmap = (ed_g2d_bitmap*)0x0;
	}
	else {
		ed_hash_code* pBitmapHashCode = LOAD_POINTER_CAST(ed_hash_code*, pTexture->hashCode.pData);
		if (pBitmapHashCode != (ed_hash_code*)0x0) {
			ed_Chunck* pT2D = LOAD_POINTER_CAST(ed_Chunck*, pBitmapHashCode->pData);
			pBitmap = reinterpret_cast<ed_g2d_bitmap*>(pT2D + 1);
		}
	}

	return pBitmap;
}

int ed3DG2DGetNeededSizeForDuplicateMaterial(ed_hash_code* pHashCode)
{
	bool bVar1;
	byte bVar2;
	int iVar3;
	int requiredSize;
	uint nbMaterials;
	int* pLayerIt;

	ed_Chunck* pMAT = LOAD_POINTER_CAST(ed_Chunck*, pHashCode->pData);
	ed_g2d_material* pMaterial = reinterpret_cast<ed_g2d_material*>(pMAT + 1);
	bVar2 = pMaterial->nbLayers;
	nbMaterials = (uint)bVar2;

	pLayerIt = pMaterial->aLayers;
	requiredSize = ((uint)(bVar2 >> 2) * 4 + 4) * 4 + 0x90;

	while (bVar1 = nbMaterials != 0, nbMaterials = nbMaterials - 1, bVar1) {
		ed_Chunck* pLAY = LOAD_POINTER_CAST(ed_Chunck*, *pLayerIt);
		ed_g2d_layer* pLayer = reinterpret_cast<ed_g2d_layer*>(pLAY + 1);
		ed_Chunck* pTEX = LOAD_POINTER_CAST(ed_Chunck*, pLayer->pTex);
		ed_g2d_texture* pTexture = reinterpret_cast<ed_g2d_texture*>(pTEX + 1);
		requiredSize = requiredSize + ((uint)(pLayer->bHasTexture >> 2) * 4 + 4) * 4 + pTexture->bHasPalette * 0x10 + 0x60;

		if (pTexture->pAnimSpeedNormalExtruder != 0x0) {
			requiredSize = requiredSize + 0x10;
		}

		pLayerIt = pLayerIt + 1;
	}

	return requiredSize;
}

ed_g2d_texture* ed3DG2DGetTextureFromMaterial(ed_g2d_material* pMaterial, int index)
{
	ed_Chunck* pLAY = LOAD_POINTER_CAST(ed_Chunck*, pMaterial->aLayers[index]);
	ed_g2d_layer* pLayer = reinterpret_cast<ed_g2d_layer*>(pLAY + 1);
	ed_Chunck* pTEX = LOAD_POINTER_CAST(ed_Chunck*, pLayer->pTex);
	return reinterpret_cast<ed_g2d_texture*>(pTEX + 1);
}

ed_g2d_material* ed3DG2DGetG2DMaterial(ed_g2d_manager* pManager, ulong hashCode)
{
	ed_hash_code* pHashCode;
	ed_g2d_material* pMaterial;

	pHashCode = edHashcodeGet(hashCode, pManager->pMATA_HASH);

	if (pHashCode == (ed_hash_code*)0x0) {
		pMaterial = (ed_g2d_material*)0x0;
	}
	else {
		ed_Chunck* pMAT = LOAD_POINTER_CAST(ed_Chunck*, pHashCode->pData);
		pMaterial = reinterpret_cast<ed_g2d_material*>(pMAT + 1);
	}

	return pMaterial;
}

ed_g2d_layer* ed3DG2DMaterialGetLayer(ed_g2d_material* pMaterial, uint index)
{
	ed_g2d_layer* pLayer = (ed_g2d_layer*)0x0;

	if (index < pMaterial->nbLayers) {
		ed_Chunck* pLAY = LOAD_POINTER_CAST(ed_Chunck*, pMaterial->aLayers[index]);
		pLayer = reinterpret_cast<ed_g2d_layer*>(pLAY + 1);
	}

	return pLayer;
}

ed_hash_code* ed3DG2DGetMaterial(ed_g2d_manager* pTextureInfo, ulong hash)
{
	ed_hash_code* pcVar1;

	pcVar1 = edHashcodeGet(hash, pTextureInfo->pMATA_HASH);
	return pcVar1;
}

struct dummy_built_g2d
{
	GXD_FileHeader fileHeader;

	ed_Chunck TWOD;
	ed_Chunck MATA;
	ed_Chunck HASH;

	ed_hash_code hashCode;

	ed_Chunck MAT;
	ed_g2d_material material;

	ed_Chunck LAYA;

	ed_Chunck LAY;
	ed_g2d_layer layer;
	char _pad_1[12];

	ed_Chunck TEXA;
	ed_Chunck TEX;
	ed_g2d_texture texture;
};

static_assert(sizeof(dummy_built_g2d) == 0x110);

struct dummy_built_g2d_source
{
	ed_Chunck MAT;
	ed_g2d_material material;
};

ed_g2d_manager* ed3DG2DDuplicateMaterial(ed_hash_code* pHashCode, char* pData, ed_g2d_manager* pG2d)
{
	bool bVar1;
	undefined uVar2;
	undefined uVar3;
	undefined uVar4;
	short sVar5;
	dummy_built_g2d_source* pdVar6;
	ed_Chunck* peVar7;
	ed_Chunck* pSrcTEX;
	edF32VECTOR4* peVar9;
	ed_hash_code* pSrcHashCodes;
	ed_hash_code* pDstHashCodes;
	undefined8 uVar12;
	int iVar13;
	ed_Chunck* peVar14;

	dummy_built_g2d* pNewG2d = reinterpret_cast<dummy_built_g2d*>(pData);

	pdVar6 = LOAD_POINTER_CAST(dummy_built_g2d_source*, pHashCode->pData);
	if (pG2d == (ed_g2d_manager*)0x0) {
		IMPLEMENTATION_GUARD(
		pG2d = GetFirstG2DMANAGERFreeAndUseIt((uchar*)pNewG2d);)
	}

	peVar14 = &pNewG2d->TWOD;
	pG2d->pFileBuffer = &pNewG2d->fileHeader;
	pG2d->textureFileLengthA = 0x10;
	pG2d->pMATA_HASH = &pNewG2d->HASH;
	pG2d->pTextureChunk = peVar14;

	pNewG2d->fileHeader.field_0x0 = 1;
	pNewG2d->fileHeader.field_0x2 = 0;
	pNewG2d->fileHeader.flags = 5;
	pNewG2d->fileHeader.field_0x8 = 0x10;
	pNewG2d->fileHeader.hash = HASH_CODE_T2D;

	(pNewG2d->TWOD).hash = HASH_CODE_2D;
	(pNewG2d->TWOD).field_0x4 = 1;
	(pNewG2d->TWOD).field_0x6 = 0;
	(pNewG2d->TWOD).size = 0xd0;
	(pNewG2d->TWOD).nextChunckOffset = 0x10;

	(pNewG2d->MATA).hash = HASH_CODE_MATA;
	(pNewG2d->MATA).field_0x4 = 1;
	(pNewG2d->MATA).field_0x6 = 0;
	(pNewG2d->MATA).size = 0x60;
	(pNewG2d->MATA).nextChunckOffset = 0x10;

	(pNewG2d->HASH).hash = HASH_CODE_HASH;
	(pNewG2d->HASH).field_0x4 = 1;
	(pNewG2d->HASH).field_0x6 = 0;
	(pNewG2d->HASH).size = 0x20;
	(pNewG2d->HASH).nextChunckOffset = 0x20;

	pDstHashCodes = reinterpret_cast<ed_hash_code*>(pNewG2d + 1);

	(pNewG2d->hashCode).hash = pHashCode->hash;
	(pNewG2d->hashCode).pData = STORE_POINTER(&pNewG2d->material);

	pNewG2d->MAT = pdVar6->MAT;
	pNewG2d->material = pdVar6->material;

	(pNewG2d->LAYA).hash = HASH_CODE_LAYA;
	(pNewG2d->LAYA).field_0x4 = 1;
	(pNewG2d->LAYA).field_0x6 = 0;
	(pNewG2d->LAYA).size = 0x50;
	(pNewG2d->LAYA).nextChunckOffset = 0x10;

	peVar7 = LOAD_POINTER_CAST(ed_Chunck*, pdVar6->material.aLayers[0]);
	pNewG2d->material.aLayers[0] = STORE_POINTER(&pNewG2d->LAY);

	pNewG2d->LAY = *peVar7;

	ed_g2d_layer* pLayerSrc = reinterpret_cast<ed_g2d_layer*>(peVar7 + 1);
	pNewG2d->layer = *pLayerSrc;

	(pNewG2d->TEXA).hash = 0x41584554;
	(pNewG2d->TEXA).field_0x4 = 1;
	(pNewG2d->TEXA).field_0x6 = 0;
	(pNewG2d->TEXA).size = 0x40;
	(pNewG2d->TEXA).nextChunckOffset = 0x10;

	pSrcTEX = LOAD_POINTER_CAST(ed_Chunck*, pLayerSrc->pTex);
	(pNewG2d->layer).pTex = STORE_POINTER(&pNewG2d->TEX);
	pSrcHashCodes = reinterpret_cast<ed_hash_code*>(pSrcTEX + 1);

	pNewG2d->TEX = *pSrcTEX;

	ed_g2d_texture* pSrcTexture = reinterpret_cast<ed_g2d_texture*>(pSrcTEX + 1);
	pNewG2d->texture = *pSrcTexture;

	iVar13 = (pNewG2d->texture).bHasPalette;
	(pNewG2d->TEXA).size = (pNewG2d->TEXA).size + iVar13 * 0x10;

	while (bVar1 = iVar13 != 0, iVar13 = iVar13 + -1, bVar1) {
		*pDstHashCodes = *pSrcHashCodes;
		pDstHashCodes = pDstHashCodes + 1;
		pSrcHashCodes = pSrcHashCodes + 1;
	}

	if (pSrcTexture->pAnimSpeedNormalExtruder == 0x0) {
		(pNewG2d->TWOD).size = (char*)pDstHashCodes - (char*)peVar14;
	}
	else {
		(pNewG2d->TEXA).size = (pNewG2d->TEXA).size + 0x10;
		edF32VECTOR4* pSrcAnimExtruder = LOAD_POINTER_CAST(edF32VECTOR4*, pSrcTexture->pAnimSpeedNormalExtruder);
		edF32VECTOR4* pDstAnimExtruder = reinterpret_cast<edF32VECTOR4*>(pDstHashCodes);
		*pDstAnimExtruder = *pSrcAnimExtruder;
		(pNewG2d->texture).pAnimSpeedNormalExtruder = STORE_POINTER(pDstAnimExtruder);
		(pNewG2d->TWOD).size = (int)((char*)pDstHashCodes + ((char*)0x10 - (char*)peVar14));
	}

	pNewG2d->fileHeader.field_0x8 = pNewG2d->fileHeader.field_0x8 + (pNewG2d->TWOD).size;
	pG2d->textureFileLengthA = pNewG2d->fileHeader.field_0x8;

	return pG2d;
}
