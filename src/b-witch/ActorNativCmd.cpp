#include "ActorNativCmd.h"
#include "MemoryStream.h"
#include "ActorNativ.h"
#include "TimeController.h"
#include "ActorHero.h"
#include "MathOps.h"
#include "CinematicManager.h"
#include "SectorManager.h"
#include "EdFileBase.h"

void CActorNativCmd::Create(ByteCode* pByteCode)
{
	S_ACTOR_STREAM_REF* pSVar1;
	uint count;
	int iVar2;
	int* pBase;
	CTalkParam* pCVar3;
	void* pvVar4;
	CTalkParam* iVar6;

	CActor::Create(pByteCode);

	pSVar1 = reinterpret_cast<S_ACTOR_STREAM_REF*>(pByteCode->currentSeekPos);
	pByteCode->currentSeekPos = pByteCode->currentSeekPos + 4;
	if (pSVar1->entryCount != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + pSVar1->entryCount * 4;
	}
	this->aNativs = pSVar1;

	this->field_0x164 = pByteCode->GetS32();

	if (this->aNativs == (S_ACTOR_STREAM_REF*)0x0) {
		iVar2 = 0;
	}
	else {
		iVar2 = this->aNativs->entryCount;
	}

	this->nbTalkParams = (int)((float)iVar2 / 2.0f);
	count = this->nbTalkParams;
	if (count != 0) {
		this->aTalkParams = new CTalkParam[count];
	}

	iVar2 = 0;
	if (0 < this->nbTalkParams) {
		do {
			iVar6 = this->aTalkParams + iVar2;
			iVar6->field_0x8 = new CTalkParamSubObj[2];
			iVar2 = iVar2 + 1;

			iVar6->nbSubObj = 2;
		} while (iVar2 < this->nbTalkParams);
	}

	this->addOnA.Create(pByteCode);
	this->addOnB.Create(pByteCode);
	this->addOnC.Create(pByteCode);
	this->addOnD.Create(pByteCode);
	return;
}

void CActorNativCmd::Init()
{
	int iVar1;
	int iVar2;
	int iVar3;
	CTalkParam* iVar4;
	S_ACTOR_STREAM_REF* pActorRef;
	int iVar5;
	CActorNativMsgParam_0xe local_20;

	CActor::Init();

	this->addOnA.Init(this);
	this->addOnB.Init(this);
	this->addOnC.Init(this);
	this->addOnD.Init(this);

	pActorRef = this->aNativs;
	for (int i = 0; i < pActorRef->entryCount; i++) {
		pActorRef->aEntries[i].Init();
	}

	iVar5 = 0;
	if (0 < this->nbTalkParams) {
		do {
			iVar4 = this->aTalkParams + iVar5;
			iVar4->field_0x10 = 0;
			iVar4->field_0x1c = 0;
			iVar4->totalTime = 0.0f;
			iVar4->field_0x18 = 0.0f;
			iVar4->field_0x1d = 0;

			iVar3 = 0;
			if (0 < iVar4->nbSubObj) {
				iVar1 = 0;
				do {
					iVar4->field_0x8[iVar3].pNativ = (CActorNativ*)0x0;
					iVar4->field_0x8[iVar3].field_0x4 = 1;
					iVar3 = iVar3 + 1;
				} while (iVar3 < iVar4->nbSubObj);
			}
			iVar5 = iVar5 + 1;
		} while (iVar5 < this->nbTalkParams);
	}

	this->field_0x1d0 = 0;

	this->addOnA.Reset();
	this->addOnB.Reset();
	this->addOnC.Reset();
	this->addOnD.Reset();

	iVar2 = 0;
	iVar5 = 0;
	while (true) {
		if (this->aNativs == (S_ACTOR_STREAM_REF*)0x0) {
			iVar3 = 0;
		}
		else {
			iVar3 = this->aNativs->entryCount;
		}

		if (iVar3 <= iVar2) break;

		local_20.type = 1;
		local_20.field_0x4 = this->field_0x164;

		DoMessage(this->aNativs->aEntries[iVar2].Get(), MESSAGE_NATIV_CMD, &local_20);

		iVar5 = iVar5 + 4;
		iVar2 = iVar2 + 1;
	}

	return;
}

void CActorNativCmd::Reset()
{
	IMPLEMENTATION_GUARD();
}

CBehaviour* CActorNativCmd::BuildBehaviour(int behaviourType)
{
	CBehaviour* pBehaviour;

	if (behaviourType == 0x2) {
		pBehaviour = &this->behaviourStand;
	}
	else {
		pBehaviour = CActor::BuildBehaviour(behaviourType);
	}

	return pBehaviour;
}

StateConfig CActorNativCmd::_gStateCfg_NVC[2] = {
	StateConfig(0x0, 0x0),
	StateConfig(0x0, 0x0)
};

StateConfig* CActorNativCmd::GetStateCfg(int state)
{
	StateConfig* pSVar1;

	if (state < 5) {
		pSVar1 = CActor::GetStateCfg(state);
	}
	else {
		pSVar1 = _gStateCfg_NVC + state + -5;
	}

	return pSVar1;
}

void CActorNativCmd::ChangeManageState(int state)
{
	int iVar1;
	int iVar3;
	CActorNativMsgParam_0xe local_20;

	CActor::ChangeManageState(state);

	iVar3 = 0;
	if (state != 0) {
		while (true) {
			if (this->aNativs == (S_ACTOR_STREAM_REF*)0x0) {
				iVar1 = 0;
			}
			else {
				iVar1 = this->aNativs->entryCount;
			}

			if (iVar1 <= iVar3) break;

			local_20.type = 1;
			local_20.field_0x4 = this->field_0x164;

			DoMessage(this->aNativs->aEntries[iVar3].Get(), MESSAGE_NATIV_CMD, &local_20);
			iVar3 = iVar3 + 1;
		}
	}

	return;
}

