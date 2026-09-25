#include "ActorFactory.h"
#include "Actor.h"

#include "ActorBoomy.h"
#include "ActorHero_Private.h"
#include "ActorMicken.h"
#include "ActorWolfen.h"
#include "ActorNativ.h"
#include "ActorMovingPlatform.h"
#include "ActorBonus.h"
#include "ActorRope.h"
#include "ActorGravityAware.h"
#include "ActorSwitch.h"
#include "ActorWind.h"
#include "ActorCommander.h"
#include "ActorAmbre.h"
#include "ActorFruit.h"
#include "ActorBridge.h"
#include "ActorClusteriser.h"
#include "ActorCompanion.h"
#include "ActorDCA.h"
#include "ActorProjectile.h"
#include "ActorCheckpointManager.h"
#include "ActorWeapon.h"
#include "ActorAraignos.h"
#include "ActorTeleporter.h"
#include "ActorAmortos.h"
#include "ActorTrap.h"
#include "ActorFx.h"
#include "ActorMoney.h"
#include "ActorBox.h"
#include "ActorAton.h"
#include "ActorBasicBox.h"
#include "ActorNativCmd.h"
#include "ActorFogManager.h"
#include "ActorShoot.h"
#include "ActorShocker.h"
#include "ActorShockerCmd.h"
#include "ActorBonusFountain.h"
#include "ActorRune.h"
#include "ActorBlazer.h"
#include "ActorEventGenerator.h"
#include "ActorWoof.h"
#include "ActorElectrolla.h"
#include "ActorExplosiveDistributor.h"
#include "ActorHedgehog.h"
#include "ActorNoseMonster.h"
#include "ActorWoodMonster.h"
#include "ActorHelperSign.h"
#include "ActorEgg.h"
#include "ActorAmortosCmd.h"
#include "ActorPunchingBall.h"
#include "ActorNativShop.h"
#include "ActorAddOnPurchase.h"
#include "ActorCareBoy.h"
#include "ActorShip.h"
#include "ActorHunter.h"
#include "ActorJamGut.h"
#include "ActorAcceleratos.h"
#include "ActorPattern.h"
#include "ActorPatternCmd.h"
#include "ActorBunch.h"
#include "ActorStiller.h"
#include "ActorBrazul.h"
#include "ActorCredits.h"
#include "ActorMiniGame.h"
#include "ActorMiniGameTimeAttack.h"
#include "ActorMiniGameBoxCounter.h"
#include "ActorMiniGamesManager.h"
#include "ActorMiniGamesOrganizer.h"
#include "ActorMiniGameBoomy.h"
#include "ActorMiniGameDistance.h"
#include "ActorBombLauncher.h"
#include "ActorWantedZoo.h"

CActorFactory::CActorFactory()
{
	edDebugPrintf("class CActor has changed, have you checked it's still aligned on 64 bytes ? \n");
	edDebugPrintf("class CActorMovable has changed, have you checked it's still aligned on 64 bytes ? \n");
	edDebugPrintf("class CActorAutonomous has changed, have you checked it's still aligned on 64 bytes ? \n");
}

class CActorSKIP_HACK : public CActor {
	virtual void Create(ByteCode* pByteCode) { SkipToNextActor(pByteCode); }
};

