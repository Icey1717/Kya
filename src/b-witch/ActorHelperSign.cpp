#include "ActorHelperSign.h"
#include "MemoryStream.h"
#include "DlistManager.h"
#include "ActorHero.h"
#include "ActorMiniGame.h"
#include "ActorMiniGamesOrganizer.h"
#include "kya.h"
#include "Pause.h"
#include "MathOps.h"
#include "edText.h"
#include "TimeController.h"
#include "BootData.h"
#include "TranslatedTextData.h"
#include "InputManager.h"
#include "CompatibilityHandlingPS2.h"
#include "Frontend.h"
#include "FrontEndDisp.h"
#include "Audio.h"

static void DrawHighScoreWheel(int curIndex, int nextIndex, S_MENU_WHEEL_DRAW* pDraw, void** pContext);

// PS2: gCallbackWheelElt 0033c8b0.
static void DrawHighScoreWheel(int curIndex, int nextIndex, S_MENU_WHEEL_DRAW* pDraw, void** pContext)
{
	CActorHelperSign* pSign = static_cast<CActorHelperSign*>(*pContext);
	CBehaviourHelperSignHighScore* pBehaviour = static_cast<CBehaviourHelperSignHighScore*>(pSign->GetBehaviour(pSign->curBehaviourId));
	CActorMiniGamesOrganizer* pOrganizer = static_cast<CActorMiniGamesOrganizer*>(pBehaviour->field_0x8.Get());
	edCTextStyle style;
	style.Reset();

	uint alpha = pDraw->alpha < pOrganizer->field_0x94c ? pDraw->alpha : pOrganizer->field_0x94c;
	style.rgbaColour = alpha | 0xffff0000;
	style.alpha = pDraw->alpha;
	style.SetScale(1.5f, 1.5f);
	style.SetHorizontalAlignment(2);
	style.SetVerticalAlignment(8);
	style.SetShadow(0x100);
	style.SetFont(BootDataFont, false);

	edCTextStyle* pOldStyle = edTextStyleSetCurrent(&style);
	if (*pContext != 0) {
		edCTextFormat format;
		CActorMiniGame* pCur = pOrganizer->GetMiniGame(curIndex);
		CActorMiniGame* pNext = pOrganizer->GetMiniGame(nextIndex);
		if (pCur != 0) {
			pSign->DrawHighScoreText(curIndex, pDraw->alpha);
			format.FormatString(pCur->FUN_003ace10());
			format.Display(pDraw->x, pDraw->y);
		}

		if (pNext != 0) {
			pSign->DrawHighScoreText(nextIndex, 255 - pDraw->alpha);
			alpha = (byte)((255.0f - (float)pDraw->alpha) / 2.0f);
			style.rgbaColour = alpha | 0xffff0000;
			style.alpha = alpha | 0x80808000;
			format.FormatString(pNext->FUN_003ace10());
			format.Display(pDraw->x, pDraw->y);
		}
	}

	edTextStyleSetCurrent(pOldStyle);

	return;
}

