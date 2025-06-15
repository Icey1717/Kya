#include "FrontEndEnemy.h"
#include "Actor.h"
#include "Pause.h"
#include "BootData.h"
#include "edStr.h"
#include "Frontend.h"
#include "kya.h"
#include "Rendering/DisplayList.h"
#include "edText.h"
#include "ActorAutonomous.h"
#include "FrontEndBank.h"
#include "LevelScheduleManager.h"

int gEnemySpritesInstalled = 0;

// Bar
char* G2D_RES_WOLF_BACK = "wolf_gauge";
char* G2D_RES_WOLF_FILL_LIFE = "wolf_fill_life";
char* G2D_RES_WOLF_FILL_HIT = "wolf_fill_hit";

// Heads
char* G2D_RES_HEAD_SCRAWNY = "head_scrawny";
char* G2D_RES_HEAD_GRUNT = "head_grunt";
char* G2D_RES_HEAD_KRONOS = "head_kronos";
char* G2D_RES_HEAD_ATON = "head_aton";
char* G2D_RES_HEAD_FRANCK = "head_franck";
char* G2D_RES_HEAD_BRAZUL = "head_brazul";
char* G2D_RES_HEAD_HUNTER = "head_hunter";

CSprite gEnemyHeadSprites[7];

edF32VECTOR2 ENEMY_POS_DRAW_ON = { 0.8828f, 0.874f };
edF32VECTOR2 ENEMY_POS_DRAW_OFF = { 1.125f, 0.874f };
edF32VECTOR2 ENEMY_COUNT_SCALE = { 0.125f, 0.125f };

CFrontendEnemyCount::CFrontendEnemyCount()
{
	this->popupTime = 0.2f;
	this->field_0x78 = 2.0f;

	this->slotOn.position = ENEMY_POS_DRAW_ON;
	this->slotOn.scale = ENEMY_COUNT_SCALE;

	this->slotOff.position = ENEMY_POS_DRAW_OFF;
	this->slotOff.scale = ENEMY_COUNT_SCALE;
}

void CFrontendEnemyCount::SetInterface(CInterface* pNewInterface)
{
	this->pInterface = pNewInterface;

	if (GetInterface() != (CInterface*)0x0) {
		if (GetInterface()->Manage() != 0) {
			this->field_0x6c = GetInterface()->GetValue();
		}
	}

	return;
}

void CFrontendEnemyCount::Reset()
{
	CInterface* pCVar1;
	long lVar2;

	CWidget::Reset();

	this->slotAlpha = 0.0f;
	this->bVisible = 1;
	this->field_0x70 = 1;
	this->bDisplayDirty = 1;

	pCVar1 = this->pInterface;
	if ((pCVar1 == (CInterface*)0x0) || (lVar2 = pCVar1->Manage(), lVar2 == 0)) {
		this->field_0x6c = 0.0f;
	}
	else {
		this->field_0x6c = this->pInterface->GetValue();
	}

	this->widgetSlotC = this->slotOff;
	this->widgetSlotB = this->slotOff;
	this->widgetSlotA = this->slotOff;

	this->field_0x74 = 3;

	return;
}

void CFrontendEnemyCount::Init()
{
	CWidget::Init();

	this->sprite.Install(&MenuBitmaps[1].materialInfo);

	this->textStyle.SetFont(BootDataFont, false);
	this->textStyle.rgbaColour = -1;
	this->textStyle.SetShadow(0x100);
	this->textStyle.SetShadowShift(2.0f, 2.0f);
	this->textStyle.SetHorizontalAlignment(2);

	Reset();

	return;
}

void CFrontendEnemyCount::Term()
{
	SetInterface((CInterface*)0x0);
	this->sprite.Remove();

	return;
}

void CFrontendEnemyCount::UpdatePos_StateWait(float time)
{
	int iVar1;

	this->slotAlpha = 1.0;
	iVar1 = this->field_0x74;
	if (iVar1 == 2) {
		this->field_0x74 = 3;
		this->bVisible = 0;
	}
	else {
		if (iVar1 != 5) {
			if (iVar1 == 4) {
				if (this->field_0x78 < time - this->prevTime) {
					this->field_0x74 = 2;
					MoveToNext(&this->slotOff);
				}
			}
			else {
				if (iVar1 == 1) {
					this->field_0x74 = 5;
				}
				else {
					if (iVar1 == 0) {
						this->field_0x74 = 4;
					}
				}
			}
		}
	}

	return;
}

bool CFrontendEnemyCount::UpdateDisp(float time)
{
	bool bSuccess;
	char local_10[16];

	bSuccess = false;
	if ((this->bDisplayDirty != 0) && (this->pInterface != (CInterface*)0x0)) {
		local_10[0] = '\0';
		this->bDisplayDirty = 0;
		edStrInt2Str((int)this->field_0x6c, local_10, 5, false);
		edStrCatMulti(this->szLabel, "x", local_10, NULL);
		bSuccess = true;
	}

	return bSuccess;
}

