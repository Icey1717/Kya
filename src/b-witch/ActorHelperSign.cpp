#include "ActorHelperSign.h"
#include "MemoryStream.h"
#include "DlistManager.h"
#include "ActorHero.h"
#include "ActorMiniGamesOrganizer.h"
#include "kya.h"
#include "Pause.h"
#include "MathOps.h"
#include "edText.h"
#include "TimeController.h"
#include "BootData.h"

void CActorHelperSign::Create(ByteCode* pByteCode)
{
	int iVar1;
	ulong uVar2;

	CActor::Create(pByteCode);
	this->field_0x168 = pByteCode->GetS32();
	uVar2 = pByteCode->GetU64();
	this->field_0x160 = uVar2;
	this->field_0x190 = 1;

	return;
}

void CActorHelperSign::Init()
{
	CActor::Init();

	this->field_0x174 = 0.0f;
	this->field_0x188 = 0.0f;
	this->field_0x18c = 0.0f;

	return;
}

void CActorHelperSign::Draw()
{
	int iVar1;
	bool bVar2;
	CBehaviourHelperSignHighScore* pCVar3;
	CBehaviourHelperSignHighScore* pCVar4;
	StateConfig* pSVar3;
	uint uVar4;
	float fVar5;
	float fVar6;
	float fVar7;
	float x;

	CActor::Draw();

	if (this->curBehaviourId == HELPER_SIGN_BEHAVIOUR_HIGHSCORE) {
		IMPLEMENTATION_GUARD_LOG(
		if ((this->actorState == HELPER_SIGN_STATE_SHOW) && (bVar2 = GuiDList_BeginCurrent(), bVar2 != false)) {
			FUN_00339ab0();
			pCVar3 = static_cast<CBehaviourHelperSignHighScore*>(GetBehaviour(this->curBehaviourId));
			pCVar3->menuWheel.Draw();
			GuiDList_EndCurrent();
		}

		if (this->actorState == 9) {
			bVar2 = Frontend2DDList_BeginCurrent();
			if (bVar2 != false) {
				pCVar4 = static_cast<CBehaviourHelperSignHighScore*>(GetBehaviour(this->curBehaviourId));
				FUN_003b4f30(0.0f, 0, 104.0f, static_cast<CActorMiniGamesOrganizer*>(pCVar4->field_0x8.Get()), &pCVar4->field_0x50);
				FrontendDList_EndCurrent();
			}

			FUN_001b1830(0);
			return;
		})
	}

	if (((GameFlags & 0x3c) == 0) && (this->curBehaviourId == HELPER_SIGN_BEHAVIOUR_STAND)) {
		if ((GetStateFlags(this->actorState) & 0x100) != 0) {
			fVar6 = this->field_0x184;
			fVar5 = this->field_0x17c;
			x = (this->field_0x178 + this->field_0x180) / 2.0f;
			fVar7 = this->field_0x180 - this->field_0x178;
			bVar2 = GuiDList_BeginCurrent();
			if (bVar2 != false) {
				CPauseManager::DrawRectangleBorder(x, (fVar5 + fVar6) / 2.0f, fVar7, fVar6 - fVar5, static_cast<float>(gVideoConfig.screenWidth) * 0.2f, static_cast<float>(gVideoConfig.screenHeight) * 0.2f, 0x40101030, 0, 0);
				GuiDList_EndCurrent();
				DrawHelpText();
			}
		}
	}
	return;
}

void CActorHelperSign::Reset()
{
	CActor::Reset();

	this->field_0x174 = 0.0f;
	this->field_0x188 = 0.0f;
	this->field_0x18c = 0.0f;

	return;
}

CBehaviour* CActorHelperSign::BuildBehaviour(int behaviourType)
{
	CBehaviour* pBehaviour;

	if (behaviourType == 3) {
		pBehaviour = new CBehaviourHelperSignHighScore;
	}
	else {
		if (behaviourType == 2) {
			pBehaviour = &this->behaviourStand;
		}
		else {
			pBehaviour = CActor::BuildBehaviour(behaviourType);
		}
	}

	return pBehaviour;
}

