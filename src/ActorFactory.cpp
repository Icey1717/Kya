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

class CActorBunch : public CActor {
	virtual void Create(ByteCode* pByteCode) { SkipToNextActor(pByteCode); }
};

class CActorBrazul : public CActor {
	virtual void Create(ByteCode* pByteCode) { SkipToNextActor(pByteCode); }
};

CActor* CActorFactory::Factory(ACTOR_CLASS classId, int count, int* outSize)
{
	if (classId < 0x57) {
		switch (classId) {
		case ACTOR:
			*outSize = sizeof(CActor);
			return new CActor[count];
		case BOOMY:
			*outSize = sizeof(CActorBoomy);
			return new CActorBoomy[count];
			break;
		case ACTOR_HERO_PRIVATE:
			*outSize = sizeof(CActorHeroPrivate);
			return new CActorHeroPrivate[count];
			break;
		case MICKEN:
			*outSize = sizeof(CActorMicken);
			return new CActorMicken[count];
			break;
		case WOLFEN:
			*outSize = sizeof(CActorWolfen);
			return new CActorWolfen[count];
			break;
		case NATIV:
			*outSize = sizeof(CActorNativ);
			return new CActorNativ[count];
			break;
		case ELECTROLLA:
			*outSize = sizeof(CActorElectrolla);
			return new CActorElectrolla[count];
			break;
		case MOVING_PLATFORM:
			*outSize = sizeof(CActorMovingPlatform);
			return new CActorMovingPlatform[count];
			break;
		case BONUS:
			*outSize = sizeof(CActorBonus);
			return new CActorBonus[count];
			break;
		case ROPE:
			*outSize = sizeof(CActorRope);
			return new CActorRope[count];
			break;
		case GRAVITY_AWARE:
			*outSize = sizeof(CActorGravityAware);
			return new CActorGravityAware[count];
			break;
		case SWITCH:
			*outSize = sizeof(CActorSwitch);
			return new CActorSwitch[count];
			break;
		case WIND:
			*outSize = sizeof(CActorWind);
			return new CActorWind[count];
			break;
		case COMMANDER:
			*outSize = sizeof(CActorCommander);
			return new CActorCommander[count];
			break;
		case AMBER:
			*outSize = sizeof(CActorAmbre);
			return new CActorAmbre[count];
			break;
		case FRUIT:
			*outSize = sizeof(CActorFruit);
			return new CActorFruit[count];
			break;
		case BRIDGE:
			*outSize = sizeof(CActorBridge);
			return new CActorBridge[count];
			break;
		case CLUSTERISER:
			*outSize = sizeof(CActorClusteriser);
			return new CActorClusteriser[count];
			break;
		case COMPANION:
			*outSize = sizeof(CActorCompanion);
			return new CActorCompanion[count];
			break;
		case DCA:
			*outSize = sizeof(CActorDCA);
			return new CActorDCA[count];
			break;
		case PROJECTILE:
			*outSize = sizeof(CActorProjectile);
			return new CActorProjectile[count];
			break;
		case EXPLOSIVE_DISTRIBUTOR:
			*outSize = sizeof(CActorExplosiveDistributor);
			return new CActorExplosiveDistributor[count];
			break;
		case CHECKPOINT_MANAGER:
			*outSize = sizeof(CActorCheckpointManager);
			return new CActorCheckpointManager[count];
			break;
		case WEAPON:
			*outSize = sizeof(CActorWeapon);
			return new CActorWeapon[count];
			break;
		case ARAIGNOS:
			*outSize = sizeof(CActorAraignos);
			return new CActorAraignos[count];
			break;
		case HEDGEHOG:
			*outSize = sizeof(CActorHedgehog);
			return new CActorHedgehog[count];
			break;
		case TELEPORTER:
			*outSize = sizeof(CActorTeleporter);
			return new CActorTeleporter[count];
			break;
		case NOSE_MONSTER:
			*outSize = sizeof(CActorNoseMonster);
			return new CActorNoseMonster[count];
			break;
		case WOOD_MONSTER:
			*outSize = sizeof(CActorWoodMonster);
			return new CActorWoodMonster[count];
			break;
		case AMORTOS:
			*outSize = sizeof(CActorAmortos);
			return new CActorAmortos[count];
			break;
		case TRAP:
			*outSize = sizeof(CActorTrap);
			return new CActorTrap[count];
			break;
		case FX:
			*outSize = sizeof(CActorFx);
			return new CActorFx[count];
			break;
		case EGG:
			*outSize = sizeof(CActorEgg);
			return new CActorEgg[count];
			break;
		case AMORTOS_CMD:
			*outSize = sizeof(CActorAmortosCmd);
			return new CActorAmortosCmd[count];
			break;
		case MONEY:
			*outSize = sizeof(CActorMoney);
			return new CActorMoney[count];
			break;
		case BOX:
			*outSize = sizeof(CActorBox);
			return new CActorBox[count];
			break;
		case ATON:
			*outSize = sizeof(CActorAton);
			return new CActorAton[count];
			break;
		case BASIC_BOX:
			*outSize = sizeof(CActorBasicBox);
			return new CActorBasicBox[count];
			break;
		case NATIV_CMD:
			*outSize = sizeof(CActorNativCmd);
			return new CActorNativCmd[count];
			break;
		case FOG_MANAGER:
			*outSize = sizeof(CActorFogManager);
			return new CActorFogManager[count];
			break;
		case SHOOT:
			*outSize = sizeof(CActorShoot);
			return new CActorShoot[count];
			break;
		case SHOCKER:
			*outSize = sizeof(CActorShocker);
			return new CActorShocker[count];
			break;
		case BLAZER:
			*outSize = sizeof(CActorBlazer);
			return new CActorBlazer[count];
			break;
		case SHOCKER_CMD:
			*outSize = sizeof(CActorShockerCmd);
			return new CActorShockerCmd[count];
			break;
		case RUNE:
			*outSize = sizeof(CActorRune);
			return new CActorRune[count];
			break;
		case BONUS_FOUNTAIN:
			*outSize = sizeof(CActorBonusFountain);
			return new CActorBonusFountain[count];
			break;
		case EVENT_GENERATOR:
			*outSize = sizeof(CActorEventGenerator);
			return new CActorEventGenerator[count];
			break;
		case WOOF:
			*outSize = sizeof(CActorWoof);
			return new CActorWoof[count];
			break;
		case HELPER_SIGN:
			*outSize = sizeof(CActorHelperSign);
			return new CActorHelperSign[count];
			break;
		case BUNCH:
			*outSize = sizeof(CActorBunch);
			return new CActorBunch[count];
			break;
		case BRAZUL:
			*outSize = sizeof(CActorBrazul);
			return new CActorBrazul[count];
			break;
		case PUNCHING_BALL:
			*outSize = sizeof(CActorPunchingBall);
			return new CActorPunchingBall[count];
			break;
		case NATIV_SHOP:
			*outSize = sizeof(CActorNativShop);
			return new CActorNativShop[count];
			break;
		case ADD_ON_PURCHASE:
			*outSize = sizeof(CActorAddOnPurchase);
			return new CActorAddOnPurchase[count];
			break;
		case CARE_BOY:
			*outSize = sizeof(CActorCareBoy);
			return new CActorCareBoy[count];
			break;
		case SHIP:
			*outSize = sizeof(CActorShip);
			return new CActorShip[count];
			break;
		case HUNTER:
			*outSize = sizeof(CActorHunter);
			return new CActorHunter[count];
			break;
		case JAMGUT:
			*outSize = sizeof(CActorJamgut);
			return new CActorJamgut[count];
			break;
		case ACCELERATOS:
			*outSize = sizeof(CActorAcceleratos);
			return new CActorAcceleratos[count];
			break;
		case PATTERN_CON:
			*outSize = sizeof(CActorPattern);
			return new CActorPattern[count];
			break;
		case PATTERN_CMD:
			*outSize = sizeof(CActorPatternCmd);
			return new CActorPatternCmd[count];
			break;
		default:
			classId;
			IMPLEMENTATION_GUARD();
			ACTOR_LOG(LogLevel::Info, "Unimplemented class: 0x{:x} ({})", classId, classId);
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
