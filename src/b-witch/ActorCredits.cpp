#include "ActorCredits.h"
#include "MemoryStream.h"
#include "Pause.h"
#include "LevelScheduler.h"
#include "kya.h"
#include "TranslatedTextData.h"
#include "DlistManager.h"
#include "MathOps.h"
#include "TimeController.h"
#include "Rendering/edCTextFont.h"

void CActorCredits::Create(ByteCode* pByteCode)
{
	S_CREDITS_MESSAGE_STREAM* pSVar1;
	S_NTF_TARGET_STREAM_REF* pSVar2;
	S_STREAM_EVENT_CAMERA* pSVar3;
	bool bVar4;
	uint uVar5;
	//CMusic* pMusic;
	int iVar6;
	float fVar7;
	CAudioManager* pAudioManager;

	CActor::Create(pByteCode);

	pSVar1 = reinterpret_cast<S_CREDITS_MESSAGE_STREAM*>(pByteCode->currentSeekPos);
	pByteCode->currentSeekPos = reinterpret_cast<char*>(pByteCode->currentSeekPos + 4);
	if (pSVar1->nbEntries != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + pSVar1->nbEntries * sizeof(ulong);
	}

	this->pMessageStream = pSVar1;

	iVar6 = 0;
	while (true) {
		if (this->pMessageStream == (S_CREDITS_MESSAGE_STREAM*)0x0) {
			bVar4 = iVar6 < 0;
		}
		else {
			bVar4 = iVar6 < this->pMessageStream->nbEntries;
		}

		if (!bVar4) break;

		iVar6 = iVar6 + 1;
	}

	this->scrollSpeed = pByteCode->GetF32();
	fVar7 = pByteCode->GetF32();
	this->textBoxX = (fVar7 / 512.0f) * static_cast<float>(gVideoConfig.screenWidth);
	fVar7 = pByteCode->GetF32();
	this->textBoxY = (fVar7 / 448.0f) * static_cast<float>(gVideoConfig.screenHeight);
	fVar7 = pByteCode->GetF32();
	this->textBoxWidth = (fVar7 / 512.0f) * static_cast<float>(gVideoConfig.screenWidth);
	fVar7 = pByteCode->GetF32();
	this->textBoxHeight = (fVar7 / 448.0f) * static_cast<float>(gVideoConfig.screenHeight);
	this->boxFillOpacityTarget = pByteCode->GetF32();
	this->boxBorderOpacity = pByteCode->GetF32();
	fVar7 = pByteCode->GetF32();
	this->boxBorderWidth = (fVar7 / 512.0f) * static_cast<float>(gVideoConfig.screenWidth);
	fVar7 = pByteCode->GetF32();
	this->boxBorderHeight = (fVar7 / 448.0f) * static_cast<float>(gVideoConfig.screenHeight);
	pAudioManager = CScene::ptable.g_AudioManager_00451698;
	uVar5 = pByteCode->GetU32();

	IMPLEMENTATION_GUARD_AUDIO(
	if ((uVar5 == 0xffffffff) || (bVar4 = pAudioManager->nbMusic <= uVar5, bVar4)) {
		pMusic = (CMusic*)0x0;
	}
	else {
		if (bVar4) {
			uVar5 = 0;
		}
		pMusic = pAudioManager->aMusic + uVar5;
	}
	this->field_0x190 = pMusic;)

	this->musicFadeInTime = pByteCode->GetF32();
	this->musicVolume = pByteCode->GetF32();
	this->musicFadeOutTime = pByteCode->GetF32();

	this->nextLevelId = pByteCode->GetS32();
	this->nextLevelParam = pByteCode->GetS32();
	this->nextLevelSpawnParam = pByteCode->GetS32();

	this->completionSwitch.Create(pByteCode);

	iVar6 = CLevelScheduler::ScenVar_Get(0);
	UpdateForFreedWolfen(iVar6);

	return;
}

