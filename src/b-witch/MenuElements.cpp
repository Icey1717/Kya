#include "MenuElements.h"
#include "TimeController.h"

bool CMenuArrows::FUN_002ef890()
{
	bool bVar1;

	if (0.0f < this->field_0x198) {
		this->field_0x198 = this->field_0x198 - GetTimer()->lastFrameTime;
		if (this->field_0x180 < this->field_0x198) {
			this->field_0x198 = this->field_0x180;
		}
		else {
			if (this->field_0x198 < 0.0f) {
				this->field_0x198 = 0.0f;
			}
		}
		bVar1 = true;
	}
	else {
		bVar1 = false;
	}

	return bVar1;
}

void CMenuArrows::FUN_002ef4e0(float param_1, float param_2, float param_3, float param_4, byte param_6)
{
	this->field_0x184 = param_1;
	this->field_0x188 = param_2;
	this->field_0x18c = param_3;
	this->field_0x190 = param_4;
	this->field_0x194 = param_6;

	FUN_002ef500();

	return;
}

void CMenuArrows::FUN_002ef500()
{
	bool cVar1;
	byte bVar2;
	uint uVar3;
	uint uVar4;
	int uVar5;
	float fVar6;
	float fVar7;

	fVar7 = ((this->field_0x180 - this->field_0x198) / this->field_0x180) * 2.0f;
	fVar6 = 1.0f;
	if ((fVar7 <= 1.0f) && (fVar6 = fVar7, fVar7 < 0.0f)) {
		fVar6 = 0.0f;
	}

	fVar6 = fVar6 * 128.0f;
	if (fVar6 < 2.147484e+09f) {
		cVar1 = this->field_0x19d;
	}
	else {
		fVar6 = fVar6 - 2.147484e+09f;
		cVar1 = this->field_0x19d;
	}

	uVar4 = static_cast<uint>(fVar6);
	uVar5 = 0x292;
	if (cVar1 != false) {
		if (this->field_0x194 == 0) {
			uVar5 = 0xc292;
		}

		if ((this->field_0x1a0 & 1) == 0) {
			if ((this->field_0x198 <= 0.0f) || (this->field_0x19c == false)) {
				bVar2 = this->field_0x1a4;
				if (0x80 < bVar2) {
					bVar2 = 0x80;
				}

				(this->field_0x0).rgba[3] = bVar2;
				this->field_0x0.Draw(1.0f, this->field_0x184, this->field_0x188, uVar5);
			}
			else {
				bVar2 = this->field_0x1a4;
				if ((uVar4 & 0xff) < static_cast<uint>(bVar2)) {
					bVar2 = (byte)(uVar4 & 0xff);
				}

				(this->field_0xc0).rgba[3] = bVar2;
				(this->field_0x0).rgba[3] = 0x80 - (byte)uVar4;
				this->field_0x0.Draw(1.0f, this->field_0x184, this->field_0x188, uVar5);
				this->field_0xc0.Draw(1.0f, this->field_0x184, this->field_0x188, uVar5);
			}
		}

		uVar5 = 0x8292;
		if (this->field_0x194 == 0) {
			uVar5 = 0x4292;
		}

		if ((this->field_0x1a0 & 2) == 0) {
			if ((this->field_0x198 <= 0.0f) || (this->field_0x19c != false)) {
				bVar2 = this->field_0x1a4;
				if (0x80 < bVar2) {
					bVar2 = 0x80;
				}

				(this->field_0x0).rgba[3] = bVar2;
				this->field_0x0.Draw(1.0f, this->field_0x18c, this->field_0x190, uVar5);
			}
			else {
				(this->field_0xc0).rgba[3] = (byte)uVar4;

				uVar3 = (uint)this->field_0x1a4;
				uVar4 = 0x80 - (uVar4 & 0xff);
				if (static_cast<int>(uVar4) < static_cast<int>(uVar3)) {
					uVar3 = uVar4;
				}
				(this->field_0x0).rgba[3] = (byte)uVar3;

				this->field_0x0.Draw(1.0f, this->field_0x18c, this->field_0x190, uVar5);
				this->field_0xc0.Draw(1.0f, this->field_0x18c, this->field_0x190, uVar5);
			}
		}
	}

	return;
}

void CMenuArrows::Init(edDList_material* param_2, edDList_material* param_3)
{
	if (this->field_0x1ac == 0) {
		this->field_0x1ac = 1;

		FUN_002ef9b0(param_2, param_3);
	}

	return;
}

void CMenuArrows::FUN_002ef9b0(edDList_material* param_2, edDList_material* param_3)
{
	if (this->field_0x1a8 == 0) {
		this->field_0x1a8 = 1;
		this->field_0x19d = false;

		if (param_2 != (edDList_material*)0x0) {
			this->field_0x0.Install(param_2);
		}

		if (param_3 != (edDList_material*)0x0) {
			this->field_0xc0.Install(param_3);
		}

		this->field_0x19d = param_2 != (edDList_material*)0x0 && param_3 != (edDList_material*)0x0;
		this->field_0x180 = 0.1f;
		this->field_0x184 = 0;
		this->field_0x188 = 0;
		this->field_0x18c = 0;
		this->field_0x190 = 0;
		this->field_0x198 = 0.0f;
		this->field_0x194 = 1;
		this->field_0x1a0 = 0;
		this->field_0x1a4 = 0x80;
	}

	return;
}

void CMenuArrows::Term()
{
	if (this->field_0x1ac == 1) {
		this->field_0x1ac = 0;

		this->field_0x0.Remove();
		this->field_0xc0.Remove();
	}

	return;
}