StateConfig CActorHelperSign::_gStateCfg_HLP[5] = {
	{ -1, 0x00000000 },
	{ -1, 0x00000100 },
	{ -1, 0x00000100 },
	{ -1, 0x00000100 },
	{ -1, 0x00000100 }
};

StateConfig* CActorHelperSign::GetStateCfg(int state)
{
	StateConfig* pStateConfig;

	if (state < 5) {
		pStateConfig = CActor::GetStateCfg(state);
	}
	else {
		assert((state - 5) < 5);
		pStateConfig = _gStateCfg_HLP + state + -5;
	}

	return pStateConfig;
}

void CActorHelperSign::ChangeManageState(int state)
{
	CNewFx* pCVar1;
	int iVar2;
	bool bVar3;

	CActor::ChangeManageState(state);

	if (state == 0) {
		if (this->field_0x194.IsValid()) {
			this->field_0x194.Stop();
		}
	}
	else {
		if ((!this->field_0x194.IsValid()) && (CScene::ptable.g_EffectsManager_004516b8 != (CFxManager*)0x0)) {
			CScene::ptable.g_EffectsManager_004516b8->GetDynamicFx(&this->field_0x194, this->field_0x168, FX_MATERIAL_SELECTOR_NONE);
			this->field_0x194.SpatializeOnActor(6, this, 0);
			this->field_0x194.Start();
		}
	}

	return;
}

int CActorHelperSign::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	int result;

	if (msg == 0x51) {
		result = 1;
	}
	else {
		if (msg == 0x52) {
			result = 1;
		}
		else {
			if (msg == 0x24) {
				this->field_0x190 = 1;
				this->flags = this->flags & 0xffffff5f;
				EvaluateDisplayState();

				result = 1;
				this->flags = this->flags & 0xfffffffc;
			}
			else {
				if (msg == 0x23) {
					if (*reinterpret_cast<int*>(pMsgParam) == 0) {
						result = 0;
					}
					else {
						this->field_0x190 = 0;
						this->flags = this->flags & 0xffffff7f;
						this->flags = this->flags | 0x20;

						EvaluateDisplayState();

						result = 1;
						this->flags = this->flags & 0xfffffffd;
						this->flags = this->flags | 1;
					}
				}
				else {
					result = CActor::InterpretMessage(pSender, msg, pMsgParam);
				}
			}
		}
	}

	return result;
}

astruct_22 astruct_22_0048fa40;

bool astruct_22::FUN_002ef890()
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

void astruct_22::FUN_002ef4e0(float param_1, float param_2, float param_3, float param_4, byte param_6)
{
	this->field_0x184 = param_1;
	this->field_0x188 = param_2;
	this->field_0x18c = param_3;
	this->field_0x190 = param_4;
	this->field_0x194 = param_6;

	FUN_002ef500();

	return;
}

void astruct_22::FUN_002ef500()
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

void astruct_22::Init(edDList_material* param_2, edDList_material* param_3)
{
	if (this->field_0x1ac == 0) {
		this->field_0x1ac = 1;

		FUN_002ef9b0(param_2, param_3);
	}

	return;
}

void astruct_22::FUN_002ef9b0(edDList_material* param_2, edDList_material* param_3)
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


void astruct_22::Term()
{
	if (this->field_0x1ac == 1) {
		this->field_0x1ac = 0;

		this->field_0x0.Remove();
		this->field_0xc0.Remove();
	}

	return;
}

