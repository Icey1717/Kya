#ifdef PLATFORM_WIN

#include "ActorManager.h"
#include "Actor.h"
#include "Animation.h"
#include "ActorFactory.h"
#include "ActorHero.h"
#include "DrawTrace.h"

void CActorManager::Level_Term()
{
	Level_TermActors();
	Level_FreeActors();
}

void CActorManager::Level_TermActors()
{
    Renderer::DrawTrace::InvalidateSources();
	int iVar3;

	iVar3 = this->nbActors + -1;
	if (-1 < iVar3) {
		do {
			this->aActors[iVar3]->Term();
			iVar3 = iVar3 + -1;
		} while (-1 < iVar3);
	}

	iVar3 = this->nbActors + -1;
	if (-1 < iVar3) {
		do {
			this->aActors[iVar3]->Destroy();
			iVar3 = iVar3 + -1;
		} while (-1 < iVar3);
	}

	return;
}

void CActorManager::Level_FreeActors()
{
	edAnmLayer* peVar1;
	uint classId;
	CClassInfo* pCVar4;

	classId = 0;
	pCVar4 = this->aClassInfo;
	do {
		if (pCVar4->aActors != (CActor*)0x0) {
			// The Windows polymorphic array helper needs the count to run every destructor.
			CActorFactory::Factory((ACTOR_CLASS)classId, pCVar4->totalCount, (int*)0x0, pCVar4->aActors);
		}

		pCVar4->aActors = (CActor*)0x0;
		pCVar4->totalCount = 0;
		pCVar4->allocatedCount = 0;
		pCVar4->size = 0;

		classId = classId + 1;
		pCVar4 = pCVar4 + 1;
	} while ((int)classId < 0x57);

	// The next level's loading camera and debug UI can still read this pointer.
	// Clear it after the hero actor array has been destroyed.
	CActorHero::_gThis = (CActorHero*)0x0;

	this->cluster.Term();
	
	delete[] this->aSectorBoundingBoxes;

	if (this->aActors != (CActor**)0x0) {
		delete[] this->aActors;
	}

	if (this->aAnimation != (CAnimation*)0x0) {
		delete[] this->aAnimation;
	}

	peVar1 = this->aAnimLayers;
	if ((peVar1 != (edAnmLayer*)0x0) && (peVar1 != (edAnmLayer*)0x0)) {
		delete[] this->aAnimLayers;
	}

	if (this->aShadows != (CShadow*)0x0) {
		DELETE_ARRAY_POLYMORPHIC(CShadow, this->aShadows, this->shadowCount);
	}

	Level_ClearInternalData();

	return;
}

#endif