int CActorNativCmd::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	char cVar1;
	int iVar2;
	int uVar3;
	int iVar3;
	int iVar4;
	CActor* pCVar5;
	CActorNativ* pReceiver;
	CTalkParam* pTalkParam;
	int iVar6;
	long lVar7;
	undefined4 local_f0[8];
	undefined4 local_d0[2];
	undefined4 local_c8;
	undefined4 local_b0[8];
	undefined4 local_90[8];
	undefined4 local_70[8];
	edF32VECTOR4 eStack80;
	CActorNativMsgParam_0xe local_40;
	float local_3c;
	float local_38;
	float local_34;
	float local_30;
	float local_2c;
	edF32VECTOR4* local_28;
	undefined4* local_24;
	undefined4* local_20;
	undefined4* local_1c;
	undefined4* local_18;
	int* local_14;
	int* local_10;
	int* local_c;
	int* local_8;
	undefined4* local_4;

	if (msg == 0x4e) {
		uVar3 = 0;
		if (pSender->typeID == 10) {
			CActorNativMsgParam_0xe* pParams = reinterpret_cast<CActorNativMsgParam_0xe*>(pMsgParam);
			iVar6 = pParams->type;
			if (iVar6 == 7) {
				IMPLEMENTATION_GUARD(
				local_40 = pMsgParams->field_0x0;
				local_3c = pMsgParams->field_0x4;
				local_38 = pMsgParams->field_0x8;
				local_34 = pMsgParams->field_0xc;
				local_30 = (pMsgParams->vectorFieldA).x;
				local_2c = (pMsgParams->vectorFieldA).y;
				local_28 = (edF32VECTOR4*)(pMsgParams->vectorFieldA).z;
				pTalkParam = (CTalkParam*)0x0;
				if ((int)local_38 < this->nbTalkParams) {
					pTalkParam = this->aTalkParams + (int)local_38;
				}
				iVar6 = pTalkParam->nbSubObj;
				iVar2 = 0;
				if (0 < iVar6) {
					iVar4 = 0;
					do {
						pCVar5 = (CActor*)0x0;
						if (iVar2 < iVar6) {
							pCVar5 = *(CActor**)((int)&pTalkParam->field_0x8->pNativ + iVar4);
						}
						if (pCVar5 == pSender) goto LAB_003958b8;
						iVar2 = iVar2 + 1;
						iVar4 = iVar4 + 8;
					} while (iVar2 < pTalkParam->nbSubObj);
				}
				iVar2 = -1;
			LAB_003958b8:
				if (iVar2 == -1) {
					local_f0[0] = 2;
					local_24 = local_f0;
					CActor::DoMessage((CActor*)this, pSender, 0x4e, (uint)local_24);
				}
				else {
					lVar7 = 0;
					if (0 < iVar6) {
						iVar6 = 0;
						do {
							pCVar5 = (CActor*)0x0;
							if (lVar7 < pTalkParam->nbSubObj) {
								pCVar5 = *(CActor**)((int)&pTalkParam->field_0x8->pNativ + iVar6);
							}
							local_d0[0] = CTalkParam::ChooseFinishOrder(pTalkParam, lVar7);
							local_c8 = 0xffffffff;
							if (pCVar5 != (CActor*)0x0) {
								local_20 = local_d0;
								CActor::DoMessage((CActor*)this, pCVar5, 0x4e, (uint)local_d0);
							}
							lVar7 = (long)((int)lVar7 + 1);
							iVar6 = iVar6 + 8;
						} while (lVar7 < pTalkParam->nbSubObj);
					}
					pTalkParam->field_0x10 = 0;
					pTalkParam->field_0x1c = 0;
					pTalkParam->totalTime = 0.0f;
					pTalkParam->field_0x18 = 0.0f;
					pTalkParam->field_0x1d = 0;
					iVar6 = 0;
					if (0 < pTalkParam->nbSubObj) {
						iVar2 = 0;
						do {
							iVar6 = iVar6 + 1;
							*(undefined4*)((int)&pTalkParam->field_0x8->pNativ + iVar2) = 0;
							*(undefined4*)((int)&pTalkParam->field_0x8->field_0x4 + iVar2) = 1;
							iVar2 = iVar2 + 8;
						} while (iVar6 < pTalkParam->nbSubObj);
					}
				})
			}
			else {
				if (iVar6 == 0x11) {
					IMPLEMENTATION_GUARD(
					local_b0[0] = 0x11;
					if (*(CActor**)&this->field_0x1d4 != (CActor*)0x0) {
						local_1c = local_b0;
						CActor::DoMessage((CActor*)this, *(CActor**)&this->field_0x1d4, 0x4e, (uint)local_1c);
					})
				}
				else {
					if (iVar6 == 0x12) {
						IMPLEMENTATION_GUARD(
						*(undefined4*)&this->field_0x1d4 = 0;
						if (pMsgParams->field_0x8 != -NAN) {
							local_18 = local_90;
							local_90[0] = 0x12;
							CActor::DoMessage((CActor*)this, this->aNativs->aEntries[(int)pMsgParams->field_0x8].pActor, 0x4e,
								(uint)local_18);
							pMsgParams->field_0x8 = -NAN;
						})
					}
					else {
						if (iVar6 == 0x10) {
							IMPLEMENTATION_GUARD(
							*(CActor**)&this->field_0x1d4 = pSender;
							FUN_00394eb0((long)(int)this, (int*)&pMsgParams->field_0x8);)
						}
						else {
							if (iVar6 == 9) {
								CActorNativ* pNativ = static_cast<CActorNativ*>(pSender);
								CActorNativMsgParam_0xe* pParams = reinterpret_cast<CActorNativMsgParam_0xe*>(pMsgParam);
								if (pNativ->field_0x378 == 1) {
									cVar1 = this->addOnB.Func_0x20(pParams->field_0x10, pSender, 0);
								}
								else {
									cVar1 = this->addOnA.Func_0x20(pParams->field_0x10, pSender, 0);
								}

								return (int)cVar1 & 0xff;
							}

							if (iVar6 == 3) {
								local_40.type = 3;
								local_38 = -NAN;
								DoMessage(pSender, MESSAGE_NATIV_CMD, &local_40);
								return 1;
							}

							if (iVar6 == 8) {
								IMPLEMENTATION_GUARD(
								local_40 = pMsgParams->field_0x0;
								local_3c = pMsgParams->field_0x4;
								local_38 = pMsgParams->field_0x8;
								local_34 = pMsgParams->field_0xc;
								local_30 = (pMsgParams->vectorFieldA).x;
								local_2c = (pMsgParams->vectorFieldA).y;
								local_28 = (edF32VECTOR4*)(pMsgParams->vectorFieldA).z;
								pTalkParam = (CTalkParam*)0x0;
								if ((int)local_38 < this->nbTalkParams) {
									pTalkParam = this->aTalkParams + (int)local_38;
								}
								iVar6 = 0;
								if (0 < pTalkParam->nbSubObj) {
									iVar2 = 0;
									do {
										pCVar5 = (CActor*)0x0;
										if (iVar6 < pTalkParam->nbSubObj) {
											pCVar5 = *(CActor**)((int)&pTalkParam->field_0x8->pNativ + iVar2);
										}
										if (pCVar5 == pSender) goto LAB_00395388;
										iVar6 = iVar6 + 1;
										iVar2 = iVar2 + 8;
									} while (iVar6 < pTalkParam->nbSubObj);
								}
								iVar6 = -1;
							LAB_00395388:
								local_4 = local_70;
								if (iVar6 == -1) {
									local_70[0] = 2;
									CActor::DoMessage((CActor*)this, pSender, 0x4e, (uint)local_4);
								}
								else {
									FUN_00392be0((int)pTalkParam, iVar6, (float*)local_28);
									iVar2 = FUN_00392d90((int)pTalkParam);
									if (iVar2 == 0) {
										local_2c = FUN_00392ad0(pTalkParam, iVar6);
										iVar2 = 0;
										if (iVar6 < pTalkParam->nbSubObj) {
											iVar2 = pTalkParam->field_0x8[iVar6].field_0x4;
										}
										local_40 = 5;
										if (iVar2 == 6) {
											local_40 = 0;
										}
										else {
											if (iVar2 == 5) {
												local_40 = 0xd;
											}
											else {
												if (iVar2 == 4) {
													local_40 = 9;
												}
												else {
													if (iVar2 == 3) {
														local_40 = 6;
													}
													else {
														if (iVar2 != 2) {
															local_40 = 0;
														}
													}
												}
											}
										}
										if (iVar6 < pTalkParam->nbSubObj) {
											pReceiver = pTalkParam->field_0x8[iVar6].pNativ;
										}
										else {
											pReceiver = (CActorNativ*)0x0;
										}
										local_10 = &local_40;
										CActor::DoMessage((CActor*)this, (CActor*)pReceiver, 0x4e, (uint)local_10);
									}
									else {
										pTalkParam->FUN_00392e30(&eStack80);
										local_28 = &eStack80;
										iVar6 = 0;
										if (0 < pTalkParam->nbSubObj) {
											iVar2 = 0;
											do {
												local_2c = FUN_00392ad0(pTalkParam, iVar6);
												iVar4 = pTalkParam->nbSubObj;
												pCVar5 = (CActor*)0x0;
												if (iVar6 < iVar4) {
													pCVar5 = *(CActor**)((int)&pTalkParam->field_0x8->pNativ + iVar2);
												}
												if (pCVar5 == pSender) {
													iVar3 = 0;
													if (iVar6 < iVar4) {
														iVar3 = *(int*)((int)&pTalkParam->field_0x8->field_0x4 + iVar2);
													}
													local_40 = 5;
													if (iVar3 == 6) {
														local_40 = 0;
													}
													else {
														if (iVar3 == 5) {
															local_40 = 0xd;
														}
														else {
															if (iVar3 == 4) {
																local_40 = 9;
															}
															else {
																if (iVar3 == 3) {
																	local_40 = 6;
																}
																else {
																	if (iVar3 != 2) {
																		local_40 = 0;
																	}
																}
															}
														}
													}
													local_34 = (float)FUN_00392860((int)pTalkParam, iVar6, 1);
													pCVar5 = (CActor*)0x0;
													if (iVar6 < pTalkParam->nbSubObj) {
														pCVar5 = *(CActor**)((int)&pTalkParam->field_0x8->pNativ + iVar2);
													}
													local_c = &local_40;
													CActor::DoMessage((CActor*)this, pCVar5, 0x4e, (uint)&local_40);
												}
												else {
													iVar3 = 0;
													if (iVar6 < iVar4) {
														iVar3 = *(int*)((int)&pTalkParam->field_0x8->field_0x4 + iVar2);
													}
													local_40 = 5;
													if (iVar3 == 6) {
														local_40 = 0;
													}
													else {
														if (iVar3 == 5) {
															local_40 = 0xd;
														}
														else {
															if (iVar3 == 4) {
																local_40 = 9;
															}
															else {
																if (iVar3 == 3) {
																	local_40 = 6;
																}
																else {
																	if (iVar3 != 2) {
																		local_40 = 0;
																	}
																}
															}
														}
													}
													local_34 = (float)FUN_00392860((int)pTalkParam, iVar6, 0);
													pCVar5 = (CActor*)0x0;
													if (iVar6 < pTalkParam->nbSubObj) {
														pCVar5 = *(CActor**)((int)&pTalkParam->field_0x8->pNativ + iVar2);
													}
													local_8 = &local_40;
													CActor::DoMessage((CActor*)this, pCVar5, 0x4e, (uint)&local_40);
												}
												iVar6 = iVar6 + 1;
												iVar2 = iVar2 + 8;
											} while (iVar6 < pTalkParam->nbSubObj);
										}
									}
								})
								return 1;
							}
						}
					}
				}
			}

			uVar3 = 0;
		}
	}
	else {
		uVar3 = CActor::InterpretMessage(pSender, msg, pMsgParam);
	}

	return uVar3;
}