void CActorCredits::Init()
{
	CActor::Init();

	this->completionSwitch.Init();

	ClearLocalData();

	return;
}

void CActorCredits::Draw()
{
	S_CREDITS_MESSAGE_STREAM* pSVar1;
	long key;
	bool bVar2;
	edCTextStyle* pNewFont;
	int iVar3;
	char* pcVar5;
	int iVar6;
	int iVar7;
	int iVar8;
	char* pcVar9;
	float fVar10;
	uint uVar11;
	float inMax;
	float inMax_00;
	float fVar12;
	float fVar13;
	float fVar14;
	float fVar15;
	edCTextStyle textStyle;

	if ((GameFlags & 0x20) == 0) {
		CActor::Draw();

		// Credits text is submitted through the GUI display list, separately from actor geometry.
		bVar2 = GuiDList_BeginCurrent();

		if (bVar2 != false) {
			textStyle.Reset();

			// fVar13 is the horizontal centre of the configured credits rectangle.
			fVar13 = this->textBoxX + this->textBoxWidth * 0.5f;
			if (0.0f < this->boxFillOpacityTarget) {
				// Optional border. field_0x1c4 is animated by the credits behaviour.
				fVar14 = this->boxFillOpacity;
				if (fVar14 < 2.147484e+09f) {
					fVar15 = this->boxBorderOpacity;
				}
				else {
					fVar14 = fVar14 - 2.147484e+09f;
					fVar15 = this->boxBorderOpacity;
				}

				if (2.147484e+09f <= fVar15) {
					fVar15 = fVar15 - 2.147484e+09f;
				}

				CPauseManager::DrawRectangleBorder(fVar13, this->textBoxY + this->textBoxHeight * 0.5f, this->textBoxWidth, this->textBoxHeight, this->boxBorderWidth, this->boxBorderHeight, static_cast<int>(fVar14) << 0x18, static_cast<int>(fVar15) << 0x18, 0);
			}

			if ((this->bCreditsComplete == 0) && (this->pCurrentMessage != (char*)0x0)) {
				// field_0x1cc is the current message; field_0x1bc is its entry in field_0x168.
				textStyle.SetFont(BootDataFont, false);
				textStyle.rgbaColour = 0xffffffff;
				textStyle.SetShadow(0x100);
				textStyle.SetShadowShift(2.0f, 2.0f);
				textStyle.SetHorizontalAlignment(2);
				textStyle.SetEolAutomatic(0x80);
				textStyle.SetHorizontalJustification(0x10);
				textStyle.SetHorizontalSize(this->textBoxWidth);
				pNewFont = edTextStyleGetCurrent();
				edTextStyleSetCurrent(&textStyle);
				edCTextFormat textFormat;
				pcVar5 = this->pCurrentMessage;
				fVar14 = this->scrollY;
				iVar8 = this->currentMessageIndex;

				// Set by the first processed line to the source position for the next frame.
				// It remains null if no line is processed (for example, an empty message).
				pcVar9 = (char*)0x0;
				bVar2 = true;
				fVar12 = this->textBoxY + this->textBoxHeight;
				inMax = this->textBoxY + this->boxBorderHeight;
				inMax_00 = (fVar12 - this->boxBorderHeight) - (float)(uint)(textStyle.pPackedFont)->field_0x16;
				fVar15 = fVar14;
				iVar6 = iVar8 << 3;
				// Render visible lines from this and subsequent messages while they fit below the box.
				while ((iVar7 = iVar6, fVar15 <= fVar12 && (*pcVar5 != '\0'))) {
					// Copy one newline- or NUL-terminated line into field_0x1d0.
					pcVar5 = FUN_00409750(pcVar5);
					if (this->lineBuffer[0] == 0) {
						// A blank line only advances by the packed font height.
						fVar10 = (float)(uint)(textStyle.pPackedFont)->field_0x16;
					}
					else {
						// Fade lines while they enter or leave the clipping region.
						if ((fVar15 < inMax) || (inMax_00 < fVar15)) {
							if (fVar15 < inMax) {
								fVar10 = edFIntervalLERP(fVar15, this->textBoxY, inMax, 0.0f, 255.0f);
								if (fVar10 < 2.147484e+09f) {
									uVar11 = static_cast<uint>(fVar10);
								}
								else {
									uVar11 = static_cast<int>(fVar10 - 2.147484e+09f) | 0x80000000;
								}
							}
							else {
								fVar10 = edFIntervalLERP(fVar15, fVar12 - (float)(uint)(textStyle.pPackedFont)->field_0x16, inMax_00, 0.0f, 255.0f);
								if (fVar10 < 2.147484e+09f) {
									uVar11 = static_cast<uint>(fVar10);
								}
								else {
									uVar11 = static_cast<int>(fVar10 - 2.147484e+09f) | 0x80000000;
								}
							}

							bVar2 = false;
							textStyle.rgbaColour = uVar11 | textStyle.rgbaColour & 0xffffff00;
							textStyle.altColour = uVar11 | textStyle.altColour & 0xffffff00U;
							textStyle.alpha = uVar11 | textStyle.alpha & 0xffffff00U;
						}
						else {
							if (!bVar2) {
								bVar2 = true;
								textStyle.rgbaColour = textStyle.rgbaColour & 0xffffff00 | 0xff;
								textStyle.altColour = textStyle.altColour & 0xffffff00U | 0xff;
								textStyle.alpha = textStyle.alpha & 0xffffff00U | 0xff;
							}
						}

						textFormat.FormatString(this->lineBuffer);
						textFormat.Display(fVar13, fVar15);
						fVar10 = textFormat.field_0xc;
					}

					fVar15 = fVar15 + fVar10;
					if (pcVar9 == (char*)0x0) {
						// Preserve the next line and its Y coordinate as the next scroll origin.
						fVar14 = fVar15;
						pcVar9 = pcVar5;
					}

					iVar6 = iVar7;
					if (*pcVar5 == '\0') {
						// Reaching a message terminator continues with the next credits entry, if any.
						pSVar1 = this->pMessageStream;
						iVar6 = iVar7 + 8;
						iVar8 = iVar8 + 1;
						if (pSVar1 == (S_CREDITS_MESSAGE_STREAM*)0x0) {
							iVar3 = 0;
						}
						else {
							iVar3 = pSVar1->nbEntries;
						}

						if (iVar8 < iVar3) {
							key = pSVar1->aKeys[iVar8];
							pcVar5 = (char*)0x0;
							if (key != 0) {
								pcVar5 = gMessageManager.get_message(key);
							}
						}
					}
				}

				// Move the scrolling origin upward after rendering this frame.
				fVar13 = this->currentScrollSpeed;
				fVar13 = this->scrollY - fVar13 * GetTimer()->cutsceneDeltaTime;
				this->scrollY = fVar13;
				if (fVar13 <= this->textBoxY) {
					// The first displayed line has passed the top edge; promote the saved next line.
					// NOTE: pcVar9 is null if the rendering loop never ran. In particular,
					// MessageManager::get_message() returns a non-null empty fallback for a missing key.
					this->pCurrentMessage = pcVar9;
					this->scrollY = fVar14;
					if (*this->pCurrentMessage == '\0') {
						this->currentMessageIndex = this->currentMessageIndex + 1;
						pSVar1 = this->pMessageStream;
						iVar8 = 0;
						if (pSVar1 != (S_CREDITS_MESSAGE_STREAM*)0x0) {
							iVar8 = pSVar1->nbEntries;
						}
						if (this->currentMessageIndex == iVar8) {
							this->bCreditsComplete = 1;
						}
						else {
							if (pSVar1->aKeys[this->currentMessageIndex] == 0) {
								this->pCurrentMessage = (char*)0x0;
							}
							else {
								pcVar5 = gMessageManager.get_message(pSVar1->aKeys[this->currentMessageIndex]);
								this->pCurrentMessage = pcVar5;
							}
						}
					}
				}

				edTextStyleSetCurrent(pNewFont);
			}

			GuiDList_EndCurrent();
		}
	}

	return;
}