void CFrontendEnemyCount::Update(float time)
{
	float currentValue;

	if ((this->pInterface != (CInterface*)0x0) && (this->pInterface->Manage() != false) && (currentValue = pInterface->GetValue(), currentValue != this->field_0x6c)) {
		int iVar2 = this->field_0x74;
		if (iVar2 == 2) {
			MoveToNext(&this->slotOn);
			this->field_0x74 = 0;
		}
		else {
			if (iVar2 == 3) {
				MoveToNext(&this->slotOn);
				this->field_0x74 = 0;
				this->bVisible = 1;
			}
			else {
				if (iVar2 == 4) {
					this->prevTime = CFrontend::GetTime();
				}
			}
		}

		this->field_0x6c = currentValue;
		this->bDisplayDirty = 1;
	}

	CWidget::Update(time);

	return;
}

float FLOAT_00448da8 = 18.0f;

void CFrontendEnemyCount::Draw()
{
	bool bVar1;
	edCTextStyle* pNewFont;
	float fVar2;
	float y;

	if (((this->field_0x74 != 0) && (this->bVisible != 0)) && (this->field_0x78 != 3)) {
		bVar1 = this->widgetSlotA.position.x == (this->sprite).field_0x5c.position.x;

		if ((bVar1) && (bVar1 = true, this->widgetSlotA.position.y != (this->sprite).field_0x5c.position.y)) {
			bVar1 = false;
		}

		if ((bVar1) && (bVar1 = true, this->widgetSlotA.scale.x != (this->sprite).field_0x5c.scale.x)) {
			bVar1 = false;
		}

		if ((bVar1) && (bVar1 = true, this->widgetSlotA.scale.y != (this->sprite).field_0x5c.scale.y)) {
			bVar1 = false;
		}

		if (!bVar1) {
			(this->sprite).bValid = false;
			(this->sprite).field_0x5c = this->widgetSlotA;
		}

		bVar1 = GuiDList_BeginCurrent();
		if (bVar1 != false) {
			this->sprite.Draw(1);
			fVar2 = this->widgetSlotA.position.x * (float)gVideoConfig.screenWidth;
			y = FLOAT_00448da8 + this->widgetSlotA.position.y * (float)gVideoConfig.screenHeight;
			pNewFont = edTextStyleSetCurrent(&this->textStyle);
			edTextDraw(fVar2, y, this->szLabel);
			edTextStyleSetCurrent(pNewFont);
			GuiDList_EndCurrent();
		}
	}

	return;
}

void CFrontendEnemyCount::ShowEnemy()
{
	switch (this->field_0x74) {
	case 2:
		CWidget::MoveToNext(&this->slotOn);
		this->field_0x74 = 0;
		break;
	case 3:
		CWidget::MoveToNext(&this->slotOn);
		this->field_0x74 = 0;
		this->bVisible = 1;
		break;
	case 4:
		this->prevTime = CFrontend::GetTime();
	}

	return;
}

edF32VECTOR2 ENEMY_POS[7] = {
	{ 0.8623f, 0.125f },
	{ 0.83f, 0.16f },
	{ 0.835f, 0.19f },
	{ 0.855f, 0.22f },
	{ 0.89f, 0.25f },
	{ 0.75f, -0.1f },
	{ 0.875f, -0.1f }
};

edF32VECTOR2 ENEMY_SCALE[2] = {
	{ 1.0f, 1.0f },
	{ 0.6666f, 0.6666f }
};

CFrontendEnemyList::CFrontendEnemyList()
{
	this->enemyInterfaceTable.nbEntries = 0;

	this->aSlots[0].position = ENEMY_POS[0];
	this->aSlots[0].scale = ENEMY_SCALE[0];

	this->aSlots[1].position = ENEMY_POS[1];
	this->aSlots[1].scale = ENEMY_SCALE[1];

	this->aSlots[2].position = ENEMY_POS[2];
	this->aSlots[2].scale = ENEMY_SCALE[1];

	this->aSlots[3].position = ENEMY_POS[3];
	this->aSlots[3].scale = ENEMY_SCALE[1];

	this->aSlots[4].position = ENEMY_POS[4];
	this->aSlots[4].scale = ENEMY_SCALE[1];

	this->aSlots[5].position = ENEMY_POS[5];
	this->aSlots[5].scale = ENEMY_SCALE[0];

	this->aSlots[6].position = ENEMY_POS[6];
	this->aSlots[6].scale = ENEMY_SCALE[1];

	this->aWidgets[0] = (CFrontendEnemy*)0x0;
	this->aWidgets[1] =	(CFrontendEnemy*)0x0;
	this->aWidgets[2] = (CFrontendEnemy*)0x0;
	this->aWidgets[3] = (CFrontendEnemy*)0x0;
	this->aWidgets[4] = (CFrontendEnemy*)0x0;

	return;
}

void CFrontendEnemyList::Init()
{
	CFrontendEnemy* pEnemy;
	int iVar2;

	iVar2 = 0;
	pEnemy = this->aEnemies;

	do {
		pEnemy->Init();
		pEnemy = pEnemy + 1;
		iVar2 = iVar2 + 1;
	} while (iVar2 < 5);

	ResetSlots();
}