CAddOnNativ::CAddOnNativ()
{
	this->nbSubObjs = 0;
	this->aSubObjs = (CAddOnSubObj*)0x0;

	return;
}

void CAddOnNativ::Create(ByteCode* pByteCode)
{
	uint uVar1;
	int* piVar2;
	void* pvVar3;
	int iVar4;
	int iVar7;
	int iVar9;
	float fVar10;

	this->nbSubObjs = pByteCode->GetS32();
	uVar1 = this->nbSubObjs;

	if (uVar1 != 0) {
		this->aSubObjs = new CAddOnSubObj[uVar1];
		iVar9 = 0;
		if (0 < this->nbSubObjs) {
			do {
				CAddOnSubObj* puVar9 = this->aSubObjs + iVar9;
				puVar9->field_0x0 = pByteCode->GetU32();
				puVar9->nbCinematics = pByteCode->GetS32();
				if (0 < puVar9->nbCinematics) {
					puVar9->aCinematicIds = new int[puVar9->nbCinematics];
				}

				iVar7 = 0;
				if (0 < puVar9->nbCinematics) {
					do {
						puVar9->aCinematicIds[iVar7] = pByteCode->GetS32();
						iVar7 = iVar7 + 1;
					} while (iVar7 < puVar9->nbCinematics);
				}

				puVar9->lastPlayedCinematicId = -1;
				iVar9 = iVar9 + 1;
			} while (iVar9 < this->nbSubObjs);
		}
	}
}

void CAddOnNativ::Init(CActor* pActor)
{
	CAddOn::Init(pActor);

	this->field_0xd = 0;

	return;
}

void CAddOnNativ::Manage()
{
	CAddOn::Manage();

	int iVar5;

	bool bVar1;
	if (GetCinematic()) {
		if ((GetCinematic()->state != CS_Stopped) && (bVar1 = true, this->field_0xc != 0)) goto LAB_003e1a88;
	}

	bVar1 = false;

LAB_003e1a88:

	if ((!bVar1) && (iVar5 = 0, 0 < this->nbSubObjs)) {
		do {
			CAddOnSubObj* pSubObj = this->aSubObjs + iVar5;
			if (pSubObj != (CAddOnSubObj*)0x0) {
				float fVar7 = pSubObj->field_0x14 + GetTimer()->cutsceneDeltaTime;
				pSubObj->field_0x14 = fVar7;

				if ((1.0f <= fVar7) && (pSubObj->pCinematic != (CCinematic*)0x0)) {
					pSubObj->SetCinematic((CCinematic*)0x0);
				}
			}

			iVar5 = iVar5 + 1;
		} while (iVar5 < this->nbSubObjs);
	}

	return;
}

void CAddOnNativ::Reset()
{
	CAddOn::Reset();

	this->field_0xd = 0;
	int iVar3 = 0;
	if (0 < this->nbSubObjs) {
		do {
			CAddOnSubObj* pSubObj = this->aSubObjs + iVar3;

			pSubObj->pCinematic = (CCinematic*)0x0;
			pSubObj->lastPlayedCinematicId = -1;
			pSubObj->field_0x14 = 0.0f;

			iVar3 = iVar3 + 1;
		} while (iVar3 < this->nbSubObjs);
	}

	return;
}

CAddOnSubObj* CAddOnNativ::GetSubObj(uint param_2, int param_3)
{
	CCinematic* pCVar1;
	CAddOnSubObj* pCVar2;
	bool bVar3;
	CCinematic* pCVar4;
	ulong uVar5;
	int iVar6;
	CAddOnSubObj* pCVar7;

	iVar6 = 0;
	if (0 < this->nbSubObjs) {
		pCVar7 = this->aSubObjs;
		do {
			if (param_2 == pCVar7->field_0x0) goto LAB_003e1c88;
			iVar6 = iVar6 + 1;
			pCVar7 = pCVar7 + 1;
		} while (iVar6 < this->nbSubObjs);
	}

	pCVar7 = (CAddOnSubObj*)0x0;
LAB_003e1c88:
	if (pCVar7 == (CAddOnSubObj*)0x0) {
		return (CAddOnSubObj*)0x0;
	}

	pCVar1 = pCVar7->pCinematic;
	if (pCVar1 == (CCinematic*)0x0) {
		return (CAddOnSubObj*)0x0;
	}

	if (param_3 != 0x0) {
		return pCVar7;
	}

	pCVar2 = this->pSubObj;
	pCVar4 = (CCinematic*)0x0;
	if (pCVar2 != (CAddOnSubObj*)0x0) {
		pCVar4 = pCVar2->pCinematic;
	}

	if (pCVar4 != (CCinematic*)0x0) {
		pCVar4 = (CCinematic*)0x0;
		if (pCVar2 != (CAddOnSubObj*)0x0) {
			pCVar4 = pCVar2->pCinematic;
		}
		if ((pCVar4->state != CS_Stopped) && (bVar3 = true, this->field_0xc != 0)) goto LAB_003e1d00;
	}

	bVar3 = false;
LAB_003e1d00:
	if ((((!bVar3) && (uVar5 = StaticEdFileBase_004497f0.FUN_00401f30(), uVar5 != 0)) &&
		(pCVar1->cineBankLoadStage_0x2b4 == 4)) && ((pCVar1->flags_0x8 & 0x80) == 0)) {
		return pCVar7;
	}

	return (CAddOnSubObj*)0x0;
}

