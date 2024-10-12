#include "ActorBonus.h"
#include "MemoryStream.h"

CSharedLights<COmniLight, 3> CActorBonus::_gBNS_Lights;

StateConfig CActorBonus::_gStateCfg_BNS[6] = {
	StateConfig(0x6, 0x0),
	StateConfig(0x6, 0x0),
	StateConfig(0x6, 0x0),
	StateConfig(0x6, 0x0),
	StateConfig(0x7, 0x0),
	StateConfig(0x7, 0x0),
};

void CActorBonus::Create(ByteCode* pByteCode)
{
	KyaUpdateObjA* pKVar1;
	int iVar2;
	int* piVar3;
	float fVar4;
	float fVar5;

	CActorMovable::Create(pByteCode);
	//(this->vector_0x1e4).x = 0.0;
	this->field_0x1dc.index = pByteCode->GetS32();
	iVar2 = pByteCode->GetS32();
	//this->particleID_0x1d4 = iVar2;
	//CActor::SV_InstallMaterialId(this->particleID_0x1d4);
	iVar2 = pByteCode->GetS32();
	//this->field_0x1d4 = iVar2;
	//CActor::SV_InstallMaterialId(this->field_0x1d4);
	//piVar3 = CActor::CreateActorSound(this, 1);
	//*(int**)&this->field_0x1d8 = piVar3;
	//pKVar1 = this->data.subObjA;
	//fVar4 = (pKVar1->boundingSphere).y;
	//fVar5 = (pKVar1->boundingSphere).z;
	//(this->vector_0x1e4).y = (pKVar1->boundingSphere).x;
	//(this->vector_0x1e4).z = fVar4;
	//(this->vector_0x1e4).w = fVar5;
	//this->field_0x1f0 = (pKVar1->boundingSphere).w;
	return;
}



void CActorBonus::Init()
{
	CShadow* pCVar1;
	KyaUpdateObjA* pKVar2;
	float fVar3;

	this->field_0xf0 = 20.0f;
	CActor::Init();
	this->field_0x1dc.Init();

	pCVar1 = this->pShadow;
	if (pCVar1 != (CShadow*)0x0) {
		IMPLEMENTATION_GUARD(
		(pCVar1->base).field_0x30 = 0x80808080;
		((this->pShadow)->base).field_0x48 = 1.3f;
		(this->pShadow)->field_0x50 = 1.3f;)
	}

	_gBNS_Lights.Init(0x40000000, 0x40800000, 0x41004, 0x10a010, 0);

	pKVar2 = this->subObjA;
	fVar3 = pKVar2->field_0x1c;
	if (pKVar2->visibilityDistance < fVar3) {
		(this->subObjA)->visibilityDistance = fVar3 + 5.0f;
	}

	(this->subObjA)->field_0x20 = 50000.0f;

	return;
}

void CActorBonus::Term()
{
	CActor::Term();
	_gBNS_Lights.Term();
	return;
}

void CActorBonus::Draw()
{
	CActor::Draw();
}

void CActorBonus::CheckpointReset()
{
	CBehaviour* pCVar1;

	pCVar1 = GetBehaviour(this->curBehaviourId);
	if (pCVar1 != (CBehaviour*)0x0) {
		static_cast<CBehaviourBonusBase*>(GetBehaviour(this->curBehaviourId))->ChangeVisibleState(0);
	}
	return;
}

void CActorBonus::SaveContext(uint*, int)
{
	IMPLEMENTATION_GUARD();
}

void CActorBonus::LoadContext(uint*, int)
{
	IMPLEMENTATION_GUARD();
}

CBehaviour* CActorBonus::BuildBehaviour(int behaviourType)
{
	CBehaviour* pCVar1;

	if (behaviourType == 2) {
		pCVar1 = (CBehaviour*)0x0;
	}
	else {
		if (behaviourType == 6) {
			pCVar1 = new CCBehaviourBonusAddOn;
		}
		else {
			if (behaviourType == 5) {
					pCVar1 = &this->behaviourBonusFlock;
			}
			else {
				if (behaviourType == 4) {
					pCVar1 = new CBehaviourBonusPath;
				}
				else {
					if (behaviourType == 3) {
						pCVar1 = new CBehaviourBonusTurn;
					}
					else {
						pCVar1 = CActor::BuildBehaviour(behaviourType);
					}
				}
			}
		}
	}
	return pCVar1;
}

StateConfig* CActorBonus::GetStateCfg(int state)
{
	StateConfig* pStateConfig;

	if (state < 5) {
		pStateConfig = CActorMovable::GetStateCfg(state);
	}
	else {
		pStateConfig = _gStateCfg_BNS + state + -5;
	}

	return pStateConfig;
}

void CActorBonus::ChangeManageState(int state)
{
	int iVar1;
	CBehaviour* pCVar2;

	CActor::ChangeManageState(state);

	iVar1 = this->curBehaviourId;
	if ((((iVar1 == 6) || (iVar1 == 5)) || (iVar1 == 4)) || ((iVar1 == 3 || (iVar1 == 2)))) {
		if (state == 0) {
			this->flags = this->flags & 0xffffff7f;
			this->flags = this->flags | 0x20;
			EvaluateDisplayState();
		}
		else {
			this->flags = this->flags & 0xffffff5f;
			EvaluateDisplayState();
		}
	}

	if ((((this->flags & 0x800000) != 0) && (state != 0)) && (pCVar2 = GetBehaviour(5), pCVar2 != (CBehaviour*)0x0)) {
		IMPLEMENTATION_GUARD(
			pCVar3 = (CBehaviourVtable*)0x0;
		if (0 < (int)pCVar2[0xc].pVTable) {
			pCVar3 = pCVar2[0x14].pVTable;
		}
		if (pCVar3 != (CBehaviourVtable*)0x0) {
			if ((int)pCVar2[0xc].pVTable < 1) {
				pCVar3 = (CBehaviourVtable*)0x0;
			}
			else {
				pCVar3 = pCVar2[0x14].pVTable;
			}
			FUN_003984f0((int)pCVar3, 1);
		});
	}

	return;
}

void CActorBonus::ChangeVisibleState(int state)
{
	int iVar1;
	CBehaviourBonusBase* pCVar2;

	CActor::ChangeVisibleState(state);

	if ((this->flags & 0x800000) == 0) {
		iVar1 = this->curBehaviourId;
		if ((((iVar1 == 6) || (iVar1 == 5)) || (iVar1 == 4)) || ((iVar1 == 3 || (iVar1 == 2)))) {
			pCVar2 = static_cast<CBehaviourBonusBase*>(GetBehaviour(this->curBehaviourId));
			pCVar2->ChangeVisibleState(state);
		}
	}
	else {
		pCVar2 = static_cast<CBehaviourBonusBase*>(GetBehaviour(this->curBehaviourId));
		if (pCVar2 != (CBehaviour*)0x0) {
			pCVar2->ChangeVisibleState(state);
		}
	}

	return;
}
