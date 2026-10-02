#include "ActorWantedZoo.h"
#include "MemoryStream.h"
#include "BootData.h"
#include "TimeController.h"
#include "DlistManager.h"
#include "TranslatedTextData.h"
#include "edText.h"
#include "kya.h"

void CActorWantedZoo::Create(ByteCode* pByteCode)
{
	CActor::Create(pByteCode);

	this->field_0x160 = pByteCode->GetS32();
	if (this->field_0x160 == 0) {
		this->field_0x164 = (edF32VECTOR2*)0x0;
	}
	else {
		this->field_0x164 = (edF32VECTOR2*)pByteCode->currentSeekPos;
		pByteCode->currentSeekPos = (char*)(this->field_0x164 + this->field_0x160);
	}

	this->field_0x168 = pByteCode->GetS32();
	if (this->field_0x168 == 0) {
		this->field_0x16c = (edF32VECTOR2*)0x0;
	}
	else {
		this->field_0x16c = (edF32VECTOR2*)pByteCode->currentSeekPos;
		pByteCode->currentSeekPos = (char*)(this->field_0x16c + this->field_0x168);
	}

	this->field_0x230 = pByteCode->GetS32();
	if (this->field_0x230 == 0) {
		this->field_0x234 = (Zoo_14*)0x0;
	}
	else {
		this->field_0x234 = new Zoo_14[this->field_0x230];
		for (int i = 0; i < this->field_0x230; i++) {
			Zoo_14* pEntry = &this->field_0x234[i];
			pEntry->scenaricCondition.Create(pByteCode);
			pEntry->field_0x4.field_0x0 = pByteCode->GetS32();
			if (pEntry->field_0x4.field_0x0 == 0) {
				pEntry->field_0x4.field_0x4 = (uint*)0x0;
			}
			else {
				pEntry->field_0x4.field_0x4 = (uint*)pByteCode->currentSeekPos;
				pByteCode->currentSeekPos += pEntry->field_0x4.field_0x0 * 8;
			}

			pEntry->field_0x4.field_0x8 = pByteCode->GetS32();
			if (pEntry->field_0x4.field_0x8 == 0) {
				// The original clears the count here rather than the data pointer.
				pEntry->field_0x4.field_0x8 = 0;
			}
			else {
				pEntry->field_0x4.field_0xc = (int*)pByteCode->currentSeekPos;
				pByteCode->currentSeekPos += pEntry->field_0x4.field_0x8 * 4;
			}
		}
	}

	return;
}

void CActorWantedZoo::Init()
{
	CActor::Init();

	this->textStyle.SetShadow(0x100);
	(this->textStyle).rgbaColour = 0xffffffff;
	(this->textStyle).alpha = 0xff;
	this->textStyle.SetHorizontalAlignment(2);
	this->textStyle.SetVerticalAlignment(8);
	this->textStyle.SetScale(1.5f, 1.5f);
	this->textStyle.SetFont(BootDataFont, false);
	this->field_0x238 = false;
	this->field_0x23c = 0.0f;

	return;
}

void CActorWantedZoo::Term()
{
	if (this->field_0x234 != (Zoo_14*)0x0) {
		delete[] this->field_0x234;
		this->field_0x234 = (Zoo_14*)0x0;
	}

	CActor::Term();

	return;
}

void CActorWantedZoo::Manage()
{
	float fVar2;

	CActor::Manage();

	if (this->field_0x238 == false) {
		if (0.0f < this->field_0x23c) {
			fVar2 = this->field_0x23c - GetTimer()->cutsceneDeltaTime * 2.0f;
			this->field_0x23c = fVar2;
			if (fVar2 < 0.0f) {
				this->field_0x23c = 0.0f;
			}
		}
	}
	else {
		if (this->field_0x23c < 1.0f) {
			fVar2 = this->field_0x23c + GetTimer()->cutsceneDeltaTime * 2.0f;
			this->field_0x23c = fVar2;
			if (1.0f < fVar2) {
				this->field_0x23c = 1.0f;
			}
		}
	}

	return;
}

void CActorWantedZoo::Draw()
{
	CActor::Draw();

	if (0.0f < this->field_0x23c) {
		Zoo_10* pEntry = (Zoo_10*)0x0;
		for (int i = 0; i < this->field_0x230; i++) {
			Zoo_14* pCond = this->field_0x234 + i;
			if (pCond->scenaricCondition.IsVerified() != 0) {
				pEntry = &pCond->field_0x4;
				break;
			}
		}

		if ((pEntry != (Zoo_10*)0x0) && (GuiDList_BeginCurrent() != false)) {
			_rgba colour;
			colour.LerpRGBA(this->field_0x23c, 0, 0xffffffff);
			this->textStyle.rgbaColour = colour.rgba;
			colour.LerpRGBA(this->field_0x23c, 0, 0xff);
			this->textStyle.alpha = colour.rgba;
			this->textStyle.SetFont(BootDataFont, false);
			edCTextStyle* pNewFont = edTextStyleSetCurrent(&this->textStyle);

			for (int i = 0; (i < pEntry->field_0x0) && (i < this->field_0x160); i++) {
				uint* pKey = pEntry->field_0x4 + i * 2;
				edF32VECTOR2* pPosition = this->field_0x164 + i;
				ulong key = ByteCode::BuildU64(pKey[0], pKey[1]);
				char* text = gMessageManager.get_message(key);
				edTextDraw(pPosition->x * (float)gVideoConfig.screenWidth,
					pPosition->y * (float)gVideoConfig.screenHeight, text);
			}

			for (int i = 0; (i < pEntry->field_0x8) && (i < this->field_0x168); i++) {
				edF32VECTOR2* pPosition = &this->field_0x16c[i];
				edTextDraw(pPosition->x * (float)gVideoConfig.screenWidth,
					pPosition->y * (float)gVideoConfig.screenHeight, "%d", pEntry->field_0xc[i]);
			}

			edTextStyleSetCurrent(pNewFont);
			GuiDList_EndCurrent();
		}
	}

	return;
}
