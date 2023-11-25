#include "CollisionManager.h"
#include "MemoryStream.h"

void CCollisionManager::Level_Create(ByteCode* pMemoryStream)
{
	int iVar1;
	//BankCollision_14* pBVar2;
	int iVar3;

	iVar1 = pMemoryStream->GetS32();

	this->field_0x4 = iVar1;
	IMPLEMENTATION_GUARD_LOG(
	this->loadedBankCount_0x8 = 0;
	if (this->field_0x4 == 0) {
		this->pBankCollisionData = (BankCollision_14*)0x0;
	}
	else {
		pBVar2 = (BankCollision_14*)operator.new.array((long)(this->field_0x4 * 0x14));
		this->pBankCollisionData = pBVar2;
	})

	for (iVar1 = 0; iVar1 < (int)this->field_0x4; iVar1 = iVar1 + 1) {
		IMPLEMENTATION_GUARD_LOG(
		this->pBankCollisionData[iVar1].pBase = (astruct_11*)0x0;
		this->pBankCollisionData[iVar1].field_0x4 = 0;
		this->pBankCollisionData[iVar1].pNextFree = (astruct_11*)0x0;
		this->pBankCollisionData[iVar1].useCount_0xc = 0;
		)
		iVar3 = pMemoryStream->GetS32();
		IMPLEMENTATION_GUARD_LOG(
		this->pBankCollisionData[iVar1].field_0x8 = iVar3;
		)
	}
	IMPLEMENTATION_GUARD_LOG(
	ClearStaticData();
	if (__bmat_tab_initialised == 0) {
		for (iVar1 = 0; iVar1 < 0xc; iVar1 = iVar1 + 1) {
			*(float*)(&DAT_0040eaa0 + iVar1 * 0x14) =
				g_FloatSineCurve_00472260[(int)(ABS(*(float*)(&DAT_0040eaa0 + iVar1 * 0x14) * 1303.797) + 0.5) & 0x1fff];
			*(float*)(&DAT_0040eaa4 + iVar1 * 0x14) =
				g_FloatSineCurve_00472260[(int)(ABS(*(float*)(&DAT_0040eaa4 + iVar1 * 0x14) * 1303.797) + 0.5) & 0x1fff];
			*(float*)(&DAT_0040eaa8 + iVar1 * 0x14) =
				g_FloatSineCurve_00472260[(int)(ABS(*(float*)(&DAT_0040eaa8 + iVar1 * 0x14) * 1303.797) + 0.5) & 0x1fff];
		}
		__bmat_tab_initialised = 1;
	})
	return;
}