void CActorHelperSign::DrawHelpText()
{
	bool bVar1;
	edCTextStyle* pNewFont;
	char* pMsg;
	long lVar2;
	float fVar3;
	float fVar4;
	float fVar5;
	float fVar6;
	float fVar7;
	float fVar8;
	float fVar9;
	float fVar10;
	Rectangle rect;
	edCTextStyle textStyle;

	bVar1 = GuiDList_BeginCurrent();
	if (bVar1 != false) {
		textStyle.Reset();
		textStyle.SetShadow(0x100);
		textStyle.alpha = 0xff;
		textStyle.SetHorizontalAlignment(1);
		textStyle.SetVerticalAlignment(4);
		textStyle.SetFont(BootDataFont, false);
		pNewFont = edTextStyleSetCurrent(&textStyle);
		pMsg = gMessageManager.get_message(0x505d4441484f4f52);
		textStyle.SetScale(this->field_0x188, this->field_0x18c);
		edTextDraw(this->field_0x180 - this->field_0x188 * static_cast<float>((gVideoConfig.screenWidth * 5) / 6 - gVideoConfig.screenWidth / 6) * 0.14f,
			this->field_0x184 - this->field_0x18c * static_cast<float>((gVideoConfig.screenHeight * 5) / 6 - gVideoConfig.screenHeight / 6) * 0.09f, pMsg);
		textStyle.SetHorizontalAlignment(0);
		textStyle.SetVerticalAlignment(0);
		textStyle.SetEolAutomatic(0x80);
		textStyle.SetHorizontalJustification(0x10);
		fVar3 = static_cast<float>((gVideoConfig.screenWidth * 5) / 6 - gVideoConfig.screenWidth / 6);
		textStyle.SetHorizontalSize((fVar3 - fVar3 * 0.16f) - fVar3 * 0.14f);
		edCTextFormat textFormat;
		textFormat.FormatString(this->field_0x16c);
		fVar5 = this->field_0x18c;
		fVar3 = static_cast<float>((gVideoConfig.screenHeight * 5) / 6 - gVideoConfig.screenHeight / 6);
		fVar9 = this->field_0x17c + fVar5 * fVar3 * 0.16f;
		fVar10 = this->field_0x184 - fVar5 * fVar3 * 0.26f;
		fVar5 = fVar9 + this->field_0x174 * fVar5;
		GetBehaviour(this->curBehaviourId);

		fVar3 = static_cast<float>(gVideoConfig.screenWidth) * 0.05f * this->field_0x188;
		if (fVar3 < 2.147484e+09f) {
			astruct_22_0048fa40.field_0x0.iWidth = static_cast<ushort>((int)fVar3);
		}
		else {
			astruct_22_0048fa40.field_0x0.iWidth = static_cast<ushort>((int)(fVar3 - 2.147484e+09f));
		}

		fVar3 = static_cast<float>(gVideoConfig.screenHeight) * 0.04f * this->field_0x18c;
		if (fVar3 < 2.147484e+09f) {
			astruct_22_0048fa40.field_0x0.iHeight = static_cast<ushort>((int)fVar3);
		}
		else {
			astruct_22_0048fa40.field_0x0.iHeight = static_cast<ushort>((int)(fVar3 - 2.147484e+09f));
		}

		fVar3 = static_cast<float>(gVideoConfig.screenWidth) * 0.05f * this->field_0x188;
		if (fVar3 < 2.147484e+09f) {
			astruct_22_0048fa40.field_0xc0.iWidth = static_cast<ushort>((int)fVar3);
		}
		else {
			astruct_22_0048fa40.field_0xc0.iWidth = static_cast<ushort>((int)(fVar3 - 2.147484e+09f));
		}

		fVar3 = static_cast<float>(gVideoConfig.screenHeight) * 0.04f * this->field_0x18c;
		if (fVar3 < 2.147484e+09f) {
			astruct_22_0048fa40.field_0xc0.iHeight = static_cast<ushort>((int)fVar3);
		}
		else {
			astruct_22_0048fa40.field_0xc0.iHeight = static_cast<ushort>((int)(fVar3 - 2.147484e+09f));
		}

		IMPLEMENTATION_GUARD_LOG(
		if (((this->actorState == 6) && (*(int**)&this->field_0x170 != (int*)0x0)) && (lVar2 = (**static_cast<code**>(**(int**)&this->field_0x170 + 200))(), lVar2 != 0)) {
			if (0.0f < *static_cast<float*>((int)lVar2 + 0x5f8)) {
				astruct_22_0048fa40.FUN_002ef920(false);
				if (fVar9 + this->field_0x174 * this->field_0x18c < fVar9 - static_cast<float>(gVideoConfig.screenHeight) * 0.01f) {
					this->field_0x174 = this->field_0x174 + 3.0f;
				}
			}

			if (*static_cast<float*>((int)lVar2 + 0x5f8) < 0.0f) {
				astruct_22_0048fa40.FUN_002ef920(true);
				if (fVar10 + static_cast<float>(gVideoConfig.screenHeight) * 0.01f < fVar9 + this->field_0x174 * this->field_0x18c + textFormat.field_0xc) {
					this->field_0x174 = this->field_0x174 - 3.0f;
				}
			}
		})

		fVar7 = this->field_0x188;
		fVar8 = this->field_0x180 - fVar7 * (static_cast<float>(gVideoConfig.screenWidth) * 0.17f - static_cast<float>(gVideoConfig.screenWidth) * 0.05f) * 0.5f;
		fVar3 = this->field_0x17c;
		fVar4 = static_cast<float>(gVideoConfig.screenHeight);
		fVar6 = fVar4 * 0.02f * fVar7;
		fVar4 = fVar7 * (fVar4 * 0.37f - fVar4 * 0.04f) * 0.5f;
		fVar7 = (this->field_0x184 - fVar4) - fVar6;
		astruct_22_0048fa40.FUN_002ef890();
		if (fVar9 - static_cast<float>(gVideoConfig.screenHeight) * 0.01f <= fVar5) {
			astruct_22_0048fa40.field_0x1a0 = astruct_22_0048fa40.field_0x1a0 | 1;
		}
		else {
			astruct_22_0048fa40.field_0x1a0 = astruct_22_0048fa40.field_0x1a0 & 0xfffffffe;
		}
		if (fVar5 + this->field_0x18c * textFormat.field_0xc < fVar10 + static_cast<float>(gVideoConfig.screenHeight) * 0.01f) {
			astruct_22_0048fa40.field_0x1a0 = astruct_22_0048fa40.field_0x1a0 | 2;
		}
		else {
			astruct_22_0048fa40.field_0x1a0 = astruct_22_0048fa40.field_0x1a0 & 0xfffffffd;
		}

		astruct_22_0048fa40.FUN_002ef4e0(fVar8, (fVar3 + fVar4) - fVar6, fVar8, fVar7, 0);

		rect.x = 0.0f;
		rect.y = -this->field_0x174;
		fVar9 = static_cast<float>((gVideoConfig.screenWidth * 5) / 6 - gVideoConfig.screenWidth / 6);
		rect.width = (fVar9 - fVar9 * 0.16f) - fVar9 * 0.14f;
		fVar3 = static_cast<float>((gVideoConfig.screenHeight * 5) / 6 - gVideoConfig.screenHeight / 6);
		rect.height = (fVar3 - fVar3 * 0.16f) - fVar3 * 0.26f;
		textFormat.DisplayWindow(this->field_0x178 + this->field_0x188 * fVar9 * 0.16f, fVar5, &rect);
		edTextStyleSetCurrent(pNewFont);
		GuiDList_EndCurrent();
	}

	return;
}