bool CAddOnNativ::Func_0x20(uint param_2, CActor* param_3, int pActor)
{
	CAddOnSubObj* pCVar1;
	bool bVar2;
	CCinematic* pCinematic;

	if (param_3 == 0) {
		param_3 = this->pOwner;
	}

	pCVar1 = this->pSubObj;
	pCinematic = (CCinematic*)0x0;
	if (pCVar1 != (CAddOnSubObj*)0x0) {
		pCinematic = pCVar1->pCinematic;
	}

	if (pCinematic == (CCinematic*)0x0) {
	LAB_003e15a8:
		bVar2 = false;
	}
	else {
		pCinematic = (CCinematic*)0x0;
		if (pCVar1 != (CAddOnSubObj*)0x0) {
			pCinematic = pCVar1->pCinematic;
		}

		if ((pCinematic->state == CS_Stopped) || (bVar2 = true, this->field_0xc == 0)) goto LAB_003e15a8;
	}

	if (((bVar2) || (this->field_0xc != 0)) && (pActor == 0x0)) {
		return false;
	}

	pCinematic = (CCinematic*)0x0;
	if (pCVar1 != (CAddOnSubObj*)0x0) {
		pCinematic = pCVar1->pCinematic;
	}

	if (pCinematic != (CCinematic*)0x0) {
		pCinematic = (CCinematic*)0x0;
		if (pCVar1 != (CAddOnSubObj*)0x0) {
			pCinematic = pCVar1->pCinematic;
		}
		if ((pCinematic->state != CS_Stopped) && (bVar2 = true, this->field_0xc != 0)) goto LAB_003e1620;
	}
	bVar2 = false;
LAB_003e1620:
	if (bVar2) {
		pCinematic = (CCinematic*)0x0;
		if (pCVar1 != (CAddOnSubObj*)0x0) {
			pCinematic = pCVar1->pCinematic;
		}

		pCinematic->FUN_001c92b0();

		pCVar1 = this->pSubObj;
		pCinematic = (CCinematic*)0x0;
		if (pCVar1 != (CAddOnSubObj*)0x0) {
			pCinematic = pCVar1->pCinematic;
		}

		bVar2 = pCinematic->Has_0x2d8();
		if (bVar2 != false) {
			pCVar1 = this->pSubObj;
			pCinematic = (CCinematic*)0x0;
			if (pCVar1 != (CAddOnSubObj*)0x0) {
				pCinematic = pCVar1->pCinematic;
			}

			pCinematic->Remove_0x2d8();
		}
	}

	Func_0x34(param_2, param_3);

	pCVar1 = GetSubObj(param_2, pActor);
	if ((pCVar1 == (CAddOnSubObj*)0x0) && (pCVar1 = this->pSubObj, pCVar1 != (CAddOnSubObj*)0x0)) {
		pCVar1->SetCinematic((CCinematic*)0x0);
	}

	this->pSubObj = pCVar1;
	pCVar1 = this->pSubObj;

	pCinematic = (CCinematic*)0x0;
	if (pCVar1 != (CAddOnSubObj*)0x0) {
		pCinematic = pCVar1->pCinematic;
	}

	if (pCinematic == (CCinematic*)0x0) {
		return false;
	}

	pCinematic = (CCinematic*)0x0;
	if (pCVar1 != (CAddOnSubObj*)0x0) {
		pCinematic = pCVar1->pCinematic;
	}

	pCinematic->TryTriggerCutscene(param_3, 0);

	this->field_0xc = 1;

	return true;
}

bool CAddOnNativ::Func_0x24(uint param_2, CActor* pActor)
{
	bool bVar1;
	int iVar2;
	CAddOnSubObj* pSubObj;
	CCinematic* pCinematic;

	if (pActor == (CActor*)0x0) {
		pActor = this->pOwner;
	}

	Func_0x30(param_2);

	if (Func_0x2c(param_2) == 0) {
		bVar1 = false;
	}
	else {
		iVar2 = 0;
		if (0 < this->nbSubObjs) {
			pSubObj = this->aSubObjs;

			do {
				if (param_2 == pSubObj->field_0x0) {
					iVar2 = pSubObj->PickCinematic();
					if (iVar2 == -1) {
						pSubObj = (CAddOnSubObj*)0x0;
					}
					goto LAB_003e1878;
				}

				iVar2 = iVar2 + 1;
				pSubObj = pSubObj + 1;
			} while (iVar2 < this->nbSubObjs);
		}

		pSubObj = (CAddOnSubObj*)0x0;

	LAB_003e1878:
		if (pSubObj == (CAddOnSubObj*)0x0) {
			bVar1 = false;
		}
		else {
			pCinematic = pSubObj->pCinematic;
			bVar1 = pCinematic->Has_0x2d8();
			if (bVar1 == false) {
				pCinematic->Add_0x2d8();
			}

			bVar1 = true;
		}
	}

	return bVar1;
}

void CAddOnNativ::ClearCinematic(int index)
{
	int iVar1;
	CAddOnSubObj* this_00;

	iVar1 = 0;
	if (0 < this->nbSubObjs) {
		this_00 = this->aSubObjs;
		do {
			if (index == this_00->field_0x0) goto LAB_003e1780;
			iVar1 = iVar1 + 1;
			this_00 = this_00 + 1;
		} while (iVar1 < this->nbSubObjs);
	}

	this_00 = (CAddOnSubObj*)0x0;
LAB_003e1780:
	if ((this_00 != (CAddOnSubObj*)0x0) && (this->field_0xc == 0)) {
		this_00->SetCinematic((CCinematic*)0x0);
	}

	return;
}

bool CAddOnNativ::Func_0x2c(uint param_2)
{
	CAddOnSubObj* pSubObj;
	int curIndex;

	pSubObj = (CAddOnSubObj*)0x0;
	curIndex = 0;
	if (0 < this->nbSubObjs) {
		do {
			pSubObj = this->aSubObjs + curIndex;

			if (param_2 == pSubObj->field_0x0) break;

			curIndex = curIndex + 1;
		} while (curIndex < this->nbSubObjs);
	}

	if (pSubObj != (CAddOnSubObj*)0x0) {
		if (pSubObj->pCinematic == (CCinematic*)0x0) {
			return true;
		}

		if (pSubObj->pCinematic->Has_0x2d8() == false) {
			return true;
		}
	}

	return false;
}

void CAddOnNativ::Func_0x30(uint param_2)
{
	int curIndex;
	CAddOnSubObj* pSubObj;

	curIndex = 0;
	if (0 < this->nbSubObjs) {
		pSubObj = this->aSubObjs;
		do {

			if (param_2 == pSubObj->field_0x0) goto LAB_003e1b78;

			curIndex = curIndex + 1;
			pSubObj = pSubObj + 1;
		} while (curIndex < this->nbSubObjs);
	}

	pSubObj = (CAddOnSubObj*)0x0;
LAB_003e1b78:
	if (pSubObj != (CAddOnSubObj*)0x0) {
		pSubObj->field_0x14 = 0.0f;
	}

	return;
}

void CBehaviourNativCmdStand::Manage()
{
	CActorNativCmd* pNativCmd;
	CActorNativ* pNativ;
	S_ACTOR_STREAM_REF* pSVar3;
	int iVar4;
	int iVar5;
	int iVar6;

	pNativCmd = this->pOwner;
	iVar5 = pNativCmd->actorState;
	if ((iVar5 != 6) && (iVar5 == 5)) {
		iVar6 = 0;
		while (true) {
			pSVar3 = pNativCmd->aNativs;
			iVar4 = 0;
			if (pSVar3 != (S_ACTOR_STREAM_REF*)0x0) {
				iVar4 = pSVar3->entryCount;
			}

			if (iVar4 <= iVar6) break;

			pNativ = reinterpret_cast<CActorNativ*>(pSVar3->aEntries[iVar6].Get());

			if ((pNativ != (CActorNativ*)0x0) && ((pNativ->flags & 4) != 0)) {
				pNativCmd->FUN_00394250(pNativ);
			}

			iVar6 = iVar6 + 1;
		}

		pNativCmd->ManageDiscussion();
		pNativCmd->ManageNativWithKim();
		pNativCmd->FUN_00393620();
		pNativCmd->FUN_00393470();
	}

	pNativCmd->addOnA.Manage();
	pNativCmd->addOnB.Manage();
	pNativCmd->addOnC.Manage();
	pNativCmd->addOnD.Manage();

	return;
}

void CBehaviourNativCmdStand::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	CActorNativCmd* pCVar1;

	this->pOwner = reinterpret_cast<CActorNativCmd*>(pOwner);
	if (newState == -1) {
		pCVar1 = this->pOwner;
		pCVar1->SetState(5, -1);
	}
	else {
		pCVar1 = this->pOwner;
		pCVar1->SetState(newState, newAnimationType);
	}
	return;
}