void CFrontendEnemyList::Term()
{
	CFrontendEnemy* pEnemy;
	CFrontendEnemy** ppWidget;
	int iVar2;

	iVar2 = 0;
	pEnemy = this->aEnemies;
	ppWidget = this->aWidgets;
	do {
		pEnemy->Term();
		*ppWidget = 0;
		iVar2 = iVar2 + 1;
		pEnemy = pEnemy + 1;
		ppWidget = ppWidget + 1;
	} while (iVar2 < 5);

	return;
}

void CFrontendEnemyList::Reset()
{
	CFrontendEnemy* pEnemy;
	int iVar2;

	iVar2 = 0;
	pEnemy = this->aEnemies;

	do {
		pEnemy->Reset();
		pEnemy = pEnemy + 1;
		iVar2 = iVar2 + 1;
	} while (iVar2 < 5);

	ResetSlots();

	return;
}

void CFrontendEnemyList::Level_Term()
{
	IMPLEMENTATION_GUARD();
}

void CFrontendEnemyList::AddFighterInterface(CInterface* pInterface)
{
	bool bVar1;
	CInterface* pCVar2;
	CFrontendEnemy* pCVar3;
	int iVar4;

	iVar4 = 0;
	pCVar3 = this->aEnemies;
	do {
		pCVar2 = pCVar3->GetInterface();
		if (pCVar2 == pInterface) goto LAB_001d82f0;
		iVar4 = iVar4 + 1;
		pCVar3 = pCVar3 + 1;
	} while (iVar4 < 5);

	iVar4 = -1;
LAB_001d82f0:
	if (iVar4 == -1) {
		bVar1 = this->enemyInterfaceTable.Contains(pInterface);
		if (bVar1 == false) {
			this->enemyInterfaceTable.Add(pInterface);
		}
	}

	return;
}

