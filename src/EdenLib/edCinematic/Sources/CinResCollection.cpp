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
	resFilePath = ((char*)this->pData) + this->pData->field_0x0 * 0xc + 5;
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

PACK(
	struct ResourceHeader {
	uint type;
	int pData; // char*
	int size;
});

char* edResCollection::LoadResource(edCinGameInterface& cinGameInterface, int offset, int* outSize)
{
	char* fileBuffer;
	int* resSeekPos;
	int iVar1;
	char* resNameStart;
	ResourceHeader* resInfo;

	resSeekPos = (int*)(this->pData + 1);
	resInfo = (ResourceHeader*)(resSeekPos + offset * 3);

	if (resInfo->pData == 0x0) {
		resNameStart = (char*)((char*)resSeekPos + this->pData->field_0x0 * 0xc + 1);
		for (iVar1 = offset + -1; -1 < iVar1; iVar1 = iVar1 + -1) {
			resNameStart = resNameStart + (byte)resNameStart[-1];
		}
		/* This will call LoadCutsceneFile and return a pointer to the start of the file buffer */
		fileBuffer = cinGameInterface.GetResource((edResCollection::RES_TYPE)(resInfo->type & 0x7fffffff), (resInfo->type & 0x80000000) != 0, resNameStart, &resInfo->size);
		resInfo->pData = STORE_SECTION(fileBuffer);
	}
	if (outSize != (int*)0x0) {
		*outSize = resInfo->size;
	}
	return (char*)LOAD_SECTION(resInfo->pData);
}

void edResCollection::FlushAllResources(edCinGameInterface& cinGameInterface)
{
	ResourceHeader* pcVar1;
	int iVar2;
	int iVar3;

	iVar3 = *(int*)this->pData->field_0x0 + -1;
	if (-1 < iVar3) {
		iVar2 = iVar3 * 0xc;
		do {
			pcVar1 = (ResourceHeader*)(this->pData->field_0x0 + iVar2);
			if ((((uint)pcVar1->pData & 0x80000000) == 0) && (pcVar1->size != 0)) {
				cinGameInterface.ReleaseResource((void*)LOAD_SECTION(pcVar1->pData), false);
				pcVar1->size = 0;
			}
			iVar3 = iVar3 + -1;
			iVar2 = iVar2 + -0xc;
		} while (-1 < iVar3);
	}
	return;
}
