#include "DebugActorBehaviour.h"
#include "Actor.h"
#include "ActorTrap.h"
#include "ActorMovingPlatform.h"
#include "ActorWolfen.h"
#include "../../../../src/ActorHero.h"
#include "../../../../src/ActorPunchingBall.h"

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
}

#define INHERITS_FROM_FIGHTER if (curBehaviourId >= FIGHTER_BEHAVIOUR_DEFAULT && curBehaviourId <= FIGHTER_BEHAVIOUR_SLAVE) return Fighter::GetBehaviourName(curBehaviourId)

namespace Hero
{
	static const char* GetBehaviourName(int curBehaviourId)
	{
		INHERITS_FROM_FIGHTER;

		switch (curBehaviourId) {
		case HERO_BEHAVIOUR_DEFAULT:
			return "Default";
		default:
			return "Unknown";
		}
	}
}

namespace PunchingBall
{
	static const char* GetBehaviourName(int curBehaviourId)
	{
		INHERITS_FROM_FIGHTER;

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
		INHERITS_FROM_FIGHTER;

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
	default:
		return std::to_string(behaviourId);
	}
}