bool CActorHelperSign::CloseEffect()
{
	int iVar2;
	int iVar3;
	float fVar4;
	float fVar5;

	fVar5 = (static_cast<float>((gVideoConfig.screenWidth * 5) / 6 - gVideoConfig.screenWidth / 6) * GetTimer()->cutsceneDeltaTime) / 0.5f;
	fVar4 = (static_cast<float>((gVideoConfig.screenHeight * 5) / 6 - gVideoConfig.screenHeight / 6) * GetTimer()->cutsceneDeltaTime) / 0.5f;
	iVar2 = gVideoConfig.screenWidth;
	if (gVideoConfig.screenWidth < 0) {
		iVar2 = gVideoConfig.screenWidth + 1;
	}

	if (this->field_0x178 <= static_cast<float>(iVar2 >> 1)) {
		this->field_0x178 = this->field_0x178 + fVar5;
	}

	iVar2 = gVideoConfig.screenHeight;
	if (gVideoConfig.screenHeight < 0) {
		iVar2 = gVideoConfig.screenHeight + 1;
	}

	if (this->field_0x17c <= static_cast<float>(iVar2 >> 1)) {
		this->field_0x17c = this->field_0x17c + fVar4;
	}

	iVar2 = gVideoConfig.screenWidth;
	if (gVideoConfig.screenWidth < 0) {
		iVar2 = gVideoConfig.screenWidth + 1;
	}

	if (static_cast<float>(iVar2 >> 1) <= this->field_0x180) {
		this->field_0x180 = this->field_0x180 - fVar5;
	}

	iVar2 = gVideoConfig.screenHeight;
	if (gVideoConfig.screenHeight < 0) {
		iVar2 = gVideoConfig.screenHeight + 1;
	}

	if (static_cast<float>(iVar2 >> 1) <= this->field_0x184) {
		this->field_0x184 = this->field_0x184 - fVar4;
	}

	iVar2 = gVideoConfig.screenWidth;
	if (gVideoConfig.screenWidth < 0) {
		iVar2 = gVideoConfig.screenWidth + 1;
	}
	if (static_cast<float>(iVar2 >> 1) < this->field_0x178) {
		iVar3 = gVideoConfig.screenHeight;
		if (gVideoConfig.screenHeight < 0) {
			iVar3 = gVideoConfig.screenHeight + 1;
		}

		if (((static_cast<float>(iVar3 >> 1) < this->field_0x17c) && (fVar4 = static_cast<float>(iVar2 >> 1), this->field_0x180 < fVar4)) && (this->field_0x184 < static_cast<float>(iVar3 >> 1))) {
			this->field_0x178 = fVar4;
			iVar2 = gVideoConfig.screenHeight;
			if (gVideoConfig.screenHeight < 0) {
				iVar2 = gVideoConfig.screenHeight + 1;
			}

			this->field_0x17c = static_cast<float>(iVar2 >> 1);
			iVar2 = gVideoConfig.screenWidth;
			if (gVideoConfig.screenWidth < 0) {
				iVar2 = gVideoConfig.screenWidth + 1;
			}

			this->field_0x180 = static_cast<float>(iVar2 >> 1);
			iVar2 = gVideoConfig.screenHeight;
			if (gVideoConfig.screenHeight < 0) {
				iVar2 = gVideoConfig.screenHeight + 1;
			}

			this->field_0x184 = static_cast<float>(iVar2 >> 1);
			this->field_0x188 = 0.0f;
			this->field_0x18c = 0.0f;
			return true;
		}
	}

	this->field_0x188 = (this->field_0x180 - this->field_0x178) / static_cast<float>((gVideoConfig.screenWidth * 5) / 6 - gVideoConfig.screenWidth / 6);
	this->field_0x18c = (this->field_0x184 - this->field_0x17c) / static_cast<float>((gVideoConfig.screenHeight * 5) / 6 - gVideoConfig.screenHeight / 6);

	if (this->field_0x188 < 0.0f) {
		this->field_0x188 = 0.0f;
	}
	if (this->field_0x18c < 0.0f) {
		this->field_0x18c = 0.0f;
	}

	return false;
}

