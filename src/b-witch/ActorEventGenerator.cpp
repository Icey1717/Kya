#include "ActorEventGenerator.h"
#include "MemoryStream.h"
#include "TimeController.h"
#include "EventManager.h"
#include "ActorHero.h"
#include "CinematicManager.h"

CActorEventGenerator* CActorEventGenerator::gGlobalEvG = NULL;
CActorEventGenerator* CActorEventGenerator::gActiveEvG = NULL;

int PTMF_Ext::CheckComboA(EVC_PHASE phase, undefined8 param_3, void* pData)
{
	CActorFighter** pMVar1;
	s_fighter_combo* psVar2;
	CActorHero* pHero;
	bool bVar3;
	CActorFighter* pAdversary;

	pHero = CActorHero::_gThis;

	pMVar1 = reinterpret_cast<CActorFighter**>(this->pData);
	if (phase == EVC_PHASE_7) {
		pAdversary = *pMVar1;
	}
	else {
		if (phase == EVC_PHASE_RESET) {
			*pMVar1 = (CActorFighter*)0x0;
		}
		else {
			if ((phase == EVC_PHASE_MANAGE) && (pAdversary = CActorHero::_gThis->pAdversary, pAdversary != (CActorFighter*)0x0)) {
				if ((pAdversary->GetLifeInterface()->GetValue() <= 0.0f) && (pHero->FUN_0031b790(pHero->actorState) != false)) {
					psVar2 = pHero->pFighterCombo;
					bVar3 = false;
					if (psVar2 != (s_fighter_combo*)0x0) {
						if ((psVar2->field_0x4.field_0x0ushort & 0x400) == 0) {
							if (((pHero->pBlow)->field_0x4.field_0x2ushort & 1) != 0) {
								bVar3 = true;
							}
						}
						else {
							IMPLEMENTATION_GUARD(
							if (((pHero->field_0x840)->field_0x30).z == 2.802597e-45) {
								bVar3 = true;
							})
						}
					}

					if ((bVar3) && (*pMVar1 != pAdversary)) {
						*pMVar1 = pAdversary;
						return 1;
					}
				}
			}
		}

		pAdversary = (CActorFighter*)0x0;
	}

	return (int)pAdversary;
}

int PTMF_Ext::ManageActorA(EVC_PHASE phase, undefined8 param_3, void* pData)
{
	S_STREAM_REF<CActor>* pActor;

	pActor = reinterpret_cast<S_STREAM_REF<CActor>*>(this->pData);
	if (phase == EVC_PHASE_MANAGE) {
		if ((pActor->Get()->IsKindOfObject(OBJ_TYPE_WOLFEN) != false) && (pActor->Get()->actorState == 0x79)) {
			return 1;
		}
	}
	else {
		if (phase == EVC_PHASE_INIT) {
			pActor->Init();
		}
		else {
			if (phase == EVC_PHASE_CREATE) {
				ByteCode* pByteCode = reinterpret_cast<ByteCode*>(pData);
				pActor->index = pByteCode->GetS32();
			}
		}
	}

	return 0;
}

int PTMF_Ext::ManageActorB(EVC_PHASE phase, undefined8 param_3, void* pData)
{
	ManageMsgBParams* pParams;

	pParams = reinterpret_cast<ManageMsgBParams*>(this->pData);
	if (phase == EVC_PHASE_MANAGE) {
		const bool bWolfen = pParams->actorRef.Get()->IsKindOfObject(OBJ_TYPE_WOLFEN);
		if (bWolfen != false) {
			const bool bVar1 = static_cast<CActorFighter*>(pParams->actorRef.Get())->FUN_001740a0();
			if (bVar1 == false) {
				pParams->bActive = 0;
			}
			else {
				if (pParams->bActive == 0x0) {
					pParams->bActive = 1;
					return 1;
				}
			}
		}
	}
	else {
		if ((phase == (EVC_PHASE_3)) || (phase == EVC_PHASE_RESET)) {
			pParams->bActive = 0;
		}
		else {
			if (phase == EVC_PHASE_INIT) {
				pParams->actorRef.Init();
				pParams->bActive = 0;
			}
			else {
				if (phase == EVC_PHASE_CREATE) {
					pParams->actorRef.index = reinterpret_cast<ByteCode*>(pData)->GetS32();
				}
			}
		}
	}

	return 0;
}

int PTMF_Ext::ManageMsgReceived(EVC_PHASE phase, undefined8 param_3, void* pData)
{
	ManageMsgAParams* pParams;

	pParams = reinterpret_cast<ManageMsgAParams*>(this->pData);

	if (phase == EVC_PHASE_INIT) {
		pParams->actorRef.Init();
	}
	else {
		if (phase == EVC_PHASE_CREATE) {
			ByteCode* pByteCode = reinterpret_cast<ByteCode*>(pData);
			pParams->msgId = pByteCode->GetU32();
			pParams->actorRef.index = pByteCode->GetS32();
		}
	}

	return 0;
}

int PTMF_Ext::DummyTempFunc(EVC_PHASE phase, undefined8 param_3, void* pData)
{
	IMPLEMENTATION_GUARD();
	return 0;
}