void CActorNativCmd::ManageDiscussion()
{
	bool bVar1;
	int iVar3;
	int iVar5;
	CActor* pReceiver;
	CTalkParam* pTalkParam;
	int lVar6;
	int iVar7;
	float fVar9;
	CActorNativMsgParam_0xe local_20;

	iVar5 = this->nbTalkParams;
	iVar7 = 0;
	if (0 < iVar5) {
		do {
			pTalkParam = (CTalkParam*)0x0;
			if (iVar7 < iVar5) {
				pTalkParam = this->aTalkParams + iVar7;
			}

			if (pTalkParam->field_0x1d == 1) {
				fVar9 = pTalkParam->totalTime + GetTimer()->cutsceneDeltaTime;
				pTalkParam->totalTime = fVar9;

				bVar1 = true;
				if (fVar9 <= pTalkParam->field_0x18) {
					iVar5 = 0;
					if (0 < pTalkParam->field_0x10) {
						do {
							CActorNativ* pNativ = (CActorNativ*)0x0;
							if (iVar5 < pTalkParam->nbSubObj) {
								pNativ = pTalkParam->field_0x8[iVar5].pNativ;
							}

							if ((pNativ->flags & 4) == 0) {
								bVar1 = true;
								goto LAB_00394120;
							}

							iVar5 = iVar5 + 1;
						} while (iVar5 < pTalkParam->field_0x10);
					}

					bVar1 = false;
				}

			LAB_00394120:
				if (bVar1) {
					lVar6 = 0;
					if (0 < pTalkParam->nbSubObj) {
						do {
							pReceiver = (CActor*)0x0;
							if (lVar6 < pTalkParam->nbSubObj) {
								pReceiver = pTalkParam->field_0x8[lVar6].pNativ;
							}

							local_20.type = pTalkParam->ChooseFinishOrder(lVar6);
							local_20.field_0x8 = -1;
							if (pReceiver != (CActor*)0x0) {
								CActor::DoMessage(pReceiver, MESSAGE_NATIV_CMD, &local_20);
							}

							lVar6 = lVar6 + 1;
						} while (lVar6 < pTalkParam->nbSubObj);
					}

					pTalkParam->field_0x10 = 0;
					pTalkParam->field_0x1c = 0;
					pTalkParam->totalTime = 0.0f;
					pTalkParam->field_0x18 = 0.0f;
					pTalkParam->field_0x1d = 0;

					iVar5 = 0;
					if (0 < pTalkParam->nbSubObj) {
						do {
							pTalkParam->field_0x8[iVar5].pNativ = (CActorNativ*)0x0;
							pTalkParam->field_0x8[iVar5].field_0x4 = 1;
							iVar5 = iVar5 + 1;
						} while (iVar5 < pTalkParam->nbSubObj);
					}
				}
			}

			iVar5 = this->nbTalkParams;
			iVar7 = iVar7 + 1;
		} while (iVar7 < iVar5);
	}

	return;
}

void CActorNativCmd::ManageNativWithKim()
{
	CActorNativ* pNativ;
	CActorNativ* pActor;
	bool bVar1;
	CActorHero* pCVar2;
	CBehaviour* pCVar3;
	CAddOnSubObj* lVar4;
	int iVar5;
	int iVar6;
	int iVar7;
	float fVar8;
	float fVar9;
	float fVar10;
	float fVar11;
	edF32VECTOR4 eStack48;
	CActorNativMsgParam_0xe local_20;
	undefined4* local_4;

	iVar7 = 0;
	iVar6 = -1;
	fVar11 = 3.402823e+38;

	do {
		if (this->aNativs == (S_ACTOR_STREAM_REF*)0x0) {
			iVar5 = 0;
		}
		else {
			iVar5 = this->aNativs->entryCount;
		}
		if (iVar5 <= iVar7) {
			iVar7 = 0;
			while (true) {
				if (this->aNativs == (S_ACTOR_STREAM_REF*)0x0) {
					iVar5 = 0;
				}
				else {
					iVar5 = this->aNativs->entryCount;
				}

				if (iVar5 <= iVar7) break;

				pActor = (CActorNativ*)this->aNativs->aEntries[iVar7].Get();
				if ((((pActor != (CActor*)0x0) && (pCVar3 = pActor->GetBehaviour(NATIVE_BEHAVIOUR_SPEAK), pCVar3 != (CBehaviour*)0x0)) && ((pActor->flags & 4) != 0)) && pActor->field_0x504 != 0) {
					FUN_00394d00(pActor);

					if (iVar7 == iVar6) {
						local_20.type = NATIVE_MESSAGE_FACE_TOWARDS;
					}
					else {
						local_20.type = NATIVE_MESSAGE_FACE_AWAY;
					}

					DoMessage(this->aNativs->aEntries[iVar7].Get(), MESSAGE_NATIV_CMD, &local_20);
				}
				iVar7 = iVar7 + 1;
			}
			return;
		}

		pNativ = (CActorNativ*)this->aNativs->aEntries[iVar7].Get();
		fVar10 = fVar11;
		iVar5 = iVar6;
		if (((pNativ != (CActorNativ*)0x0) && (pCVar3 = pNativ->GetBehaviour(3), pCVar3 != (CBehaviour*)0x0)) && (((pNativ->flags & 4) != 0 && (pNativ->field_0x504 != 0)))) {
			if (pNativ == (CActorNativ*)0x0) {
			LAB_00393de0:
				bVar1 = false;
			}
			else {
				if (pNativ->field_0x378 == 1) {
					lVar4 = this->addOnB.GetSubObj(pNativ->FUN_00162a70(), 0x0);
				}
				else {
					lVar4 = this->addOnA.GetSubObj(pNativ->FUN_00162a70(), 0x0);
				}

				bVar1 = true;

				if (lVar4 == 0) goto LAB_00393de0;
			}

			pCVar2 = CActorHero::_gThis;
			if (bVar1) {
				edF32Vector4SubHard(&eStack48, &CActorHero::_gThis->currentLocation, &pNativ->currentLocation);
				edF32Vector4SafeNormalize0Hard(&eStack48, &eStack48);
				fVar10 = pCVar2->currentLocation.x - pNativ->currentLocation.x;
				fVar8 = pCVar2->currentLocation.y - pNativ->currentLocation.y;
				fVar9 = pCVar2->currentLocation.z - pNativ->currentLocation.z;
				fVar10 = sqrtf(fVar10 * fVar10 + fVar8 * fVar8 + fVar9 * fVar9);
				fVar8 = edF32Vector4DotProductHard(&pNativ->rotationQuat, &eStack48);
				fVar9 = edF32Vector4DotProductHard(&pCVar2->rotationQuat, &eStack48);
				if (((fVar8 <= 0.0f) || (0.0f <= fVar9)) || (3.0f <= fVar10)) {
					fVar10 = 3.402823e+38f;
				}

				iVar5 = iVar7;
				if (fVar11 <= fVar10) {
					fVar10 = fVar11;
					iVar5 = iVar6;
				}
			}
		}

		iVar6 = iVar5;
		iVar7 = iVar7 + 1;
		fVar11 = fVar10;
	} while (true);
}