void CFrontendEnemyList::Update(float time)
{
	int* piVar1;
	undefined4 uVar2;
	CFrontendEnemy* pCVar3;
	bool bVar4;
	edF32VECTOR2* peVar5;
	bool bVar6;
	CLifeInterface* pCVar7;
	int iVar8;
	CInterface* pCVar9;
	CInterface* pCVar10;
	int iVar11;
	int iVar12;
	int iVar13;
	long lVar14;
	CFrontendEnemy** pEnemyWidgetIt;
	CFrontendEnemy* pEnemyIt;
	float fVar17;
	float fVar18;
	float fVar19;
	float fVar20;
	CFrontendEnemy** ppWidget;

	bVar4 = false;
	iVar12 = 0;
	pEnemyWidgetIt = this->aWidgets;
	pEnemyIt = this->aEnemies;
	do {
		pCVar7 = (CLifeInterface*)pEnemyIt->GetInterface();
		if ((pEnemyIt->state_0x37c == 2) && (pCVar7 != (CLifeInterface*)0x0)) {
			iVar8 = pCVar7->GetPriority();
			if ((iVar8 != 0) &&
				((bVar6 = this->enemyInterfaceTable.Contains(pCVar7), bVar6 == false && ((this->enemyInterfaceTable).nbEntries < 0x11)))) {
				this->enemyInterfaceTable.Add(pCVar7);
			}
			pEnemyIt->SetInterface((CInterface*)0x0);
		}

		if ((*pEnemyWidgetIt != 0x0) && ((*pEnemyWidgetIt)->GetInterface() != (CInterface*)0x0)) {
			pCVar7 = (CLifeInterface*)(*pEnemyWidgetIt)->GetInterface();
			iVar8 = pCVar7->GetPriority();
			if (iVar8 == 0) {
				ppWidget = this->aWidgets + iVar12;
				pCVar9 = (*ppWidget)->GetInterface();
				iVar13 = 0;
				pEnemyIt = this->aEnemies;
				goto LAB_001d8470;
			}
		}

		iVar12 = iVar12 + 1;
		pEnemyIt = pEnemyIt + 1;
		pEnemyWidgetIt = pEnemyWidgetIt + 1;
	} while (iVar12 < 5);

	goto LAB_001d8578;

	while (true) {
		iVar13 = iVar13 + 1;
		pEnemyIt = pEnemyIt + 1;
		if (4 < iVar13) break;
	LAB_001d8470:
		pCVar10 = pEnemyIt->GetInterface();

		if (pCVar10 == pCVar9) goto LAB_001d84a8;
	}
	iVar13 = -1;

LAB_001d84a8:
	iVar11 = 5;
	if (iVar12 != 0) {
		iVar11 = 6;
	}

	CFrontendEnemy* pEnemy = this->aEnemies + iVar13;
	iVar12 = pEnemy->state_0x37c;
	if (iVar12 == 3) {
		pEnemy->MoveToNext(this->aSlots + iVar11);
		pEnemy->state_0x37c = 1;
		pEnemy->field_0x384 = pEnemy->field_0x380;
		pEnemy->field_0x380 = 0;
	}
	else {
		if (((iVar12 != 2) && (iVar12 != 1)) && (iVar12 == 0)) {
			pEnemy->MoveToNext(this->aSlots + iVar11);
			pEnemy->state_0x37c = 1;
			pEnemy->field_0x384 = pEnemy->field_0x380;
			pEnemy->field_0x380 = 0;
		}
	}

	*ppWidget = (CFrontendEnemy*)0x0;
	bVar4 = true;

LAB_001d8578:
	iVar12 = 5;
	if (!bVar4) {
		iVar13 = 0;
		iVar8 = 5;
		pEnemyWidgetIt = this->aWidgets;
		do {
			CFrontendEnemy* pWidget = *pEnemyWidgetIt;
			if ((pWidget == (CFrontendEnemy*)0x0) && (0 < iVar12)) {
			LAB_001d85d8:
				iVar8 = iVar13;
				if (pWidget == (CFrontendEnemy*)0x0) {
					iVar12 = 0;
				}
				else {
					pCVar7 = (CLifeInterface*)pWidget->GetInterface();
					iVar12 = pCVar7->GetPriority();
				}
			}
			else {
				if (pWidget != (CFrontendEnemy*)0x0) {
					pCVar7 = (CLifeInterface*)pWidget->GetInterface();
					iVar12 = pCVar7->GetPriority();

					if (iVar11 < iVar12) goto LAB_001d85d8;
				}
			}

			if ((pWidget != (CFrontendEnemy*)0x0) && (iVar8 < iVar13)) {
				pCVar7 = (CLifeInterface*)pWidget->GetInterface();
				iVar12 = pCVar7->GetPriority();
				if ((iVar12 < iVar11) &&
					(((this->aWidgets[iVar8] == 0 || (this->aWidgets[iVar8]->state_0x37c == 3)) && (this->aWidgets[iVar8]->state_0x37c == 3)))) {
					// Swap.
					CFrontendEnemy** pWidgeA = this->aWidgets + iVar13;
					CFrontendEnemy** pWidgeB = this->aWidgets + iVar8;
					*pWidgeB = *pWidgeA;
					*pWidgeA = *pWidgeB;
					(*pWidgeB)->UpdateSpecial(this->aSlots + iVar8, iVar8 == 0);
					if ((*pWidgeA) != (CFrontendEnemy*)0x0) {
						(*pWidgeA)->UpdateSpecial(this->aSlots + iVar13, iVar13 == 0);
					}

					bVar4 = true;
					break;
				}
			}

			iVar13 = iVar13 + 1;
			pEnemyWidgetIt = pEnemyWidgetIt + 1;
		} while (iVar13 < 5);
	}

	if ((!bVar4) && (iVar12 = 0, pEnemyIt = this->aEnemies, (this->enemyInterfaceTable).nbEntries != 0)) {
		do {
			pCVar9 = pEnemyIt->GetInterface();

			if (pCVar9 == (CInterface*)0x0) goto LAB_001d8750;

			iVar12 = iVar12 + 1;
			pEnemyWidgetIt = pEnemyWidgetIt + 1;
		} while (iVar12 < 5);
		iVar12 = -1;

	LAB_001d8750:
		iVar8 = 0;
		pEnemyWidgetIt = this->aWidgets;
		if (iVar12 != -1) {
			do {
				if (*pEnemyWidgetIt == (CFrontendEnemy*)0x0) goto LAB_001d8790;

				iVar8 = iVar8 + 1;
				pEnemyWidgetIt = pEnemyWidgetIt + 1;
			} while (iVar8 < 5);

			iVar8 = -1;

		LAB_001d8790:
			if (iVar8 != -1) {
				pCVar7 = (CLifeInterface*)this->enemyInterfaceTable.Pop();
				iVar13 = pCVar7->GetPriority();
				if (iVar13 != 0) {
					this->aWidgets[iVar8] = this->aEnemies + iVar12;
					this->aWidgets[iVar8]->SetInterface(pCVar7);
					iVar12 = 5;
					if (iVar8 != 0) {
						iVar12 = 6;
					}

					this->aWidgets[iVar8]->widgetSlotC.position = this->aSlots[iVar12].position;
					this->aWidgets[iVar8]->widgetSlotC.scale = this->aSlots[iVar12].scale;

					this->aWidgets[iVar8]->UpdateSpecial(this->aSlots + iVar8, iVar8 == 0);
				}
			}
		}
	}

	iVar12 = 0;
	do {
		pCVar9 = this->aEnemies[iVar12].GetInterface();
		if (pCVar9 != (CInterface*)0x0) {
			pCVar7 = (CLifeInterface*)this->aEnemies[iVar12].GetInterface();
			fVar17 = pCVar7->GetPercent();
			if (this->aEnemies[iVar12].field_0x378 != fVar17) {
				this->aEnemies[iVar12].bDisplayDirty = 1;
				fVar20 = this->aEnemies[iVar12].field_0x378;
				this->aEnemies[iVar12].field_0x378 = fVar17;
				this->aEnemies[iVar12].fillAlpha = fVar17;
				fVar17 = this->aEnemies[iVar12].fillLifeMin;
				this->aEnemies[iVar12].spriteFillLife.ClampX(1.0f - (fVar17 + (this->aEnemies[iVar12].fillLifeMax - fVar17) * this->aEnemies[iVar12].fillAlpha), 1.0f - fVar17);
				fVar17 = CFrontend::GetTime();
				this->aEnemies[iVar12].field_0x58 = this->aEnemies[iVar12].field_0x4c + fVar17;
				fVar17 = this->aEnemies[iVar12].field_0x378;
				fVar19 = fVar17;
				if (fVar17 <= fVar20) {
					fVar19 = fVar20;
				}
				fVar18 = this->aEnemies[iVar12].hitAlpha;
				if (fVar19 < fVar18) {
					this->aEnemies[iVar12].hitAlpha = fVar18;
				}
				else {
					if (fVar17 <= fVar20) {
						fVar17 = fVar20;
					}
					this->aEnemies[iVar12].hitAlpha = fVar17;
				}
				fVar19 = this->aEnemies[iVar12].fillLifeMin;
				fVar17 = this->aEnemies[iVar12].fillLifeMax - fVar19;
				this->aEnemies[iVar12].spriteFillHit.ClampX(1.0f - (fVar19 + fVar17 * this->aEnemies[iVar12].hitAlpha), 1.0f - (fVar19 + fVar17 * this->aEnemies[iVar12].fillAlpha));
			}

			this->aEnemies[iVar12].Update(time);
		}

		iVar12 = iVar12 + 1;
	} while (iVar12 < 5);

	return;
}

