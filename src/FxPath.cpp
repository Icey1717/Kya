#include "FxPath.h"
#include "PathFollow.h"
#include "TimeController.h"

CFxPath::CFxPath()
	: CFx()
{
	this->count_0x50 = 0;
	this->field_0x58 = (EffectsManagerObj_60*)0x0;

	(this->vector_0x60).x = 0.0f;
	(this->vector_0x60).y = 0.0f;
	(this->vector_0x60).z = 0.0f;
	(this->vector_0x60).w = 0.0f;
	this->field_0x5c = -1;
	this->field_0x40 = 1;

	this->flags = this->flags | 3;
	this->flags = this->flags & 0xfffffff7;
	this->flags = this->flags & 0xfffffffd;

	return;
}

void CFxPath::AddAll(ByteCode* pByteCode, CFx* aFx, int count)
{
	IMPLEMENTATION_GUARD();
}

void CFxPath::ManageAll(uint count, CFx* aFx)
{
	bool bVar1;
	uint uVar2;
	Timer* pTVar3;
	EffectsManagerObj_60* pEVar4;
	int iVar5;
	int iVar6;
	float fVar7;
	float fVar8;
	float fVar9;
	float fVar10;
	float fVar11;
	S_PATHREADER_POS_INFO SStack16;

	CFxPath* pFx = reinterpret_cast<CFxPath*>(aFx);

	iVar6 = count - 1;
	if (count != 0) {
		do {
			if ((pFx->flags & 1U) != 0) {
				iVar5 = pFx->count_0x50;
				pEVar4 = pFx->field_0x58;
				while (iVar5 != 0) {
					IMPLEMENTATION_GUARD(
					iVar5 = iVar5 + -1;
					if (((pEVar4->field_0x58 & 4) == 0) && ((pEVar4->field_0x58 & 1) == 0)) {
						fVar11 = (pFx->vector_0x60).w + (pEVar4->field_0x48).z;
						pTVar3 = Timer::GetTimer();
						fVar7 = pTVar3->cutsceneDeltaTime;
						fVar8 = fmodf(fVar11, (pEVar4->pActor44_20).barFullAmount_0x4);
						fVar7 = fmodf(fVar11 - fVar7, (pEVar4->pActor44_20).barFullAmount_0x4);
						if ((fVar8 < fVar7) && ((pEVar4->field_0x58 & 8) != 0)) {
							pEVar4->field_0x58 = pEVar4->field_0x58 & 0xfffffff7;
							fVar7 = (float)pEVar4->field_0x24;
							*(uint*)((int)fVar7 + 0x44) = *(uint*)((int)fVar7 + 0x44) | 3;
							*(uint*)((int)fVar7 + 0x44) = *(uint*)((int)fVar7 + 0x44) & 0xfffffff7;
						}
						fVar7 = (pFx->vector_0x60).y;
						fVar11 = 0.0;
						if (fVar8 < fVar7) {
							fVar11 = 1.0 - fVar8 / fVar7;
						}
						else {
							fVar7 = (pFx->vector_0x60).z;
							fVar8 = fVar8 - ((pEVar4->pActor44_20).barFullAmount_0x4 - fVar7);
							if (0.0 < fVar8) {
								fVar11 = fVar8 / fVar7;
							}
						}
						CPathFollowReaderAbsolute::ComputePosition
						((pFx->vector_0x60).w + (pEVar4->field_0x48).z, &pEVar4->pActor44_20, (edF32VECTOR4*)pEVar4,
							(edF32VECTOR4*)0x0, &SStack16);
						*(float*)&pEVar4->field_0x20 = SStack16.field_0x8;
						if ((pEVar4->field_0x58 & 8) == 0) {
							fVar7 = (float)pEVar4->field_0x24;
							fVar10 = *(float*)&pEVar4->field_0x4;
							fVar8 = *(float*)&pEVar4->field_0x8;
							fVar9 = *(float*)&pEVar4->field_0xc;
							*(undefined4*)((int)fVar7 + 0x10) = *(undefined4*)pEVar4;
							*(float*)((int)fVar7 + 0x14) = fVar10;
							*(float*)((int)fVar7 + 0x18) = fVar8;
							*(float*)((int)fVar7 + 0x1c) = fVar9;
							*(float*)(pEVar4->field_0x24 + 0x48) = fVar11;
						}
					}
					pEVar4 = pEVar4 + 1;)
				}

				if ((pFx->flags & 8) == 0) {
					(pFx->vector_0x60).w = (pFx->vector_0x60).w + Timer::GetTimer()->cutsceneDeltaTime;
				}
				else {
					pFx->flags = pFx->flags & 0xfffffffe;
				}
			}

			pFx = pFx + 1;
			bVar1 = iVar6 != 0;
			iVar6 = iVar6 + -1;
		} while (bVar1);
	}

	return;
}

void CFxPath::Draw()
{
	return;
}