bool CActorHelperSign::OpenEffect()
{
	bool bVar1;
	float fVar3;
	float fVar4;

	this->field_0x174 = 0.0f;
	fVar4 = (static_cast<float>((gVideoConfig.screenWidth * 5) / 6 - gVideoConfig.screenWidth / 6) * GetTimer()->cutsceneDeltaTime) / 0.5f;
	fVar3 = (static_cast<float>((gVideoConfig.screenHeight * 5) / 6 - gVideoConfig.screenHeight / 6) * GetTimer()->cutsceneDeltaTime) / 0.5f;

	if (static_cast<float>(gVideoConfig.screenWidth / 6) <= this->field_0x178) {
		this->field_0x178 = this->field_0x178 - fVar4;
	}

	if (static_cast<float>(gVideoConfig.screenHeight / 6) <= this->field_0x17c) {
		this->field_0x17c = this->field_0x17c - fVar3;
	}

	if (this->field_0x180 <= static_cast<float>((gVideoConfig.screenWidth * 5) / 6)) {
		this->field_0x180 = this->field_0x180 + fVar4;
	}

	if (this->field_0x184 <= static_cast<float>((gVideoConfig.screenHeight * 5) / 6)) {
		this->field_0x184 = this->field_0x184 + fVar3;
	}

	fVar3 = static_cast<float>(gVideoConfig.screenWidth / 6);
	if ((((fVar3 <= this->field_0x178) || (static_cast<float>(gVideoConfig.screenHeight / 6) <= this->field_0x17c)) || (this->field_0x180 <= static_cast<float>((gVideoConfig.screenWidth * 5) / 6))) ||
		(this->field_0x184 <= static_cast<float>((gVideoConfig.screenHeight * 5) / 6))) {
		this->field_0x188 = (this->field_0x180 - this->field_0x178) / static_cast<float>((gVideoConfig.screenWidth * 5) / 6 - gVideoConfig.screenWidth / 6);
		this->field_0x18c = (this->field_0x184 - this->field_0x17c) / static_cast<float>((gVideoConfig.screenHeight * 5) / 6 - gVideoConfig.screenHeight / 6);

		if (this->field_0x188 < 0.0f) {
			this->field_0x188 = 0.0f;
		}

		if (this->field_0x18c < 0.0f) {
			this->field_0x18c = 0.0f;
		}

		bVar1 = false;
	}
	else {
		this->field_0x178 = fVar3;
		bVar1 = true;
		this->field_0x17c = static_cast<float>(gVideoConfig.screenHeight / 6);
		this->field_0x180 = static_cast<float>((gVideoConfig.screenWidth * 5) / 6);
		this->field_0x184 = static_cast<float>((gVideoConfig.screenHeight * 5) / 6);
		this->field_0x188 = 1.0f;
		this->field_0x18c = 1.0f;
	}

	return bVar1;
}