int PTMF_Ext::__ptmf_scall_NOT(PTMF_Ext::EVC_PHASE phase, uint param_3, void* pData)
{
	uint uVar1;
	uint uVar2;
	uint uVar3;
	uint result = 0;

	if (phase < 8) {
		switch (phase) {
		case EVC_PHASE_CREATE:
		{
			ByteCode* pByteCode = reinterpret_cast<ByteCode*>(pData);
			uVar1 = pByteCode->GetU32();
			if (uVar1 != 0) {
				this->flagA = this->flagA | 1;
			}

			for (; uVar1 != 0; uVar1 = uVar1 - 1) {
				this->field_0x0 = pByteCode->GetU32();
				(this->*this->ptmf)(EVC_PHASE_CREATE, 0, pByteCode);
			}

			this->flagB = this->flagA;
		}
			break;
		case EVC_PHASE_INIT:
			if ((this->flagB & 1) != 0) {
				(this->*this->ptmf)(EVC_PHASE_INIT, 0, 0);
			}
			break;
		case EVC_PHASE_MANAGE:
			result = param_3;
			if ((this->flagB & 1) != 0) {
				if ((this->*this->ptmf)(EVC_PHASE_MANAGE, 0, 0) != 0) {
					this->flagB = this->flagB | 2;
				}

				uVar1 = this->field_0x0;
				result = 0;
				uVar3 = (uint)((this->flagB & 2) != 0);
				if (uVar1 == 2) {
					result = param_3 ^ uVar3;
				}
				else {
					if (uVar1 == 1) {
						result = param_3 | uVar3;
					}
					else {
						if (uVar1 == 0) {
							result = param_3 & uVar3;
						}
					}
				}
			}
			break;
		case 3:
			if ((this->flagB & 1) != 0) {
				(this->*this->ptmf)((PTMF_Ext::EVC_PHASE)3, 0, 0);
			}
			break;
		case EVC_PHASE_RESET:
			result = param_3;
			if ((this->flagB & 1) != 0) {
				this->flagB = this->flagA;
				result = (this->*this->ptmf)(EVC_PHASE_RESET, 0, 0);
			}
			break;
		case 5:
			result = param_3;
			if ((this->flagB & 1) != 0) {				
				result = result | (this->*this->ptmf)((PTMF_Ext::EVC_PHASE)5, 0, 0);
			}
			break;
		case 6:
			result = param_3;
			if ((this->flagB & 1) != 0) {
				(this->*this->ptmf)((PTMF_Ext::EVC_PHASE)6, 0, 0);
				result = result | 0x1ff;
			}
			break;
		case 7:
			if (((this->flagB & 1) != 0) && (uVar1 = (this->*this->ptmf)((PTMF_Ext::EVC_PHASE)7, 0, 0), uVar1 != 0)) {
				result = uVar1;
			}
			break;
		default:
			IMPLEMENTATION_GUARD();
			break;
		}
	}

	return result;
}

static PTMF_Ext::PTMF_TYPE CheckComboA = &PTMF_Ext::CheckComboA;
static PTMF_Ext::PTMF_TYPE ManageActorB = &PTMF_Ext::ManageActorB;
static PTMF_Ext::PTMF_TYPE ManageActorA = &PTMF_Ext::ManageActorA;
static PTMF_Ext::PTMF_TYPE ManageMsgReceived = &PTMF_Ext::ManageMsgReceived;
static PTMF_Ext::PTMF_TYPE DummyTempFunc = &PTMF_Ext::DummyTempFunc;

void CActorEventGenerator::Create(ByteCode* pByteCode)
{
	int iVar1;
	undefined* puVar2;
	uint uVar3;
	ed_zone_3d* peVar4;
	CActorEventGenerator* ppCVar5;
	undefined8 uVar5;
	int iVar6;
	CActorEventGenerator* ppCVar6;
	//CActorFighterVTable** ppCVar7;
	PTMF_Ext* pCVar8;

	CActor::Create(pByteCode);

	this->flags_0x164 = 0;

	uVar3 = pByteCode->GetU32();
	(this->zoneRef).index = pByteCode->GetS32();

	if (uVar3 == 1) {
		gGlobalEvG = this;
		this->flags_0x164 = this->flags_0x164 | 2;
	}
	else {
		if (-1 < this->zoneRef.index) {
			this->flags_0x164 = this->flags_0x164 | 2;
		}
	}

	this->nbRegisteredMemberFunctions = 0;

	Register12(&DummyTempFunc, 0x0); //&EventGenerator_12_004265a8, &this->field_0x530);
	Register12(&CheckComboA, &this->field_0x538);
	Register12(&DummyTempFunc, 0x0); //&EventGenerator_12_004265c8, &this->field_0x53c);
	Register12(&DummyTempFunc, 0x0); //&EventGenerator_12_004265d8, &this->field_0x540);
	Register12(&DummyTempFunc, 0x0); //&EventGenerator_12_004265e8, &this->field_0x544);
	Register12(&ManageMsgReceived, &this->field_0x550);
	Register12(&DummyTempFunc, 0x0); //&EventGenerator_12_00426608, &this->field_0x558);
	Register12(&DummyTempFunc, 0x0); //&EventGenerator_12_00426618, &this->field_0x564);
	Register12(&DummyTempFunc, 0x0); //&EventGenerator_12_00426628, &this->field_0x570);
	Register12(&DummyTempFunc, 0x0); //&EventGenerator_12_00426638, &this->field_0x574);
	Register12(&ManageActorB, &this->field_0x580);
	Register12(&ManageActorA, &this->field_0x578);

	uVar5 = 0;
	iVar6 = 0;
	pCVar8 = this->field_0x170;
	if (0 < this->nbRegisteredMemberFunctions) {
		do {
			uVar5 = pCVar8->__ptmf_scall_NOT(PTMF_Ext::EVC_PHASE_CREATE, (int)uVar5, pByteCode);
			iVar6 = iVar6 + 1;
			pCVar8 = pCVar8 + 1;
		} while (iVar6 < this->nbRegisteredMemberFunctions);
	}
	return;
}

void CActorEventGenerator::Init()
{
	int iVar1;
	PTMF_Ext* pCVar2;
	int iVar3;

	CActor::Init();

	this->zoneRef.Init();

	iVar1 = 0;
	iVar3 = 0;
	pCVar2 = this->field_0x170;
	if (0 < this->nbRegisteredMemberFunctions) {
		do {
			iVar1 = pCVar2->__ptmf_scall_NOT(PTMF_Ext::EVC_PHASE_INIT, iVar1, 0);
			iVar3 = iVar3 + 1;
			pCVar2 = pCVar2 + 1;
		} while (iVar3 < this->nbRegisteredMemberFunctions);
	}

	gActiveEvG = (CActorEventGenerator*)0x0;

	iVar1 = 0;
	iVar3 = 0;
	pCVar2 = this->field_0x170;
	if (0 < this->nbRegisteredMemberFunctions) {
		do {
			iVar1 = pCVar2->__ptmf_scall_NOT(PTMF_Ext::EVC_PHASE_RESET, iVar1, 0);
			iVar3 = iVar3 + 1;
			pCVar2 = pCVar2 + 1;
		} while (iVar3 < this->nbRegisteredMemberFunctions);
	}

	this->field_0x160 = 1;
	this->flags_0x164 = this->flags_0x164 | 2;

	if (gGlobalEvG == this) {
		this->flags = this->flags | 2;
		this->flags = this->flags & 0xfffffffe;
	}
	else {
		this->flags = this->flags & 0xfffffffc;
	}

	return;
}

