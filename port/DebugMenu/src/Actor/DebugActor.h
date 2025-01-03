#pragma once

#include <functional>

class CActor;

namespace Debug {
	namespace Actor {
		using PerActorFunc = std::function<void(CActor*)>;

		void ShowMenu(bool* bOpen);
		void ForEachActiveActor(PerActorFunc pFunc);
		void ForEachSectorActor(PerActorFunc pFunc);
		void ForEachActor(PerActorFunc pFunc);
		const char* GetActorTypeString(int type);
	}
}