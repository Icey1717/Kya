#include "DebugActorBehaviour.h"
#include "Actor.h"
#include "ActorTrap.h"
#include "ActorMovingPlatform.h"
#include "ActorWolfen.h"
#include "ActorHero.h"
#include "ActorPunchingBall.h"
#include "ActorWeapon.h"
#include "ActorNativ.h"
#include "ActorAraignos.h"
#include "ActorBonus.h"
#include "ActorMoney.h"
#include "ActorBridge.h"
#include "ActorProjectile.h"
#include "ActorSwitch.h"
#include "ActorMicken.h"

namespace Aton
{
	static const char* GetBehaviourName(int curBehaviourId)
	{
		switch (curBehaviourId) {
		case 0x3:
			return "Escape";
		default:
			return "Unknown";
		}
	}
}

namespace Fighter
{
	static const char* GetBehaviourName(int curBehaviourId)
	{
		switch (curBehaviourId) {
		case FIGHTER_BEHAVIOUR_DEFAULT:
			return "Fighter - Default";
		case FIGHTER_BEHAVIOUR_PROJECTED:
			return "Fighter - Projected";
		case FIGHTER_BEHAVIOUR_RIDDEN:
			return "Fighter - Ridden";
		case FIGHTER_BEHAVIOUR_SLAVE:
			return "Fighter - Slave";
		default:
			return "Unknown";
		}
	}

	static const char* GetDefaultStateName(int state)
	{
		switch (state) {
		case FIGHTER_DEFAULT_STATE_IDLE:
			return "Idle";
		default:
			return "Unknown";
		}
	}

	static const char* GetStateName(int curBehaviourId, int state)
	{
		switch (curBehaviourId) {
		case FIGHTER_BEHAVIOUR_DEFAULT:
			return GetDefaultStateName(state);
		default:
			return "Unknown";
		}
	}
}

#define INHERITS_FROM_FIGHTER_BEHAVIOUR(curBehaviourId) if (curBehaviourId >= FIGHTER_BEHAVIOUR_DEFAULT && curBehaviourId <= FIGHTER_BEHAVIOUR_SLAVE) return Fighter::GetBehaviourName(curBehaviourId)
#define INHERITS_FROM_FIGHTER_STATE(curBehaviourId, state) if (curBehaviourId >= FIGHTER_BEHAVIOUR_DEFAULT && curBehaviourId <= FIGHTER_BEHAVIOUR_SLAVE) return Fighter::GetStateName(curBehaviourId, state)

namespace Hero
{
	static const char* GetBehaviourName(int curBehaviourId)
	{
		INHERITS_FROM_FIGHTER_BEHAVIOUR(curBehaviourId);

		switch (curBehaviourId) {
		case HERO_BEHAVIOUR_DEFAULT:
			return "Default";
		default:
			return "Unknown";
		}
	}

