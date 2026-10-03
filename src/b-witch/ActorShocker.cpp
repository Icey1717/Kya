#include "ActorShocker.h"
#include "MemoryStream.h"

CActorShocker::CActorShocker()
{}

CActorShocker::~CActorShocker()
{

}

void CActorShocker::Create(ByteCode* pByteCode)
{
	
	return;
}

void CActorShocker::Init()
{
	CActorAutonomous::Init();
	this->addOnGenerator.Init(0);

	return;
}