// PS2: DrawHighScoreText 00339360, FUN_003aad80.
void CActorHelperSign::DrawHighScoreText(int index, uint alpha)
{
	CBehaviourHelperSignHighScore* pBehaviour = static_cast<CBehaviourHelperSignHighScore*>(GetBehaviour(this->curBehaviourId));
	CActorMiniGamesOrganizer* pOrganizer = static_cast<CActorMiniGamesOrganizer*>(pBehaviour->field_0x8.Get());
	CActorMiniGame* pGame = pOrganizer->GetMiniGame(index);
	edCTextStyle style;
	style.Reset();
	style.SetFont(BootDataFont, false);
	style.SetEolAutomatic(0x80);
	style.SetHorizontalAlignment(2);
	style.SetVerticalAlignment(8);
	style.rgbaColour = 0xffff00ff;
	style.SetShadow(0x100);
	alpha &= 0xff;
	style.alpha = alpha;
	edCTextStyle* pOldStyle = edTextStyleSetCurrent(&style);
	edCTextFormat format;
	float width = (float)gVideoConfig.screenWidth;
	float height = (float)gVideoConfig.screenHeight;
	style.SetScale(1.0f, 1.0f);
	style.SetRotation(0.2f);
	style.SetHorizontalSize(width * 0.25f);
	style.rgbaColour = alpha | 0xf0000000;
	format.FormatString(gMessageManager.get_message(0x5d59544553091216));
	format.Display(width * 0.16f, height * 0.37f);
	style.SetRotation(0.0f);
	style.rgbaColour = alpha | 0xffff0000;
	style.SetEolAutomatic(0);
	uint count = pGame->GetNbAvailableMode();
	float x = -0.67f;
	if (count == 3) {
		x = width * 0.16f;
	}
	else if (count == 2) {
		x = width * 0.16f + width * 0.325f / 2.0f;
	}
	else if (count == 1) {
		x = width * 0.325f + width * 0.16f;
	}
	style.SetHorizontalJustification(0x10);
	style.SetEolAutomatic(0x80);
	style.SetScale(0.87f, 0.87f);
	style.SetHorizontalSize(width * 0.33f);
	if (pGame->GetBehaviour(3) != 0) {
		style.SetRotation(-0.03f);
		format.FormatString(gMessageManager.get_message(0x1e160c0c13414d45));
		format.Display(x, height * 0.63f);
		style.SetRotation(0.0f);
		pGame->GetUnity();
		pGame->FormatScore(pGame->defaultScore.score, &format, pGame->defaultScore.name, 0, 1);
		format.Display(x, height * 0.73f);
		x += width * 0.325f;
	}
	if (pGame->GetBehaviour(4) != 0) {
		style.SetRotation(-0.13f);
		style.rgbaColour = alpha | 0xf0008000;
		format.FormatString(gMessageManager.get_message(0x52575a5959150415));
		format.Display(x, height * 0.4f);
		style.SetRotation(0.0f);
		style.rgbaColour = alpha | 0xffff0000;
		pGame->DrawHighScoreArray(x, height * 0.52f, width * 0.1f, 0.9f, &pGame->GetBhvMulti()->scoreList);
		x += width * 0.325f + 0.005f;
	}
	if (pGame->GetBehaviour(2) != 0) {
		style.SetRotation(0.13f);
		format.FormatString(gMessageManager.get_message(0x50511a1b0608030c));
		format.Display(x, height * 0.4f);
		style.SetRotation(0.0f);
		pGame->DrawHighScoreArray(x, height * 0.52f, width * 0.1f, 0.9f, &pGame->GetBhvTraining()->scoreList);
	}
	style.SetHorizontalJustification(0);
	style.SetEolAutomatic(0);
	style.SetVerticalAlignment(8);
	style.rgbaColour = alpha | 0xffffff00;
	style.altColour = style.rgbaColour;
	style.SetScale(0.8f, 0.8f);
	format.FormatString(gMessageManager.get_message(0x4b425f5e40151207));
	format.Display(width * 0.5f, height * 0.95f);
	edTextStyleSetCurrent(pOldStyle);

	return;
}

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
				static_cast<CActorMiniGamesOrganizer*>(pCVar4->field_0x8.Get())->FUN_003b4f30(0.0f, 0.0f, 104.0f, &pCVar4->field_0x50);
				FrontendDList_EndCurrent();
			}

			DrawDisconnectedController(0);

			return;
		}
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

CMenuArrows astruct_22_0048fa40;