	static const char* GetDefaultStateName(int state) {
		switch (state) {
		case STATE_HERO_SHOP:
			return "StateHeroShop";
			break;
		case STATE_HERO_STAND:
			return "StateHeroStand";
			break;
		case STATE_HERO_BOOMY:
			return "StateHeroBoomy";
			break;
		case STATE_HERO_BOOMY_CATCH:
			return "StateHeroBoomyCatch";
			break;
		case STATE_HERO_JUMP_1_1_STAND:
			return "StateHeroJump_1_1_Stand";
			break;
		case STATE_HERO_JUMP_2_3_STAND:
			return "StateHeroJump_2_3_Stand";
			break;
		case STATE_HERO_JUMP_3_3_STAND:
			return "StateHeroJump_3_3_Stand";
			break;
		case STATE_HERO_RUN:
			return "StateHeroRun";
			break;
		case STATE_HERO_RUN_B:
			return "StateHeroRunB";
			break;
		case STATE_HERO_JOKE:
			return "StateHeroJoke";
			break;
		case STATE_HERO_SLIDE_SLIP_A:
			return "StateHeroSlideSlipA";
			break;
		case STATE_HERO_SLIDE_SLIP_B:
			return "StateHeroSlideSlipB";
			break;
		case STATE_HERO_SLIDE_SLIP_C:
			return "StateHeroSlideSlipC";
			break;
		case STATE_HERO_SLIDE_A:
			return "StateHeroSlideA";
			break;
		case STATE_HERO_SLIDE_B:
			return "StateHeroSlideB";
			break;
		case STATE_HERO_U_TURN:
			return "StateHeroUTurn";
			break;
		case STATE_HERO_JUMP_1_1_RUN:
			return "StateHeroJump_1_1_Run";
			break;
		case STATE_HERO_JUMP_2_3_RUN:
			return "StateHeroJump_2_3_Run";
			break;
		case STATE_HERO_JUMP_3_3_RUN:
			return "StateHeroJump_3_3_Run";
			break;
		case STATE_HERO_FALL_A:
			return "StateHeroFallA";
			break;
		case STATE_HERO_FALL_B:
			return "StateHeroFallB";
			break;
		case STATE_HERO_FALL_BOUNCE:
			return "StateHeroFallBounce";
			break;
		case STATE_HERO_FALL_BOUNCE_1_2:
			return "StateHeroFallBounce1_2";
			break;
		case STATE_HERO_FALL_BOUNCE_2_2:
			return "StateHeroFallBounce2_2";
			break;
		case STATE_HERO_STAND_TO_CROUCH_A:
			return "StateHeroStandToCrouchA";
			break;
		case STATE_HERO_STAND_TO_CROUCH_B:
			return "StateHeroStandToCrouchB";
			break;
		case STATE_HERO_CROUCH_A:
			return "StateHeroCrouchA";
			break;
		case STATE_HERO_CROUCH_WALK_A:
			return "StateHeroCrouchWalkA";
			break;
		case STATE_HERO_CROUCH_B:
			return "StateHeroCrouchB";
			break;
		case STATE_HERO_JUMP_TO_CROUCH:
			return "StateHeroJumpToCrouch";
			break;
		case STATE_HERO_ROLL:
			return "StateHeroRoll";
			break;
		case STATE_HERO_CROUCH_C:
			return "StateHeroCrouchC";
			break;
		case STATE_HERO_ROLL_2_CROUCH:
			return "StateHeroRoll2Crouch";
			break;
		case STATE_HERO_ROLL_FALL:
			return "StateHeroRollFall";
			break;
		case STATE_HERO_WIND_HURT:
			return "StateHeroWindHurt";
			break;
		case STATE_HERO_WIND_WALL_HURT:
			return "StateHeroWindWallHurt";
			break;
		case STATE_HERO_WIND_SLIDE_HURT:
			return "StateHeroWindSlideHurt";
			break;
		case STATE_HERO_TOBOGGAN_JUMP_HURT:
			return "StateHeroTobogganJumpHurt";
			break;
		case STATE_HERO_HURT_A:
			return "StateHeroHurtA";
			break;
		case STATE_HERO_HURT_B:
			return "StateHeroHurtB";
			break;
		case STATE_HERO_FALL_DEATH:
			return "StateHeroFallDeath";
			break;
		case STATE_HERO_DROWN_DEATH:
			return "StateHeroDrownDeath";
			break;
		case STATE_HERO_COL_WALL:
			return "StateHeroColWall";
			break;
		case STATE_HERO_KICK_A:
			return "StateHeroKickA";
			break;
		case STATE_HERO_KICK_B:
			return "StateHeroKickB";
			break;
		case STATE_HERO_KICK_C:
			return "StateHeroKickC";
			break;
		case STATE_HERO_CLIMB_STAND_A:
			return "StateHeroClimbStandA";
			break;
		case STATE_HERO_CLIMB_STAND_B:
			return "StateHeroClimbStandB";
			break;
		case STATE_HERO_CLIMB_STAND_C:
			return "StateHeroClimbStandC";
			break;
		case STATE_HERO_CLIMB_STAND_D:
			return "StateHeroClimbStandD";
			break;
		case STATE_HERO_CLIMB_MOVE_A:
			return "StateHeroClimbMoveA";
			break;
		case STATE_HERO_CLIMB_MOVE_B:
			return "StateHeroClimbMoveB";
			break;
		case STATE_HERO_CLIMB_MOVE_C:
			return "StateHeroClimbMoveC";
			break;
		case STATE_HERO_CLIMB_MOVE_D:
			return "StateHeroClimbMoveD";
			break;
		case STATE_HERO_CLIMB_MOVE_E:
			return "StateHeroClimbMoveE";
			break;
		case STATE_HERO_CLIMB_MOVE_F:
			return "StateHeroClimbMoveF";
			break;
		case STATE_HERO_CLIMB_MOVE_G:
			return "StateHeroClimbMoveG";
			break;
		case STATE_HERO_CLIMB_MOVE_H:
			return "StateHeroClimbMoveH";
			break;
		case STATE_HERO_CLIMB_MOVE_I:
			return "StateHeroClimbMoveI";
			break;
		case STATE_HERO_CLIMB_MOVE_J:
			return "StateHeroClimbMoveJ";
			break;
		case STATE_HERO_CLIMB_MOVE_K:
			return "StateHeroClimbMoveK";
			break;
		case STATE_HERO_CLIMB_MOVE_L:
			return "StateHeroClimbMoveL";
			break;
		case STATE_HERO_CLIMB_JUMP_A:
			return "StateHeroClimbJumpBackA";
			break;
		case STATE_HERO_CLIMB_JUMP_B:
			return "StateHeroClimbJumpBackB";
			break;
		case STATE_HERO_CLIMB_FROM_AIR:
			return "StateHeroClimbFromAir";
			break;
		case STATE_HERO_GRIP_B:
			return "StateHeroGripB";
			break;
		case STATE_HERO_GRIP_C:
			return "StateHeroGripC";
			break;
		case STATE_HERO_GRIP_HANG_IDLE:
			return "StateHeroGripHangIdle";
			break;
		case STATE_HERO_GRIP_LEFT:
			return "StateHeroGripLeft";
			break;
		case STATE_HERO_GRIP_RIGHT:
			return "StateHeroGripRight";
			break;
		case STATE_HERO_GRIP_ANGLE_A:
			return "StateHeroGripAngleA";
			break;
		case STATE_HERO_GRIP_ANGLE_B:
			return "StateHeroGripAngleB";
			break;
		case STATE_HERO_GRIP_ANGLE_C:
			return "StateHeroGripAngleC";
			break;
		case STATE_HERO_GRIP_ANGLE_D:
			return "StateHeroGripAngleD";
			break;
		case STATE_HERO_GRIP_UP:
			return "StateHeroGripUp";
			break;
		case STATE_HERO_JUMP_2_3_GRIP:
			return "StateHeroJump_2_3_Grip";
			break;
		case STATE_HERO_JUMP_GRIP_UNKOWN_A:
			return "StateHeroJumpGripUnknownA";
			break;
		case STATE_HERO_JUMP_GRIP_UNKOWN_B:
			return "StateHeroJumpGripUnknownB";
			break;
		case STATE_HERO_GRIP_GRAB:
			return "StateHeroGripGrab";
			break;
		case STATE_HERO_BOUNCE_SOMERSAULT_1:
			return "StateHeroBounceSomersault1";
			break;
		case STATE_HERO_BOUNCE_SOMERSAULT_2:
			return "StateHeroBounceSomersault2";
			break;
		case STATE_HERO_BOOMY_PREPARE_FIGHT_BLOW:
			return "StateHeroBoomyPrepareFightBlow";
			break;
		case STATE_HERO_BOOMY_EXECUTE_FIGHT_BLOW:
			return "StateHeroBoomyExecuteFightBlow";
			break;
		case STATE_HERO_BOOMY_RETURN_FIGHT_BLOW:
			return "StateHeroBoomyReturnFightBlow";
			break;
		case STATE_HERO_TOBOGGAN_3:
			return "StateHeroToboggan3";
			break;
		case STATE_HERO_TOBOGGAN_JUMP_3:
			return "StateHeroTobogganJump3";
			break;
		case STATE_HERO_TOBOGGAN_JUMP_1:
			return "StateHeroTobogganJump1";
			break;
		case STATE_HERO_TOBOGGAN_JUMP_2:
			return "StateHeroTobogganJump2";
			break;
			case STATE_HERO_TOBOGGAN_CRASH:
				return "StateHeroTobogganCrash";
				break;
		case STATE_HERO_TOBOGGAN_2:
			return "StateHeroToboggan2";
			break;
		case STATE_HERO_TOBOGGAN:
			return "StateHeroToboggan";
			break;
		case STATE_HERO_GLIDE_1:
			return "StateHeroGlide1";
			break;
		case STATE_HERO_GLIDE_2:
			return "StateHeroGlide2";
			break;
		case STATE_HERO_GLIDE_3:
			return "StateHeroGlide3";
			break;
		case STATE_HERO_WIND_CANON:
			return "StateHeroWindCanon";
			break;
		case STATE_HERO_WIND_CANON_B:
			return "StateHeroWindCanonB";
			break;
		case STATE_HERO_WIND_FLY:
			return "StateHeroWindFly";
			break;
		case STATE_HERO_WIND_SLIDE:
			return "StateHeroWindSlide";
			break;
		case STATE_HERO_WIND_WALL_MOVE_A:
			return "StateHeroWindSlideMoveA";
			break;
		case STATE_HERO_WIND_WALL_MOVE_B:
			return "StateHeroWindSlideMoveB";
			break;
		case STATE_HERO_WIND_WALL_MOVE_E:
			return "StateHeroWindSlideMoveE";
			break;
		case STATE_HERO_WIND_WALL_MOVE_F:
			return "StateHeroWindSlideMoveF";
			break;
		case STATE_HERO_WIND_WALL_MOVE_C:
			return "StateHeroWindSlideMoveC";
			break;
		case STATE_HERO_WIND_WALL_MOVE_D:
			return "StateHeroWindSlideMoveD";
			break;
		case STATE_HERO_GRIP_UP_A:
			return "StateHeroGripUpA";
			break;
		case STATE_HERO_GRIP_UP_B:
			return "StateHeroGripUpB";
			break;
		case STATE_HERO_WIND_WALL_MOVE_JUMP:
			return "StateHeroWindWallMoveJump";
			break;
		case STATE_HERO_WIND_FLY_B:
			return "StateHeroWindFlyB";
			break;
		case STATE_HERO_WIND_FLY_C:
			return "StateHeroWindFlyC";
			break;
		case STATE_HERO_LEVER_1_2:
			return "StateHeroLever1_2";
			break;
		case STATE_HERO_LEVER_2_2:
			return "StateHeroLever2_2";
			break;
		case STATE_HERO_DCA_A:
			return "StateHeroDCA_A";
			break;
		case STATE_HERO_REGENERATE:
			return "StateHeroRegenerate";
			break;
		case STATE_HERO_UNLOCK_SWITCH:
			return "StateHeroUnlockSwitch";
			break;
		case STATE_HERO_EXORCISE_BEGIN:
			return "StateHeroExorciseBegin";
			break;
		case STATE_HERO_EXORCISE:
			return "StateHeroExorcise";
			break;
		case STATE_HERO_TRAMPOLINE_JUMP_1_2_A:
			return "StateHeroTrampolineJump_1_2_A";
			break;
		case STATE_HERO_TRAMPOLINE_JUMP_2_3:
			return "StateHeroTrampolineJump_2_3";
			break;
		case STATE_HERO_TRAMPOLINE_JUMP_1_2_B:
			return "StateHeroTrampolineJump_1_2_B";
			break;
		case STATE_HERO_TRAMPOLINE_STOMACH_TO_FALL:
			return "StateHeroTrampolineJump_1_2_C";
			break;
		case STATE_HERO_CEILING_CLIMB_A:
			return "StateHeroCeilingClimbA";
			break;
		case STATE_HERO_CEILING_CLIMB_B:
			return "StateHeroCeilingClimbB";
			break;
		case STATE_HERO_CEILING_CLIMB_C:
			return "StateHeroCeilingClimbC";
			break;
		case STATE_HERO_CEILING_CLIMB_D:
			return "StateHeroCeilingClimbD";
			break;
		case STATE_HERO_GRIP_2_CEILING:
			return "StateHeroGrip2Ceiling";
			break;
		case STATE_HERO_CEILING_2_GRIP:
			return "StateHeroCeiling2Grip";
			break;
		case STATE_HERO_CEILING_CLIMB_GRAB:
			return "StateHeroCeilingClimbGrab";
			break;
		case STATE_HERO_CAUGHT_TRAP_1:
			return "StateHeroCaughtTrap1";
			break;
		case STATE_HERO_CAUGHT_TRAP_2:
			return "StateHeroCaughtTrap2";
			break;
		case 0x3:
			return "Cinematic";
			break;
		default:
			assert(false);
			return "UKNOWN ERROR";
			break;
		}
	}