void CActorCredits::Reset()
{
	CActor:Reset();

	this->completionSwitch.Reset(this);

	ClearLocalData();

	return;
}

CBehaviour* CActorCredits::BuildBehaviour(int behaviourType)
{
	CBehaviour* pBehaviour;

	if (behaviourType == CREDITS_BEHAVIOUR_DEFAULT) {
		pBehaviour = &this->behaviourDefault;
	}
	else {
		pBehaviour = CActor::BuildBehaviour(behaviourType);
	}

	return pBehaviour;
}

int CActorCredits::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	ulong uVar1;
	char* pcVar2;
	int** ppiVar3;
	int iVar4;
	int iVar5;

	if (msg == 0x10) {
		this->flags = this->flags & 0xfffffffd;
		this->flags = this->flags | 1;
		this->flags = this->flags & 0xffffff7f;
		this->flags = this->flags | 0x20;
		EvaluateDisplayState();

		IMPLEMENTATION_GUARD_AUDIO(
		if (this->field_0x190 != (CMusic*)0x0) {
			MusicStop(this->musicFadeOutTime, 0, (CScene::ptable.g_AudioManager_00451698)->field_0x38, this->musicHandle);
			this->musicHandle = -1;
		})

		iVar4 = 1;
	}
	else {
		if (msg == 0xf) {
			this->flags = this->flags | 2;
			this->flags = this->flags & 0xfffffffe;
			this->flags = this->flags | 0x80;
			this->flags = this->flags & 0xffffffdf;

			EvaluateDisplayState();

			ClearLocalData();

			IMPLEMENTATION_GUARD_AUDIO(
			if (this->field_0x190 != (CMusic*)0x0) {
				ppiVar3 = CMusicManager::Start(this->musicFadeInTime, 1.0f, this->musicVolume, 0.0f, static_cast<CMusicManager*>(CScene::ptable.g_AudioManager_00451698)->field_0x38, this->field_0x190, static_cast<int*>(0x19));
				this->musicHandle = static_cast<int>(ppiVar3);
			})
			iVar4 = 1;
		}
		else {
			if (msg == 0xe) {
				if ((this->flags & 4) == 0) {
					this->flags = this->flags | 2;
					this->flags = this->flags & 0xfffffffe;
					this->flags = this->flags | 0x80;
					this->flags = this->flags & 0xffffffdf;
					EvaluateDisplayState();

					ClearLocalData();

					IMPLEMENTATION_GUARD_AUDIO(
					if (this->field_0x190 != (CMusic*)0x0) {
						ppiVar3 = CMusicManager::Start(this->musicFadeInTime, 1.0f, this->musicVolume, 0.0f, static_cast<CMusicManager*>(CScene::ptable.g_AudioManager_00451698)->field_0x38, this->field_0x190, static_cast<int*>(0x19));
						this->musicHandle = static_cast<int>(ppiVar3);
					})
				}
				else {
					this->flags = this->flags & 0xfffffffd;
					this->flags = this->flags | 1;
					this->flags = this->flags & 0xffffff7f;
					this->flags = this->flags | 0x20;

					EvaluateDisplayState();

					IMPLEMENTATION_GUARD_AUDIO(
					if (this->field_0x190 != (CMusic*)0x0) {
						MusicStop(this->musicFadeOutTime, 0, (CScene::ptable.g_AudioManager_00451698)->field_0x38, this->musicHandle);
						this->musicHandle = -1;
					})
				}
			}

			iVar4 = InterpretMessage(pSender, msg, static_cast<_msg_params_get_position*>(pMsgParam));
		}
	}

	return iVar4;
}