void CBehaviourHelperSign::Create(ByteCode* pByteCode)
{
	return;
}

void CBehaviourHelperSign::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	this->pOwner = static_cast<CActorHelperSign*>(pOwner);

	return;
}

int CBehaviourHelperSign::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	return 0;
}

void CBehaviourHelperSignStand::Create(ByteCode* pByteCode)
{
	return;
}

void CBehaviourHelperSignStand::Init(CActor * pOwner)
{
	astruct_22_0048fa40.Init(&MenuBitmaps[9].materialInfo, &MenuBitmaps[10].materialInfo);

	astruct_22_0048fa40.field_0x0.rgba[0] = 0x78;
	astruct_22_0048fa40.field_0x0.rgba[1] = 0x78;
	astruct_22_0048fa40.field_0x0.rgba[2] = 0x78;
	astruct_22_0048fa40.field_0x0.rgba[3] = 0x78;
	astruct_22_0048fa40.field_0xc0.rgba[0] = 0xfa;
	astruct_22_0048fa40.field_0xc0.rgba[1] = 0xfa;
	astruct_22_0048fa40.field_0xc0.rgba[2] = 0xfa;
	astruct_22_0048fa40.field_0xc0.rgba[3] = 0x80;

	return;
}

void CBehaviourHelperSignStand::Term()
{
	astruct_22_0048fa40.Term();

	return;
}