void CActorNativCmd::FUN_00393620()
{
	CActorNativ* pReceiver;
	S_ACTOR_STREAM_REF* pSVar1;
	bool bVar2;
	int iVar3;
	StateConfig* pSVar4;
	edF32VECTOR4* lVar5;
	uint uVar6;
	int iVar7;
	CTalkParam* pCVar8;
	int iVar9;
	int iVar10;
	int iVar11;
	CActorNativ* pNativ;
	int iVar13;
	float fVar15;
	CActorNativMsgParam_0xe local_70;
	CActorNativMsgParam_0xe local_50;
	edF32VECTOR4 eStack48;
	edF32VECTOR4 eStack32;
	undefined4* local_8;
	undefined4* local_4;

	iVar13 = 0;
	do {
		pSVar1 = this->aNativs;
		iVar3 = 0;
		if (pSVar1 != (S_ACTOR_STREAM_REF*)0x0) {
			iVar3 = pSVar1->entryCount;
		}

		if (iVar3 <= iVar13) {
			return;
		}

		pReceiver = static_cast<CActorNativ*>(pSVar1->aEntries[iVar13].Get());

		if (pReceiver != (CActorNativ*)0x0) {
			uVar6 = pReceiver->GetStateFlags(pReceiver->actorState);

			if ((uVar6 & 0x20000) != 0) {
				lVar5 = FUN_00393230(pReceiver);
				if (lVar5 == 0) {
					local_70.type = 0xf;
				}
				else {
					local_70.type = 0xe;
					local_70.field_0x18 = lVar5;
				}

				DoMessage(pReceiver, MESSAGE_NATIV_CMD, &local_70);
			}
		}

		if (pReceiver != (CActorNativ*)0x0) {
			uVar6 = pReceiver->GetStateFlags(pReceiver->actorState) & 0x8000;

			if (uVar6 != 0) {
				if (pReceiver->FUN_00162750() != 0) {
					iVar3 = 0;
					iVar11 = 0;
					while (true) {
						pSVar1 = this->aNativs;
						iVar10 = 0;
						if (pSVar1 != (S_ACTOR_STREAM_REF*)0x0) {
							iVar10 = pSVar1->entryCount;
						}

						iVar9 = -1;

						if (iVar10 <= iVar3) break;

						pNativ = static_cast<CActorNativ*>(this->aNativs->aEntries[iVar3].Get());

						uVar6 = pNativ->GetStateFlags(pNativ->actorState);

						if (((uVar6 & 0x10000) != 0) && (pReceiver != pNativ)) {
							iVar10 = pNativ->FUN_00162750();
							if (iVar10 != 0) {
								edF32Vector4SubHard(&eStack32, &pNativ->currentLocation, &pReceiver->currentLocation);
								fVar15 = edF32Vector4GetDistHard(&eStack32);
								iVar9 = iVar3;

								if (fVar15 <= 2.2f) break;
							}
						}

						iVar3 = iVar3 + 1;
					}

					if (iVar9 != -1) {
						iVar3 = this->nbTalkParams;
						iVar11 = 0;

						if (0 < iVar3) {
							do {
								CTalkParam* pTalkParam = (CTalkParam*)0x0;
								if (iVar11 < iVar3) {
									pTalkParam = this->aTalkParams + iVar11;
								}

								if (((pTalkParam->field_0x10 != 0) || (pTalkParam->totalTime != 0.0f)) ||
									(bVar2 = true, pTalkParam->field_0x1c != 0)) {
									bVar2 = false;
								}

								if (bVar2) goto LAB_003938b8;

								iVar11 = iVar11 + 1;
							} while (iVar11 < iVar3);
						}

						iVar11 = -1;
					LAB_003938b8:
						if (iVar11 == -1) {
							return;
						}

						pNativ = static_cast<CActorNativ*>(this->aNativs->aEntries[iVar9].Get());

						if (iVar11 < iVar3) {
							pCVar8 = this->aTalkParams + iVar11;
						}
						else {
							pCVar8 = (CTalkParam*)0x0;
						}

						if (pCVar8->field_0x10 + 1 <= pCVar8->nbSubObj) {
							pCVar8->field_0x8[pCVar8->field_0x10].pNativ = pReceiver;
							pCVar8->field_0x10 = pCVar8->field_0x10 + 1;
						}

						pCVar8 = (CTalkParam*)0x0;
						if (iVar11 < this->nbTalkParams) {
							pCVar8 = this->aTalkParams + iVar11;
						}

						if (pCVar8->field_0x10 + 1 <= pCVar8->nbSubObj) {
							pCVar8->field_0x8[pCVar8->field_0x10].pNativ = pNativ;
							pCVar8->field_0x10 = pCVar8->field_0x10 + 1;
						}

						if (iVar11 < this->nbTalkParams) {
							pCVar8 = this->aTalkParams + iVar11;
						}
						else {
							pCVar8 = (CTalkParam*)0x0;
						}

						pCVar8->field_0x1c = 1;

						if (iVar11 < this->nbTalkParams) {
							pCVar8 = this->aTalkParams + iVar11;
						}
						else {
							pCVar8 = (CTalkParam*)0x0;
						}

						pCVar8->field_0x18 = 5.0f;

						pCVar8 = (CTalkParam*)0x0;
						if (iVar11 < this->nbTalkParams) {
							pCVar8 = this->aTalkParams + iVar11;
						}

						if (0 < pCVar8->nbSubObj) {
							pCVar8->field_0x8->field_0x4 = 5;
						}

						pCVar8 = (CTalkParam*)0x0;
						if (iVar11 < this->nbTalkParams) {
							pCVar8 = this->aTalkParams + iVar11;
						}

						if (1 < pCVar8->nbSubObj) {
							pCVar8->field_0x8[1].field_0x4 = 5;
						}

						if (iVar11 < this->nbTalkParams) {
							pCVar8 = this->aTalkParams + iVar11;
						}
						else {
							pCVar8 = (CTalkParam*)0x0;
						}

						pCVar8->field_0x1d = 1;

						if (iVar11 < this->nbTalkParams) {
							pCVar8 = this->aTalkParams + iVar11;
						}
						else {
							pCVar8 = (CTalkParam*)0x0;
						}

						pCVar8->FUN_00392e30(&eStack48);

						iVar10 = 0;
						iVar3 = 0;
						while (true) {
							iVar9 = this->nbTalkParams;
							iVar7 = 0xc;
							if (iVar11 < iVar9) {
								iVar7 = this->aTalkParams[iVar11].nbSubObj;
							}

							if (iVar7 <= iVar10) break;

							pCVar8 = (CTalkParam*)0x0;
							if (iVar11 < iVar9) {
								pCVar8 = this->aTalkParams + iVar11;
							}

							pNativ = (CActorNativ*)0x0;
							if (iVar10 < pCVar8->nbSubObj) {
								pNativ = pCVar8->field_0x8[iVar10].pNativ;
							}

							pCVar8 = (CTalkParam*)0x0;
							if (iVar11 < iVar9) {
								pCVar8 = this->aTalkParams + iVar11;
							}

							iVar9 = 0;
							if (iVar10 < pCVar8->nbSubObj) {
								iVar9 = pCVar8->field_0x8[iVar10].field_0x4;
							}

							local_50.type = 5;
							if (iVar9 == 6) {
								local_50.type = 0;
							}
							else {
								if (iVar9 == 5) {
									local_50.type = 0xd;
								}
								else {
									if (iVar9 == 4) {
										local_50.type = 9;
									}
									else {
										if (iVar9 == 3) {
											local_50.type = 6;
										}
										else {
											if (iVar9 != 2) {
												local_50.type = 0;
											}
										}
									}
								}
							}

							local_50.field_0x18 = &eStack48;
							pCVar8 = (CTalkParam*)0x0;
							if (iVar11 < this->nbTalkParams) {
								pCVar8 = this->aTalkParams + iVar11;
							}

							local_50.field_0x8 = iVar11;
							local_50.field_0x14 = pCVar8->FUN_00392ad0(iVar10);

							if (pNativ == pReceiver) {
								pCVar8 = (CTalkParam*)0x0;
								if (iVar11 < this->nbTalkParams) {
									pCVar8 = this->aTalkParams + iVar11;
								}

								local_50.field_0xc = pCVar8->FUN_00392860(iVar10, 1);
							}
							else {
								pCVar8 = (CTalkParam*)0x0;
								if (iVar11 < this->nbTalkParams) {
									pCVar8 = this->aTalkParams + iVar11;
								}

								local_50.field_0xc = pCVar8->FUN_00392860(iVar10, 0);
							}

							DoMessage(pNativ, MESSAGE_NATIV_CMD, &local_50);
							iVar10 = iVar10 + 1;
						}
					}
				}
			}
		}

		iVar13 = iVar13 + 1;
	} while (true);
}


void CActorNativCmd::FUN_00393470()
{
	S_ACTOR_STREAM_REF* pSVar1;
	CBehaviour* pCVar2;
	int iVar3;
	uint uVar5;
	uint uVar6;
	int iVar7;
	uint uVar8;
	CActorNativ* pNativ;

	uVar6 = 0;
	uVar8 = 0;
	iVar7 = 0;
	while (true) {
		pSVar1 = this->aNativs;
		iVar3 = 0;
		if (pSVar1 != (S_ACTOR_STREAM_REF*)0x0) {
			iVar3 = pSVar1->entryCount;
		}

		if (iVar3 <= iVar7) break;

		pNativ = static_cast<CActorNativ*>(pSVar1->aEntries[iVar7].Get());
		pCVar2 = pNativ->GetBehaviour(NATIVE_BEHAVIOUR_SPEAK);
		if ((pCVar2 != (CBehaviour*)0x0) && (pNativ->InZone_00162970(CActorHero::_gThis) != false)) {
			uVar5 = pNativ->FUN_00162a70();
			uVar5 = 1 << (uVar5 & 0x1f);

			if (pNativ->field_0x378 == 1) {
				uVar8 = uVar8 | uVar5;
			}
			else {
				uVar6 = uVar6 | uVar5;
			}
		}

		iVar7 = iVar7 + 1;
	}

	uVar5 = 0;
	do {
		if ((uVar6 & 1 << (uVar5 & 0x1f)) == 0) {
			this->addOnA.ClearCinematic(uVar5);
		}
		else {
			this->addOnA.Func_0x24(uVar5, 0);
		}
		uVar5 = uVar5 + 1;
	} while (uVar5 < 0x11);

	uVar6 = 0;
	do {
		if ((uVar8 & 1 << (uVar6 & 0x1f)) == 0) {
			this->addOnB.ClearCinematic(uVar6);
		}
		else {
			this->addOnB.Func_0x24(uVar6, 0);
		}

		uVar6 = uVar6 + 1;
	} while (uVar6 < 0x11);

	return;
}