void CActorCredits::ClearLocalData()
{
	ulong key;
	char* pcVar1;

	for (int i = 0; i < 64; i++) {
		this->lineBuffer[i] = 0;
	}

	this->bCreditsComplete = 0;
	this->currentMessageIndex = 0;
	this->currentScrollSpeed = this->scrollSpeed;
	this->boxFillOpacity = 0.0f;
	this->scrollY = this->textBoxY + this->boxBorderHeight;
	key = this->pMessageStream->aKeys[this->currentMessageIndex];
	pcVar1 = (char*)0x0;
	if (key != 0) {
		pcVar1 = gMessageManager.get_message(key);
	}
	this->pCurrentMessage = pcVar1;
	this->musicHandle = -1;

	return;
}

char* CActorCredits::FUN_00409750(char* param_2)
{
	char* iVar1;
	int iVar3;

	iVar3 = 0;
	do {
		iVar1 = this->lineBuffer + iVar3;
		*iVar1 = *param_2;
		if (*iVar1 == '\n') {
			*iVar1 = '\0';
		}

		if (*param_2 != '\0') {
			param_2 = param_2 + 1;
		}

	} while ((*iVar1 != '\0') && (iVar3 = iVar3 + 1, iVar3 < 0x40));

	if ((iVar3 == 0x40) && (this->lineBuffer[iVar3 - 1] != '\0')) {
		this->lineBuffer[iVar3 - 1] = '\0';
	}

	return param_2;
}