CActor* CActorFactory::Factory(ACTOR_CLASS classId, int count, int* outSize, void* pAlloc)
{
	if (classId < 0x57) {
		switch (classId) {
		case ACTOR:
			if (pAlloc != (void*)0x0) { CActor* pType = reinterpret_cast<CActor*>(pAlloc); DELETE_ARRAY_POLYMORPHIC(CActor, pType, count); return 0x0; }
			*outSize = sizeof(CActor);
			return NEW_ARRAY_POLYMORPHIC(CActor, count);
		case BOOMY:
			if (pAlloc != (void*)0x0) { CActorBoomy* pType = reinterpret_cast<CActorBoomy*>(pAlloc); DELETE_ARRAY_POLYMORPHIC(CActorBoomy, pType, count); return 0x0;
			}
			*outSize = sizeof(CActorBoomy);
			return NEW_ARRAY_POLYMORPHIC(CActorBoomy, count);
			break;
		case ACTOR_HERO_PRIVATE:
			if (pAlloc != (void*)0x0) { CActorHeroPrivate* pType = reinterpret_cast<CActorHeroPrivate*>(pAlloc); DELETE_ARRAY_POLYMORPHIC(CActorHeroPrivate, pType, count); return 0x0; }
			*outSize = sizeof(CActorHeroPrivate);
			return NEW_ARRAY_POLYMORPHIC(CActorHeroPrivate, count);
			break;
		case MICKEN:
			if (pAlloc != (void*)0x0) { CActorMicken* pType = reinterpret_cast<CActorMicken*>(pAlloc); DELETE_ARRAY_POLYMORPHIC(CActorMicken, pType, count); return 0x0; }
			*outSize = sizeof(CActorMicken);
			return NEW_ARRAY_POLYMORPHIC(CActorMicken, count);
			break;
		case WOLFEN:
			if (pAlloc != (void*)0x0) { CActorWolfen* pType = reinterpret_cast<CActorWolfen*>(pAlloc); DELETE_ARRAY_POLYMORPHIC(CActorWolfen, pType, count); return 0x0; }
			*outSize = sizeof(CActorWolfen);
			return NEW_ARRAY_POLYMORPHIC(CActorWolfen, count);
			break;
		case NATIV:
			if (pAlloc != (void*)0x0) { CActorNativ* pType = reinterpret_cast<CActorNativ*>(pAlloc); DELETE_ARRAY_POLYMORPHIC(CActorNativ, pType, count); return 0x0; }
			*outSize = sizeof(CActorNativ);
			return NEW_ARRAY_POLYMORPHIC(CActorNativ, count);
			break;
		case ELECTROLLA:
			if (pAlloc != (void*)0x0) { CActorElectrolla* pType = reinterpret_cast<CActorElectrolla*>(pAlloc); DELETE_ARRAY_POLYMORPHIC(CActorElectrolla, pType, count); return 0x0; }
			*outSize = sizeof(CActorElectrolla);
			return NEW_ARRAY_POLYMORPHIC(CActorElectrolla, count);
			break;
		case MOVING_PLATFORM:
			if (pAlloc != (void*)0x0) { CActorMovingPlatform* pType = reinterpret_cast<CActorMovingPlatform*>(pAlloc); DELETE_ARRAY_POLYMORPHIC(CActorMovingPlatform, pType, count); return 0x0; }
			*outSize = sizeof(CActorMovingPlatform);
			return NEW_ARRAY_POLYMORPHIC(CActorMovingPlatform, count);
			break;
		case BONUS:
			if (pAlloc != (void*)0x0) { CActorBonus* pType = reinterpret_cast<CActorBonus*>(pAlloc); DELETE_ARRAY_POLYMORPHIC(CActorBonus, pType, count); return 0x0; }
			*outSize = sizeof(CActorBonus);
			return NEW_ARRAY_POLYMORPHIC(CActorBonus, count);
			break;
		case ROPE:
			if (pAlloc != (void*)0x0) { CActorRope* pType = reinterpret_cast<CActorRope*>(pAlloc); DELETE_ARRAY_POLYMORPHIC(CActorRope, pType, count); return 0x0; }
			*outSize = sizeof(CActorRope);
			return NEW_ARRAY_POLYMORPHIC(CActorRope, count);
			break;
		case GRAVITY_AWARE:
			if (pAlloc != (void*)0x0) { CActorGravityAware* pType = reinterpret_cast<CActorGravityAware*>(pAlloc); DELETE_ARRAY_POLYMORPHIC(CActorGravityAware, pType, count); return 0x0; }
			*outSize = sizeof(CActorGravityAware);
			return NEW_ARRAY_POLYMORPHIC(CActorGravityAware, count);
			break;
		case SWITCH:
			if (pAlloc != (void*)0x0) { CActorSwitch* pType = reinterpret_cast<CActorSwitch*>(pAlloc); DELETE_ARRAY_POLYMORPHIC(CActorSwitch, pType, count); return 0x0; }
			*outSize = sizeof(CActorSwitch);
			return NEW_ARRAY_POLYMORPHIC(CActorSwitch, count);
			break;
		case WIND:
			if (pAlloc != (void*)0x0) { CActorWind* pType = reinterpret_cast<CActorWind*>(pAlloc); DELETE_ARRAY_POLYMORPHIC(CActorWind, pType, count); return 0x0; }
			*outSize = sizeof(CActorWind);
			return NEW_ARRAY_POLYMORPHIC(CActorWind, count);
			break;
		case COMMANDER:
			if (pAlloc != (void*)0x0) { CActorCommander* pType = reinterpret_cast<CActorCommander*>(pAlloc); DELETE_ARRAY_POLYMORPHIC(CActorCommander, pType, count); return 0x0; }
			*outSize = sizeof(CActorCommander);
			return NEW_ARRAY_POLYMORPHIC(CActorCommander, count);
			break;
		case AMBER:
			if (pAlloc != (void*)0x0) { CActorAmbre* pType = reinterpret_cast<CActorAmbre*>(pAlloc); DELETE_ARRAY_POLYMORPHIC(CActorAmbre, pType, count); return 0x0; }
			*outSize = sizeof(CActorAmbre);
			return NEW_ARRAY_POLYMORPHIC(CActorAmbre, count);
			break;
		case FRUIT:
			if (pAlloc != (void*)0x0) { CActorFruit* pType = reinterpret_cast<CActorFruit*>(pAlloc); DELETE_ARRAY_POLYMORPHIC(CActorFruit, pType, count); return 0x0; }
			*outSize = sizeof(CActorFruit);
			return NEW_ARRAY_POLYMORPHIC(CActorFruit, count);
			break;
		case BRIDGE:
			if (pAlloc != (void*)0x0) { CActorBridge* pType = reinterpret_cast<CActorBridge*>(pAlloc); DELETE_ARRAY_POLYMORPHIC(CActorBridge, pType, count); return 0x0; }
			*outSize = sizeof(CActorBridge);
			return NEW_ARRAY_POLYMORPHIC(CActorBridge, count);
			break;
		case CLUSTERISER:
			if (pAlloc != (void*)0x0) { CActorClusteriser* pType = reinterpret_cast<CActorClusteriser*>(pAlloc); DELETE_ARRAY_POLYMORPHIC(CActorClusteriser, pType, count); return 0x0; }
			*outSize = sizeof(CActorClusteriser);
			return NEW_ARRAY_POLYMORPHIC(CActorClusteriser, count);
			break;
		case COMPANION:
			if (pAlloc != (void*)0x0) { CActorCompanion* pType = reinterpret_cast<CActorCompanion*>(pAlloc); DELETE_ARRAY_POLYMORPHIC(CActorCompanion, pType, count); return 0x0; }
			*outSize = sizeof(CActorCompanion);
			return NEW_ARRAY_POLYMORPHIC(CActorCompanion, count);
			break;
		case DCA:
			if (pAlloc != (void*)0x0) { CActorDCA* pType = reinterpret_cast<CActorDCA*>(pAlloc); DELETE_ARRAY_POLYMORPHIC(CActorDCA, pType, count); return 0x0; }
			*outSize = sizeof(CActorDCA);
			return NEW_ARRAY_POLYMORPHIC(CActorDCA, count);
			break;
		case PROJECTILE:
			if (pAlloc != (void*)0x0) { CActorProjectile* pType = reinterpret_cast<CActorProjectile*>(pAlloc); DELETE_ARRAY_POLYMORPHIC(CActorProjectile, pType, count); return 0x0; }
			*outSize = sizeof(CActorProjectile);
			return NEW_ARRAY_POLYMORPHIC(CActorProjectile, count);
			break;
		case EXPLOSIVE_DISTRIBUTOR:
			if (pAlloc != (void*)0x0) { CActorExplosiveDistributor* pType = reinterpret_cast<CActorExplosiveDistributor*>(pAlloc); DELETE_ARRAY_POLYMORPHIC(CActorExplosiveDistributor, pType, count); return 0x0; }
			*outSize = sizeof(CActorExplosiveDistributor);
			return NEW_ARRAY_POLYMORPHIC(CActorExplosiveDistributor, count);
			break;
		case CHECKPOINT_MANAGER:
			if (pAlloc != (void*)0x0) { CActorCheckpointManager* pType = reinterpret_cast<CActorCheckpointManager*>(pAlloc); DELETE_ARRAY_POLYMORPHIC(CActorCheckpointManager, pType, count); return 0x0; }
			*outSize = sizeof(CActorCheckpointManager);
			return NEW_ARRAY_POLYMORPHIC(CActorCheckpointManager, count);
			break;
		case WEAPON:
			if (pAlloc != (void*)0x0) { CActorWeapon* pType = reinterpret_cast<CActorWeapon*>(pAlloc); DELETE_ARRAY_POLYMORPHIC(CActorWeapon, pType, count); return 0x0; }
			*outSize = sizeof(CActorWeapon);
			return NEW_ARRAY_POLYMORPHIC(CActorWeapon, count);
			break;
		case ARAIGNOS:
			if (pAlloc != (void*)0x0) { CActorAraignos* pType = reinterpret_cast<CActorAraignos*>(pAlloc); DELETE_ARRAY_POLYMORPHIC(CActorAraignos, pType, count); return 0x0; }
			*outSize = sizeof(CActorAraignos);
			return NEW_ARRAY_POLYMORPHIC(CActorAraignos, count);
			break;
		case HEDGEHOG:
			if (pAlloc != (void*)0x0) { CActorHedgehog* pType = reinterpret_cast<CActorHedgehog*>(pAlloc); DELETE_ARRAY_POLYMORPHIC(CActorHedgehog, pType, count); return 0x0; }
			*outSize = sizeof(CActorHedgehog);
			return NEW_ARRAY_POLYMORPHIC(CActorHedgehog, count);
			break;
		case TELEPORTER:
			if (pAlloc != (void*)0x0) { CActorTeleporter* pType = reinterpret_cast<CActorTeleporter*>(pAlloc); DELETE_ARRAY_POLYMORPHIC(CActorTeleporter, pType, count); return 0x0; }
			*outSize = sizeof(CActorTeleporter);
			return NEW_ARRAY_POLYMORPHIC(CActorTeleporter, count);
			break;
		case NOSE_MONSTER:
			if (pAlloc != (void*)0x0) { CActorNoseMonster* pType = reinterpret_cast<CActorNoseMonster*>(pAlloc); DELETE_ARRAY_POLYMORPHIC(CActorNoseMonster, pType, count); return 0x0; }
			*outSize = sizeof(CActorNoseMonster);
			return NEW_ARRAY_POLYMORPHIC(CActorNoseMonster, count);
			break;
		case WOOD_MONSTER:
			if (pAlloc != (void*)0x0) { CActorWoodMonster* pType = reinterpret_cast<CActorWoodMonster*>(pAlloc); DELETE_ARRAY_POLYMORPHIC(CActorWoodMonster, pType, count); return 0x0; }
			*outSize = sizeof(CActorWoodMonster);
			return NEW_ARRAY_POLYMORPHIC(CActorWoodMonster, count);
			break;
		case AMORTOS:
			if (pAlloc != (void*)0x0) { CActorAmortos* pType = reinterpret_cast<CActorAmortos*>(pAlloc); DELETE_ARRAY_POLYMORPHIC(CActorAmortos, pType, count); return 0x0; }
			*outSize = sizeof(CActorAmortos);
			return NEW_ARRAY_POLYMORPHIC(CActorAmortos, count);
			break;
		case TRAP:
			if (pAlloc != (void*)0x0) { CActorTrap* pType = reinterpret_cast<CActorTrap*>(pAlloc); DELETE_ARRAY_POLYMORPHIC(CActorTrap, pType, count); return 0x0; }
			*outSize = sizeof(CActorTrap);
			return NEW_ARRAY_POLYMORPHIC(CActorTrap, count);
			break;
		case FX:
			if (pAlloc != (void*)0x0) { CActorFx* pType = reinterpret_cast<CActorFx*>(pAlloc); DELETE_ARRAY_POLYMORPHIC(CActorFx, pType, count); return 0x0; }
			*outSize = sizeof(CActorFx);
			return NEW_ARRAY_POLYMORPHIC(CActorFx, count);
			break;
		case EGG:
			if (pAlloc != (void*)0x0) { CActorEgg* pType = reinterpret_cast<CActorEgg*>(pAlloc); DELETE_ARRAY_POLYMORPHIC(CActorEgg, pType, count); return 0x0; }
			*outSize = sizeof(CActorEgg);
			return NEW_ARRAY_POLYMORPHIC(CActorEgg, count);
			break;
		case AMORTOS_CMD:
			if (pAlloc != (void*)0x0) { CActorAmortosCmd* pType = reinterpret_cast<CActorAmortosCmd*>(pAlloc); DELETE_ARRAY_POLYMORPHIC(CActorAmortosCmd, pType, count); return 0x0; }
			*outSize = sizeof(CActorAmortosCmd);
			return NEW_ARRAY_POLYMORPHIC(CActorAmortosCmd, count);
			break;
		case MONEY:
			if (pAlloc != (void*)0x0) { CActorMoney* pType = reinterpret_cast<CActorMoney*>(pAlloc); DELETE_ARRAY_POLYMORPHIC(CActorMoney, pType, count); return 0x0; }
			*outSize = sizeof(CActorMoney);
			return NEW_ARRAY_POLYMORPHIC(CActorMoney, count);
			break;
		case BOX:
			if (pAlloc != (void*)0x0) { CActorBox* pType = reinterpret_cast<CActorBox*>(pAlloc); DELETE_ARRAY_POLYMORPHIC(CActorBox, pType, count); return 0x0; }
			*outSize = sizeof(CActorBox);
			return NEW_ARRAY_POLYMORPHIC(CActorBox, count);
			break;
		case ATON:
			if (pAlloc != (void*)0x0) { CActorAton* pType = reinterpret_cast<CActorAton*>(pAlloc); DELETE_ARRAY_POLYMORPHIC(CActorAton, pType, count); return 0x0; }
			*outSize = sizeof(CActorAton);
			return NEW_ARRAY_POLYMORPHIC(CActorAton, count);
			break;
		case BASIC_BOX:
			if (pAlloc != (void*)0x0) { CActorBasicBox* pType = reinterpret_cast<CActorBasicBox*>(pAlloc); DELETE_ARRAY_POLYMORPHIC(CActorBasicBox, pType, count); return 0x0; }
			*outSize = sizeof(CActorBasicBox);
			return NEW_ARRAY_POLYMORPHIC(CActorBasicBox, count);
			break;
		case NATIV_CMD:
			if (pAlloc != (void*)0x0) { CActorNativCmd* pType = reinterpret_cast<CActorNativCmd*>(pAlloc); DELETE_ARRAY_POLYMORPHIC(CActorNativCmd, pType, count); return 0x0; }
			*outSize = sizeof(CActorNativCmd);
			return NEW_ARRAY_POLYMORPHIC(CActorNativCmd, count);
			break;
		case FOG_MANAGER:
			if (pAlloc != (void*)0x0) { CActorFogManager* pType = reinterpret_cast<CActorFogManager*>(pAlloc); DELETE_ARRAY_POLYMORPHIC(CActorFogManager, pType, count); return 0x0; }
			*outSize = sizeof(CActorFogManager);
			return NEW_ARRAY_POLYMORPHIC(CActorFogManager, count);
			break;
		case SHOOT:
			if (pAlloc != (void*)0x0) { CActorShoot* pType = reinterpret_cast<CActorShoot*>(pAlloc); DELETE_ARRAY_POLYMORPHIC(CActorShoot, pType, count); return 0x0; }
			*outSize = sizeof(CActorShoot);
			return NEW_ARRAY_POLYMORPHIC(CActorShoot, count);
			break;
		case SHOCKER:
			if (pAlloc != (void*)0x0) { CActorShocker* pType = reinterpret_cast<CActorShocker*>(pAlloc); DELETE_ARRAY_POLYMORPHIC(CActorShocker, pType, count); return 0x0; }
			*outSize = sizeof(CActorShocker);
			return NEW_ARRAY_POLYMORPHIC(CActorShocker, count);
			break;
		case STILLER:
			if (pAlloc != (void*)0x0) { CActorStiller* pType = reinterpret_cast<CActorStiller*>(pAlloc); DELETE_ARRAY_POLYMORPHIC(CActorStiller, pType, count); return 0x0; }
			*outSize = sizeof(CActorStiller);
			return NEW_ARRAY_POLYMORPHIC(CActorStiller, count);
			break;
		case BLAZER:
			if (pAlloc != (void*)0x0) { CActorBlazer* pType = reinterpret_cast<CActorBlazer*>(pAlloc); DELETE_ARRAY_POLYMORPHIC(CActorBlazer, pType, count); return 0x0; }
			*outSize = sizeof(CActorBlazer);
			return NEW_ARRAY_POLYMORPHIC(CActorBlazer, count);
			break;
		case SHOCKER_CMD:
			if (pAlloc != (void*)0x0) { CActorShockerCmd* pType = reinterpret_cast<CActorShockerCmd*>(pAlloc); DELETE_ARRAY_POLYMORPHIC(CActorShockerCmd, pType, count); return 0x0; }
			*outSize = sizeof(CActorShockerCmd);
			return NEW_ARRAY_POLYMORPHIC(CActorShockerCmd, count);
			break;
		case RUNE:
			if (pAlloc != (void*)0x0) { CActorRune* pType = reinterpret_cast<CActorRune*>(pAlloc); DELETE_ARRAY_POLYMORPHIC(CActorRune, pType, count); return 0x0; }
			*outSize = sizeof(CActorRune);
			return NEW_ARRAY_POLYMORPHIC(CActorRune, count);
			break;
		case BONUS_FOUNTAIN:
			if (pAlloc != (void*)0x0) { CActorBonusFountain* pType = reinterpret_cast<CActorBonusFountain*>(pAlloc); DELETE_ARRAY_POLYMORPHIC(CActorBonusFountain, pType, count); return 0x0; }
			*outSize = sizeof(CActorBonusFountain);
			return NEW_ARRAY_POLYMORPHIC(CActorBonusFountain, count);
			break;
		case EVENT_GENERATOR:
			if (pAlloc != (void*)0x0) { CActorEventGenerator* pType = reinterpret_cast<CActorEventGenerator*>(pAlloc); DELETE_ARRAY_POLYMORPHIC(CActorEventGenerator, pType, count); return 0x0; }
			*outSize = sizeof(CActorEventGenerator);
			return NEW_ARRAY_POLYMORPHIC(CActorEventGenerator, count);
			break;
		case WOOF:
			if (pAlloc != (void*)0x0) { CActorWoof* pType = reinterpret_cast<CActorWoof*>(pAlloc); DELETE_ARRAY_POLYMORPHIC(CActorWoof, pType, count); return 0x0; }
			*outSize = sizeof(CActorWoof);
			return NEW_ARRAY_POLYMORPHIC(CActorWoof, count);
			break;
		case HELPER_SIGN:
			if (pAlloc != (void*)0x0) { CActorHelperSign* pType = reinterpret_cast<CActorHelperSign*>(pAlloc); DELETE_ARRAY_POLYMORPHIC(CActorHelperSign, pType, count); return 0x0; }
			*outSize = sizeof(CActorHelperSign);
			return NEW_ARRAY_POLYMORPHIC(CActorHelperSign, count);
			break;
		case BUNCH:
			if (pAlloc != (void*)0x0) { CActorBunch* pType = reinterpret_cast<CActorBunch*>(pAlloc); DELETE_ARRAY_POLYMORPHIC(CActorBunch, pType, count); return 0x0; }
			*outSize = sizeof(CActorBunch);
			return NEW_ARRAY_POLYMORPHIC(CActorBunch, count);
			break;
		case BRAZUL:
			if (pAlloc != (void*)0x0) { CActorBrazul* pType = reinterpret_cast<CActorBrazul*>(pAlloc); DELETE_ARRAY_POLYMORPHIC(CActorBrazul, pType, count); return 0x0; }
			*outSize = sizeof(CActorBrazul);
			return NEW_ARRAY_POLYMORPHIC(CActorBrazul, count);
			break;
		case PUNCHING_BALL:
			if (pAlloc != (void*)0x0) { CActorPunchingBall* pType = reinterpret_cast<CActorPunchingBall*>(pAlloc); DELETE_ARRAY_POLYMORPHIC(CActorPunchingBall, pType, count); return 0x0; }
			*outSize = sizeof(CActorPunchingBall);
			return NEW_ARRAY_POLYMORPHIC(CActorPunchingBall, count);
			break;
		case NATIV_SHOP:
			if (pAlloc != (void*)0x0) { CActorNativShop* pType = reinterpret_cast<CActorNativShop*>(pAlloc); DELETE_ARRAY_POLYMORPHIC(CActorNativShop, pType, count); return 0x0; }
			*outSize = sizeof(CActorNativShop);
			return NEW_ARRAY_POLYMORPHIC(CActorNativShop, count);
			break;
		case ADD_ON_PURCHASE:
			if (pAlloc != (void*)0x0) { CActorAddOnPurchase* pType = reinterpret_cast<CActorAddOnPurchase*>(pAlloc); DELETE_ARRAY_POLYMORPHIC(CActorAddOnPurchase, pType, count); return 0x0; }
			*outSize = sizeof(CActorAddOnPurchase);
			return NEW_ARRAY_POLYMORPHIC(CActorAddOnPurchase, count);
			break;
		case CARE_BOY:
			if (pAlloc != (void*)0x0) { CActorCareBoy* pType = reinterpret_cast<CActorCareBoy*>(pAlloc); DELETE_ARRAY_POLYMORPHIC(CActorCareBoy, pType, count); return 0x0; }
			*outSize = sizeof(CActorCareBoy);
			return NEW_ARRAY_POLYMORPHIC(CActorCareBoy, count);
			break;
		case SHIP:
			if (pAlloc != (void*)0x0) { CActorShip* pType = reinterpret_cast<CActorShip*>(pAlloc); DELETE_ARRAY_POLYMORPHIC(CActorShip, pType, count); return 0x0; }
			*outSize = sizeof(CActorShip);
			return NEW_ARRAY_POLYMORPHIC(CActorShip, count);
			break;
		case HUNTER:
			if (pAlloc != (void*)0x0) { CActorHunter* pType = reinterpret_cast<CActorHunter*>(pAlloc); DELETE_ARRAY_POLYMORPHIC(CActorHunter, pType, count); return 0x0; }
			*outSize = sizeof(CActorHunter);
			return NEW_ARRAY_POLYMORPHIC(CActorHunter, count);
			break;
		case JAMGUT:
			if (pAlloc != (void*)0x0) { CActorJamGut* pType = reinterpret_cast<CActorJamGut*>(pAlloc); DELETE_ARRAY_POLYMORPHIC(CActorJamGut, pType, count); return 0x0; }
			*outSize = sizeof(CActorJamGut);
			return NEW_ARRAY_POLYMORPHIC(CActorJamGut, count);
			break;
		case ACCELERATOS:
			if (pAlloc != (void*)0x0) { CActorAcceleratos* pType = reinterpret_cast<CActorAcceleratos*>(pAlloc); DELETE_ARRAY_POLYMORPHIC(CActorAcceleratos, pType, count); return 0x0; }
			*outSize = sizeof(CActorAcceleratos);
			return NEW_ARRAY_POLYMORPHIC(CActorAcceleratos, count);
			break;
		case PATTERN_CON:
			if (pAlloc != (void*)0x0) { CActorPattern* pType = reinterpret_cast<CActorPattern*>(pAlloc); DELETE_ARRAY_POLYMORPHIC(CActorPattern, pType, count); return 0x0; }
			*outSize = sizeof(CActorPattern);
			return NEW_ARRAY_POLYMORPHIC(CActorPattern, count);
			break;
		case PATTERN_CMD:
			if (pAlloc != (void*)0x0) { CActorPatternCmd* pType = reinterpret_cast<CActorPatternCmd*>(pAlloc); DELETE_ARRAY_POLYMORPHIC(CActorPatternCmd, pType, count); return 0x0; }
			*outSize = sizeof(CActorPatternCmd);
			return NEW_ARRAY_POLYMORPHIC(CActorPatternCmd, count);
			break;
		case CREDITS:
			if (pAlloc != (void*)0x0) { CActorCredits* pType = reinterpret_cast<CActorCredits*>(pAlloc); DELETE_ARRAY_POLYMORPHIC(CActorCredits, pType, count); return 0x0; }
			*outSize = sizeof(CActorCredits);
			return NEW_ARRAY_POLYMORPHIC(CActorCredits, count);
			break;
		case MINI_GAME:
			if (pAlloc != (void*)0x0) { CActorMiniGame* pType = reinterpret_cast<CActorMiniGame*>(pAlloc); DELETE_ARRAY_POLYMORPHIC(CActorMiniGame, pType, count); return 0x0; }
			*outSize = sizeof(CActorMiniGame);
			return NEW_ARRAY_POLYMORPHIC(CActorMiniGame, count);
			break;
		case MINI_GAME_TIME_ATTACK:
			if (pAlloc != (void*)0x0) { CActorMiniGameTimeAttack* pType = reinterpret_cast<CActorMiniGameTimeAttack*>(pAlloc); DELETE_ARRAY_POLYMORPHIC(CActorMiniGameTimeAttack, pType, count); return 0x0; }
			*outSize = sizeof(CActorMiniGameTimeAttack);
			return NEW_ARRAY_POLYMORPHIC(CActorMiniGameTimeAttack, count);
			break;
		case MINI_GAME_BOX_COUNTER:
			if (pAlloc != (void*)0x0) { CActorMiniGameBoxCounter* pType = reinterpret_cast<CActorMiniGameBoxCounter*>(pAlloc); DELETE_ARRAY_POLYMORPHIC(CActorMiniGameBoxCounter, pType, count); return 0x0; }
			*outSize = sizeof(CActorMiniGameBoxCounter);
			return NEW_ARRAY_POLYMORPHIC(CActorMiniGameBoxCounter, count);
			break;
		case MINI_GAMES_MANAGER:
			if (pAlloc != (void*)0x0) { CActorMiniGamesManager* pType = reinterpret_cast<CActorMiniGamesManager*>(pAlloc); DELETE_ARRAY_POLYMORPHIC(CActorMiniGamesManager, pType, count); return 0x0; }
			*outSize = sizeof(CActorMiniGamesManager);
			return NEW_ARRAY_POLYMORPHIC(CActorMiniGamesManager, count);
			break;
		case MINI_GAMES_ORGANIZER:
			if (pAlloc != (void*)0x0) { CActorMiniGamesOrganizer* pType = reinterpret_cast<CActorMiniGamesOrganizer*>(pAlloc); DELETE_ARRAY_POLYMORPHIC(CActorMiniGamesOrganizer, pType, count); return 0x0; }
			*outSize = sizeof(CActorMiniGamesOrganizer);
			return NEW_ARRAY_POLYMORPHIC(CActorMiniGamesOrganizer, count);
			break;
		case MINI_GAME_BOOMY:
			if (pAlloc != (void*)0x0) { CActorMiniGameBoomy* pType = reinterpret_cast<CActorMiniGameBoomy*>(pAlloc); DELETE_ARRAY_POLYMORPHIC(CActorMiniGameBoomy, pType, count); return 0x0; }
			*outSize = sizeof(CActorMiniGameBoomy);
			return NEW_ARRAY_POLYMORPHIC(CActorMiniGameBoomy, count);
			break;
		case MINI_GAME_DISTANCE:
			if (pAlloc != (void*)0x0) { CActorMiniGameDistance* pType = reinterpret_cast<CActorMiniGameDistance*>(pAlloc); DELETE_ARRAY_POLYMORPHIC(CActorMiniGameDistance, pType, count); return 0x0; }
			*outSize = sizeof(CActorMiniGameDistance);
			return NEW_ARRAY_POLYMORPHIC(CActorMiniGameDistance, count);
			break;
		case BOMB_LAUNCHER:
			if (pAlloc != (void*)0x0) { CActorBombLauncher* pType = reinterpret_cast<CActorBombLauncher*>(pAlloc); DELETE_ARRAY_POLYMORPHIC(CActorBombLauncher, pType, count); return 0x0; }
			*outSize = sizeof(CActorBombLauncher);
			return NEW_ARRAY_POLYMORPHIC(CActorBombLauncher, count);
			break;
		case WANTED_ZOO:
			if (pAlloc != (void*)0x0) { CActorWantedZoo* pType = reinterpret_cast<CActorWantedZoo*>(pAlloc); DELETE_ARRAY_POLYMORPHIC(CActorWantedZoo, pType, count); return 0x0; }
			*outSize = sizeof(CActorWantedZoo);
			return NEW_ARRAY_POLYMORPHIC(CActorWantedZoo, count);
			break;
		default:
			classId;
			IMPLEMENTATION_GUARD();
			ACTOR_LOG(LogLevel::Info, "Unimplemented class: 0x{:x} ({})", (int)classId, (int)classId);

			if (pAlloc != (void*)0x0) { CActorSKIP_HACK* pType = reinterpret_cast<CActorSKIP_HACK*>(pAlloc); DELETE_ARRAY_POLYMORPHIC(CActorSKIP_HACK, pType, count); return 0x0; }
			*outSize = sizeof(CActorSKIP_HACK);
			return NEW_ARRAY_POLYMORPHIC(CActorSKIP_HACK, count);
			break;
		}
	}
	*outSize = 0;
	return (CActor*)0x0;
}

