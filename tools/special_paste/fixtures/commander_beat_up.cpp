void CActorCommander::StateCommanderBeatUp(CActorCommander *this)

{
  int iVar1;
  CCameraManager *pCameraManager;
  CPathFinderClient *pPathFinderClient;
  CChessBoard *pChessboard;
  float fVar2;
  float fVar3;
  float fVar4;
  edF32VECTOR4 local_10;
  CActorFighter *pFighter;

  iVar1 = CSquad::NbElt(&this->squad);
  if (iVar1 != 0) {
    pCameraManager = (CCameraManager *)CScene::GetManager(MO_Camera);
    local_10.x = (pCameraManager->transformationMatrix).ca;
    local_10.y = (pCameraManager->transformationMatrix).cb;
    local_10.z = (pCameraManager->transformationMatrix).cc;
    local_10.w = (pCameraManager->transformationMatrix).cd;
    pChessboard = &(this->squad).chessboard;
    pFighter = this->field_0x9f4->field_0x354;
    fVar4 = (pFighter->base).base.base.currentLocation.y;
    fVar2 = (pFighter->base).base.base.currentLocation.z;
    fVar3 = (pFighter->base).base.base.currentLocation.w;
    (this->squad).chessboard.field_0x230.x = (pFighter->base).base.base.currentLocation.x;
    (this->squad).chessboard.field_0x230.y = fVar4;
    (this->squad).chessboard.field_0x230.z = fVar2;
    (this->squad).chessboard.field_0x230.w = fVar3;
    fVar4 = gF32Vector4UnitZ.w;
    fVar3 = gF32Vector4UnitZ.z;
    fVar2 = gF32Vector4UnitZ.y;
    (this->squad).chessboard.field_0x240.x = gF32Vector4UnitZ.x;
    (this->squad).chessboard.field_0x240.y = fVar2;
    (this->squad).chessboard.field_0x240.z = fVar3;
    (this->squad).chessboard.field_0x240.w = fVar4;
    (this->squad).chessboard.writeLayerMask = 3;
    (this->squad).chessboard.readLayerMask = 3;
    CChessBoard::ValidateAll(pChessboard);
    local_10.y = 0.0;
    edF32Vector4GetNegHard(&local_10,&local_10);
    edF32Vector4NormalizeHard(&local_10,&local_10);
    (this->squad).chessboard.writeLayerMask = 1;
    (this->squad).chessboard.readLayerMask = 1;
    CChessBoard::InvalidateSection(1.570796,pChessboard,&local_10);
    pFighter = this->field_0x9f4;
    if (((pFighter->base).base.base.flags & 0x1000) == 0) {
      local_10.x = (pFighter->base).base.base.rotationQuat.x;
      local_10.y = (pFighter->base).base.base.rotationQuat.y;
      local_10.z = (pFighter->base).base.base.rotationQuat.z;
      local_10.w = (pFighter->base).base.base.rotationQuat.w;
    }
    else {
      SetVectorFromAngleY((pFighter->base).base.base.rotationEuler.y,&local_10);
    }
    edF32Vector4GetNegHard(&local_10,&local_10);
    (this->squad).chessboard.writeLayerMask = 1;
    (this->squad).chessboard.readLayerMask = 1;
    CChessBoard::InvalidateSection(3.141593,pChessboard,&local_10);
    pPathFinderClient = (*((this->field_0x9f4->base).base.base.pVTable)->GetPathfinderClient)((CActorAutonomous *)this->field_0x9f4);
    if (pPathFinderClient->id == -1) {
      (this->squad).chessboard.writeLayerMask = 2;
      (this->squad).chessboard.readLayerMask = 2;
      CChessBoard::InvalidateByZone(pChessboard,this->guardAreaZoneId);
    }
    else {
      (this->squad).chessboard.writeLayerMask = 2;
      (this->squad).chessboard.readLayerMask = 2;
      CChessBoard::InvalidateByPath(pChessboard,pPathFinderClient);
    }
    (*(code *)this->pVTable->_UpdatePattern)(this);
  }
  return;
}
