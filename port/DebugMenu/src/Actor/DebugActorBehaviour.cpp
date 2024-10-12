#include "DebugActorBehaviour.h"
#include "Actor.h"
#include "ActorTrap.h"
#include "ActorMovingPlatform.h"

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

namespace Hero
{
	static const char* GetBehaviourName(int curBehaviourId)
	{
		switch (curBehaviourId) {
		case 0x7:
			return "Default";
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
	default:
		return std::to_string(behaviourId);
	}
}