void CFrontendEnemyList::Draw()
{
	bool bVar1;
	undefined8 uVar2;
	int iVar3;
	CFrontendEnemyList* pCVar4;
	CFrontendEnemy* pEnemyIt;

	if ((GameFlags & 0x1c) == 0) {
		iVar3 = 0;
		pEnemyIt = this->aEnemies;
		do {
			pEnemyIt->DrawSpecial();
			iVar3 = iVar3 + 1;
			pEnemyIt = pEnemyIt + 1;
		} while (iVar3 < 5);

		bVar1 = GuiDList_BeginCurrent();
		iVar3 = 0;
		pEnemyIt = this->aEnemies;
		if (bVar1 != false) {
			do {
				if (pEnemyIt->field_0x388 != 0) {
					pEnemyIt->sprite_0x398.Install(&gEnemyHeadSprites[pEnemyIt->field_0x458].materialInfo);
					pEnemyIt->sprite_0x398.Draw(1);
				}
				iVar3 = iVar3 + 1;
				pEnemyIt = pEnemyIt + 1;
			} while (iVar3 < 5);

			uVar2 = 1;
			iVar3 = 0;
			pEnemyIt = this->aEnemies;
			do {
				if (pEnemyIt->field_0x38c != 0) {
					pEnemyIt->spriteGauge.Draw(uVar2);
					uVar2 = 0;
				}

				iVar3 = iVar3 + 1;
				pEnemyIt = pEnemyIt + 1;
			} while (iVar3 < 5);
			uVar2 = 1;
			iVar3 = 0;

			pEnemyIt = this->aEnemies;
			do {
				if (pEnemyIt->field_0x390 != 0) {
					pEnemyIt->spriteFillHit.Draw(uVar2);
					uVar2 = 0;
				}

				iVar3 = iVar3 + 1;
				pEnemyIt = pEnemyIt + 1;
			} while (iVar3 < 5);

			uVar2 = 1;
			iVar3 = 0;
			pEnemyIt = this->aEnemies;
			do {
				if (pEnemyIt->field_0x394 != 0) {
					pEnemyIt->spriteFillLife.Draw(uVar2);
					uVar2 = 0;
				}

				iVar3 = iVar3 + 1;
				pEnemyIt = pEnemyIt + 1;
			} while (iVar3 < 5);

			GuiDList_EndCurrent();
		}
	}

	return;
}

void CFrontendEnemyList::ResetSlots()
{
	this->aWidgets[0] = 0;

	this->aEnemies[0].widgetSlotC.position = this->aSlots[5].position;
	this->aEnemies[0].widgetSlotC.scale = this->aSlots[5].scale;

	this->aEnemies[0].widgetSlotB.position = this->aSlots[5].position;
	this->aEnemies[0].widgetSlotB.scale = this->aSlots[5].scale;

	this->aWidgets[1] = 0;

	this->aEnemies[1].widgetSlotC.position = this->aSlots[6].position;
	this->aEnemies[1].widgetSlotC.scale = this->aSlots[6].scale;

	this->aEnemies[1].widgetSlotB.position = this->aSlots[6].position;
	this->aEnemies[1].widgetSlotB.scale = this->aSlots[6].scale;

	this->aWidgets[2] = 0;

	this->aEnemies[2].widgetSlotC.position = this->aSlots[6].position;
	this->aEnemies[2].widgetSlotC.scale = this->aSlots[6].scale;

	this->aEnemies[2].widgetSlotB.position = this->aSlots[6].position;
	this->aEnemies[2].widgetSlotB.scale = this->aSlots[6].scale;

	this->aWidgets[3] = 0;

	this->aEnemies[3].widgetSlotC.position = this->aSlots[6].position;
	this->aEnemies[3].widgetSlotC.scale = this->aSlots[6].scale;

	this->aEnemies[3].widgetSlotB.position = this->aSlots[6].position;
	this->aEnemies[3].widgetSlotB.scale = this->aSlots[6].scale;

	this->aWidgets[4] = 0;

	this->aEnemies[4].widgetSlotC.position = this->aSlots[6].position;
	this->aEnemies[4].widgetSlotC.scale = this->aSlots[6].scale;

	this->aEnemies[4].widgetSlotB.position = this->aSlots[6].position;
	this->aEnemies[4].widgetSlotB.scale = this->aSlots[6].scale;

	return;
}