void CActorEventGenerator::Reset()
{
	int iVar1;
	PTMF_Ext* pExt;
	int iVar2;

	CActor::Reset();

	gActiveEvG = (CActorEventGenerator*)0x0;

	iVar1 = 0;
	iVar2 = 0;
	pExt = this->field_0x170;
	if (0 < this->nbRegisteredMemberFunctions) {
		do {
			iVar1 = pExt->__ptmf_scall_NOT(PTMF_Ext::EVC_PHASE_RESET, iVar1, 0);
			iVar2 = iVar2 + 1;
			pExt = pExt + 1;
		} while (iVar2 < this->nbRegisteredMemberFunctions);
	}

	this->field_0x160 = 1;
	this->flags_0x164 = this->flags_0x164 | 2;

	if (gGlobalEvG == this) {
		this->flags = this->flags | 2;
		this->flags = this->flags & 0xfffffffe;
	}
	else {
		this->flags = this->flags & 0xfffffffc;
	}

	return;
}

void CActorEventGenerator::CheckpointReset()
{
	int iVar1;
	PTMF_Ext* pExt;
	int iVar2;

	CActor::CheckpointReset();

	gActiveEvG = (CActorEventGenerator*)0x0;

	iVar1 = 0;
	iVar2 = 0;
	pExt = this->field_0x170;
	if (0 < this->nbRegisteredMemberFunctions) {
		do {
			iVar1 = pExt->__ptmf_scall_NOT(PTMF_Ext::EVC_PHASE_RESET, iVar1, 0);
			iVar2 = iVar2 + 1;
			pExt = pExt + 1;
		} while (iVar2 < this->nbRegisteredMemberFunctions);
	}

	this->field_0x160 = 1;
	this->flags_0x164 = this->flags_0x164 | 2;

	if (gGlobalEvG == this) {
		this->flags = this->flags | 2;
		this->flags = this->flags & 0xfffffffe;
	}
	else {
		this->flags = this->flags & 0xfffffffc;
	}

	return;
}

CBehaviour* CActorEventGenerator::BuildBehaviour(int behaviourType)
{
	CBehaviour* pBehaviour;

	if (behaviourType == 2) {
		pBehaviour = &behaviourEventGen;
	}
	else {
		pBehaviour = CActor::BuildBehaviour(behaviourType);
	}
	return pBehaviour;
}

int CActorEventGenerator::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	int iVar1;
	PTMF_Ext* pManageMsgReceived;

	iVar1 = 0;
	PTMF_Ext* pCVar5 = this->field_0x170;

	PTMF_Ext::PTMF_TYPE func = ManageMsgReceived;

	while ((iVar1 < this->nbRegisteredMemberFunctions && (pCVar5->ptmf != ManageMsgReceived))) {
		pCVar5 = pCVar5 + 1;
		iVar1 = iVar1 + 1;
	}

	if ((this->nbRegisteredMemberFunctions <= iVar1) || (pManageMsgReceived = this->field_0x170 + iVar1, (pManageMsgReceived->flagB & 1) == 0)) {
		pManageMsgReceived = (PTMF_Ext*)0x0;
	}

	ManageMsgAParams* pParams = pManageMsgReceived ? reinterpret_cast<ManageMsgAParams*>(pManageMsgReceived->pData) : (ManageMsgAParams*)0x0;

	if (((pManageMsgReceived != (PTMF_Ext*)0x0) && (msg == pParams->msgId)) && (pSender == pParams->actorRef.Get())) {
		pManageMsgReceived->flagB = pManageMsgReceived->flagB | 2;
	}

	iVar1 = CActor::InterpretMessage(pSender, msg, pMsgParam);
	return iVar1;
}

void CActorEventGenerator::Register12(PTMF_Ext::PTMF_TYPE* p12, void* pData)
{
	this->field_0x170[this->nbRegisteredMemberFunctions].ptmf = *p12;

	this->field_0x170[this->nbRegisteredMemberFunctions].flagB = 0;
	this->field_0x170[this->nbRegisteredMemberFunctions].flagA = 0;
	this->field_0x170[this->nbRegisteredMemberFunctions].pData = pData;

	this->nbRegisteredMemberFunctions = this->nbRegisteredMemberFunctions + 1;

	return;
}

