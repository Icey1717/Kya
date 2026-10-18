void CActorMovingPlatform::SetSoundPosition(CActorMovingPlatform *this)

{
  CActorMovingPlatform_SubObj *pCVar1;
  Timer *pTVar2;
  float fVar3;
  edF32VECTOR4 local_20;
  edF32VECTOR4 local_10;

  pCVar1 = this->pProperties;
  local_10.x = (pCVar1->field_0x10).x;
  local_10.y = (pCVar1->field_0x10).y;
  local_10.z = (pCVar1->field_0x10).z;
  local_10.w = 1.0;
  edF32Matrix4MulF32Vector4Hard(&local_10,(edF32MATRIX4 *)(this->base).base.pMeshTransform,&local_10);
  local_20.x = (this->base).base.vector_0x120.position.x;
  local_20.y = (this->base).base.vector_0x120.position.y;
  local_20.z = (this->base).base.vector_0x120.position.z;
  local_20.w = 1.0;
  edF32Vector4SubHard(&local_20,&local_10,&local_20);
  pTVar2 = Timer::GetTimer();
  if (pTVar2->cutsceneDeltaTime != 0.0) {
    fVar3 = 1.0 / pTVar2->cutsceneDeltaTime;
    local_20.x = local_20.x * fVar3;
    local_20.y = local_20.y * fVar3;
    local_20.z = local_20.z * fVar3;
  }
  (this->base).base.vector_0x120.position.x = local_10.x;
  (this->base).base.vector_0x120.position.y = local_10.y;
  (this->base).base.vector_0x120.position.z = local_10.z;
  (this->base).base.vector_0x120.rotation.x = local_20.x;
  (this->base).base.vector_0x120.rotation.y = local_20.y;
  (this->base).base.vector_0x120.rotation.z = local_20.z;
  return;
}
