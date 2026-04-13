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

namespace Wolfen
{
	const char* GetBehaviourName(int curBehaviourId);
	const char* GetStateName(int state);
}