void CBehaviourHelperSignStand::Manage()
{
	undefined8 uVar1;
	int iVar2;
	edF32VECTOR4 local_10;
	CActorHelperSign* pHelperSign;

	pHelperSign = this->pOwner;

	iVar2 = pHelperSign->actorState;
	if (iVar2 == HELPER_SIGN_STATE_CLOSE) {
		iVar2 = pHelperSign->CloseEffect();
		if (iVar2 != 0) {
			pHelperSign->SetState(HELPER_SIGN_STATE_STAND, -1);
		}
	}
	else {
		if ((iVar2 == HELPER_SIGN_STATE_OPEN) && (iVar2 = pHelperSign->OpenEffect(), iVar2 != 0)) {
			pHelperSign->SetState(HELPER_SIGN_STATE_SHOW, -1);
		}
	}

	if (pHelperSign->pTiedActor != (CActor*)0x0) {
		local_10 = pHelperSign->baseLocation;
		pHelperSign->SV_UpdatePosition_Rel(&local_10, 0, 0, (CActorsTable*)0x0, (edF32VECTOR4*)0x0);
	}

	return;
}

void CBehaviourHelperSignStand::Begin(CActor * pOwner, int newState, int newAnimationType)
{
	CActorHelperSign* pHelperSign;

	this->pOwner = static_cast<CActorHelperSign*>(pOwner);

	if (newState == -1) {
		pHelperSign = this->pOwner;
		pHelperSign->SetState(HELPER_SIGN_STATE_STAND, -1);
	}
	else {
		pHelperSign = this->pOwner;
		pHelperSign->SetState(newState, newAnimationType);
	}

	return;
}

void CBehaviourHelperSignStand::InitState(int newState)
{
	CActorHelperSign* pHelperSign;

	pHelperSign = this->pOwner;
	if (newState == HELPER_SIGN_STATE_STAND) {
		pHelperSign->UpdatePosition(&pHelperSign->currentLocation, true);
	}
	else {
		if (newState == HELPER_SIGN_STATE_OPEN) {
			pHelperSign->flags = pHelperSign->flags | 0x80;
			pHelperSign->flags = pHelperSign->flags & 0xffffffdf;
			pHelperSign->EvaluateDisplayState();
		}
	}

	return;
}

void CBehaviourHelperSignStand::TermState(int oldState, int newState)
{
	CActorHelperSign* pHelperSign;

	pHelperSign = this->pOwner;

	if (oldState == HELPER_SIGN_STATE_CLOSE) {
		pHelperSign->flags = pHelperSign->flags & 0xffffff5f;
		pHelperSign->EvaluateDisplayState();
	}

	return;
}