edF32VECTOR2 ENEMY_HEAD_OFFSET = { 0.0f, 0.0f };
edF32VECTOR2 ENEMY_HEAD_SCALE = { 0.125f, 0.125f };

edF32VECTOR2 ENEMY_FILL_OFFSET = { 0.0f, 0.0f };
edF32VECTOR2 ENEMY_FILL_SCALE = { 1.0f, 1.0f };

CFrontendEnemy::CFrontendEnemy()
	: CFrontendBitmapGauge()
{
	this->field_0x458 = 0;
	this->fillLifeMin = 0.2222f;
	this->fillLifeMax = 0.8f;
	this->popupTime = 0.2f;

	this->sprite_0x398.UpdateSlotPosition(ENEMY_HEAD_OFFSET.x, ENEMY_HEAD_OFFSET.y);
	this->sprite_0x398.UpdateSlotScale(ENEMY_HEAD_SCALE.x, ENEMY_HEAD_SCALE.y);

	this->spriteFillLife.UpdateSlotPosition(ENEMY_FILL_OFFSET.x, ENEMY_FILL_OFFSET.y);
	this->spriteFillLife.UpdateSlotScale(ENEMY_FILL_SCALE.x, ENEMY_FILL_SCALE.y);

	this->spriteFillHit.UpdateSlotPosition(ENEMY_FILL_OFFSET.x, ENEMY_FILL_OFFSET.y);
	this->spriteFillHit.UpdateSlotScale(ENEMY_FILL_SCALE.x, ENEMY_FILL_SCALE.y);

	ResetInternal();

	return;
}

void CFrontendEnemy::SetInterface(CInterface* pNewInterface)
{
	CInterface* pCVar1;
	CLifeInterface* pLife;
	long lVar2;
	float fVar3;
	float fVar4;
	float fVar5;
	float fVar6;

	this->pInterface = pNewInterface;

	if (GetInterface() != (CInterface*)0x0) {
		if (GetInterface()->Manage() != 0) {
			pLife = (CLifeInterface*)GetInterface();
			fVar3 = pLife->GetPercent();
			if (this->field_0x378 != fVar3) {
				this->bDisplayDirty = 1;
				fVar6 = this->field_0x378;
				this->field_0x378 = fVar3;
				this->fillAlpha = fVar3;
				fVar3 = this->fillLifeMin;
				this->spriteFillLife.ClampX(1.0f - (fVar3 + (this->fillLifeMax - fVar3) * this->fillAlpha), 1.0f - fVar3);
				fVar3 = CFrontend::GetTime();
				this->field_0x58 = this->field_0x4c + fVar3;
				fVar3 = this->field_0x378;
				fVar5 = fVar3;
				if (fVar3 <= fVar6) {
					fVar5 = fVar6;
				}

				fVar4 = this->hitAlpha;
				if (fVar5 < fVar4) {
					this->hitAlpha = fVar4;
				}
				else {
					if (fVar3 <= fVar6) {
						fVar3 = fVar6;
					}
					this->hitAlpha = fVar3;
				}

				fVar5 = this->fillLifeMin;
				fVar3 = this->fillLifeMax - fVar5;
				this->spriteFillHit.ClampX(1.0f - (fVar5 + fVar3 * this->hitAlpha), 1.0f - (fVar5 + fVar3 * this->fillAlpha));
			}
		}
	}

	return;
}

void CFrontendEnemy::Reset()
{
	CFrontendBitmapGauge::Reset();
	
	ResetInternal();

	return;
}

void CFrontendEnemy::Init()
{
	CFrontendBitmapGauge::Init();

	this->spriteGauge.Install(CScene::ptable.g_FrontEndBank_00451674->GetG2D(0), G2D_RES_WOLF_BACK);
	this->spriteFillLife.Install(CScene::ptable.g_FrontEndBank_00451674->GetG2D(0), G2D_RES_WOLF_FILL_LIFE);
	this->spriteFillHit.Install(CScene::ptable.g_FrontEndBank_00451674->GetG2D(0), G2D_RES_WOLF_FILL_HIT);

	if (gEnemySpritesInstalled == 0) {
		gEnemySpritesInstalled = 1;
		gEnemyHeadSprites[0].Install(CScene::ptable.g_FrontEndBank_00451674->GetG2D(0), G2D_RES_HEAD_SCRAWNY);
		gEnemyHeadSprites[1].Install(CScene::ptable.g_FrontEndBank_00451674->GetG2D(0), G2D_RES_HEAD_GRUNT);
		gEnemyHeadSprites[2].Install(CScene::ptable.g_FrontEndBank_00451674->GetG2D(0), G2D_RES_HEAD_KRONOS);
		gEnemyHeadSprites[3].Install(CScene::ptable.g_FrontEndBank_00451674->GetG2D(0), G2D_RES_HEAD_ATON);
		gEnemyHeadSprites[4].Install(CScene::ptable.g_FrontEndBank_00451674->GetG2D(0), G2D_RES_HEAD_FRANCK);
		gEnemyHeadSprites[5].Install(CScene::ptable.g_FrontEndBank_00451674->GetG2D(0), G2D_RES_HEAD_BRAZUL);
		gEnemyHeadSprites[6].Install(CScene::ptable.g_FrontEndBank_00451674->GetG2D(0), G2D_RES_HEAD_HUNTER);
		this->field_0x458 = 0;
	}

	ResetInternal();

	return;
}

