#include "ActorWolfen.h"
#include "MemoryStream.h"

void CActorWolfen::Create(ByteCode* pByteCode)
{
	SkipToNextActor(pByteCode);
}

CBehaviour* CActorWolfen::BuildBehaviour(int behaviourType)
{
	CBehaviour* pBehaviour;

	switch (behaviourType) {
	case 0x8:
		pBehaviour = new CBehaviourWatchDog;
		break;
	case 0xa:
		pBehaviour = new CBehaviourGuardArea;
		break;
	case 0xd:
		pBehaviour = new CBehaviourEscape;
		break;
	case 0xe:
		pBehaviour = &behaviourExorcism;
		break;
	case 0xf:
		pBehaviour = &behaviourTrack;
		break;
	case 0x11:
		pBehaviour = new CBehaviourTrackWeapon;
		break;
	case 0x12:
		pBehaviour = new CBehaviourTrackWeaponStand;
		break;
	case 0x13:
		pBehaviour = new CBehaviourTrackWeaponSnipe;
		break;
	case 0x15:
		pBehaviour = new CBehaviourSnipe;
		break;
	case 0x16:
		pBehaviour = &behaviourLost;
		break;
	case 0x17:
		pBehaviour = new CBehaviourWolfenUnknown;
		break;
	case 0x18:
		pBehaviour = new CBehaviourDCA;
		break;
	case 0x19:
		pBehaviour = new CBehaviourAvoid;
		break;
	default:
		assert(behaviourType < 7);
		pBehaviour = CActorFighter::BuildBehaviour(behaviourType);
		break;
	}
	return pBehaviour;
}
