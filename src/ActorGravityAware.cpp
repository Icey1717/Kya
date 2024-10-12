#include "ActorGravityAware.h"
#include "MemoryStream.h"

void CActorGravityAware::Create(ByteCode* pByteCode)
{
	CCollision* pCVar1;
	CBehaviour* pCVar2;
	int* piVar3;

	CActorAutonomous::Create(pByteCode);
	//pCVar1 = (this->base).data.pCollisionData;
	//if (pCVar1 != (CollisionData*)0x0) {
	//	pCVar1->flags_0x0 = pCVar1->flags_0x0 | 0x1000180;
	//}
	//(this->base).data.flags = (this->base).data.flags | 0x1000;
	//pCVar2 = CActor::GetBehaviour(this, 4);
	//if ((pCVar2 == (CBehaviour*)0x0) && (pCVar2 = CActor::GetBehaviour(this, 3), pCVar2 == (CBehaviour*)0x0)) {
	//	this[1].base.data.flags = 0;
	//}
	//else {
	//	piVar3 = CActor::CreateActorSound(this, 1);
	//	this[1].base.data.flags = (uint)piVar3;
	//}
	return;
}