void CFrontendEnemy::Term()
{
	CSprite* pSprite;
	int iVar1;

	if (gEnemySpritesInstalled != 0) {
		iVar1 = 0;
		pSprite = gEnemyHeadSprites;
		do {
			pSprite->Remove();
			iVar1 = iVar1 + 1;
			pSprite = pSprite + 1;
		} while (iVar1 < 7);

		gEnemySpritesInstalled = 0;
	}

	SetInterface((CInterface*)0x0);

	this->sprite_0x398.Remove();
	this->field_0x458 = 0;
	
	// Likely inlined term from CFrontendBitmapGauge.
	this->spriteGauge.Remove();
	this->spriteFillLife.Remove();
	this->spriteFillHit.Remove();

	SetInterface((CInterface*)0x0);

	return;
}

void CFrontendEnemy::UpdatePos_StateWait(float time)
{
	int iVar1;

	iVar1 = this->state_0x37c;
	if ((iVar1 != 3) && (iVar1 != 2)) {
		if (iVar1 == 1) {
			this->state_0x37c = 2;
			this->bVisible = 0;
		}
		else {
			if (iVar1 == 0) {
				this->state_0x37c = 3;
			}
		}
	}

	this->slotAlpha = 1.0f;

	return;
}

bool CFrontendEnemy::UpdateGauge(float time)
{
	bool bVar1;
	float fVar2;
	float fVar3;

	bVar1 = CFrontendBitmapGauge::UpdateGauge(time);
	fVar3 = this->fillLifeMin;
	fVar2 = this->fillLifeMax - fVar3;
	this->spriteFillHit.ClampX(1.0f - (fVar3 + fVar2 * this->hitAlpha), 1.0f - (fVar3 + fVar2 * this->fillAlpha));

	return bVar1;
}

void CFrontendEnemy::UpdateSpecial(CWidgetSlot* pSlot, int param_3)
{
	int iVar1;

	iVar1 = this->state_0x37c;
	if (iVar1 == 3) {
		MoveToNext(pSlot);
		this->state_0x37c = 0;
		this->field_0x384 = this->field_0x380;
		this->field_0x380 = param_3;
	}
	else {
		if (iVar1 == 2) {
			MoveToNext(pSlot);
			this->state_0x37c = 0;
			this->field_0x384 = this->field_0x380;
			this->field_0x380 = param_3;
			this->bVisible = 1;
		}
		else {
			if (iVar1 == 1) {
				MoveToNext(pSlot);
				this->state_0x37c = 0;
				this->field_0x384 = this->field_0x380;
				this->field_0x380 = param_3;
			}
		}
	}

	return;
}

int CFrontendEnemy::FUN_001d94b0()
{
	int piVar1;
	long lVar2;
	int iVar3;

	piVar1 = reinterpret_cast<CLifeInterface*>(this->pInterface)->field_0x10;
	iVar3 = 0;
	if (piVar1 != (int)0x0) {
		IMPLEMENTATION_GUARD(
		lVar2 = (**(code**)(*piVar1 + 0xc))(piVar1, 0x10);
		if (lVar2 == 0) {
			iVar3 = 6;
		}
		else {
			switch (piVar1[0x2dd]) {
			case 0:
				break;
			case 1:
				iVar3 = 1;
				break;
			case 2:
				iVar3 = 2;
				break;
			case 3:
				iVar3 = 3;
				break;
			case 4:
				iVar3 = 4;
				break;
			case 5:
				iVar3 = 5;
			}
		})
	}
	return iVar3;
}