	static const char* GetStateName(int curBehaviourId, int state)
	{
		INHERITS_FROM_FIGHTER_STATE(curBehaviourId, state);

		switch (curBehaviourId) {
		case HERO_BEHAVIOUR_DEFAULT:
			return GetDefaultStateName(state);
		default:
			return "Unknown";
		}
	}
}

namespace PunchingBall
{
	static const char* GetBehaviourName(int curBehaviourId)
	{
		INHERITS_FROM_FIGHTER_BEHAVIOUR(curBehaviourId);

		switch (curBehaviourId) {
		case PUNCHING_BALL_BEHAVIOUR_STAND:
			return "Stand";
		default:
			return "Unknown";
		}
	}
}

namespace Trap
{
	static const char* GetBehaviourName(int curBehaviourId)
	{
		switch (curBehaviourId) {
		case TRAP_BEHAVIOUR_STAND:
			return "Stand";
		case TRAP_BEHAVIOUR_INACTIVE:
			return "Inactive";
		default:
			return "Unknown";
		}
	}
}

namespace MovingPlatform
{
	static const char* GetBehaviourName(int curBehaviourId)
	{
		switch (curBehaviourId) {
		case MOVING_PLATFORM_BEHAVIOUR_TRAJECTORY:
			return "Trajectory";
		case MOVING_PLATFORM_BEHAVIOUR_SLAB:
			return "Slab";
		case MOVING_PLATFORM_BEHAVIOUR_WEIGHING_MACHINE_SLAVE:
			return "Weighing Machine Slave";
		case MOVING_PLATFORM_BEHAVIOUR_WEIGHING_MACHINE_MASTER:
			return "Weighing Machine Master";
		case MOVING_PLATFORM_BEHAVIOUR_DESTROYED:
			return "Destroyed";
		case MOVING_PLATFORM_BEHAVIOUR_STAND:
			return "Stand";
		case MOVING_PLATFORM_BEHAVIOUR_SELECTOR_MASTER:
			return "Selector Master";
		case MOVING_PLATFORM_BEHAVIOUR_SELECTOR_NEW:
			return "Selector New";
		default:
			return "Unknown";
		}
	}
}

