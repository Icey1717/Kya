#include "ActorProjectile.h"
#include "MemoryStream.h"

void CActorProjectile::Create(ByteCode* pByteCode)
{
	SkipToNextActor(pByteCode);
}

CBehaviour* CActorProjectile::BuildBehaviour(int behaviourType)
{
	CBehaviour* pBehaviour;

	switch (behaviourType) {
	case 0x3:
	{
		pBehaviour = &behaviourProjectileStand;
	}
		break;
	case 0x4:
		pBehaviour = new CBehaviourProjectileStraight;
		break;
	case 0x5:
		pBehaviour = new CBehaviourProjectilePortable;
		break;
	case 0x9:
		pBehaviour = new CBehaviourInventory;
		break;
	case 0xb:
		pBehaviour = new CBehaviourInactive;
		break;
	default:
		assert(behaviourType < 3);
		pBehaviour = CActorAutonomous::BuildBehaviour(behaviourType);
		break;
	}
	return pBehaviour;
}

void CActorProjectile::AppearToDie()
{
	IMPLEMENTATION_GUARD_LOG();
}
