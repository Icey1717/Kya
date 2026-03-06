#include "DebugSceneryCollision.h"

#include "CollisionManager.h"
#include "LargeObject.h"
#include "MathOps.h"
#include "port/pointer_conv.h"
#include "edCollision/edCollisions.h"
#include "Native/NativeDebugShapes.h"
#include "DebugCollisionDrawing.h"

namespace Debug {
	namespace Scenery {
		void DrawDebugShapes(const Debug::Collision::ObbDrawConfig& config)
		{
			if (!config.bEnabled) {
				return;
			}

			CCollisionManager* pColMgr = CScene::ptable.g_CollisionManager_00451690;
			if (!pColMgr) {
				return;
			}

			for (int i = 0; i < pColMgr->staticCollisionCount; i++) {
				if (pColMgr->aStaticCollisionRefs[i]) {
					Debug::Collision::DrawObbNode(pColMgr->aStaticCollisionRefs[i], config);
				}
			}
		}
	}
}