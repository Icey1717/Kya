#include "DebugActorBehaviour.h"
#include "Actor.h"
#include "ActorTrap.h"
#include "ActorMovingPlatform.h"
#include "ActorWolfen.h"
#include "../../../../src/ActorHero.h"
#include "../../../../src/ActorPunchingBall.h"
#include "../../../../src/ActorWeapon.h"
#include "../../../../src/ActorNativ.h"

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
		case STATE_HERO_STAND:
			return "StateHeroStand";
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
		case STATE_HERO_GRIP_B:
			return "StateHeroGripB";
			break;
		case STATE_HERO_GRIP_C:
			return "StateHeroGripC";
			break;
		case STATE_HERO_GRIP_HANG_IDLE:
			return "StateHeroGripHangIdle";
			break;
		case STATE_HERO_GRIP_UP:
			return "StateHeroGripUp";
			break;
		case STATE_HERO_JUMP_2_3_GRIP:
			return "StateHeroJump_2_3_Grip";
			break;
		case STATE_HERO_GRIP_GRAB:
			return "StateHeroGripGrab";
			break;
		case STATE_HERO_TOBOGGAN_3:
			return "StateHeroToboggan3";
			break;
		case STATE_HERO_TOBOGGAN_JUMP_1:
			return "StateHeroTobogganJump1";
			break;
		case STATE_HERO_TOBOGGAN_JUMP_2:
			return "StateHeroTobogganJump2";
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

#define WOLFEN_BEHAVIOUR_WATCH_DOG 0x8
#define WOLFEN_BEHAVIOUR_GUARD_AREA 0xa
#define WOLFEN_BEHAVIOUR_SLEEP 0xc
#define WOLFEN_BEHAVIOUR_ESCAPE 0xd
#define WOLFEN_BEHAVIOUR_EXORCISM 0xe
#define WOLFEN_BEHAVIOUR_TRACK 0xf
#define WOLFEN_BEHAVIOUR_TRACK_WEAPON 0x11
#define WOLFEN_BEHAVIOUR_TRACK_WEAPON_STAND 0x12
#define WOLFEN_BEHAVIOUR_TRACK_WEAPON_SNIPE 0x13
#define WOLFEN_BEHAVIOUR_SNIPE 0x15
#define WOLFEN_BEHAVIOUR_LOST 0x16
#define WOLFEN_BEHAVIOUR_WOLFEN_UNKNOWN 0x17
#define WOLFEN_BEHAVIOUR_DCA 0x18
#define WOLFEN_BEHAVIOUR_AVOID 0x19

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
		case WOLFEN_BEHAVIOUR_WOLFEN_UNKNOWN:
			return "Wolfen Unknown";
		case WOLFEN_BEHAVIOUR_DCA:
			return "DCA";
		case WOLFEN_BEHAVIOUR_AVOID:
			return "Avoid";
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
			case NATIVE_BEHAVIOUR_SELLER:
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
	default:
		std::stringstream sstream;
		sstream << "0x" << std::hex << pActor->actorState;
		return sstream.str();
	}
}