void CActorEventGenerator::BehaviourEventGenerator_Manage()
{
	bool bVar1;
	int iVar2;
	uint iVar3;
	uint uVar4;
	ed_zone_3d* pZoneB;
	int iVar6;
	PTMF_Ext* pEntry;
	ed_zone_3d* pZoneA;

	if (this->actorState == 0) {
		if (this->field_0x160 == 1) {
			pZoneA = (this->zoneRef).Get();
			pZoneB = (ed_zone_3d*)0x0;

			if (pZoneA != (ed_zone_3d*)0x0) {
				pZoneB = pZoneA;
			}

			bVar1 = false;
			if (this == gGlobalEvG) {
				if ((gActiveEvG == (CActorEventGenerator*)0x0) &&
					((pZoneB == (ed_zone_3d*)0x0 ||
						((pZoneB != (ed_zone_3d*)0x0 &&
							(iVar2 = edEventComputeZoneAgainstVertex((CScene::ptable.g_EventManager_006f5080)->activeChunkId, pZoneB,
								&CActorHero::_gThis->currentLocation, 0), iVar2 != 1
								)))))) {
					bVar1 = true;
				}
			}
			else {
				if ((pZoneB != (ed_zone_3d*)0x0) &&
					(iVar2 = edEventComputeZoneAgainstVertex((CScene::ptable.g_EventManager_006f5080)->activeChunkId, pZoneB,
						&CActorHero::_gThis->currentLocation, 0), iVar2 == 1))
				{
					bVar1 = true;
					gActiveEvG = this;
				}
			}

			if (!bVar1) {
				if (this == gActiveEvG) {
					gActiveEvG = (CActorEventGenerator*)0x0;
				}

				this->flags_0x164 = this->flags_0x164 & 0xfffffffe;
				return;
			}

			this->flags_0x164 = this->flags_0x164 | 1;
			this->field_0x160 = 2;
			this->flags_0x164 = this->flags_0x164 | 2;
		}

		if ((this->field_0x160 == 2) || (this->field_0x160 == 5)) {
			iVar2 = this->nbRegisteredMemberFunctions;
			iVar3 = 1;
			iVar6 = 0;
			pEntry = this->field_0x170;
			if (0 < iVar2) {
				do {
					iVar3 = pEntry->__ptmf_scall_NOT(PTMF_Ext::EVC_PHASE_MANAGE, iVar3, 0);
					iVar2 = this->nbRegisteredMemberFunctions;
					iVar6 = iVar6 + 1;
					pEntry = pEntry + 1;
				} while (iVar6 < iVar2);
			}

			if (iVar3 == 0) {
				this->field_0x308 = this->field_0x308 + GetTimer()->lastFrameTime;
			}
			else {
				uVar4 = 0;
				iVar3 = 0;
				pEntry = this->field_0x170;
				if (0 < iVar2) {
					do {
						uVar4 = pEntry->__ptmf_scall_NOT((PTMF_Ext::EVC_PHASE)6, uVar4, 0);
						iVar3 = iVar3 + 1;
						pEntry = pEntry + 1;
					} while (iVar3 < this->nbRegisteredMemberFunctions);
				}

				this->field_0x168 = uVar4;
				iVar2 = 0;
				this->field_0x168 = this->field_0x168 << 4;

				iVar3 = 0;
				pEntry = this->field_0x170;
				if (0 < this->nbRegisteredMemberFunctions) {
					do {
						iVar2 = pEntry->__ptmf_scall_NOT((PTMF_Ext::EVC_PHASE)3, iVar2, 0);
						iVar3 = iVar3 + 1;
						pEntry = pEntry + 1;
					} while (iVar3 < this->nbRegisteredMemberFunctions);
				}

				if ((this->field_0x168 != 0) && (this->field_0x160 == 2)) {
					this->field_0x160 = 3;
					this->flags_0x164 = this->flags_0x164 | 2;
				}
			}
		}
	}

	return;
}

CBehaviourEventGen::PTMF_TYPE ManageCameraA =		&CBehaviourEventGen::ManageCamera;
CBehaviourEventGen::PTMF_TYPE ManageCinematic =		&CBehaviourEventGen::ManageCinematic;
CBehaviourEventGen::PTMF_TYPE ManageMessage =		&CBehaviourEventGen::ManageMessage;
CBehaviourEventGen::PTMF_TYPE ManageSlowDownA =		&CBehaviourEventGen::ManageSlowDown;
CBehaviourEventGen::PTMF_TYPE ManageEarthQuakeA =	&CBehaviourEventGen::ManageEarthQuakeA;
CBehaviourEventGen::PTMF_TYPE ManageEarthQuakeB =	&CBehaviourEventGen::ManageEarthQuakeB;
CBehaviourEventGen::PTMF_TYPE ManageEarthQuakeC =	&CBehaviourEventGen::ManageEarthQuakeC;
CBehaviourEventGen::PTMF_TYPE DrawA =				&CBehaviourEventGen::DrawA;
CBehaviourEventGen::PTMF_TYPE DrawB =				&CBehaviourEventGen::DrawB;
CBehaviourEventGen::PTMF_TYPE ManageCameraB =		&CBehaviourEventGen::ManageCamera;
CBehaviourEventGen::PTMF_TYPE ManageSlowDownB =		&CBehaviourEventGen::ManageSlowDown;


void CBehaviourEventGen::Create(ByteCode* pByteCode)
{
	this->field_0xc = pByteCode->GetS32();
	this->field_0x10 = pByteCode->GetF32();
	this->nbRegisteredMemberFunctions = 0;

	RegisterFunction(&ManageCameraA,		0x10);
	RegisterFunction(&::ManageCinematic,		0x20);
	RegisterFunction(&::ManageMessage,		0x40);
	RegisterFunction(&ManageSlowDownA,		0x80);
	RegisterFunction(&::ManageEarthQuakeA,	0x100);
	RegisterFunction(&::ManageEarthQuakeB,	0x200);
	RegisterFunction(&::ManageEarthQuakeC,	0x408);
	RegisterFunction(&::DrawA,				0x800);
	RegisterFunction(&::DrawB,				0x1000);

	int entryIndex = 0;
	BHVR_PTMF* pEntry = this->field_0x120;
	if (0 < this->nbRegisteredMemberFunctions) {
		do {
			(this->*pEntry->ptmf)(pEntry, EVG_PHASE_CREATE, pByteCode);
			entryIndex = entryIndex + 1;
			pEntry = pEntry + 1;
		} while (entryIndex < this->nbRegisteredMemberFunctions);
	}

	return;
}

void CBehaviourEventGen::Init(CActor* pOwner)
{
	this->pOwner = static_cast<CActorEventGenerator*>(pOwner);

	int entryIndex = 0;
	BHVR_PTMF* pEntry = this->field_0x120;
	if (0 < this->nbRegisteredMemberFunctions) {
		do {
			(this->*pEntry->ptmf)(pEntry, EVG_PHASE_INIT, (void*)0x0);
			entryIndex = entryIndex + 1;
			pEntry = pEntry + 1;
		} while (entryIndex < this->nbRegisteredMemberFunctions);
	}

	return;
}