namespace Wolfen
{
	static const char* GetBehaviourName(int curBehaviourId)
	{
		INHERITS_FROM_FIGHTER_BEHAVIOUR(curBehaviourId);

		switch (curBehaviourId) {
		case WOLFEN_BEHAVIOUR_WATCH_DOG:
			return "Watch Dog";
		case WOLFEN_BEHAVIOUR_GUARD_AREA:
			return "Guard Area";
		case WOLFEN_BEHAVIOUR_SLEEP:
			return "Sleep";
		case WOLFEN_BEHAVIOUR_ESCAPE:
			return "Escape";
		case WOLFEN_BEHAVIOUR_EXORCISM:
			return "Exorcism";
		case WOLFEN_BEHAVIOUR_TRACK:
			return "Track";
		case WOLFEN_BEHAVIOUR_TRACK_WEAPON:
			return "Track Weapon";
		case WOLFEN_BEHAVIOUR_TRACK_WEAPON_STAND:
			return "Track Weapon Stand";
		case WOLFEN_BEHAVIOUR_TRACK_WEAPON_SNIPE:
			return "Track Weapon Snipe";
		case WOLFEN_BEHAVIOUR_SNIPE:
			return "Snipe";
		case WOLFEN_BEHAVIOUR_LOST:
			return "Lost";
		case WOLFEN_BEHAVIOUR_WOLFEN_DCA:
			return "Wolfen DCA";
		case WOLFEN_BEHAVIOUR_AVOID:
			return "Avoid";
		case WOLFEN_BEHAVIOUR_UNKNOWN:
			return "Unkown";
		default:
			return "Unknown";
		}
	}
}