void CActorHelperSign::DrawHelpText()
{
	bool bVar1;
	edCTextStyle* pNewFont;
	char* pMsg;
	CPlayerInput* pPlayerInput;
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

		if (((this->actorState == 6) && (this->field_0x170 != (CActor*)0x0)) && (pPlayerInput = this->field_0x170->GetInputManager(0, 0), pPlayerInput != (CPlayerInput*)0x0)) {
			if (0.0f < pPlayerInput->aAnalogSticks[0].y) {
				astruct_22_0048fa40.MoveMenuArrow(false);
				if (fVar9 + this->field_0x174 * this->field_0x18c < fVar9 - static_cast<float>(gVideoConfig.screenHeight) * 0.01f) {
					this->field_0x174 = this->field_0x174 + 3.0f;
				}
			}

			if (pPlayerInput->aAnalogSticks[0].y < 0.0f) {
				astruct_22_0048fa40.MoveMenuArrow(true);
				if (fVar10 + static_cast<float>(gVideoConfig.screenHeight) * 0.01f < fVar9 + this->field_0x174 * this->field_0x18c + textFormat.field_0xc) {
					this->field_0x174 = this->field_0x174 - 3.0f;
				}
			}
		}

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

void CBehaviourHelperSignHighScore::Create(ByteCode* pByteCode)
{
	this->field_0x8.index = pByteCode->GetS32();

	return;
}

// PS2: Init 0033b7c0; mesh setup is also used by InitState 0033b2e0.
void CBehaviourHelperSignHighScore::Init(CActor* pOwner)
{
	this->field_0x8.Init();
	this->field_0xc = 0;

	CActorMiniGamesOrganizer* pOrganizer = static_cast<CActorMiniGamesOrganizer*>(this->field_0x8.Get());
	CActor::SV_InstallMaterialId(pOrganizer->materialId_0x178);

	this->menuWheel.Init(0, &MenuBitmaps[9].materialInfo, &MenuBitmaps[10].materialInfo);
	this->menuWheel.field_0x278 = 1;
	this->menuWheel.field_0x274 = pOrganizer->pMiniGameStreamRefs == 0 ? 0 : pOrganizer->pMiniGameStreamRefs->entryCount;
	this->menuWheel.field_0x28c = 1020.0f;
	this->menuWheel.pFunc = DrawHighScoreWheel;
	this->menuWheel.field_0x298 = &this->field_0x590;
	this->menuWheel.field_0xc0.field_0x0.color = 0x8000005d;
	this->menuWheel.field_0xc0.field_0xc0.color = 0x8000626a;
	this->menuWheel.field_0xc0.field_0x0.iWidth = (ushort)(int)((float)gVideoConfig.screenWidth * 0.06f);
	this->menuWheel.field_0xc0.field_0x0.iHeight = (ushort)(int)((float)gVideoConfig.screenHeight * 0.06f);
	this->menuWheel.field_0xc0.field_0xc0.iWidth = this->menuWheel.field_0xc0.field_0x0.iWidth;
	this->menuWheel.field_0xc0.field_0xc0.iHeight = this->menuWheel.field_0xc0.field_0x0.iHeight;

	edF32VECTOR2 left, right;
	left.x = (float)gVideoConfig.screenWidth * 0.5f - (float)gVideoConfig.screenWidth * 0.58f / 2.0f;
	left.y = (float)gVideoConfig.screenHeight * 0.13f;
	right.x = (float)gVideoConfig.screenWidth * 0.5f + (float)gVideoConfig.screenWidth * 0.58f / 2.0f;
	right.y = left.y;
	this->menuWheel.FUN_002efa80((float)(int)((float)gVideoConfig.screenWidth * 0.5f), (float)(int)left.y, &right, &left);
	memset(&this->field_0x10, 0, sizeof(this->field_0x10));

	this->field_0x30.x = 0.0f;
	this->field_0x30.y = 0.0f;
	this->field_0x30.z = 0.0f;
	this->field_0x30.w = 100.0f;
	this->field_0x40 = 150.0f;
	this->field_0x10.pBoundingSphere = &this->field_0x30;
	this->field_0x10.clipping_0x0 = &this->field_0x40;

	StaticMeshComponent* meshes[] = {
		&this->field_0x50, &this->field_0xb0, &this->field_0x110, &this->field_0x170,
		&this->field_0x1d0, &this->field_0x230, &this->field_0x290
	};

	for (int i = 0; i < 7; i++) {
		StaticMeshComponent* pMesh = meshes[i];
		pMesh->textureIndex = pOrganizer->textureIndex_0x170;
		pMesh->meshIndex = pOrganizer->field_0x174;
		pMesh->Reset();
	}

	return;
}

void CBehaviourHelperSignHighScore::InitMeshes(bool all)
{
	StaticMeshComponent* meshes[] = {
		&this->field_0x50, &this->field_0xb0, &this->field_0x110, &this->field_0x170,
		&this->field_0x1d0, &this->field_0x230, &this->field_0x290
	};

	char* names[] = {
		(char*)"Gam_back_01", (char*)"Gam_03_pan_02", (char*)"Gam_03_pan_03", (char*)"Gam_06_pan_01",
		(char*)"Gam_06_pan_02", (char*)"Gam_06_pan_03", (char*)"Gam_Symbole_01"
	};

	for (int i = 0; i < 7; i++) {
		StaticMeshComponent* pMesh = meshes[i];
		if ((pMesh->textureIndex == -1) || (pMesh->meshIndex == -1)) {
			return;
		}
	}

	for (int i = 0; i < (all ? 7 : 1); i++) {
		meshes[i]->Init(CFrontend::_scene_handle, 0, &this->field_0x10, names[i]);
	}

	return;
}

// PS2: StateHelperSignHighScoreTerm 0033a5a0.
void CBehaviourHelperSignHighScore::TermMeshes()
{
	StaticMeshComponent* meshes[] = {
		&this->field_0x50, &this->field_0xb0, &this->field_0x110, &this->field_0x170,
		&this->field_0x1d0, &this->field_0x230, &this->field_0x290
	};

	for (int i = 0; i < 7; i++) {
		StaticMeshComponent* pMesh = meshes[i];
		if (pMesh->HasMesh()) {
			pMesh->Term(CFrontend::_scene_handle);
		}
	}

	return;
}

void CBehaviourHelperSignHighScore::Term()
{
	return;
}

// PS2: Manage 0033bbc0, ManageFrozen 0033bbb0, shared routine 0033ae90.
void CBehaviourHelperSignHighScore::Manage()
{
	CActorHelperSign* pSign = this->pOwner;
	if (!gCompatibilityHandlingPtr->HandleDisconnectedDevices(0) || ((GameFlags & 0x1c) != 0)) {
		if (pSign->actorState == 9) {
			pSign->SetState(pSign->prevActorState, -1);
		}
	}
	else if (pSign->actorState == HELPER_SIGN_STATE_SHOW) {
		pSign->SetState(9, -1);
	}

	if (pSign->actorState == HELPER_SIGN_STATE_CLOSE) {
		this->menuWheel.MoveWheel();
		pSign->SetState(HELPER_SIGN_STATE_STAND, -1);
	}
	else if (pSign->actorState == HELPER_SIGN_STATE_OPEN) {
		pSign->GetBehaviour(pSign->curBehaviourId);
		pSign->SetState(HELPER_SIGN_STATE_SHOW, -1);
	}
	else if (pSign->actorState == HELPER_SIGN_STATE_SHOW) {
		ManageHighScore();
	}

	if (pSign->pTiedActor != 0) {
		edF32VECTOR4 location = pSign->baseLocation;
		pSign->SV_UpdatePosition_Rel(&location, 0, 0, 0, 0);
	}

	return;
}

void CBehaviourHelperSignHighScore::ManageFrozen()
{
	CBehaviourHelperSignHighScore::Manage();

	return;
}

// PS2: InitState 0033b2e0, TermState 0033b1f0.
void CBehaviourHelperSignHighScore::InitState(int newState)
{
	CActorHelperSign* pSign = this->pOwner;
	if ((newState == HELPER_SIGN_STATE_STAND) || (newState == HELPER_SIGN_STATE_CLOSE) || (newState == HELPER_SIGN_STATE_OPEN)) {
		pSign->flags = (pSign->flags | 0x80) & 0xffffffdf;
		pSign->EvaluateDisplayState();
	}
	else if (newState == HELPER_SIGN_STATE_SHOW) {
		CBehaviourHelperSignHighScore* pBehaviour = static_cast<CBehaviourHelperSignHighScore*>(pSign->GetBehaviour(pSign->curBehaviourId));
		pBehaviour->InitMeshes(true);
		pBehaviour = static_cast<CBehaviourHelperSignHighScore*>(pSign->GetBehaviour(pSign->curBehaviourId));
		pBehaviour->field_0x590 = pSign;
		_msg_enter_shop message = { 0, 1, 1, 1 };
		pSign->DoMessage(pSign->field_0x170, MESSAGE_ENTER_SHOP, &message);
		CScene::ptable.g_FrontendManager_00451680->SetActive(false);
		GameFlags |= 0x80;
		CallPauseChange(1);
	}
	else if (newState == 9) {
		CScene::ptable.g_FrontendManager_00451680->SetActive(false);
		GameFlags |= 0x80;
		CallPauseChange(1);
		CBehaviourHelperSignHighScore* pBehaviour = static_cast<CBehaviourHelperSignHighScore*>(pSign->GetBehaviour(pSign->curBehaviourId));
		pBehaviour->InitMeshes(false);
	}

	return;
}

void CBehaviourHelperSignHighScore::TermState(int oldState, int newState)
{
	CActorHelperSign* pSign = this->pOwner;
	CBehaviourHelperSignHighScore* pBehaviour;

	if (oldState == 9) {
		CScene::ptable.g_FrontendManager_00451680->SetActive(true);
		GameFlags &= ~0x80;
		pBehaviour = static_cast<CBehaviourHelperSignHighScore*>(pSign->GetBehaviour(pSign->curBehaviourId));
		pBehaviour->TermMeshes();
		CallPauseChange(0);
	}
	else if (oldState == HELPER_SIGN_STATE_SHOW) {
		pBehaviour = static_cast<CBehaviourHelperSignHighScore*>(pSign->GetBehaviour(pSign->curBehaviourId));
		pBehaviour->TermMeshes();
		pBehaviour = static_cast<CBehaviourHelperSignHighScore*>(pSign->GetBehaviour(pSign->curBehaviourId));
		pBehaviour->menuWheel.MoveWheel();
		CScene::ptable.g_FrontendManager_00451680->SetActive(true);
		CallPauseChange(0);
		GameFlags &= ~0x80;
		pSign->DoMessage(pSign->field_0x170, (ACTOR_MESSAGE)0x24, 0);
	}

	return;
}

// PS2: FUN_003b7b90.
static void PlayHighScoreSound(CActorMiniGamesOrganizer* pOrganizer, uint soundId)
{
	CSoundInstance* pInstance = &pOrganizer->field_0xa00;
	pInstance->pSound = CScene::ptable.g_AudioManager_00451698->GetSound(soundId);
	if ((NoAudio == 0) && (pInstance->pSound3dData = 0, pInstance->pSound != 0)) {
		pInstance->field_0x20 = 0xffffffff;
		pInstance->soundId = pInstance->pSound->Play(pInstance->soundId, pInstance->field_0x20, 0, pInstance, 0, &pInstance->soundId);
	}

	return;
}

// PS2: StateHelperSignHighScore 0033a6e0.
void CBehaviourHelperSignHighScore::ManageHighScore()
{
	CActorHelperSign* pSign = this->pOwner;
	CPlayerInput* pInput = pSign->field_0x170->GetInputManager(0, 0);
	CBehaviourHelperSignHighScore* pBehaviour = static_cast<CBehaviourHelperSignHighScore*>(pSign->GetBehaviour(pSign->curBehaviourId));
	CBehaviourHelperSignHighScore* pCurrentBehaviour;
	CActorMiniGamesOrganizer* pOrganizer = static_cast<CActorMiniGamesOrganizer*>(pBehaviour->field_0x8.Get());
	pBehaviour->menuWheel.Manage();
	if (pInput == 0) {
		return;
	}

	if (((pInput->pressedBitfield & 0x800000) != 0) || ((pInput->pressedBitfield & 2) != 0)) {
		PlayHighScoreSound(pOrganizer, pOrganizer->field_0x190);
		pCurrentBehaviour = static_cast<CBehaviourHelperSignHighScore*>(pSign->GetBehaviour(pSign->curBehaviourId));
		pCurrentBehaviour->TermMeshes();
		pCurrentBehaviour = static_cast<CBehaviourHelperSignHighScore*>(pSign->GetBehaviour(pSign->curBehaviourId));
		pCurrentBehaviour->menuWheel.MoveWheel();
		CScene::ptable.g_FrontendManager_00451680->SetActive(true);
		CallPauseChange(0);
		GameFlags &= ~0x80;
		pSign->DoMessage(pSign->field_0x170, (ACTOR_MESSAGE)0x24, 0);
		pCurrentBehaviour = static_cast<CBehaviourHelperSignHighScore*>(pSign->GetBehaviour(pSign->curBehaviourId));
		pCurrentBehaviour->InitMeshes(true);
		pCurrentBehaviour = static_cast<CBehaviourHelperSignHighScore*>(pSign->GetBehaviour(pSign->curBehaviourId));
		pCurrentBehaviour->field_0x590 = pSign;
		_msg_enter_shop message = { 0, 1, 1, 1 };
		pSign->DoMessage(pSign->field_0x170, MESSAGE_ENTER_SHOP, &message);
		CScene::ptable.g_FrontendManager_00451680->SetActive(false);
		GameFlags |= 0x80;
		CallPauseChange(1);
		pBehaviour->field_0x5a0 = 0.1f;
		pBehaviour->menuWheel.MoveWheel(true);
		int count = pOrganizer->pMiniGameStreamRefs == 0 ? 0 : pOrganizer->pMiniGameStreamRefs->entryCount;
		assert(count != 0);
		pBehaviour->field_0xc = (pBehaviour->field_0xc + 1) % count;
	}

	if (((pInput->pressedBitfield & 0x400000) != 0) || ((pInput->pressedBitfield & 1) != 0)) {
		PlayHighScoreSound(pOrganizer, pOrganizer->field_0x190);
		pCurrentBehaviour = static_cast<CBehaviourHelperSignHighScore*>(pSign->GetBehaviour(pSign->curBehaviourId));
		pCurrentBehaviour->TermMeshes();
		pCurrentBehaviour = static_cast<CBehaviourHelperSignHighScore*>(pSign->GetBehaviour(pSign->curBehaviourId));
		pCurrentBehaviour->menuWheel.MoveWheel();
		CScene::ptable.g_FrontendManager_00451680->SetActive(true);
		CallPauseChange(0);
		GameFlags &= ~0x80;
		pSign->DoMessage(pSign->field_0x170, (ACTOR_MESSAGE)0x24, 0);
		pCurrentBehaviour = static_cast<CBehaviourHelperSignHighScore*>(pSign->GetBehaviour(pSign->curBehaviourId));
		pCurrentBehaviour->InitMeshes(true);
		pCurrentBehaviour = static_cast<CBehaviourHelperSignHighScore*>(pSign->GetBehaviour(pSign->curBehaviourId));
		pCurrentBehaviour->field_0x590 = pSign;
		_msg_enter_shop message = { 0, 1, 1, 1 };
		pSign->DoMessage(pSign->field_0x170, MESSAGE_ENTER_SHOP, &message);
		CScene::ptable.g_FrontendManager_00451680->SetActive(false);
		GameFlags |= 0x80;
		CallPauseChange(1);
		pBehaviour->field_0x59c = 0.1f;
		pBehaviour->menuWheel.MoveWheel(false);
		int count = pOrganizer->pMiniGameStreamRefs == 0 ? 0 : pOrganizer->pMiniGameStreamRefs->entryCount;
		assert(count != 0);
		pBehaviour->field_0xc = (pBehaviour->field_0xc - 1 + count) % count;
	}

	if ((pInput->pressedBitfield & 0x4000000) != 0) {
		PlayHighScoreSound(pOrganizer, pOrganizer->field_0x18c);
		pSign->SetState(HELPER_SIGN_STATE_CLOSE, -1);
	}

	return;
}

void CBehaviourHelperSignHighScore::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	this->pOwner = static_cast<CActorHelperSign*>(pOwner);
	this->field_0x50.Reset();
	this->field_0xb0.Reset();
	this->field_0x110.Reset();
	this->field_0x170.Reset();
	this->field_0x1d0.Reset();
	this->field_0x230.Reset();
	this->field_0x290.Reset();

	if (newState == -1) {
		this->pOwner->SetState(HELPER_SIGN_STATE_STAND, -1);
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	return;
}

int CBehaviourHelperSignHighScore::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	if (msg == 0x12) {
		return this->pOwner->DetectTrigger(pSender) ? 9 : 0;
	}

	if ((msg == 0x14) && ((this->pOwner->GetStateFlags(this->pOwner->actorState) & 0x100) == 0)) {
		this->pOwner->field_0x170 = pSender;
		this->pOwner->SetState(HELPER_SIGN_STATE_OPEN, -1);
		return 1;
	}

	return 0;
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

void CActorHelperSign::FUN_00339ab0()
{
	CActorMiniGamesOrganizer* pMiniGamesOrganizer;
	CActorMiniGame* pMiniGame;
	CBehaviourHelperSignHighScore* pBehaviourHelperSignHighScore;
	uint uVar1;
	CBehaviour* pCVar2;
	float fVar3;

	pBehaviourHelperSignHighScore = (CBehaviourHelperSignHighScore*)CActor::GetBehaviour(this->curBehaviourId);
	pMiniGamesOrganizer = (CActorMiniGamesOrganizer*)(pBehaviourHelperSignHighScore->field_0x8).Get();
	pMiniGame = (CActorMiniGame*)pMiniGamesOrganizer->pMiniGameStreamRefs->aEntries[pBehaviourHelperSignHighScore->field_0xc].Get();
	uVar1 = pMiniGame->GetNbAvailableMode();
	fVar3 = -0.67f;
	if (uVar1 != 3) {
		if (uVar1 == 2) {
			fVar3 = -0.3375f;
		}
		else {
			fVar3 = -0.67f;
			if (uVar1 == 1) {
				fVar3 = -0.004999995f;
			}
		}
	}
	pMiniGamesOrganizer->FUN_003b4f30(0.0f, 0.0f, 104.0f, &pBehaviourHelperSignHighScore->field_0x50);
	pMiniGamesOrganizer->FUN_003b4f30(0.0f, 0.745f, 103.0f, &pBehaviourHelperSignHighScore->field_0xb0);
	pMiniGamesOrganizer->FUN_003b4f30(-0.58f, 0.2f, 103.0f, &pBehaviourHelperSignHighScore->field_0x110);

	pCVar2 = pMiniGame->GetBehaviour(MINI_GAME_BEHAVIOUR_BETTING);
	if (pCVar2 != (CBehaviour*)0x0) {
		pMiniGamesOrganizer->FUN_003b4f30(fVar3, -0.19f, 103.0f, &pBehaviourHelperSignHighScore->field_0x170);
		fVar3 = fVar3 + 0.665f;
	}

	pCVar2 = pMiniGame->GetBehaviour(MINI_GAME_BEHAVIOUR_MULTI);
	if (pCVar2 != (CBehaviour*)0x0) {
		pMiniGamesOrganizer->FUN_003b4f30(fVar3, -0.19f, 103.0f, &pBehaviourHelperSignHighScore->field_0x1d0);
		fVar3 = fVar3 + 0.665f;
	}

	pCVar2 = pMiniGame->GetBehaviour(MINI_GAME_BEHAVIOUR_TRAINING);
	if (pCVar2 != (CBehaviour*)0x0) {
		pMiniGamesOrganizer->FUN_003b4f30(fVar3, -0.19f, 103.0f, &pBehaviourHelperSignHighScore->field_0x230);
	}

	pMiniGamesOrganizer->FUN_003b4f30(0.0f, 0.0f, 99.0f, &pBehaviourHelperSignHighScore->field_0x290);

	return;
}