void CBehaviourEventGen::Manage()
{
	ushort uVar1;
	CActorEventGenerator* pEventGenerator;
	bool bVar3;
	bool bVar4;
	uint uVar5;
	long lVar6;
	BHVR_PTMF* pBhvEntry;
	int entryIndex;
	int iVar8;
	float fVar9;
	PTMF_Ext* pEntry;

	this->pOwner->BehaviourEventGenerator_Manage();

	if (this->pOwner->field_0x160 == 3) {
		iVar8 = this->field_0xc;
		bVar3 = false;
		if (iVar8 == 2) {
			bVar3 = this->field_0x10 <= this->field_0x8;
		}
		else {
			if (iVar8 == 1) {
				fVar9 = this->field_0x10;
				iVar8 = rand();
				bVar3 = (int)(fVar9 * ((float)iVar8 / 2.147484e+09f)) == 0;
			}
			else {
				if (iVar8 == 0) {
					bVar3 = true;
				}
			}
		}

		if (bVar3) {
			this->field_0x8 = 0.0f;
			pEventGenerator = this->pOwner;
			pEventGenerator->field_0x160 = 4;
			pEventGenerator->flags_0x164 = pEventGenerator->flags_0x164 | 2;
		}
		else {
			pEventGenerator = this->pOwner;
			iVar8 = 0;
			entryIndex = 0;
			pEntry = pEventGenerator->field_0x170;
			if (0 < pEventGenerator->nbRegisteredMemberFunctions) {
				do {
					iVar8 = pEntry->__ptmf_scall_NOT((PTMF_Ext::EVC_PHASE)3, iVar8, 0);
					entryIndex = entryIndex + 1;
					pEntry = pEntry + 1;
				} while (entryIndex < pEventGenerator->nbRegisteredMemberFunctions);
			}
			pEventGenerator = this->pOwner;
			pEventGenerator->field_0x160 = 6;
			pEventGenerator->flags_0x164 = pEventGenerator->flags_0x164 | 2;
		}
	}

	iVar8 = this->pOwner->field_0x160;
	if ((iVar8 == 4) || (iVar8 == 5)) {
		uVar5 = FUN_0038fba0();
		iVar8 = 0;
		pBhvEntry = this->field_0x120;
		if (0 < this->nbRegisteredMemberFunctions) {
			do {
				uVar1 = pBhvEntry->flagB;
				bVar3 = false;
				if ((uVar1 & 1) != 0) {
					bVar4 = (uVar1 & 4) == 0;
					if (!bVar4) {
						bVar4 = (uVar1 & 8) != 0;
					}
					if (bVar4) {
						bVar3 = true;
					}
				}

				if ((bVar3) && ((uVar5 & pBhvEntry->flagA) != 0)) {
					(this->*pBhvEntry->ptmf)(pBhvEntry, EVG_PHASE_MANAGE, 0);
				}
				else {
					pBhvEntry->flagB = pBhvEntry->flagB | 4;
				}

				iVar8 = iVar8 + 1;
				pBhvEntry = pBhvEntry + 1;
			} while (iVar8 < this->nbRegisteredMemberFunctions);
		}

		pEventGenerator = this->pOwner;
		pEventGenerator->field_0x160 = 5;
		pEventGenerator->flags_0x164 = pEventGenerator->flags_0x164 | 2;
	}

	if (this->pOwner->field_0x160 == 5) {
		bVar3 = false;
		iVar8 = 0;
		pBhvEntry = this->field_0x120;
		if (0 < this->nbRegisteredMemberFunctions) {
			do {
				if (((pBhvEntry->flagB & 2) != 0) &&
					(lVar6 = (this->*pBhvEntry->ptmf)(pBhvEntry, EVG_PHASE_DRAW, 0), lVar6 != 0)) {
					bVar3 = true;
				}
				iVar8 = iVar8 + 1;
				pBhvEntry = pBhvEntry + 1;
			} while (iVar8 < this->nbRegisteredMemberFunctions);
		}

		if (!bVar3) {
			this->field_0x8 = 0.0f;
			iVar8 = 0;
			pBhvEntry = this->field_0x120;
			if (0 < this->nbRegisteredMemberFunctions) {
				do {
					(this->*pBhvEntry->ptmf)(pBhvEntry, EVG_PHASE_BEGIN, 0);
					iVar8 = iVar8 + 1;
					pBhvEntry = pBhvEntry + 1;
				} while (iVar8 < this->nbRegisteredMemberFunctions);
			}

			pEventGenerator = this->pOwner;
			iVar8 = 0;
			entryIndex = 0;
			pEntry = pEventGenerator->field_0x170;
			if (0 < pEventGenerator->nbRegisteredMemberFunctions) {
				do {
					iVar8 = pEntry->__ptmf_scall_NOT(PTMF_Ext::EVC_PHASE_RESET, iVar8, 0);
					entryIndex = entryIndex + 1;
					pEntry = pEntry + 1;
				} while (entryIndex < pEventGenerator->nbRegisteredMemberFunctions);
			}

			pEventGenerator = this->pOwner;
			pEventGenerator->field_0x160 = 6;
			pEventGenerator->flags_0x164 = pEventGenerator->flags_0x164 | 2;
			if (this->field_0xc == 0) {
				pEventGenerator = this->pOwner;
				pEventGenerator->flags = pEventGenerator->flags & 0xfffffffd;
				pEventGenerator->flags = pEventGenerator->flags | 1;
			}
		}
	}

	pEventGenerator = this->pOwner;

	if (pEventGenerator->field_0x160 == 6) {
		iVar8 = 0;
		entryIndex = 0;
		pEntry = pEventGenerator->field_0x170;
		if (0 < pEventGenerator->nbRegisteredMemberFunctions) {
			do {
				iVar8 = pEntry->__ptmf_scall_NOT((PTMF_Ext::EVC_PHASE)5, iVar8, 0);
				entryIndex = entryIndex + 1;
				pEntry = pEntry + 1;
			} while (entryIndex < pEventGenerator->nbRegisteredMemberFunctions);
		}
		if (iVar8 == 0) {
			pEventGenerator = this->pOwner;
			iVar8 = 0;
			entryIndex = 0;
			pEntry = pEventGenerator->field_0x170;
			if (0 < pEventGenerator->nbRegisteredMemberFunctions) {
				do {
					iVar8 = pEntry->__ptmf_scall_NOT(PTMF_Ext::EVC_PHASE_RESET, iVar8, 0);
					entryIndex = entryIndex + 1;
					pEntry = pEntry + 1;
				} while (entryIndex < pEventGenerator->nbRegisteredMemberFunctions);
			}
			pEventGenerator = this->pOwner;
			pEventGenerator->field_0x160 = 1;
			pEventGenerator->flags_0x164 = pEventGenerator->flags_0x164 | 2;
		}
	}

	return;
}