namespace Weapon
{
	static const char* GetBehaviourName(int curBehaviourId)
	{
		switch (curBehaviourId) {
		case WEAPON_BEHAVIOUR_SNIPER:
			return "Sniper";
		case WEAPON_BEHAVIOUR_RPG:
			return "RPG";
		case WEAPON_BEHAVIOUR_PISTOL:
			return "Pistol";
		default:
			return "Unknown";
		}
	};

	static const char* GetStateName(int state)
	{
		switch (state) {
		case WEAPON_STATE_IDLE:
			return "Idle";
		case WEAPON_STATE_FIRE:
			return "Fire";
		case WEAPON_STATE_RELOAD:
			return "Reload";
		default:
			return "Unknown";
		}
	}
}

namespace Nativ
{
	static const char* GetBehaviourName(int curBehaviourId)
	{
		switch (curBehaviourId) {
			case NATIVE_BEHAVIOUR_SPEAK:
				return "Speak";
			case NATIVE_BEHAVIOUR_EXORCISM:
				return "Exorcism";
			case NATIVE_BEHAVIOUR_LIVE:
				return "Live";
			case NATIVE_BEHAVIOUR_TAKE_AND_PUT:
				return "Take and Put";
			case NATIVE_BEHAVIOUR_AKASA:
				return "Seller";
		default:
			return "Unknown";
		}
	};