int CBehaviourHelperSignStand::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	int result;
	StateConfig* pSVar1;
	uint uVar2;
	char* pcVar3;
	int iVar4;
	_msg_enter_shop local_20;
	CActorHelperSign* pHelperSign;

	if (msg == 0x12) {
		pHelperSign = this->pOwner;
		if ((pHelperSign->field_0x160 == 0) || (iVar4 = pHelperSign->DetectTrigger(pSender), iVar4 == 0)) {
			result = 0;
		}
		else {
			result = 9;
		}
	}
	else {
		if (msg == 0x14) {
			if (pMsgParam == (void*)0x0) {
				pHelperSign = this->pOwner;

				if ((pHelperSign->GetStateFlags(pHelperSign->actorState) & 0x100) == 0) {
					this->pOwner->field_0x170 = pSender;
					local_20.field_0x0 = 0;
					local_20.field_0x4 = 1;
					local_20.field_0x8 = 1;
					local_20.field_0xc = 1;
					pHelperSign = this->pOwner;
					pHelperSign->DoMessage(pHelperSign->field_0x170, MESSAGE_ENTER_SHOP, &local_20);
					pcVar3 = gMessageManager.get_message((this->pOwner)->field_0x160);

					(this->pOwner)->field_0x16c = pcVar3;
					iVar4 = gVideoConfig.screenWidth;
					if (gVideoConfig.screenWidth < 0) {
						iVar4 = gVideoConfig.screenWidth + 1;
					}
					(this->pOwner)->field_0x178 = static_cast<float>(iVar4 >> 1);
					iVar4 = gVideoConfig.screenHeight;
					if (gVideoConfig.screenHeight < 0) {
						iVar4 = gVideoConfig.screenHeight + 1;
					}

					(this->pOwner)->field_0x17c = static_cast<float>(iVar4 >> 1);
					iVar4 = gVideoConfig.screenWidth;
					if (gVideoConfig.screenWidth < 0) {
						iVar4 = gVideoConfig.screenWidth + 1;
					}

					(this->pOwner)->field_0x180 = static_cast<float>(iVar4 >> 1);
					iVar4 = gVideoConfig.screenHeight;
					if (gVideoConfig.screenHeight < 0) {
						iVar4 = gVideoConfig.screenHeight + 1;
					}

					(this->pOwner)->field_0x184 = static_cast<float>(iVar4 >> 1);
					pHelperSign = this->pOwner;
					pHelperSign->SetState(HELPER_SIGN_STATE_OPEN, -1);

					result = 1;
				}
				else {
					result = 0;
				}
			}
			else {
				pHelperSign = this->pOwner;
				pHelperSign->SetState(HELPER_SIGN_STATE_CLOSE, -1);
				result = 1;
			}
		}
		else {
			result = 0;
		}
	}

	return result;
}

bool CActorHelperSign::DetectTrigger(CActor* pSender)
{
	int iVar1;
	float fVar2;
	float fVar3;
	float fVar4;
	float fVar5;
	float fVar6;
	float fVar7;
	edF32VECTOR4 local_40;
	edF32VECTOR4 local_30;
	edF32VECTOR4 local_20;
	edF32VECTOR4 eStack16;

	edF32Vector4SubHard(&eStack16, &pSender->currentLocation, &this->currentLocation);
	edF32Vector4SafeNormalize0Hard(&eStack16, &eStack16);
	fVar2 = (pSender->currentLocation).x - this->currentLocation.x;
	fVar3 = (pSender->currentLocation).y - this->currentLocation.y;
	fVar4 = (pSender->currentLocation).z - this->currentLocation.z;
	fVar7 = sqrtf(fVar2 * fVar2 + fVar3 * fVar3 + fVar4 * fVar4);
	fVar2 = edF32Vector4DotProductHard(&this->rotationQuat, &eStack16);
	local_20.x = this->rotationQuat.z;
	local_20.y = 0.0f;
	local_20.z = -this->rotationQuat.x;
	edF32Vector4ScaleHard(1.0f, &local_30, &local_20);
	edF32Vector4AddHard(&local_30, &this->currentLocation, &local_30);
	edF32Vector4SubHard(&local_30, &pSender->currentLocation, &local_30);
	fVar6 = (pSender->rotationQuat).x * local_30.z - local_30.x * (pSender->rotationQuat).z;
	edF32Vector4ScaleHard(-1.0f, &local_40, &local_20);
	edF32Vector4AddHard(&local_40, &this->currentLocation, &local_40);
	edF32Vector4SubHard(&local_40, &pSender->currentLocation, &local_40);
	fVar4 = (pSender->rotationQuat).x;
	fVar5 = (pSender->rotationQuat).z;

	fVar3 = edF32Vector4DotProductHard(&pSender->rotationQuat, &eStack16);
	if ((fVar7 < 2.0f) && (fVar3 < 0.0f)) {
		iVar1 = 1;
		if (fVar2 <= 0.0f) {
			iVar1 = -1;
		}

		if (fVar6 * static_cast<float>(iVar1) < 0.0f) {
			iVar1 = 1;
			if (fVar2 <= 0.0f) {
				iVar1 = -1;
			}

			if (0.0f < (fVar4 * local_40.z - local_40.x * fVar5) * static_cast<float>(iVar1)) {
				return true;
			}
		}
	}

	return false;
}