void CBehaviourEventGen::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	int entryIndex;
	CActorEventGenerator* pEventGenerator;

	this->pOwner->SetState(0, -1);

	this->field_0x8 = this->field_0x10;
	pEventGenerator = this->pOwner;
	pEventGenerator->field_0x160 = 1;
	pEventGenerator->flags_0x164 = pEventGenerator->flags_0x164 | 2;

	entryIndex = 0;
	BHVR_PTMF* pEntry = this->field_0x120;
	if (0 < this->nbRegisteredMemberFunctions) {
		do {
			(this->*pEntry->ptmf)(pEntry, EVG_PHASE_BEGIN, (void*)0x0);
			entryIndex = entryIndex + 1;
			pEntry = pEntry + 1;
		} while (entryIndex < this->nbRegisteredMemberFunctions);
	}
	return;
}

uint CBehaviourEventGen::FUN_0038fba0()
{
	bool bVar1;
	BHVR_PTMF* ppCVar2;
	BHVR_PTMF* pEntry;
	uint uVar3;
	uint uVar4;
	uint* puVar5;
	int iVar6;
	uint uVar7;
	int iVar8;
	uint local_60[16];

	uVar7 = this->pOwner->field_0x168;
	if (CActorEventGenerator::gGlobalEvG == this->pOwner) {
		iVar8 = 0;
		uVar4 = 0;
		iVar6 = 0;
		pEntry = this->field_0x120;
		if (0 < this->nbRegisteredMemberFunctions) {
			do {
				if ((pEntry->flagA & 1) != 0) {
					uVar4 = uVar4 | 1 << (iVar6 + 4U & 0x1f);
				}

				iVar6 = iVar6 + 1;
				pEntry = pEntry + 1;
			} while (iVar6 < this->nbRegisteredMemberFunctions);
		}

		uVar3 = 4;
		puVar5 = local_60;
		do {
			if ((uVar7 & uVar4 & 1 << (uVar3 & 0x1f)) != 0) {
				*puVar5 = uVar3;
				iVar8 = iVar8 + 1;
				puVar5 = puVar5 + 1;
			}
			uVar3 = uVar3 + 1;
		} while ((int)uVar3 < 0xd);

		iVar6 = rand();
		if (iVar8 == 0) {
			trap(7);
		}

		uVar7 = 1 << (local_60[iVar6 % iVar8] & 0x1f);
		if ((uVar7 & 0x90) != 0) {
			iVar6 = 0;
			pEntry = this->field_0x120;
			while ((iVar6 < this->nbRegisteredMemberFunctions && (bVar1 = pEntry->ptmf == ManageCameraB, bVar1 != false))) {
				pEntry = pEntry + 1;
				iVar6 = iVar6 + 1;
			}

			if ((iVar6 < this->nbRegisteredMemberFunctions) && ((this->field_0x120[iVar6].flagB & 1) != 0)) {
				ppCVar2 = this->field_0x120 + iVar6;
			}
			else {
				ppCVar2 = (BHVR_PTMF*)0x0;
			}

			if (ppCVar2 != (BHVR_PTMF*)0x0) {
				iVar6 = 0;
				pEntry = this->field_0x120;
				while ((iVar6 < this->nbRegisteredMemberFunctions && (bVar1 = pEntry->ptmf, ManageSlowDownB, bVar1 != false))) {
					pEntry = pEntry + 1;
					iVar6 = iVar6 + 1;
				}

				if ((iVar6 < this->nbRegisteredMemberFunctions) && ((this->field_0x120[iVar6].flagB & 1) != 0)) {
					ppCVar2 = this->field_0x120 + iVar6;
				}
				else {
					ppCVar2 = (BHVR_PTMF*)0x0;
				}

				if ((ppCVar2 != (BHVR_PTMF*)0x0) && (uVar7 = 0x90, (CScene::ptable.g_CameraManager_0045167c)->pActiveCamera->pOtherTarget != (CActor*)0x0)) {
					uVar7 = 0;
				}
			}
		}
	}

	return uVar7;
}

int CBehaviourEventGen::ManageCamera(BHVR_PTMF* pPMTF, EVG_PHASE phase, void* pData)
{
	ByteCode* pByteCode = reinterpret_cast<ByteCode*>(pData);

	switch (phase) {
		case EVG_PHASE_CREATE:
			this->cameraRef.index = pByteCode->GetS32();
			if (-1 < this->cameraRef.index) {
				pPMTF->flagA = pPMTF->flagA | 1;
			}
			break;
		case EVG_PHASE_INIT:
			if (-1 < this->cameraRef.index) {
				this->cameraRef.Init();
			}
			break;
		case EVG_PHASE_BEGIN:
			pPMTF->flagB = pPMTF->flagA;
			break;
		case EVG_PHASE_MANAGE:
			IMPLEMENTATION_GUARD();
			break;
		case EVG_PHASE_DRAW:
			IMPLEMENTATION_GUARD();
			break;
	}
	return 0;
}