	static const char* GetStateName(int state)
	{
		switch (state) {
		case NATIVE_STATE_TAKE_PUT_WALK:
			return "Take Put Walk";
		case NATIVE_STATE_TAKE_PUT_TURN_TO:
			return "Take Put Turn To";
		default:
			return "Unknown";
		}
	}
}

namespace Araignos
{
	static const char* GetBehaviourName(int curBehaviourId)
	{
		switch (curBehaviourId) {
		case ARAIGNOS_BEHAVIOUR_DEFAULT:
			return "Default";
		default:
			return "Unknown";
		}
	}

	static const char* GetStateName(int state)
	{
		switch (state) {
		case ARAIGNOS_STATE_DEFAULT:
			return "Default";
		case ARAIGNOS_STATE_DIE:
			return "Die";
		default:
			return "Unknown";
		}
	}
}

namespace Bonus
{
	static const char* GetBehaviourName(int curBehaviourId)
	{
		switch (curBehaviourId) {
		case BONUS_BEHAVIOUR_TURN:
			return "Turn";
		case BONUS_BEHAVIOUR_PATH:
			return "Path";
		case BONUS_BEHAVIOUR_FLOCK:
			return "Flock";
		case BONUS_BEHAVIOUR_ADD_ON:
			return "Add On";
		default:
			return "Unknown";
		}
	}
}

namespace Money
{
	static const char* GetBehaviourName(int curBehaviourId)
	{
		switch (curBehaviourId) {
		case MONEY_BEHAVIOUR_FLOCK:
			return "Flock";
		case MONEY_BEHAVIOUR_ADD_ON:
			return "Add On";
		default:
			return "Unknown";
		}
	}
}

namespace Projectile
{
	static const char* GetBehaviourName(int curBehaviourId)
	{
		switch (curBehaviourId) {
		case PROJECTILE_BEHAVIOUR_STAND:
			return "Stand";
		case PROJECTILE_BEHAVIOUR_STRAIGHT:
			return "Straight";
		case PROJECTILE_BEHAVIOUR_PORTABLE:
			return "Portable";
		case PROJECTILE_BEHAVIOUR_INVENTORY:
			return "Inventory";
		case PROJECTILE_BEHAVIOUR_LAVA_BALL:
			return "Lava Ball";
		case PROJECTILE_BEHAVIOUR_EXCUSE:
			return "Excuse";
		case PROJECTILE_BEHAVIOUR_INACTIVE:
			return "Inactive";
		default:
			return "Unknown";
			break;
		}
	}

	static const char* GetStateName(int state)
	{
		switch (state) {
		case PROJECTILE_STATE_FLYING_LAVA_BALL:
			return "Flying Lava Ball";
		case PROJECTILE_STATE_FLYING_PROJECTED:
			return "Flying Projected";
		case PROJECTILE_STATE_FLYING:
			return "Flying";
		case PROJECTILE_STATE_FLYING_DIRECTED:
			return "Flying Directed";
		case PROJECTILE_STATE_LIVING:
			return "Living";
		case PROJECTILE_STATE_DIE:
			return "Die";
		case PROJECTILE_STATE_DYING:
			return "Dying";
		case PROJECTILE_STATE_SOCCER:
			return "Soccer";
		case PROJECTILE_STATE_AUT_KICKED:
			return "Auto Kicked";
		case PROJECTILE_STATE_AUT_ROLL_ON_GROUND:
			return "Auto Roll On Ground";
		default:
			return "Unknown";
			break;
		}
	}
}