void CBehaviourCredits::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	this->pOwner = static_cast<CActorCredits*>(pOwner);

	return;
}

void CBehaviourCreditsDefault::Manage()
{
	int levelID;
	int iVar1;
	int iVar2;
	CActorCredits* pCredits;

	pCredits = this->pOwner;

	pCredits->completionSwitch.pStreamEventCamera->Manage(pCredits);

	if (0.0f < pCredits->boxFillOpacityTarget) {
		if (pCredits->bCreditsComplete == 0) {
			pCredits->SV_UpdateValue(pCredits->boxFillOpacityTarget, 50.0f, &pCredits->boxFillOpacity);
		}
		else {
			pCredits->SV_UpdateValue(0.0f, 50.0f, &pCredits->boxFillOpacity);
		}
	}
	if ((pCredits->bCreditsComplete != 0) && (pCredits->boxFillOpacity == 0.0f)) {
		pCredits->bCreditsComplete = 0;
		pCredits->completionSwitch.Switch(pCredits);
		pCredits->flags = pCredits->flags & 0xfffffffd;
		pCredits->flags = pCredits->flags | 1;
		pCredits->flags = pCredits->flags & 0xffffff7f;
		pCredits->flags = pCredits->flags | 0x20;
		pCredits->EvaluateDisplayState();

		IMPLEMENTATION_GUARD_AUDIO(
		if (((edF32VECTOR4*)&pCredits->field_0x190)->x != 0.0f) {
			MusicStop(pCredits->musicFadeOutTime, 0, (CScene::ptable.g_AudioManager_00451698)->field_0x38, static_cast<int>((edF32VECTOR4*)&pCredits->musicHandle)->x);
			((edF32VECTOR4*)&pCredits->musicHandle)->x = -NAN;
		})

		levelID = pCredits->nextLevelId;
		if (levelID != 0x10) {
			if (levelID == 0xe) {
				CLevelScheduler::gThis->ExitLevel(0);
			}
			else {
				CLevelScheduler::gThis->Level_Run(0, levelID, pCredits->nextLevelParam, pCredits->nextLevelSpawnParam, 0xffffffff, pCredits->nextLevelSpawnParam == -1);
			}
		}
	}

	return;
}

void CBehaviourCreditsDefault::Begin(CActor * pOwner, int newState, int newAnimationType)
{
	CBehaviourCredits::Begin(pOwner, newState, newAnimationType);

	if (newState == -1) {
		this->pOwner->SetState(0, -1);
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	return;
}