void CFrontendEnemy::DrawSpecial()
{
	CLifeInterface* pLife;
	bool bVar1;
	float fVar2;
	float fVar3;
	CInterface* pCVar4;
	int iVar5;
	int iVar6;
	byte bVar7;
	uint uVar8;
	int iVar9;
	float fVar10;
	float fVar11;

	if ((this->bVisible == 0) || (GetInterface() == (CInterface*)0x0)) {
		this->field_0x394 = 0;
		this->field_0x390 = 0;
		this->field_0x38c = 0;
		this->field_0x388 = 0;
	}
	else {
		bVar1 = this->widgetSlotA.position.x == (this->sprite_0x2b8).field_0x5c.position.x;
		if ((bVar1) &&
			(bVar1 = true, this->widgetSlotA.position.y != (this->sprite_0x2b8).field_0x5c.position.y)) {
			bVar1 = false;
		}

		if ((bVar1) && (bVar1 = true, this->widgetSlotA.scale.x != (this->sprite_0x2b8).field_0x5c.scale.x)) {
			bVar1 = false;
		}

		if ((bVar1) && (bVar1 = true, this->widgetSlotA.scale.y != (this->sprite_0x2b8).field_0x5c.scale.y)) {
			bVar1 = false;
		}

		if (!bVar1) {
			(this->sprite_0x2b8).bValid = false;
			(this->sprite_0x2b8).field_0x5c.position = this->widgetSlotA.position;
			(this->sprite_0x2b8).field_0x5c.scale = this->widgetSlotA.scale;
		}

		pLife = (CLifeInterface*)this->pInterface;
		if (pLife != (CLifeInterface*)0x0) {
			pLife->GetValueMax();
			CLevelScheduler::ScenVar_Get(2);
			static edF32VECTOR2 edF32VECTOR2_00448848 = { 0.25f, 0.0625f };
			static edF32VECTOR2 edF32VECTOR2_00448850 = { -0.1631f, -0.0244f };
			fVar3 = edF32VECTOR2_00448850.y;
			fVar2 = edF32VECTOR2_00448850.x;
			fVar11 = edF32VECTOR2_00448848.y;
			fVar10 = edF32VECTOR2_00448848.x * 1.0f;
			this->spriteGauge.bValid = false;
			this->spriteGauge.field_0x5c.scale.x = fVar10;
			this->spriteGauge.field_0x5c.scale.y = fVar11;
			this->spriteGauge.bValid = false;
			this->spriteGauge.field_0x5c.position.x = fVar2 * 1.0;
			this->spriteGauge.field_0x5c.position.y = fVar3;
		}

		if ((this->sprite_0x2b8).bValid == false) {
			this->spriteGauge.bValid = false;
			(this->sprite_0x398).bValid = false;
			this->spriteFillHit.bValid = false;
			this->spriteFillLife.bValid = false;
		}

		iVar5 = FUN_001d94b0();
		this->field_0x458 = iVar5;
		iVar5 = 0x7f;
		iVar6 = 0x7f;
		iVar9 = 0x7f;
		if (this->field_0x384 == 0) {
			iVar5 = 0;
			iVar6 = 0;
		}
		if (this->field_0x380 == 0) {
			iVar9 = 0;
		}
		uVar8 = (int)((float)iVar6 + (float)(iVar9 - iVar5) * this->slotAlpha) & 0xff;
		if (uVar8 == 0) {
			this->field_0x388 = 0;
			this->field_0x38c = 0;
		}
		else {
			this->spriteGauge.rgba[0] = 0x7f;
			this->spriteGauge.rgba[1] = 0x7f;
			this->spriteGauge.rgba[2] = 0x7f;
			bVar7 = (byte)uVar8;
			this->spriteGauge.rgba[3] = bVar7;
			this->field_0x38c = 1;
			(this->sprite_0x398).rgba[0] = 0x7f;
			(this->sprite_0x398).rgba[1] = 0x7f;
			(this->sprite_0x398).rgba[2] = 0x7f;
			(this->sprite_0x398).rgba[3] = bVar7;
			this->field_0x388 = 1;
		}

		this->field_0x390 = 1;
		this->field_0x394 = 1;
	}

	return;
}

void CFrontendEnemy::ResetInternal()
{
	this->sprite_0x398.SetParent(&this->sprite_0x2b8);
	this->spriteGauge.SetParent(&this->sprite_0x2b8);

	this->spriteFillLife.SetParent(&this->spriteGauge);
	this->spriteFillHit.SetParent(&this->spriteGauge);

	this->field_0x378 = -1.0f;
	this->state_0x37c = 2;
	this->spriteFillLife.ClampX(this->fillLifeMin, this->fillLifeMax);
	this->spriteFillHit.ClampX(this->fillLifeMin, this->fillLifeMax);

	this->field_0x384 = 0;
	this->field_0x380 = 0;
	this->field_0x394 = 0;
	this->field_0x390 = 0;
	this->field_0x38c = 0;
	this->field_0x388 = 0;

	return;
}

bool EnemyInterfaceTable::Contains(CInterface* pInterface)
{
	for (int i = 0; i < this->nbEntries; i++) {
		if (this->aEntries[i] == pInterface) {
			return true;
		}
	}

	return false;
}

void EnemyInterfaceTable::Add(CInterface* pInterface)
{
	if (this->nbEntries < 0x10) {
		this->aEntries[this->nbEntries] = pInterface;
		this->nbEntries = this->nbEntries + 1;
	}

	return;
}

CInterface* EnemyInterfaceTable::Pop()
{
	int iVar1;
	CInterface* pInterface;

	pInterface = (CInterface*)0x0;
	if (this->nbEntries != 0) {
		iVar1 = this->nbEntries;
		this->nbEntries = iVar1 + -1;
		pInterface = this->aEntries[iVar1 + -1];
	}

	return pInterface;
}
