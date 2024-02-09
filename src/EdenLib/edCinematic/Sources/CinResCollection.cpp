#include "CinResCollection.h"
#include "port/pointer_conv.h"
#include "Cinematic.h"

char* edResCollection::GetResFilename(const int offset)
{
	char* resFilePath;
	int iVar1;

	/* Dereference the pointer to the res buffer */
	/* Dereference the res buffer again and load into v1 */
	/* Shift the bits left by 1 (4 will become 8) */
	/* Add v0 to v1 */
	/* Shift v0 another 2 bits to the left */
	resFilePath = (char*)this->pData + (this->pData->resCount * sizeof(edResCollectionTag)) + sizeof(edResCollectionHeader) + 1;
	/* Reduce param 2 by 1 */
	/* Move v0 bits along the res buffer */
	/* Check if num < 0 */
	for (iVar1 = offset + -1; -1 < iVar1; iVar1 = iVar1 + -1) {
		resFilePath = resFilePath + (byte)resFilePath[-1];
		/* Reduce num by 1 */
		/* Repeat if num is still > 0 */
	}
	return resFilePath;
}

char* edResCollection::LoadResource(edCinGameInterface& cinGameInterface, int offset, int* outSize)
{
	char* fileBuffer;
	int* resSeekPos;
	int iVar1;
	char* resNameStart;
	edResCollectionTag* pTag;

	edResCollectionHeader* pHeader = this->pData;

	pTag = &pHeader->aTags[offset];

	if (pTag->pData == 0x0) {
		resNameStart = (char*)this->pData + (pHeader->resCount * sizeof(edResCollectionTag)) + sizeof(edResCollectionHeader) + 1;
		for (iVar1 = offset + -1; -1 < iVar1; iVar1 = iVar1 + -1) {
			resNameStart = resNameStart + (byte)resNameStart[-1];
		}
		/* This will call LoadCutsceneFile and return a pointer to the start of the file buffer */
		fileBuffer = cinGameInterface.GetResource((edResCollection::RES_TYPE)(pTag->flags & 0x7fffffff), (pTag->flags & 0x80000000) != 0, resNameStart, &pTag->size);
		pTag->pData = STORE_SECTION(fileBuffer);
	}
	if (outSize != (int*)0x0) {
		*outSize = pTag->size;
	}
	return (char*)LOAD_SECTION(pTag->pData);
}

void edResCollection::FlushAllResources(edCinGameInterface& cinGameInterface)
{
	edResCollectionTag* pcVar1;
	int iVar2;
	int iVar3;

	iVar3 = this->pData->resCount + -1;
	if (-1 < iVar3) {
		do {
			pcVar1 = &this->pData->aTags[iVar3];
			if (((pcVar1->flags & 0x80000000) == 0) && (pcVar1->pData != 0)) {
				cinGameInterface.ReleaseResource(pcVar1->flags, false, (void*)LOAD_SECTION(pcVar1->pData));
				pcVar1->pData = 0;
			}
			iVar3 = iVar3 + -1;
		} while (-1 < iVar3);
	}
	return;
}