void CActorNativCmd::FUN_00394250(CActorNativ* pNativ)
{
	int iVar1;
	CTakePutTrajectoryParam* pCVar2;
	StateConfig* pSVar3;
	int iVar4;
	CPathFinderClient* lVar5;
	uint uVar6;
	ulong uVar7;
	ActionEntry* pAction;
	int iVar9;
	int iVar10;
	float fVar11;
	float fVar12;
	CActorNativMsgParam_0xe local_50;
	undefined4 local_30[10];
	undefined4* local_8;
	undefined4* local_4;

	if ((pNativ != (CActorNativ*)0x0) && (pNativ->field_0x3f0 == this)) {
		iVar9 = pNativ->actorState;
		if (iVar9 == -1) {
			uVar6 = 0;
		}
		else {
			pSVar3 = pNativ->GetStateCfg(iVar9);
			uVar6 = pSVar3->flags_0x4 & 0x800;
		}

		if (uVar6 != 0) {
			fVar11 = pNativ->FUN_00164070();
			iVar9 = -1;
			local_50.field_0x4 = 0;
			iVar10 = 0;
			fVar12 = 0.0f;
			fVar11 = (fVar11 * (float)CScene::Rand()) / 32767.0f;
			if (0 < pNativ->nbActions) {
				do {
					pAction = pNativ->aActions + iVar10;
					iVar1 = pAction->field_0x0;
					if (iVar9 != iVar1) {
						local_50.field_0x4 = 0;
						iVar9 = iVar1;
					}

					if (iVar1 == 2) {
						pCVar2 = (pNativ->behaviourTakeAndPut).aTrajectoryParams;
						lVar5 = pNativ->GetPathfinderClient();
						iVar4 = pCVar2[local_50.field_0x4].IsWayPointValidPosition(lVar5);
						if (iVar4 != 0) {
							fVar12 = fVar12 + pAction->field_0x4;
						}
					}
					else {
						fVar12 = fVar12 + (float)pAction->field_0x4;
					}

					if (fVar11 < fVar12) {
						if (iVar1 == 2) {
							local_50.type = 4;
							DoMessage(pNativ, MESSAGE_NATIV_CMD, &local_50);
							return;
						}
						if (iVar1 != 1) {
							if (iVar1 != 0) {
								return;
							}
							local_4 = local_30;
							local_30[0] = 2;
							DoMessage(pNativ, MESSAGE_NATIV_CMD, local_4);
							return;
						}

						IMPLEMENTATION_GUARD(
						FUN_003944e0(pNativ, local_50.field_0x4);)
						return;
					}
					iVar10 = iVar10 + 1;
					local_50.field_0x4 = local_50.field_0x4 + 1;
				} while (iVar10 < pNativ->nbActions);
			}
		}
	}

	return;
}


void CActorNativCmd::FUN_00394d00(CActorNativ* pNativ)
{
	CActorNativ* pCVar1;
	CActor* pReceiver;
	int iVar3;
	int iVar4;
	CTalkParam* pCVar5;
	CActorNativMsgParam_0xe local_20;
	undefined4* local_4;

	iVar4 = (pNativ->behaviourSpeak).field_0x24;
	if (iVar4 != -1) {
		pCVar5 = (CTalkParam*)0x0;

		if (iVar4 < this->nbTalkParams) {
			pCVar5 = this->aTalkParams + iVar4;
		}

		if (pCVar5->field_0x1c != 0) {
			iVar4 = pCVar5->nbSubObj;
			iVar3 = 0;

			if (0 < iVar4) {
				do {
					pCVar1 = (CActorNativ*)0x0;
					if (iVar3 < iVar4) {
						pCVar1 = pCVar5->field_0x8[iVar3].pNativ;
					}

					if (pCVar1 == pNativ) goto LAB_00394db8;

					iVar3 = iVar3 + 1;
				} while (iVar3 < pCVar5->nbSubObj);
			}

			iVar3 = -1;
		LAB_00394db8:
			if (iVar3 < iVar4) {
				pCVar5->field_0x8[iVar3].field_0x4 = 6;
			}
			if (pCVar5->field_0x1d != 0) {
				pCVar5->field_0x1d = 0;
				iVar4 = 0;
				if (0 < pCVar5->field_0x10) {
					do {
						CActorNativ* iVar2 = (CActorNativ*)0x0;
						if (iVar4 < pCVar5->nbSubObj) {
							iVar2 = pCVar5->field_0x8[iVar4].pNativ;
						}

						if (iVar2->field_0x504 == 0) {
							if (iVar4 < pCVar5->nbSubObj) {
								pCVar5->field_0x8[iVar4].field_0x4 = 3;
							}

							local_20.type = 7;
							pReceiver = (CActor*)0x0;
							if (iVar4 < pCVar5->nbSubObj) {
								pReceiver = pCVar5->field_0x8[iVar4].pNativ;
							}

							DoMessage(pReceiver, MESSAGE_NATIV_CMD, &local_20);
						}
						iVar4 = iVar4 + 1;
					} while (iVar4 < pCVar5->field_0x10);
				}
			}
		}
	}
	return;
}

edF32VECTOR4* CActorNativCmd::FUN_00393230(CActor* pActor)
{
	//CBehaviourFighterVTable* pCVar1;
	S_ACTOR_STREAM_REF* piVar2;
	CBehaviour* pCVar3;
	StateConfig* pSVar4;
	uint uVar5;
	int iVar6;
	edF32VECTOR4* peVar7;
	int iVar8;
	int iVar9;
	CActorHero* pCVar10;
	CActorHero* pCVar11;
	CActorHero* pCVar12;
	float fVar13;
	float fVar14;
	float fVar15;
	float fVar16;
	edF32VECTOR4 eStack16;
	CTalkParam* pCVar7;

	fVar16 = 3.402823e+38f;
	pCVar3 = pActor->GetBehaviour(NATIVE_BEHAVIOUR_SPEAK);
	if (pCVar3 != (CBehaviour*)0x0) {
		CBehaviourNativSpeak* pSpeakBehaviour = static_cast<CBehaviourNativSpeak*>(pActor->GetBehaviour(NATIVE_BEHAVIOUR_SPEAK));
		uVar5 = pActor->GetStateFlags(pActor->actorState);

		if ((uVar5 & 0x2000) != 0) {
			return &CActorHero::_gThis->currentLocation;
		}

		iVar8 = pSpeakBehaviour->field_0x24;
		if (-1 < iVar8) {
			if (iVar8 < this->nbTalkParams) {
				pCVar7 = this->aTalkParams + iVar8;
			}
			else {
				pCVar7 = (CTalkParam*)0x0;
			}

			if (((pCVar7 != (CTalkParam*)0x0) && (pCVar7->field_0x1c != 0)) && (pCVar7->field_0x1d != 0)) {
				return &pSpeakBehaviour->field_0x10;
			}
		}
	}

	iVar9 = 0;
	pCVar11 = (CActorHero*)0x0;
	while (true) {
		piVar2 = this->aNativs;
		iVar6 = 0;
		if (piVar2 != (S_ACTOR_STREAM_REF*)0x0) {
			iVar6 = piVar2->entryCount;
		}

		if (iVar6 <= iVar9) break;

		pCVar12 = static_cast<CActorHero*>(piVar2->aEntries[iVar9].Get());
		fVar14 = fVar16;
		pCVar10 = pCVar11;
		if ((CActorHero*)pActor != pCVar12) {
			edF32Vector4SubHard(&eStack16, &pCVar12->currentLocation, &pActor->currentLocation);
			fVar15 = edF32Vector4NormalizeHard(&eStack16, &eStack16);
			fVar13 = edF32Vector4DotProductHard(&eStack16, &pActor->rotationQuat);
			fVar14 = fVar16;
			if ((fVar15 <= fVar16) && (fVar14 = fVar15, pCVar10 = pCVar12, fVar13 <= 0.7071)) {
				fVar14 = fVar16;
				pCVar10 = pCVar11;
			}
		}

		iVar9 = iVar9 + 1;
		fVar16 = fVar14;
		pCVar11 = pCVar10;
	}

	edF32Vector4SubHard(&eStack16, &CActorHero::_gThis->currentLocation, &pActor->currentLocation);
	fVar14 = edF32Vector4NormalizeHard(&eStack16, &eStack16);
	fVar15 = edF32Vector4DotProductHard(&eStack16, &pActor->rotationQuat);
	pCVar12 = pCVar11;

	if ((fVar14 <= fVar16) && (pCVar12 = CActorHero::_gThis, fVar15 <= 0.7071f)) {
		pCVar12 = pCVar11;
	}

	peVar7 = (edF32VECTOR4*)0x0;
	if (pCVar12 != (CActorHero*)0x0) {
		peVar7 = &pCVar12->currentLocation;
	}
	return peVar7;
}