int CBehaviourEventGen::ManageCinematic(BHVR_PTMF* pPMTF, EVG_PHASE phase, void* pData)
{
	bool bVar1;
	CCinematic* pCinematic;

	switch (phase) {
	case EVG_PHASE_CREATE:
		cinematicId = reinterpret_cast<ByteCode*>(pData)->GetS32();

		if (-1 < this->cinematicId) {
			pPMTF->flagA = pPMTF->flagA | 1;
		}
		break;
	case EVG_PHASE_BEGIN:
		pPMTF->flagB = pPMTF->flagA;
		break;
	case EVG_PHASE_MANAGE:
		if ((pPMTF->flagB & 1) != 0) {
			pCinematic = g_CinematicManager_0048efc->GetCinematic(this->cinematicId);
			if (pCinematic != (CCinematic*)0x0) {
				pCinematic->TryTriggerCutscene(this->pOwner, 0);
			}

			pPMTF->flagB = pPMTF->flagB | 2;
			pPMTF->flagB = pPMTF->flagB | 4;
		}
		break;
	case EVG_PHASE_DRAW:
		if (((pPMTF->flagB & 2) != 0) &&
			(pCinematic = g_CinematicManager_0048efc->GetCinematic(this->cinematicId), pCinematic != (CCinematic*)0x0)) {
			bVar1 = pCinematic->pCineBankEntry != (edCBankBufferEntry*)0x0;
			if (bVar1) {
				bVar1 = pCinematic->cineBankLoadStage_0x2b4 != 4;
			}

			if ((!bVar1) && (pCinematic->state == CS_Stopped)) {
				pPMTF->flagB = pPMTF->flagB & 0xfffd;
				return false;
			}
		}
		break;
	}

	return 0;
}

int CBehaviourEventGen::ManageMessage(BHVR_PTMF* pPMTF, EVG_PHASE phase, void* pData)
{
	uint uVar1;

	ByteCode* pByteCode = reinterpret_cast<ByteCode*>(pData);

	switch (phase) {
	case EVG_PHASE_CREATE:
		uVar1 = pByteCode->GetU32();
		if (uVar1 != 0) {
			pPMTF->flagA = pPMTF->flagA | 1;
		}

		for (; uVar1 != 0; uVar1 = uVar1 - 1) {
			this->msgActorRef.index = pByteCode->GetS32();
			this->msgId = pByteCode->GetU32();
			this->msgParams = pByteCode->GetU32();
		}
		break;
	case EVG_PHASE_INIT:
		if (pPMTF->flagA & 1) {
			this->msgActorRef.Init();
		}
		break;
	case EVG_PHASE_BEGIN:
		pPMTF->flagB = pPMTF->flagA;
		break;
	case EVG_PHASE_MANAGE:
		if ((pPMTF->flagB & 1) != 0) {
			this->pOwner->DoMessage((this->msgActorRef).Get(), (ACTOR_MESSAGE)this->msgId, (MSG_PARAM)this->msgParams);
			pPMTF->flagB = pPMTF->flagB | 2;
			pPMTF->flagB = pPMTF->flagB | 4;
		}
		break;
	case EVG_PHASE_DRAW:
		pPMTF->flagB = pPMTF->flagB & 0xfffd;
		return 0;
		break;
	}
	return 0;
}

int CBehaviourEventGen::ManageSlowDown(BHVR_PTMF* pPMTF, EVG_PHASE phase, void* pData)
{
	uint uVar1;

	ByteCode* pByteCode = reinterpret_cast<ByteCode*>(pData);

	switch (phase) {
	case EVG_PHASE_CREATE:
		uVar1 = pByteCode->GetU32();
		if (uVar1 != 0) {
			pPMTF->flagA = pPMTF->flagA | 1;
		}

		for (; uVar1 != 0; uVar1 = uVar1 - 1) {
			this->slowDownfield_0x38 = 0.0f;
			this->slowDownfield_0x28 = pByteCode->GetF32();
			this->slowDownfield_0x2c = pByteCode->GetF32();
			this->slowDownfield_0x30 = pByteCode->GetF32();
			this->slowDownfield_0x34 = pByteCode->GetF32();
		}
		break;
	case EVG_PHASE_BEGIN:
		pPMTF->flagB = pPMTF->flagA;
		this->slowDownfield_0x38 = 0.0f;
		GetTimer()->timeScale = 1.0f;
		break;
	case EVG_PHASE_MANAGE:
		IMPLEMENTATION_GUARD();
		break;
	case EVG_PHASE_DRAW:
		IMPLEMENTATION_GUARD();
		break;
	}
	return 0;
}

int CBehaviourEventGen::ManageEarthQuakeA(BHVR_PTMF* pPMTF, EVG_PHASE phase, void* pData)
{
	uint uVar1;

	float fVar2;
	float fVar3;
	float fVar4;
	float fVar5;

	ByteCode* pByteCode = reinterpret_cast<ByteCode*>(pData);

	switch (phase) {
	case EVG_PHASE_CREATE:
		uVar1 = pByteCode->GetU32();
		if (uVar1 != 0) {
			pPMTF->flagA = pPMTF->flagA | 1;
		}

		for (; uVar1 != 0; uVar1 = uVar1 - 1) {
			fVar2 = pByteCode->GetF32();
			fVar3 = pByteCode->GetF32();
			fVar4 = pByteCode->GetF32();
			fVar5 = pByteCode->GetF32();
			IMPLEMENTATION_GUARD();
			//CAM_QUAKE::FUN_00194730(fVar4, fVar3, fVar5, &this->field_0x40);
			//CAM_QUAKE::FUN_00194760(fVar2, fVar2, fVar2, 0.0f, &this->field_0x40);
			//CAM_QUAKE::FUN_00194740(20.0f, 20.0f, 20.0f, 0.0f, &this->field_0x40);
			//CAM_QUAKE::SetMode(&this->field_0x40, 0x2a);
		}
		break;
	case EVG_PHASE_BEGIN:
		pPMTF->flagB = pPMTF->flagA;
		break;
	case EVG_PHASE_MANAGE:
		IMPLEMENTATION_GUARD();
		break;
	case EVG_PHASE_DRAW:
		IMPLEMENTATION_GUARD();
		break;
	}
	return 0;
}