CMenuWheel::CMenuWheel()
{
	this->field_0xc0.field_0x1a8 = 0;
}

void CMenuWheel::Init(edDList_material* pMaterial, edDList_material* pArrow, edDList_material* pArrowHighlight)
{
	this->field_0x274 = 0;
	this->field_0xc0.centerX = 0.0f;
	this->field_0x270 = 0.0f;
	this->field_0x284 = 255.0f;
	this->field_0x280 = 255.0f;
	this->field_0x28c = 255.0f;
	this->field_0x27c = 0;
	this->field_0x278 = 1;
	this->pFunc = 0;
	this->field_0x298 = 0;
	this->field_0x290 = false;

	if (pMaterial != (edDList_material*)0x0) {
		this->field_0x0.Install(pMaterial);
		this->field_0x290 = true;
	}

	this->field_0xc0.FUN_002ef9b0(pArrow, pArrowHighlight);

	return;
}

void CMenuWheel::Reset()
{
	this->field_0x284 = 255.0f;
	this->field_0x280 = 255.0f;
	this->field_0x27c = 0;
	this->field_0xc0.field_0x198 = 0.0f;

	return;
}

void CMenuWheel::MoveWheel()
{
	this->field_0x284 = this->field_0x280;
	this->field_0xc0.field_0x198 = 0.0f;

	return;
}

void CMenuWheel::MoveWheel(bool bNext)
{
	int iVar1;
	float fVar2;

	iVar1 = this->field_0x274;
	if (1 < iVar1) {
		if (!bNext) {
			this->field_0x27c = (this->field_0x27c + -1 + iVar1) % iVar1;
		}
		else {
			this->field_0x27c = (this->field_0x27c + 1) % iVar1;
		}

		if (bNext == true) {
			this->field_0xc0.field_0x19c = false;
		}
		else {
			this->field_0xc0.field_0x19c = true;
		}

		if (this->field_0xc0.field_0x198 == 0.0f) {
			this->field_0xc0.field_0x198 = this->field_0xc0.field_0x180;
		}

		fVar2 = this->field_0xc0.field_0x180 / 2.0f;
		if (this->field_0xc0.field_0x198 <= fVar2) {
			this->field_0xc0.field_0x198 = fVar2;
		}

		this->field_0x280 = 255.0f;
		this->field_0x284 = 0.0f;
		this->field_0x288 = bNext;
	}

	return;
}

bool CMenuWheel::Manage()
{
	Timer* pTVar1;
	bool bVar2;
	float fVar3;
	float fVar4;

	fVar3 = this->field_0x284;
	fVar4 = this->field_0x280;
	pTVar1 = GetTimer();

	if (fabsf(fVar3 - fVar4) < this->field_0x28c * pTVar1->lastFrameTime) {
		this->field_0x284 = 255.0f;
	}

	if (0.0f < this->field_0xc0.field_0x198) {
		pTVar1 = GetTimer();
		fVar3 = this->field_0xc0.field_0x198 - pTVar1->lastFrameTime;
		this->field_0xc0.field_0x198 = fVar3;
		fVar4 = this->field_0xc0.field_0x180;
		if (fVar4 < fVar3) {
			this->field_0xc0.field_0x198 = fVar4;
		}
		else {
			if (fVar3 < 0.0f) {
				this->field_0xc0.field_0x198 = 0.0f;
			}
		}
	}

	if ((this->field_0x278 == 0) || (this->field_0x280 <= this->field_0x284)) {
		bVar2 = false;
		this->field_0x284 = this->field_0x280;
	}
	else {
		bVar2 = true;
		pTVar1 = GetTimer();
		this->field_0x284 = this->field_0x284 + this->field_0x28c * pTVar1->lastFrameTime;
	}

	return bVar2;
}

void CMenuWheel::FUN_002efa80(float x, float y, edF32VECTOR2* pRight, edF32VECTOR2* pLeft)
{
	this->field_0xc0.centerX = x;
	this->field_0x270 = y;

	if (!this->field_0x290) {
		if (pRight == (edF32VECTOR2*)0x0) return;
		this->field_0xc0.field_0x184 = pLeft->x;
		this->field_0xc0.field_0x188 = pLeft->y;

		this->field_0xc0.field_0x18c = pRight->x;
		this->field_0xc0.field_0x190 = pRight->y;
	}
	else {
		this->field_0xc0.field_0x184 = x - (float)this->field_0x0.iWidth / 2.0f;
		this->field_0xc0.field_0x188 = y;

		this->field_0xc0.field_0x18c = x + (float)this->field_0x0.iWidth / 2.0f;
		this->field_0xc0.field_0x190 = y;
	}

	return;
}

void CMenuWheel::Draw()
{
	S_MENU_WHEEL_DRAW local_10;
	int iVar1;
	if (this->field_0x290) {
		this->field_0x0.Draw(1.0f, this->field_0xc0.centerX, this->field_0x270, 0x12);
	}

	if (1 < this->field_0x274) {
		this->field_0xc0.FUN_002ef500();
	}

	local_10.x = this->field_0xc0.centerX;
	local_10.y = this->field_0x270;
	local_10.alpha = (byte)(int)this->field_0x284;
	if (!this->field_0x288) {
		iVar1 = (this->field_0x27c + 1) % this->field_0x274;
	}
	else {
		iVar1 = (this->field_0x27c + -1 + this->field_0x274) % this->field_0x274;
	}

	this->pFunc(this->field_0x27c, iVar1, &local_10, this->field_0x298);

	return;
}