namespace Switch
{
	static const char* GetBehaviourName(int curBehaviourId)
	{
		switch (curBehaviourId) {
		case SWITCH_BEHAVIOUR_LEVER:
			return "Lever";
		case SWITCH_BEHAVIOUR_MAGIC:
			return "Magic";
		case SWITCH_BEHAVIOUR_WOLFEN_COUNTER:
			return "Wolfen Counter";
		case SWITCH_BEHAVIOUR_MULTI_CONDITION:
			return "Multi Condition";
		case SWITCH_BEHAVIOUR_TARGET:
			return "Target";
		case SWITCH_BEHAVIOUR_SEQUENCE:
			return "Sequence";
		case SWITCH_BEHAVIOUR_NEW:
			return "New";
		default:
			return "Unknown";
			break;
		}
	}
}

namespace Micken
{
	static const char* GetBehaviourName(int curBehaviourId)
	{
		switch (curBehaviourId) {
		case MICKEN_BEHAVIOUR_EAT:
			return "Eat";
		default:
			return "Unknown";
		}
	}

	static const char* GetStateName(int state)
	{
		switch (state) {
		case MICKEN_EAT_STATE_STAND:
			return "Stand";
		case MICKEN_EAT_STATE_EAT:
			return "Eat";
		case MICKEN_EAT_STATE_WALK_TO_FRUIT:
			return "Walk to Fruit";
		case MICKEN_EAT_STATE_CHEW:
			return "Chew";
		case MICKEN_EAT_STATE_HOLE:
			return "Hole";
		case MICKEN_EAT_STATE_ROLL_IN_THE_WIND:
			return "Roll in the Wind";
		default:
			return "Unknown";
		}
	}
}

namespace Bridge
{
	static const char* GetBehaviourName(int curBehaviourId)
	{
		switch (curBehaviourId) {
		case BRIDGE_BEHAVIOUR_BASIC:
			return "Basic";
		case BRIDGE_BEHAVIOUR_WIND_AWARE:
			return "Wind Aware";
		default:
			return "Unknown";
		}
	}

	static const char* GetStateName(int state)
	{
		switch (state) {
		case BRIDGE_BASIC_STATE_STAND:
			return "Stand";
		default:
			return "Unknown";
		}
	}
}

std::string Debug::Actor::Behaviour::GetActorBehaviourName(CActor* pActor)
{
	const int behaviourId = pActor->curBehaviourId;

	switch (behaviourId) {
	case 0x0:
		return "Actor Stand";
	case 0x1:
		return "Actor Cinematic";
	}

	switch (pActor->typeID) {
	case ATON:
		return Aton::GetBehaviourName(behaviourId);
	case ACTOR_HERO_PRIVATE:
		return Hero::GetBehaviourName(behaviourId);
	case TRAP:
		return Trap::GetBehaviourName(behaviourId);
	case MOVING_PLATFORM:
		return MovingPlatform::GetBehaviourName(behaviourId);
	case WOLFEN:
		return Wolfen::GetBehaviourName(behaviourId);
	case PUNCHING_BALL:
		return PunchingBall::GetBehaviourName(behaviourId);
	case WEAPON:
		return Weapon::GetBehaviourName(behaviourId);
	case NATIV:
		return Nativ::GetBehaviourName(behaviourId);
	case ARAIGNOS:
		return Araignos::GetBehaviourName(behaviourId);
	case BONUS:
		return Bonus::GetBehaviourName(behaviourId);
	case MONEY:
		return Money::GetBehaviourName(behaviourId);
	case PROJECTILE:
		return Projectile::GetBehaviourName(behaviourId);
	case SWITCH:
		return Switch::GetBehaviourName(behaviourId);
	case MICKEN:
		return Micken::GetBehaviourName(behaviourId);
	case BRIDGE:
		return Bridge::GetBehaviourName(behaviourId);
	default:
		return std::to_string(behaviourId);
	}
}

std::string Debug::Actor::State::GetActorStateName(CActor* pActor)
{
	switch (pActor->typeID) {
	case ACTOR_HERO_PRIVATE:
		return Hero::GetStateName(pActor->curBehaviourId, pActor->actorState);
	case WEAPON:
		return Weapon::GetStateName(pActor->actorState);
	case NATIV:
		return Nativ::GetStateName(pActor->actorState);
	case ARAIGNOS:
		return Araignos::GetStateName(pActor->actorState);
	case PROJECTILE:
		return Projectile::GetStateName(pActor->actorState);
	case MICKEN:
		return Micken::GetStateName(pActor->actorState);
	case BRIDGE:
		return Bridge::GetStateName(pActor->actorState);
	default:
		std::stringstream sstream;
		sstream << "0x" << std::hex << pActor->actorState;
		return sstream.str();
	}
}