int CBehaviourEventGen::ManageEarthQuakeB(BHVR_PTMF* pPMTF, EVG_PHASE phase, void* pData)
{
	uint uVar1;

	ByteCode* pByteCode = reinterpret_cast<ByteCode*>(pData);

	switch (phase) {
	case EVG_PHASE_CREATE:
		uVar1 = pByteCode->GetU32();
		if (uVar1 != 0) {
			pPMTF->flagA = pPMTF->flagA | 1;
		}

		for (; uVar1 != 0; uVar1 = uVar1 - 1) {
			pByteCode->GetF32();
			//CAM_QUAKE::FUN_00194760(0.0f, 0.0f, 0.0f, pByteCode->GetF32(), &this->field_0x70);
			this->angleRotY = pByteCode->GetU32();
			(this->field_0x70).field_0x28 = pByteCode->GetF32();
			(this->field_0x70).field_0x24 = pByteCode->GetF32();
			(this->field_0x70).field_0x2c = pByteCode->GetF32();
			IMPLEMENTATION_GUARD();
			//CAM_QUAKE::FUN_00194740(0.0f, 0.0f, 0.0f, 0.0f, &this->field_0x70);
			//CAM_QUAKE::SetMode(&this->field_0x70, 0x100);
		}
		break;
	case EVG_PHASE_BEGIN:
		pPMTF->flagB = pPMTF->flagA;
		break;
	case EVG_PHASE_MANAGE:
		IMPLEMENTATION_GUARD();
		break;
	case EVG_PHASE_DRAW:
		IMPLEMENTATION_GUARD();
		break;
	}
	return 0;
}

int CBehaviourEventGen::ManageEarthQuakeC(BHVR_PTMF* pPMTF, EVG_PHASE phase, void* pData)
{
	uint uVar1;

	ByteCode* pByteCode = reinterpret_cast<ByteCode*>(pData);

	switch (phase) {
	case EVG_PHASE_CREATE:
		uVar1 = pByteCode->GetU32();
		if (uVar1 != 0) {
			pPMTF->flagA = pPMTF->flagA | 1;
		}

		for (; uVar1 != 0; uVar1 = uVar1 - 1) {
			(this->field_0xe0).field_0x28 = pByteCode->GetF32();
			(this->field_0xe0).field_0x24 = pByteCode->GetF32();
			(this->field_0xe0).field_0x2c = pByteCode->GetF32();

			this->field_0x110 = pByteCode->GetF32();
			this->field_0x114 = pByteCode->GetF32();
			this->field_0x118 = pByteCode->GetF32();
			this->field_0x11c = pByteCode->GetF32();
			IMPLEMENTATION_GUARD();

			//CAM_QUAKE::FUN_00194760(0.0f, 0.0f, 0.0f, 0.0f, &this->field_0xe0);
			//CAM_QUAKE::FUN_00194740(0.0f, 0.0f, 0.0f, 0.0f, &this->field_0xe0);
			//CAM_QUAKE::SetMode(&this->field_0xe0, 0);
		}
		break;
	case EVG_PHASE_BEGIN:
		pPMTF->flagB = pPMTF->flagA;
		break;
	case EVG_PHASE_MANAGE:
		IMPLEMENTATION_GUARD();
		break;
	case EVG_PHASE_DRAW:
		IMPLEMENTATION_GUARD();
		break;
	}
	return 0;
}

int CBehaviourEventGen::DrawA(BHVR_PTMF* pPMTF, EVG_PHASE phase, void* pData)
{
	uint uVar1;

	ByteCode* pByteCode = reinterpret_cast<ByteCode*>(pData);

	switch (phase) {
	case EVG_PHASE_CREATE:
		this->field_0xb8 = 0.0f;
		this->field_0xb0.rgba = pByteCode->GetU32();
		this->field_0xb4 = pByteCode->GetF32();
		break;
	case EVG_PHASE_BEGIN:
		pPMTF->flagB = pPMTF->flagA;
		this->field_0xb8 = 0.0f;
		break;
	case EVG_PHASE_MANAGE:
		IMPLEMENTATION_GUARD();
		break;
	case EVG_PHASE_DRAW:
		IMPLEMENTATION_GUARD();
		break;
	}
	return 0;
}

int CBehaviourEventGen::DrawB(BHVR_PTMF* pPMTF, EVG_PHASE phase, void* pData)
{
	uint uVar1;

	ByteCode* pByteCode = reinterpret_cast<ByteCode*>(pData);

	switch (phase) {
	case EVG_PHASE_CREATE:
		uVar1 = pByteCode->GetU32();
		if (uVar1 != 0) {
			pPMTF->flagA = pPMTF->flagA | 1;
		}

		for (; uVar1 != 0; uVar1 = uVar1 - 1) {
			this->field_0xbc.rgba = pByteCode->GetU32();
			this->field_0xc0.rgba = pByteCode->GetU32();
			this->field_0xc4 = pByteCode->GetF32();
			this->field_0xc8 = pByteCode->GetF32();
			this->field_0xcc = pByteCode->GetF32();
			this->field_0xd0 = 0.0f;

			(this->field_0xbc).r = (this->field_0xbc).r << 1;
			(this->field_0xbc).g = (this->field_0xbc).g << 1;
			(this->field_0xbc).b = (this->field_0xbc).b << 1;
			(this->field_0xbc).a = (this->field_0xbc).a << 1;

			(this->field_0xc0).r = (this->field_0xc0).r << 1;
			(this->field_0xc0).g = (this->field_0xc0).g << 1;
			(this->field_0xc0).b = (this->field_0xc0).b << 1;
			(this->field_0xc0).a = (this->field_0xc0).a << 1;
		}
		break;
	case EVG_PHASE_BEGIN:
		pPMTF->flagB = pPMTF->flagA;
		this->field_0xd0 = 0.0f;
		break;
	case EVG_PHASE_MANAGE:
		IMPLEMENTATION_GUARD();
		break;
	case EVG_PHASE_DRAW:
		IMPLEMENTATION_GUARD();
		break;
	}
	return 0;
}

void CBehaviourEventGen::RegisterFunction(PTMF_TYPE* pFunc, uint flag)
{
	this->field_0x120[this->nbRegisteredMemberFunctions].ptmf = *pFunc;
	this->field_0x120[this->nbRegisteredMemberFunctions].flagA = 0x10;
	this->field_0x120[this->nbRegisteredMemberFunctions].flagB = 0x10;
	this->nbRegisteredMemberFunctions = this->nbRegisteredMemberFunctions + 1;

	return;
}
