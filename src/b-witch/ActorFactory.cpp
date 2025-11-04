#include "ActorFactory.h"
#include "Actor.h"

#include "ActorBoomy.h"
#include "ActorHeroPrivate.h"
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
			if (pAlloc != (void*)0x0) { CActor* pType = reinterpret_cast<CActor*>(pAlloc); delete[] pType; return 0x0; }
			*outSize = sizeof(CActor);
			return new CActor[count];
		case BOOMY:
			if (pAlloc != (void*)0x0) { CActorBoomy* pType = reinterpret_cast<CActorBoomy*>(pAlloc); delete[] pType; return 0x0;
			}
			*outSize = sizeof(CActorBoomy);
			return new CActorBoomy[count];
			break;
		case ACTOR_HERO_PRIVATE:
			if (pAlloc != (void*)0x0) { CActorHeroPrivate* pType = reinterpret_cast<CActorHeroPrivate*>(pAlloc); delete[] pType; return 0x0; }
			*outSize = sizeof(CActorHeroPrivate);
			return new CActorHeroPrivate[count];
			break;
		case MICKEN:
			if (pAlloc != (void*)0x0) { CActorMicken* pType = reinterpret_cast<CActorMicken*>(pAlloc); delete[] pType; return 0x0; }
			*outSize = sizeof(CActorMicken);
			return new CActorMicken[count];
			break;
		case WOLFEN:
			if (pAlloc != (void*)0x0) { CActorWolfen* pType = reinterpret_cast<CActorWolfen*>(pAlloc); delete[] pType; return 0x0; }
			*outSize = sizeof(CActorWolfen);
			return new CActorWolfen[count];
			break;
		case NATIV:
			if (pAlloc != (void*)0x0) { CActorNativ* pType = reinterpret_cast<CActorNativ*>(pAlloc); delete[] pType; return 0x0; }
			*outSize = sizeof(CActorNativ);
			return new CActorNativ[count];
			break;
		case ELECTROLLA:
			if (pAlloc != (void*)0x0) { CActorElectrolla* pType = reinterpret_cast<CActorElectrolla*>(pAlloc); delete[] pType; return 0x0; }
			*outSize = sizeof(CActorElectrolla);
			return new CActorElectrolla[count];
			break;
		case MOVING_PLATFORM:
			if (pAlloc != (void*)0x0) { CActorMovingPlatform* pType = reinterpret_cast<CActorMovingPlatform*>(pAlloc); delete[] pType; return 0x0; }
			*outSize = sizeof(CActorMovingPlatform);
			return new CActorMovingPlatform[count];
			break;
		case BONUS:
			if (pAlloc != (void*)0x0) { CActorBonus* pType = reinterpret_cast<CActorBonus*>(pAlloc); delete[] pType; return 0x0; }
			*outSize = sizeof(CActorBonus);
			return new CActorBonus[count];
			break;
		case ROPE:
			if (pAlloc != (void*)0x0) { CActorRope* pType = reinterpret_cast<CActorRope*>(pAlloc); delete[] pType; return 0x0; }
			*outSize = sizeof(CActorRope);
			return new CActorRope[count];
			break;
		case GRAVITY_AWARE:
			if (pAlloc != (void*)0x0) { CActorGravityAware* pType = reinterpret_cast<CActorGravityAware*>(pAlloc); delete[] pType; return 0x0; }
			*outSize = sizeof(CActorGravityAware);
			return new CActorGravityAware[count];
			break;
		case SWITCH:
			if (pAlloc != (void*)0x0) { CActorSwitch* pType = reinterpret_cast<CActorSwitch*>(pAlloc); delete[] pType; return 0x0; }
			*outSize = sizeof(CActorSwitch);
			return new CActorSwitch[count];
			break;
		case WIND:
			if (pAlloc != (void*)0x0) { CActorWind* pType = reinterpret_cast<CActorWind*>(pAlloc); delete[] pType; return 0x0; }
			*outSize = sizeof(CActorWind);
			return new CActorWind[count];
			break;
		case COMMANDER:
			if (pAlloc != (void*)0x0) { CActorCommander* pType = reinterpret_cast<CActorCommander*>(pAlloc); delete[] pType; return 0x0; }
			*outSize = sizeof(CActorCommander);
			return new CActorCommander[count];
			break;
		case AMBER:
			if (pAlloc != (void*)0x0) { CActorAmbre* pType = reinterpret_cast<CActorAmbre*>(pAlloc); delete[] pType; return 0x0; }
			*outSize = sizeof(CActorAmbre);
			return new CActorAmbre[count];
			break;
		case FRUIT:
			if (pAlloc != (void*)0x0) { CActorFruit* pType = reinterpret_cast<CActorFruit*>(pAlloc); delete[] pType; return 0x0; }
			*outSize = sizeof(CActorFruit);
			return new CActorFruit[count];
			break;
		case BRIDGE:
			if (pAlloc != (void*)0x0) { CActorBridge* pType = reinterpret_cast<CActorBridge*>(pAlloc); delete[] pType; return 0x0; }
			*outSize = sizeof(CActorBridge);
			return new CActorBridge[count];
			break;
		case CLUSTERISER:
			if (pAlloc != (void*)0x0) { CActorClusteriser* pType = reinterpret_cast<CActorClusteriser*>(pAlloc); delete[] pType; return 0x0; }
			*outSize = sizeof(CActorClusteriser);
			return new CActorClusteriser[count];
			break;
		case COMPANION:
			if (pAlloc != (void*)0x0) { CActorCompanion* pType = reinterpret_cast<CActorCompanion*>(pAlloc); delete[] pType; return 0x0; }
			*outSize = sizeof(CActorCompanion);
			return new CActorCompanion[count];
			break;
		case DCA:
			if (pAlloc != (void*)0x0) { CActorDCA* pType = reinterpret_cast<CActorDCA*>(pAlloc); delete[] pType; return 0x0; }
			*outSize = sizeof(CActorDCA);
			return new CActorDCA[count];
			break;
		case PROJECTILE:
			if (pAlloc != (void*)0x0) { CActorProjectile* pType = reinterpret_cast<CActorProjectile*>(pAlloc); delete[] pType; return 0x0; }
			*outSize = sizeof(CActorProjectile);
			return new CActorProjectile[count];
			break;
		case EXPLOSIVE_DISTRIBUTOR:
			if (pAlloc != (void*)0x0) { CActorExplosiveDistributor* pType = reinterpret_cast<CActorExplosiveDistributor*>(pAlloc); delete[] pType; return 0x0; }
			*outSize = sizeof(CActorExplosiveDistributor);
			return new CActorExplosiveDistributor[count];
			break;
		case CHECKPOINT_MANAGER:
			if (pAlloc != (void*)0x0) { CActorCheckpointManager* pType = reinterpret_cast<CActorCheckpointManager*>(pAlloc); delete[] pType; return 0x0; }
			*outSize = sizeof(CActorCheckpointManager);
			return new CActorCheckpointManager[count];
			break;
		case WEAPON:
			if (pAlloc != (void*)0x0) { CActorWeapon* pType = reinterpret_cast<CActorWeapon*>(pAlloc); delete[] pType; return 0x0; }
			*outSize = sizeof(CActorWeapon);
			return new CActorWeapon[count];
			break;
		case ARAIGNOS:
			if (pAlloc != (void*)0x0) { CActorAraignos* pType = reinterpret_cast<CActorAraignos*>(pAlloc); delete[] pType; return 0x0; }
			*outSize = sizeof(CActorAraignos);
			return new CActorAraignos[count];
			break;
		case HEDGEHOG:
			if (pAlloc != (void*)0x0) { CActorHedgehog* pType = reinterpret_cast<CActorHedgehog*>(pAlloc); delete[] pType; return 0x0; }
			*outSize = sizeof(CActorHedgehog);
			return new CActorHedgehog[count];
			break;
		case TELEPORTER:
			if (pAlloc != (void*)0x0) { CActorTeleporter* pType = reinterpret_cast<CActorTeleporter*>(pAlloc); delete[] pType; return 0x0; }
			*outSize = sizeof(CActorTeleporter);
			return new CActorTeleporter[count];
			break;
		case NOSE_MONSTER:
			if (pAlloc != (void*)0x0) { CActorNoseMonster* pType = reinterpret_cast<CActorNoseMonster*>(pAlloc); delete[] pType; return 0x0; }
			*outSize = sizeof(CActorNoseMonster);
			return new CActorNoseMonster[count];
			break;
		case WOOD_MONSTER:
			if (pAlloc != (void*)0x0) { CActorWoodMonster* pType = reinterpret_cast<CActorWoodMonster*>(pAlloc); delete[] pType; return 0x0; }
			*outSize = sizeof(CActorWoodMonster);
			return new CActorWoodMonster[count];
			break;
		case AMORTOS:
			if (pAlloc != (void*)0x0) { CActorAmortos* pType = reinterpret_cast<CActorAmortos*>(pAlloc); delete[] pType; return 0x0; }
			*outSize = sizeof(CActorAmortos);
			return new CActorAmortos[count];
			break;
		case TRAP:
			if (pAlloc != (void*)0x0) { CActorTrap* pType = reinterpret_cast<CActorTrap*>(pAlloc); delete[] pType; return 0x0; }
			*outSize = sizeof(CActorTrap);
			return new CActorTrap[count];
			break;
		case FX:
			if (pAlloc != (void*)0x0) { CActorFx* pType = reinterpret_cast<CActorFx*>(pAlloc); delete[] pType; return 0x0; }
			*outSize = sizeof(CActorFx);
			return new CActorFx[count];
			break;
		case EGG:
			if (pAlloc != (void*)0x0) { CActorEgg* pType = reinterpret_cast<CActorEgg*>(pAlloc); delete[] pType; return 0x0; }
			*outSize = sizeof(CActorEgg);
			return new CActorEgg[count];
			break;
		case AMORTOS_CMD:
			if (pAlloc != (void*)0x0) { CActorAmortosCmd* pType = reinterpret_cast<CActorAmortosCmd*>(pAlloc); delete[] pType; return 0x0; }
			*outSize = sizeof(CActorAmortosCmd);
			return new CActorAmortosCmd[count];
			break;
		case MONEY:
			if (pAlloc != (void*)0x0) { CActorMoney* pType = reinterpret_cast<CActorMoney*>(pAlloc); delete[] pType; return 0x0; }
			*outSize = sizeof(CActorMoney);
			return new CActorMoney[count];
			break;
		case BOX:
			if (pAlloc != (void*)0x0) { CActorBox* pType = reinterpret_cast<CActorBox*>(pAlloc); delete[] pType; return 0x0; }
			*outSize = sizeof(CActorBox);
			return new CActorBox[count];
			break;
		case ATON:
			if (pAlloc != (void*)0x0) { CActorAton* pType = reinterpret_cast<CActorAton*>(pAlloc); delete[] pType; return 0x0; }
			*outSize = sizeof(CActorAton);
			return new CActorAton[count];
			break;
		case BASIC_BOX:
			if (pAlloc != (void*)0x0) { CActorBasicBox* pType = reinterpret_cast<CActorBasicBox*>(pAlloc); delete[] pType; return 0x0; }
			*outSize = sizeof(CActorBasicBox);
			return new CActorBasicBox[count];
			break;
		case NATIV_CMD:
			if (pAlloc != (void*)0x0) { CActorNativCmd* pType = reinterpret_cast<CActorNativCmd*>(pAlloc); delete[] pType; return 0x0; }
			*outSize = sizeof(CActorNativCmd);
			return new CActorNativCmd[count];
			break;
		case FOG_MANAGER:
			if (pAlloc != (void*)0x0) { CActorFogManager* pType = reinterpret_cast<CActorFogManager*>(pAlloc); delete[] pType; return 0x0; }
			*outSize = sizeof(CActorFogManager);
			return new CActorFogManager[count];
			break;
		case SHOOT:
			if (pAlloc != (void*)0x0) { CActorShoot* pType = reinterpret_cast<CActorShoot*>(pAlloc); delete[] pType; return 0x0; }
			*outSize = sizeof(CActorShoot);
			return new CActorShoot[count];
			break;
		case SHOCKER:
			if (pAlloc != (void*)0x0) { CActorShocker* pType = reinterpret_cast<CActorShocker*>(pAlloc); delete[] pType; return 0x0; }
			*outSize = sizeof(CActorShocker);
			return new CActorShocker[count];
			break;
		case STILLER:
			if (pAlloc != (void*)0x0) { CActorStiller* pType = reinterpret_cast<CActorStiller*>(pAlloc); delete[] pType; return 0x0; }
			*outSize = sizeof(CActorStiller);
			return new CActorStiller[count];
			break;
		case BLAZER:
			if (pAlloc != (void*)0x0) { CActorBlazer* pType = reinterpret_cast<CActorBlazer*>(pAlloc); delete[] pType; return 0x0; }
			*outSize = sizeof(CActorBlazer);
			return new CActorBlazer[count];
			break;
		case SHOCKER_CMD:
			if (pAlloc != (void*)0x0) { CActorShockerCmd* pType = reinterpret_cast<CActorShockerCmd*>(pAlloc); delete[] pType; return 0x0; }
			*outSize = sizeof(CActorShockerCmd);
			return new CActorShockerCmd[count];
			break;
		case RUNE:
			if (pAlloc != (void*)0x0) { CActorRune* pType = reinterpret_cast<CActorRune*>(pAlloc); delete[] pType; return 0x0; }
			*outSize = sizeof(CActorRune);
			return new CActorRune[count];
			break;
		case BONUS_FOUNTAIN:
			if (pAlloc != (void*)0x0) { CActorBonusFountain* pType = reinterpret_cast<CActorBonusFountain*>(pAlloc); delete[] pType; return 0x0; }
			*outSize = sizeof(CActorBonusFountain);
			return new CActorBonusFountain[count];
			break;
		case EVENT_GENERATOR:
			if (pAlloc != (void*)0x0) { CActorEventGenerator* pType = reinterpret_cast<CActorEventGenerator*>(pAlloc); delete[] pType; return 0x0; }
			*outSize = sizeof(CActorEventGenerator);
			return new CActorEventGenerator[count];
			break;
		case WOOF:
			if (pAlloc != (void*)0x0) { CActorWoof* pType = reinterpret_cast<CActorWoof*>(pAlloc); delete[] pType; return 0x0; }
			*outSize = sizeof(CActorWoof);
			return new CActorWoof[count];
			break;
		case HELPER_SIGN:
			if (pAlloc != (void*)0x0) { CActorHelperSign* pType = reinterpret_cast<CActorHelperSign*>(pAlloc); delete[] pType; return 0x0; }
			*outSize = sizeof(CActorHelperSign);
			return new CActorHelperSign[count];
			break;
		case BUNCH:
			if (pAlloc != (void*)0x0) { CActorBunch* pType = reinterpret_cast<CActorBunch*>(pAlloc); delete[] pType; return 0x0; }
			*outSize = sizeof(CActorBunch);
			return new CActorBunch[count];
			break;
		case BRAZUL:
			if (pAlloc != (void*)0x0) { CActorBrazul* pType = reinterpret_cast<CActorBrazul*>(pAlloc); delete[] pType; return 0x0; }
			*outSize = sizeof(CActorBrazul);
			return new CActorBrazul[count];
			break;
		case PUNCHING_BALL:
			if (pAlloc != (void*)0x0) { CActorPunchingBall* pType = reinterpret_cast<CActorPunchingBall*>(pAlloc); delete[] pType; return 0x0; }
			*outSize = sizeof(CActorPunchingBall);
			return new CActorPunchingBall[count];
			break;
		case NATIV_SHOP:
			if (pAlloc != (void*)0x0) { CActorNativShop* pType = reinterpret_cast<CActorNativShop*>(pAlloc); delete[] pType; return 0x0; }
			*outSize = sizeof(CActorNativShop);
			return new CActorNativShop[count];
			break;
		case ADD_ON_PURCHASE:
			if (pAlloc != (void*)0x0) { CActorAddOnPurchase* pType = reinterpret_cast<CActorAddOnPurchase*>(pAlloc); delete[] pType; return 0x0; }
			*outSize = sizeof(CActorAddOnPurchase);
			return new CActorAddOnPurchase[count];
			break;
		case CARE_BOY:
			if (pAlloc != (void*)0x0) { CActorCareBoy* pType = reinterpret_cast<CActorCareBoy*>(pAlloc); delete[] pType; return 0x0; }
			*outSize = sizeof(CActorCareBoy);
			return new CActorCareBoy[count];
			break;
		case SHIP:
			if (pAlloc != (void*)0x0) { CActorShip* pType = reinterpret_cast<CActorShip*>(pAlloc); delete[] pType; return 0x0; }
			*outSize = sizeof(CActorShip);
			return new CActorShip[count];
			break;
		case HUNTER:
			if (pAlloc != (void*)0x0) { CActorHunter* pType = reinterpret_cast<CActorHunter*>(pAlloc); delete[] pType; return 0x0; }
			*outSize = sizeof(CActorHunter);
			return new CActorHunter[count];
			break;
		case JAMGUT:
			if (pAlloc != (void*)0x0) { CActorJamgut* pType = reinterpret_cast<CActorJamgut*>(pAlloc); delete[] pType; return 0x0; }
			*outSize = sizeof(CActorJamgut);
			return new CActorJamgut[count];
			break;
		case ACCELERATOS:
			if (pAlloc != (void*)0x0) { CActorAcceleratos* pType = reinterpret_cast<CActorAcceleratos*>(pAlloc); delete[] pType; return 0x0; }
			*outSize = sizeof(CActorAcceleratos);
			return new CActorAcceleratos[count];
			break;
		case PATTERN_CON:
			if (pAlloc != (void*)0x0) { CActorPattern* pType = reinterpret_cast<CActorPattern*>(pAlloc); delete[] pType; return 0x0; }
			*outSize = sizeof(CActorPattern);
			return new CActorPattern[count];
			break;
		case PATTERN_CMD:
			if (pAlloc != (void*)0x0) { CActorPatternCmd* pType = reinterpret_cast<CActorPatternCmd*>(pAlloc); delete[] pType; return 0x0; }
			*outSize = sizeof(CActorPatternCmd);
			return new CActorPatternCmd[count];
			break;
		case CREDITS:
			if (pAlloc != (void*)0x0) { CActorCredits* pType = reinterpret_cast<CActorCredits*>(pAlloc); delete[] pType; return 0x0; }
			*outSize = sizeof(CActorCredits);
			return new CActorCredits[count];
			break;
		case MINI_GAME:
			if (pAlloc != (void*)0x0) { CActorMiniGame* pType = reinterpret_cast<CActorMiniGame*>(pAlloc); delete[] pType; return 0x0; }
			*outSize = sizeof(CActorMiniGame);
			return new CActorMiniGame[count];
			break;
		case MINI_GAME_TIME_ATTACK:
			if (pAlloc != (void*)0x0) { CActorMiniGameTimeAttack* pType = reinterpret_cast<CActorMiniGameTimeAttack*>(pAlloc); delete[] pType; return 0x0; }
			*outSize = sizeof(CActorMiniGameTimeAttack);
			return new CActorMiniGameTimeAttack[count];
			break;
		case MINI_GAME_BOX_COUNTER:
			if (pAlloc != (void*)0x0) { CActorMiniGameBoxCounter* pType = reinterpret_cast<CActorMiniGameBoxCounter*>(pAlloc); delete[] pType; return 0x0; }
			*outSize = sizeof(CActorMiniGameBoxCounter);
			return new CActorMiniGameBoxCounter[count];
			break;
		case MINI_GAMES_MANAGER:
			if (pAlloc != (void*)0x0) { CActorMiniGamesManager* pType = reinterpret_cast<CActorMiniGamesManager*>(pAlloc); delete[] pType; return 0x0; }
			*outSize = sizeof(CActorMiniGamesManager);
			return new CActorMiniGamesManager[count];
			break;
		case MINI_GAMES_ORGANIZER:
			if (pAlloc != (void*)0x0) { CActorMiniGamesOrganizer* pType = reinterpret_cast<CActorMiniGamesOrganizer*>(pAlloc); delete[] pType; return 0x0; }
			*outSize = sizeof(CActorMiniGamesOrganizer);
			return new CActorMiniGamesOrganizer[count];
			break;
		case MINI_GAME_BOOMY:
			if (pAlloc != (void*)0x0) { CActorMiniGameBoomy* pType = reinterpret_cast<CActorMiniGameBoomy*>(pAlloc); delete[] pType; return 0x0; }
			*outSize = sizeof(CActorMiniGameBoomy);
			return new CActorMiniGameBoomy[count];
			break;
		case MINI_GAME_DISTANCE:
			if (pAlloc != (void*)0x0) { CActorMiniGameDistance* pType = reinterpret_cast<CActorMiniGameDistance*>(pAlloc); delete[] pType; return 0x0; }
			*outSize = sizeof(CActorMiniGameDistance);
			return new CActorMiniGameDistance[count];
			break;
		case BOMB_LAUNCHER:
			if (pAlloc != (void*)0x0) { CActorBombLauncher* pType = reinterpret_cast<CActorBombLauncher*>(pAlloc); delete[] pType; return 0x0; }
			*outSize = sizeof(CActorBombLauncher);
			return new CActorBombLauncher[count];
			break;
		case WANTED_ZOO:
			if (pAlloc != (void*)0x0) { CActorWantedZoo* pType = reinterpret_cast<CActorWantedZoo*>(pAlloc); delete[] pType; return 0x0; }
			*outSize = sizeof(CActorWantedZoo);
			return new CActorWantedZoo[count];
			break;
		default:
			classId;
			IMPLEMENTATION_GUARD();
			ACTOR_LOG(LogLevel::Info, "Unimplemented class: 0x{:x} ({})", (int)classId, (int)classId);

			if (pAlloc != (void*)0x0) { CActorSKIP_HACK* pType = reinterpret_cast<CActorSKIP_HACK*>(pAlloc); delete[] pType; return 0x0; }
			*outSize = sizeof(CActorSKIP_HACK);
			return new CActorSKIP_HACK[count];
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
