#pragma once
#include <string>

class CActor;

namespace Debug::Actor::Behaviour
{
	std::string GetActorBehaviourName(CActor* pActor);
}

namespace Debug::Actor::State
{
	std::string GetActorStateName(CActor* pActor);
}