void CTalkParam::FUN_00392e30(edF32VECTOR4* param_2)
{
	int iVar1;
	int iVar2;
	int iVar4;
	float fVar5;

	param_2->x = 0.0f;
	fVar5 = 0.0f;
	param_2->y = 0.0f;
	param_2->z = 0.0f;
	param_2->w = 1.0f;
	iVar2 = this->nbSubObj;
	iVar4 = 0;
	if (0 < iVar2) {
		do {
			iVar1 = 0;
			if (iVar4 < iVar2) {
				iVar1 = this->field_0x8[iVar4].field_0x4;
			}

			if (iVar1 == 4) {
			LAB_00392ee8:
				CActorNativ* pNativ = (CActorNativ*)0x0;
				if (iVar4 < iVar2) {
					pNativ = this->field_0x8[iVar4].pNativ;
				}

				edF32Vector4AddHard(param_2, param_2, &pNativ->currentLocation);
				fVar5 = fVar5 + 1.0f;
			}
			else {
				iVar1 = 0;
				if (iVar4 < iVar2) {
					iVar1 = this->field_0x8[iVar4].field_0x4;
				}

				if (iVar1 == 3) goto LAB_00392ee8;

				iVar1 = 0;
				if (iVar4 < iVar2) {
					iVar1 = this->field_0x8[iVar4].field_0x4;
				}

				if (iVar1 == 5) goto LAB_00392ee8;
			}

			iVar2 = this->nbSubObj;
			iVar4 = iVar4 + 1;
		} while (iVar4 < iVar2);
	}

	if (fVar5 != 0.0f) {
		edF32Vector4ScaleHard(1.0f / fVar5, param_2, param_2);
	}

	param_2->w = 1.0f;

	return;
}


float CTalkParam::FUN_00392ad0(int index)
{
	int iVar1;
	float fVar2;

	iVar1 = 0;
	if (index < this->nbSubObj) {
		iVar1 = this->field_0x8[index].field_0x4;
	}

	if (iVar1 == 6) {
		fVar2 = 2.2f;
	}
	else {
		if (iVar1 == 5) {
			fVar2 = 1.1f;
		}
		else {
			if (iVar1 == 4) {
				fVar2 = 1.1f;
			}
			else {
				if (iVar1 == 3) {
					fVar2 = 2.2f;
				}
				else {
					if (iVar1 == 2) {
						if (index == 1) {
							fVar2 = 0.5f;
						}
						else {
							fVar2 = 2.2f;
						}
					}
					else {
						fVar2 = 2.2f;
					}
				}
			}
		}
	}

	return fVar2;
}

int CTalkParam::FUN_00392860(int index, int param_3)
{
	int iVar1;

	iVar1 = 0;
	if (index < this->nbSubObj) {
		iVar1 = this->field_0x8[index].field_0x4;
	}
	if (iVar1 == 6) {
		iVar1 = -1;
	}
	else {
		if (iVar1 == 5) {
			if (param_3 == 0) {
				iVar1 = ((int)((CScene::Rand()) * 3) >> 0xf) + 2;
			}
			else {
				iVar1 = (int)((CScene::Rand()) * 2) >> 0xf;
			}
		}
		else {
			if (iVar1 == 4) {
				if (param_3 == 0) {
					iVar1 = ((int)((CScene::Rand()) * 3) >> 0xf) + 2;
				}
				else {
					iVar1 = (int)((CScene::Rand()) * 2) >> 0xf;
				}
			}
			else {
				if (iVar1 == 3) {
					iVar1 = -1;
				}
				else {
					if (iVar1 == 2) {
						iVar1 = -1;
					}
					else {
						iVar1 = -1;
					}
				}
			}
		}
	}

	return iVar1;
}

int CTalkParam::ChooseFinishOrder(int index)
{
	float fVar1;
	float fVar2;
	int iVar3;
	CActorNativ* pCVar4;
	CActorNativ* pCVar5;
	int iVar6;
	float fVar7;

	if (index < this->nbSubObj) {
		iVar6 = this->field_0x8[index].field_0x4;
	}
	else {
		iVar6 = 0;
	}

	if (iVar6 == 6) {
		iVar3 = 2;
	}
	else {
		if (iVar6 == 5) {
			iVar3 = 3;
		}
		else {
			if (iVar6 == 4) {
				if (index < this->nbSubObj) {
					pCVar4 = this->field_0x8[index].pNativ;
				}
				else {
					pCVar4 = (CActorNativ*)0x0;
				}

				if (index < this->nbSubObj) {
					pCVar5 = this->field_0x8[index].pNativ;
				}
				else {
					pCVar5 = (CActorNativ*)0x0;
				}

				fVar1 = (pCVar4->field_0x570).x - pCVar5->currentLocation.x;
				fVar2 = (pCVar4->field_0x570).z - pCVar5->currentLocation.z;

				if (index < this->nbSubObj) {
					pCVar4 = this->field_0x8[index].pNativ;
				}
				else {
					pCVar4 = (CActorNativ*)0x0;
				}

				fVar7 = pCVar4->FUN_00120250(0.0f);
				iVar3 = 0xc;
				if (sqrtf(fVar1 * fVar1 + 0.0f + fVar2 * fVar2) <= fVar7) {
					iVar3 = 2;
				}
			}
			else {
				if (iVar6 == 3) {
					if (index < this->nbSubObj) {
						pCVar4 = this->field_0x8[index].pNativ;
					}
					else {
						pCVar4 = (CActorNativ*)0x0;
					}

					if (index < this->nbSubObj) {
						pCVar5 = this->field_0x8[index].pNativ;
					}
					else {
						pCVar5 = (CActorNativ*)0x0;
					}

					fVar1 = (pCVar4->field_0x570).x - pCVar5->currentLocation.x;
					fVar2 = (pCVar4->field_0x570).z - pCVar5->currentLocation.z;

					if (index < this->nbSubObj) {
						pCVar4 = this->field_0x8[index].pNativ;
					}
					else {
						pCVar4 = (CActorNativ*)0x0;
					}

					fVar7 = pCVar4->FUN_00120250(0.0f);
					iVar3 = 0xc;
					if (sqrtf(fVar1 * fVar1 + 0.0f + fVar2 * fVar2) <= fVar7) {
						iVar3 = 2;
					}
				}
				else {
					iVar3 = 2;
					if (iVar6 == 2) {
						if (index < this->nbSubObj) {
							pCVar4 = this->field_0x8[index].pNativ;
						}
						else {
							pCVar4 = (CActorNativ*)0x0;
						}

						if (index < this->nbSubObj) {
							pCVar5 = this->field_0x8[index].pNativ;
						}
						else {
							pCVar5 = (CActorNativ*)0x0;
						}

						fVar1 = (pCVar4->field_0x570).x - pCVar5->currentLocation.x;
						fVar2 = (pCVar4->field_0x570).z - pCVar5->currentLocation.z;
						if (index < this->nbSubObj) {
							pCVar4 = this->field_0x8[index].pNativ;
						}
						else {
							pCVar4 = (CActorNativ*)0x0;
						}

						fVar7 = pCVar4->FUN_00120250(0.0f);
						iVar3 = 0xc;
						if (sqrtf(fVar1 * fVar1 + 0.0f + fVar2 * fVar2) <= fVar7) {
							iVar3 = 2;
						}
					}
				}
			}
		}
	}

	return iVar3;
}