ActorClassProperties CActorFactory::gClassProperties[ACTOR_NUM_CLASSES] = {
{
0x80000000,
0x40000000,
0x0,
0x0,
0x0,
0x0,
}, {
0x80001800,
0x40000000,
0x0,
0x0,
0x0,
0x0,
}, {
0x80000000,
0x40000000,
0x0,
0x0,
0x0,
0x0,
}, {
0x80000000,
0x40000000,
0x0,
0x0,
0x0,
0x0,
}, {
0x80000000,
0x40000000,
0xC8,
0x1F4,
0x0,
0x0,
}, {
0x80000A00,
0x40000000,
0x0,
0x0,
0x0,
0x0,
}, {
0xA00090F6,
0x40000601,
0x0,
0x1F4,
0x3,
0x138,
}, {
0xA0008457,
0x40002221,
0x64,
0x190,
0x0,
0x0,
}, {
0xA0008252,
0x40000221,
0x64,
0x64,
0x0,
0x0,
}, {
0x800191F7,
0x40000621,
0xC8,
0x1F4,
0x1,
0x4,
}, {
0x80001043,
0x40000201,
0x0,
0x0,
0x0,
0x0,
}, {
0x80028800,
0x40000221,
0x1F4,
0x190,
0x20001,
0xC,
}, {
0x80020000,
0x40000221,
0x1F4,
0x0,
0x0,
0x0,
}, {
0x80002810,
0x400195FF,
0x12C,
0x0,
0x20004,
0x24,
}, {
0x80040A20,
0x40000000,
0x0,
0x0,
0x1,
0x20,
}, {
0x80000800,
0x4001A820,
0x12C,
0x0,
0x20001,
0x10,
}, {
0x80022040,
0x40008049,
0x0,
0x0,
0x1,
0x10,
}, {
0x80028901,
0x40000221,
0x190,
0x12C,
0x20001,
0x18,
}, {
0x80028800,
0x40000000,
0x0,
0x190,
0x1,
0x4,
}, {
0x80000000,
0x40000000,
0x0,
0x0,
0x0,
0x0,
}, {
0x80028900,
0x40000201,
0x0,
0xC8,
0x0,
0x0,
}, {
0x80000000,
0x40000001,
0x1F4,
0x0,
0x0,
0x0,
}, {
0x8000045B,
0x40000221,
0x12C,
0x0,
0x0,
0x0,
}, {
0x80000004,
0x40000000,
0x0,
0x0,
0x1,
0x4,
}, {
0x80000000,
0x40000000,
0x0,
0x0,
0x0,
0x0,
}, {
0x80000062,
0x40000201,
0x0,
0x0,
0x1,
0x20,
}, {
0x80020004,
0x40000000,
0x0,
0x0,
0x0,
0x0,
}, {
0x8002A801,
0x4000C059,
0x0,
0xC8,
0x0,
0x0,
}, {
0xA0010453,
0x40000220,
0x258,
0x0,
0x0,
0x0,
}, {
0x8002A801,
0x4000806D,
0x12C,
0x12C,
0x0,
0x0,
}, {
0x80000000,
0x40000000,
0x0,
0x0,
0x0,
0x0,
}, {
0x80000000,
0x40000000,
0x0,
0x0,
0x0,
0x0,
}, {
0x80000800,
0x40000000,
0x0,
0x0,
0x0,
0x0,
}, {
0x80030810,
0x4000A221,
0x1F4,
0x0,
0x0,
0x0,
}, {
0xA0018052,
0x40002221,
0x1F4,
0x190,
0x0,
0x0,
}, {
0x80018042,
0x40002221,
0x1F4,
0x190,
0x0,
0x0,
}, {
0x80000820,
0x40008000,
0x0,
0x0,
0x3,
0x10,
}, {
0x80008810,
0x40008221,
0x1F4,
0x190,
0x0,
0x0,
}, {
0x80008810,
0x40000221,
0x1F4,
0x190,
0x1,
0x4,
}, {
0x80008002,
0x40000001,
0x0,
0x190,
0x0,
0x0,
}, {
0xA00090F7,
0x40000621,
0xC8,
0x1F4,
0x0,
0x0,
}, {
0x80028810,
0x4000C421,
0x12C,
0x190,
0x0,
0x0,
}, {
0xA0009052,
0x40008221,
0x1F4,
0x190,
0x1,
0x4,
}, {
0x80020803,
0x4000A001,
0x0,
0x0,
0x0,
0x0,
}, {
0x80028811,
0x40002621,
0x12C,
0x12C,
0x0,
0x0,
}, {
0x80020800,
0x40000000,
0x0,
0x0,
0x0,
0x0,
}, {
0x80020000,
0x40000000,
0x0,
0x0,
0x0,
0x0,
}, {
0x80000820,
0x40000000,
0x0,
0x0,
0x1,
0x4,
}, {
0x80029802,
0x40000201,
0xC8,
0xC8,
0x0,
0x0,
}, {
0x80000000,
0x40000000,
0x0,
0x0,
0x0,
0x0,
}, {
0x80000220,
0x40000000,
0x0,
0x0,
0x1,
0x20,
}, {
0x80009057,
0x40000601,
0x0,
0x64,
0x2,
0x30,
}, {
0x80000021,
0x40000000,
0x0,
0x0,
0x4,
0x4,
}, {
0x8003E850,
0x400094FB,
0x12C,
0xC8,
0x1,
0x4,
}, {
0x80000272,
0x40008201,
0x64,
0x0,
0x1,
0x10,
}, {
0x80000810,
0x40008001,
0x0,
0x0,
0x0,
0x0,
}, {
0x80001062,
0x40000201,
0x0,
0x0,
0x1,
0x30,
}, {
0x8003E810,
0x4000846B,
0x12C,
0xC8,
0x1,
0x4,
}, {
0x80020800,
0x40008000,
0x0,
0x0,
0x0,
0x0,
}, {
0x80000000,
0x40000000,
0x0,
0x0,
0x0,
0x0,
}, {
0x80000000,
0x40000000,
0x0,
0x0,
0x0,
0x0,
}, {
0x80000000,
0x40000000,
0x0,
0x0,
0x0,
0x0,
}, {
0x80010012,
0x40008221,
0x1F4,
0x0,
0x1,
0x4,
}, {
0x80000002,
0x40000221,
0x1F4,
0x0,
0x0,
0x0,
}, {
0x80000000,
0x40000000,
0x0,
0x0,
0x0,
0x0,
}, {
0x80000002,
0x40004221,
0x1F4,
0x0,
0x0,
0x0,
}, {
0x80000000,
0x40000000,
0x0,
0x0,
0x0,
0x0,
}, {
0x80000001,
0x40000000,
0x0,
0x0,
0x0,
0x0,
}, {
0x80000000,
0x40000000,
0x0,
0x0,
0x0,
0x0,
}, {
0x80000020,
0x40000000,
0x0,
0x0,
0x1,
0x7C,
}, {
0x80000020,
0x40000000,
0x0,
0x0,
0x1,
0x7C,
}, {
0x80000020,
0x40000000,
0x0,
0x0,
0x1,
0x7C,
}, {
0x80000252,
0x40008201,
0x64,
0x0,
0x0,
0x0,
}, {
0x80000021,
0x40000000,
0x0,
0x0,
0x1,
0x4,
}, {
0x80000800,
0x40000000,
0x0,
0x0,
0x0,
0x0,
}, {
0x80010002,
0x40000221,
0x1F4,
0x0,
0x0,
0x0,
}, {
0x80000800,
0x40008221,
0x1F4,
0x0,
0x0,
0x0,
}, {
0x80000810,
0x400090ED,
0x1F4,
0x0,
0x0,
0x0,
}, {
0x800091F7,
0x40000621,
0xC8,
0x1F4,
0x1,
0x4,
}, {
0x800091F7,
0x40000621,
0xC8,
0x1F4,
0x1,
0x4,
}, {
0x80000000,
0x40000000,
0x0,
0x0,
0x0,
0x0,
}, {
0x80000020,
0x40000000,
0x0,
0x0,
0x0,
0x0,
}, {
0x80000002,
0x40000221,
0x1F4,
0x190,
0x0,
0x0,
}, {
0x80000000,
0x40000000,
0x0,
0x0,
0x0,
0x0,
}, {
0x80000000,
0x40000000,
0x0,
0x0,
0x0,
0x0,
}, {
0x80000020,
0x40000000,
0x0,
0x0,
0x1,
0x7C,
}, {
0x80000000,
0x40000000,
0x0,
0x0,
0x0,
0x0,
}
};
