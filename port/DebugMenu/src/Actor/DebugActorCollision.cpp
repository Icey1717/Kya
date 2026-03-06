#include "DebugActorCollision.h"

#include "DebugActor.h"
#include "Actor.h"
#include "CollisionManager.h"
#include "port/pointer_conv.h"
#include "edCollision/edCollisions.h"
#include "Native/NativeDebugShapes.h"
#include "DebugCollisionDrawing.h"

namespace Debug {
	namespace Actor {
		void DrawDebugShapes(const Debug::Collision::ObbDrawConfig& config)
		{
			if (!config.bEnabled) {
				return;
			}

			ForEachActiveActor([&config](CActor* pActor) {
				if (pActor->pCollisionData && pActor->pCollisionData->pObbTree) {
					Debug::Collision::DrawObbNode(pActor->pCollisionData->pObbTree, config);
				}
			});
		}
	}
}
