#include "CameraViewManager.h"
#include "ed3D.h"
#include "MemoryStream.h"
#include "MathOps.h"
#include <math.h>

#ifdef PLATFORM_PS2
#include <libvu0.h>
#endif

CameraViewManager* g_CameraViewManager_00448e98 = NULL;
CameraViewManager* g_CameraViewManager_00448e9c = NULL;
edFCamera CameraObj_130_0044a100 = { 0 };
edFCamera g_CameraObj_0044ae10 = { 0 };
edFCamera CameraObj_130_ARRAY_0044a230[10] = { 0 };

CameraViewManagerSubObj::CameraViewManagerSubObj()
{
	CameraViewManagerSubObj* pCVar1;
	int iVar2;

	this->field_0x20c = 0;
	iVar2 = 0;
	this->field_0x210 = 0;
	this->field_0x208 = 0;
	this->field_0x214 = 0;
	this->field_0x218 = 0.0;
	pCVar1 = this;
	do {
		*(undefined4*)pCVar1->aSubObj40 = 0;
		*(undefined4*)&pCVar1->aSubObj40[0].field_0x4 = 0;
		iVar2 = iVar2 + 8;
		*(undefined4*)&pCVar1->aSubObj40[0].field_0x8 = 0;
		*(undefined4*)&pCVar1->aSubObj40[0].field_0xc = 0;
		*(undefined4*)&pCVar1->aSubObj40[0].field_0x10 = 0;
		*(undefined4*)&pCVar1->aSubObj40[0].field_0x14 = 0;
		*(undefined4*)&pCVar1->aSubObj40[0].field_0x18 = 0;
		*(undefined4*)&pCVar1->aSubObj40[0].field_0x1c = 0;
		*(undefined4*)&pCVar1->aSubObj40[0].field_0x20 = 0;
		*(undefined4*)&pCVar1->aSubObj40[0].field_0x24 = 0;
		*(undefined4*)&pCVar1->aSubObj40[0].field_0x28 = 0;
		*(undefined4*)&pCVar1->aSubObj40[0].field_0x2c = 0;
		*(undefined4*)&pCVar1->aSubObj40[0].field_0x30 = 0;
		*(undefined4*)&pCVar1->aSubObj40[0].field_0x34 = 0;
		*(undefined4*)&pCVar1->aSubObj40[0].field_0x38 = 0;
		*(undefined4*)&pCVar1->aSubObj40[0].field_0x3c = 0;
		pCVar1 = (CameraViewManagerSubObj*)(pCVar1->aSubObj40 + 1);
	} while (iVar2 < 0x40);
	this->pActiveCameraView_0x204 = (CameraViewBase*)0x0;
	return;
}

CameraViewManager::CameraViewManager()
{
	//FUN_003fedc0((long)&this->field_0x710);
	g_CameraViewManager_00448e98 = this;
	this->field_0xa7c = 0;
	SetupFunc_00197770();
	return;
}

void CameraViewManager::Game_Init()
{
	(this->field_0x470).x = 1.333333;
	return;
}

void MatrixFunc_0025a370(float param_1, float param_2, float param_3, edFCamera* m0)
{
	m0->aMatrices[0].db = param_1 * param_2;
	m0->aMatrices[0].dc = param_1;
	m0->aMatrices[0].dd = param_1 / param_3;
	return;
}

void MatrixFunc_001983f0(edFCamera* inputMatrix)
{
	float fVar1;
	float fVar2;
	float fVar3;
	Vector rowDiffVector;
	Vector rotationVector;
	edF32MATRIX4 transformedMatrix;

	transformedMatrix.aa = g_IdentityMatrix.aa;
	transformedMatrix.ab = g_IdentityMatrix.ab;
	transformedMatrix.ac = g_IdentityMatrix.ac;
	transformedMatrix.ad = g_IdentityMatrix.ad;
	transformedMatrix.ba = g_IdentityMatrix.ba;
	transformedMatrix.bb = g_IdentityMatrix.bb;
	transformedMatrix.bc = g_IdentityMatrix.bc;
	transformedMatrix.bd = g_IdentityMatrix.bd;
	transformedMatrix.ca = g_IdentityMatrix.ca;
	transformedMatrix.cb = g_IdentityMatrix.cb;
	transformedMatrix.cc = g_IdentityMatrix.cc;
	transformedMatrix.cd = g_IdentityMatrix.cd;
	transformedMatrix.da = g_IdentityMatrix.da;
	transformedMatrix.db = g_IdentityMatrix.db;
	transformedMatrix.dc = g_IdentityMatrix.dc;
	transformedMatrix.dd = g_IdentityMatrix.dd;
	rowDiffVector.x = inputMatrix->aMatrices[0].aa - inputMatrix->aMatrices[0].ba;
	rowDiffVector.y = inputMatrix->aMatrices[0].ab - inputMatrix->aMatrices[0].bb;
	rowDiffVector.z = inputMatrix->aMatrices[0].ac - inputMatrix->aMatrices[0].bc;
	rowDiffVector.w = inputMatrix->aMatrices[0].ad - inputMatrix->aMatrices[0].bd;
	//MY_LOG("Input\n");
	//PRINT_MATRIX(&inputMatrix->aMatrices[0]);
	//MY_LOG("rowDiffVector\n");
	//PRINT_VECTOR(&rowDiffVector);
	sceVu0Normalize((TO_SCE_VECTOR)&rowDiffVector, (TO_SCE_VECTOR)&rowDiffVector);
	CalculatePitchAngles2D_00193c70(&rotationVector, &rowDiffVector);
	//MY_LOG("Rotation Vector\n");
	//PRINT_VECTOR(&rotationVector);
	fVar3 = inputMatrix->aMatrices[0].da;
	if (fVar3 != 0.0) {
		CalculateRotationMatrix_002673f0(fVar3, &transformedMatrix, &transformedMatrix);
	}
	RotateMatrixByAngle(rotationVector.x, &transformedMatrix, &transformedMatrix);
	//MY_LOG("Rotation Matrix A\n");
	//PRINT_MATRIX(&transformedMatrix);
	ApplyRotationToMatrix(rotationVector.y, &transformedMatrix, &transformedMatrix);
	//MY_LOG("Rotation Matrix B\n");
	//PRINT_MATRIX(&transformedMatrix);
	sceVu0CopyMatrix((TO_SCE_MTX)(inputMatrix->aMatrices + 1), (TO_SCE_MTX)&transformedMatrix);
	//MY_LOG("inputMatrix->aMatrices + 1 A\n");
	//PRINT_MATRIX(inputMatrix->aMatrices + 1);
	fVar2 = inputMatrix->aMatrices[0].ab;
	fVar3 = inputMatrix->aMatrices[0].ac;
	fVar1 = inputMatrix->aMatrices[0].ad;
	inputMatrix->aMatrices[1].da = inputMatrix->aMatrices[0].aa;
	inputMatrix->aMatrices[1].db = fVar2;
	inputMatrix->aMatrices[1].dc = fVar3;
	inputMatrix->aMatrices[1].dd = fVar1;
	//MY_LOG("inputMatrix->aMatrices + 1 B\n");
	//PRINT_MATRIX(inputMatrix->aMatrices + 1);
	sceVu0InverseMatrix(inputMatrix->aMatrices + 2, inputMatrix->aMatrices + 1);
	//MY_LOG("inputMatrix->aMatrices + 2\n");
	//PRINT_MATRIX(inputMatrix->aMatrices + 2);
	fVar2 = inputMatrix->aMatrices[1].ab;
	fVar3 = inputMatrix->aMatrices[1].ac;
	fVar1 = inputMatrix->aMatrices[1].ad;
	(inputMatrix->vector_0x110).x = inputMatrix->aMatrices[1].aa;
	(inputMatrix->vector_0x110).y = fVar2;
	(inputMatrix->vector_0x110).z = fVar3;
	(inputMatrix->vector_0x110).w = fVar1;
	return;
}

void CameraViewManager::SetupFunc_00197770()
{
	CameraViewManager* pCVar1;
	edFCamera* m0;
	int iVar2;

	CameraObj_130_0044a100.aMatrices[0].aa = 0.0f;
	CameraObj_130_0044a100.aMatrices[0].ab = 0.0f;
	CameraObj_130_0044a100.aMatrices[0].ac = 0.0f;
	CameraObj_130_0044a100.aMatrices[0].ad = 1.0f;
	CameraObj_130_0044a100.aMatrices[0].ba = 0.0f;
	CameraObj_130_0044a100.aMatrices[0].bb = 0.0f;
	CameraObj_130_0044a100.aMatrices[0].bc = 1.0f;
	CameraObj_130_0044a100.aMatrices[0].bd = 1.0f;
	CameraObj_130_0044a100.aMatrices[0].da = 0.0f;
	MatrixFunc_0025a370(0.0001875f, 1.333333f, 0.25f, &CameraObj_130_0044a100);
	MatrixFunc_001983f0(&CameraObj_130_0044a100);
	g_CameraObj_0044ae10.aMatrices[0].aa = 0.0f;
	g_CameraObj_0044ae10.aMatrices[0].ab = 0.0f;
	g_CameraObj_0044ae10.aMatrices[0].ac = 0.0f;
	g_CameraObj_0044ae10.aMatrices[0].ad = 1.0f;
	g_CameraObj_0044ae10.aMatrices[0].ba = 0.0f;
	g_CameraObj_0044ae10.aMatrices[0].bb = 0.0f;
	g_CameraObj_0044ae10.aMatrices[0].bc = 1.0f;
	g_CameraObj_0044ae10.aMatrices[0].bd = 1.0f;
	g_CameraObj_0044ae10.aMatrices[0].da = 0.0f;
	MatrixFunc_0025a370(0.0001875f, 1.333333f, 0.25f, &g_CameraObj_0044ae10);
	MatrixFunc_001983f0(&g_CameraObj_0044ae10);
	iVar2 = 0;
	m0 = CameraObj_130_ARRAY_0044a230;
	pCVar1 = this;
	do {
		m0->aMatrices[0].aa = 0.0;
		m0->aMatrices[0].ab = 0.0;
		m0->aMatrices[0].ac = 0.0;
		m0->aMatrices[0].ad = 1.0;
		m0->aMatrices[0].ba = 0.0;
		m0->aMatrices[0].bb = 0.0;
		m0->aMatrices[0].bc = 1.0;
		m0->aMatrices[0].bd = 1.0;
		m0->aMatrices[0].da = 0.0;
		MatrixFunc_0025a370(0.0001875f, 1.333333f, 0.25f, m0);
		MatrixFunc_001983f0(m0);
		iVar2 = iVar2 + 1;
		pCVar1->pShadowSunView_0x4bc = (CameraViewBase*)0x0;
		m0 = m0 + 1;
		pCVar1 = (CameraViewManager*)&pCVar1->time_0x4;
	} while (iVar2 < 10);
	//SetupCameraViewManagerSubObj_00198fe0(&this->cameraViewManagerSubObj_0x4ec);
	//SetParam1280bToMaxint32(&this->field_0x710);
	this->loadedCameraCount_0x9f0 = 0;
	this->count_0x9f4 = 0;
	this->field_0x9f8 = 0;
	this->count_0x9fc = 0;
	this->pInitialView_0x4b4 = (CameraViewBase*)0x0;
	this->pShadowSunView_0x4b8 = (CameraViewBase*)0x0;
	this->pFrontendCamera_0x4e4 = (CameraViewBase*)0x0;
	this->pMouseQuakeCamera_0x4e8 = (CameraViewBase*)0x0;
	this->field_0xa78 = 0;
	this->time_0x4 = 0.0;
	return;
}

CameraViewBase* CameraViewManager::AddCamera(ECameraType type, ByteCode* pMemoryStream, char* objName)
{
	CameraViewBase* pCVar1;
	CameraViewBase* pCVar2;
	MainCamera* pMVar3;
	//MouseCameraView* pMVar4;
	//VSilverBoomy* pVVar5;
	//CinematicCameraView* pCinematicCameraView;
	//VExtendedCameraView_110* pVVar6;
	void* pvVar7;
	//FrontendCameraView* pFVar8;
	//DeathCameraViewLocationObject* pDeathCameraView;
	CameraViewBase* pCVar9;
	CameraViewBase* newObjectPtr;

	newObjectPtr = (CameraViewBase*)0x0;
	switch (type) {
	case CT_MouseQuake:
	IMPLEMENTATION_GUARD(
		pMVar4 = (MouseCameraView*)operator.new(0xe0);
		newObjectPtr = (long)(int)pMVar4;
		if (newObjectPtr != 0) {
			MouseCameraViewConstruction(pMVar4);
			(pMVar4->frontendCameraView).viewBase.pVTable = &CameraFunctionData_0043cf40;
			(pMVar4->vector_0xd0).y = -NAN;
			(pMVar4->vector_0xd0).x = 1.0;
			(pMVar4->frontendCameraView).viewBase.flags_0xc = (pMVar4->frontendCameraView).viewBase.flags_0xc | 0x40000;
			(pMVar4->frontendCameraView).viewBase.pVTable = &CameraFunctionData_0043cfa0;
			(pMVar4->frontendCameraView).viewBase.field_0x74 = 0.6;
		})
		break;
	case CT_MouseAround:
		IMPLEMENTATION_GUARD(
		pMVar4 = (MouseCameraView*)operator.new(0x100);
		newObjectPtr = (long)(int)pMVar4;
		if (newObjectPtr != 0) {
			MouseCameraViewConstruction(pMVar4);
			(pMVar4->frontendCameraView).viewBase.pVTable = &CameraFunctionData_0043cf40;
			(pMVar4->vector_0xd0).y = -NAN;
			(pMVar4->vector_0xd0).x = 1.0;
			(pMVar4->frontendCameraView).viewBase.flags_0xc = (pMVar4->frontendCameraView).viewBase.flags_0xc | 0x40000;
			(pMVar4->frontendCameraView).viewBase.pVTable = &CameraFunctionData_0043cee0;
			(pMVar4->frontendCameraView).viewBase.field_0x74 = 0.6;
			pMVar4[1].frontendCameraView.viewBase.matrix_0x10.aa = 0.0;
		})
		break;
	case CT_AroundSpecial:
		IMPLEMENTATION_GUARD(
		pMVar4 = (MouseCameraView*)operator.new(0x100);
		newObjectPtr = (long)(int)pMVar4;
		if (newObjectPtr != 0) {
			MouseCameraViewConstruction(pMVar4);
			(pMVar4->frontendCameraView).viewBase.pVTable = &CameraFunctionData_0043cf40;
			(pMVar4->vector_0xd0).y = -NAN;
			(pMVar4->vector_0xd0).x = 1.0;
			(pMVar4->frontendCameraView).viewBase.flags_0xc = (pMVar4->frontendCameraView).viewBase.flags_0xc | 0x40000;
			(pMVar4->frontendCameraView).viewBase.pVTable = &CameraFunctionData_0043cee0;
			(pMVar4->frontendCameraView).viewBase.field_0x74 = 0.6;
			pMVar4[1].frontendCameraView.viewBase.matrix_0x10.aa = 1.401298e-45;
		})
		break;
	case 3:
	case 4:
	case CT_KyaWindWall:
	case 6:
	case CT_KyaJamgut:
	case CT_MainCamera:
		pMVar3 = new MainCamera(type, pMemoryStream);
		newObjectPtr = (CameraViewBase*)pMVar3;
		break;
	case CT_SilverBoomy:
		IMPLEMENTATION_GUARD(
		pVVar5 = (VSilverBoomy*)operator.new(0x180);
		newObjectPtr = (long)(int)pVVar5;
		if (newObjectPtr != 0) {
			newObjectPtr = VSilverBoomy::Constructor_0018adb0(pVVar5);
		})
		break;
	case CT_IntView:
		IMPLEMENTATION_GUARD(
		pvVar7 = operator.new(0x110);
		newObjectPtr = (long)(int)pvVar7;
		if (newObjectPtr != 0) {
			newObjectPtr = IntViewConstructor ? (newObjectPtr);
		})
		break;
	case CT_ShadowSun:
		IMPLEMENTATION_GUARD(
		pvVar7 = operator.new(0x1710);
		newObjectPtr = (long)(int)pvVar7;
		if (newObjectPtr != 0) {
			newObjectPtr = SunConstructor ? (newObjectPtr);
		})
		break;
	case 0xd:
		IMPLEMENTATION_GUARD(
		pvVar7 = operator.new(0x1710);
		newObjectPtr = (long)(int)pvVar7;
		if (newObjectPtr != 0) {
			newObjectPtr = SunConstructor ? (newObjectPtr);
		})
		break;
	case 0xe:
		IMPLEMENTATION_GUARD(
		pVVar6 = (VExtendedCameraView_110*)operator.new(0x110);
		newObjectPtr = (long)(int)pVVar6;
		if (newObjectPtr != 0) {
			newObjectPtr = VExtendedCameraView_110::Deserialize_0018c630(pVVar6, pMemoryStream);
		})
		break;
	case 0x11:
		IMPLEMENTATION_GUARD(
		pvVar7 = operator.new(0x100);
		newObjectPtr = (long)(int)pvVar7;
		if (newObjectPtr != 0) {
			newObjectPtr = FUN_0018dbe0(newObjectPtr, pMemoryStream, objName);
		})
		break;
	case 0x12:
		IMPLEMENTATION_GUARD(
		pvVar7 = operator.new(0x110);
		newObjectPtr = (long)(int)pvVar7;
		if (newObjectPtr != 0) {
			newObjectPtr = FUN_0018d490(newObjectPtr, (int)pMemoryStream);
		})
		break;
	case 0x13:
		IMPLEMENTATION_GUARD(
		pvVar7 = operator.new(0x120);
		newObjectPtr = (long)(int)pvVar7;
		if (newObjectPtr != 0) {
			newObjectPtr = FUN_0018cb70(newObjectPtr, (int)pMemoryStream);
		})
		break;
	case 0x14:
		IMPLEMENTATION_GUARD(
		pvVar7 = operator.new(0x110);
		newObjectPtr = (long)(int)pvVar7;
		if (newObjectPtr != 0) {
			newObjectPtr = View::DeserializeView_0018c420(newObjectPtr, pMemoryStream);
		})
		break;
	case CT_Cinematic:
		IMPLEMENTATION_GUARD(
		pCinematicCameraView = (CinematicCameraView*)operator.new(0xc0);
		newObjectPtr = (long)(int)pCinematicCameraView;
		if (newObjectPtr != 0) {
			newObjectPtr = CinematicViewConstructor(pCinematicCameraView);
		})
		break;
	case 0x17:
		IMPLEMENTATION_GUARD(
		pFVar8 = (FrontendCameraView*)operator.new(0xc0);
		newObjectPtr = (long)(int)pFVar8;
		if (newObjectPtr != 0) {
			FrontendCameraViewConstructor(pFVar8);
			(pFVar8->viewBase).pVTable = (CameraFunctionData*)&DAT_00440940;
		})
		break;
	case CT_Frontend:
		IMPLEMENTATION_GUARD(
		pFVar8 = (FrontendCameraView*)operator.new(0xb0);
		newObjectPtr = (long)(int)pFVar8;
		if (newObjectPtr != 0) {
			newObjectPtr = FrontendCameraViewConstructor(pFVar8);
		})
		break;
	case CT_Death:
		IMPLEMENTATION_GUARD(
		pDeathCameraView = (DeathCameraViewLocationObject*)operator.new(0x120);
		newObjectPtr = (long)(int)pDeathCameraView;
		if (newObjectPtr != 0) {
			newObjectPtr = DeathViewConstructor(pDeathCameraView);
		})
	}
	pCVar9 = newObjectPtr;
	if (newObjectPtr != (CameraViewBase*)0x0) {
		pCVar1 = this->pInitialView_0x4b4;
		if (pCVar1 == (CameraViewBase*)0x0) {
			this->pInitialView_0x4b4 = pCVar9;
			g_CameraViewManager_00448e9c = this;
		}
		else {
			pCVar2 = pCVar1->pNextCameraView_0xa4;
			while (pCVar2 != (CameraViewBase*)0x0) {
				pCVar1 = pCVar1->pNextCameraView_0xa4;
				pCVar2 = pCVar1->pNextCameraView_0xa4;
			}
			pCVar1->pNextCameraView_0xa4 = pCVar9;
		}
		this->loadedCameraCount_0x9f0 = this->loadedCameraCount_0x9f0 + 1;
	}
	return pCVar9;
}

CameraViewBase::CameraViewBase(ByteCode* pMemoryStream)
{
	CheckpointManagerSubObjB* pCVar1;
	int* piVar2;
	int iVar3;
	uint uVar4;
	EventChunk_24* pEVar5;
	float fVar6;
	Vector local_10;

	//this->pVTable = (CameraFunctionData*)&ParticleDataBase::VTable_0043a3b0;
	this->field_0x4 = 0xffffffff;
	//this->pVTable = &VTable_0043d130;
	this->field_0x78 = (char*)0x0;
	this->pNextCameraView_0xa4 = (CameraViewBase*)0x0;
	*(undefined*)&this->field_0xa0 = 0;
	Func_0x34(0);
	fVar6 = pMemoryStream->GetF32();
	this->field_0x74 = fVar6;
	local_10.x = pMemoryStream->GetF32();
	local_10.y = pMemoryStream->GetF32();
	local_10.z = pMemoryStream->GetF32();
	local_10.w = 1.0;
	(this->matrix_0x10).da = local_10.x;
	(this->matrix_0x10).db = local_10.y;
	(this->matrix_0x10).dc = local_10.z;
	(this->matrix_0x10).dd = 1.0f;
	local_10.x = pMemoryStream->GetF32();
	local_10.y = pMemoryStream->GetF32();
	local_10.z = pMemoryStream->GetF32();
	local_10.w = 1.0;
	(this->field_0x60).x = local_10.x;
	(this->field_0x60).y = local_10.y;
	(this->field_0x60).z = local_10.z;
	(this->field_0x60).w = 1.0f;
	ps2_vu0_sub_vector(&local_10, &this->field_0x60, (Vector*)&(this->matrix_0x10).da);
	sceVu0Normalize((TO_SCE_VECTOR)&local_10, (TO_SCE_VECTOR)&local_10);
	(this->matrix_0x10).ca = local_10.x;
	(this->matrix_0x10).cb = local_10.y;
	(this->matrix_0x10).cc = local_10.z;
	(this->matrix_0x10).cd = local_10.w;
	local_10.x = pMemoryStream->GetF32();
	local_10.y = pMemoryStream->GetF32();
	local_10.z = pMemoryStream->GetF32();
	local_10.w = 0.0;
	(this->matrix_0x10).ba = local_10.x;
	(this->matrix_0x10).bb = local_10.y;
	(this->matrix_0x10).bc = local_10.z;
	(this->matrix_0x10).bd = 0.0;
	ps2_vu0_outer_product(&local_10, (Vector*)&(this->matrix_0x10).ba, (Vector*)&(this->matrix_0x10).ca);
	local_10.w = 0.0;
	(this->matrix_0x10).aa = local_10.x;
	(this->matrix_0x10).ab = local_10.y;
	(this->matrix_0x10).ac = local_10.z;
	(this->matrix_0x10).ad = 0.0f;
	iVar3 = pMemoryStream->GetS32();
	this->field_0x4 = iVar3;
	iVar3 = pMemoryStream->GetS32();
	this->field_0x8 = iVar3;
	uVar4 = pMemoryStream->GetU32();
	this->flags_0xc = uVar4 | 0x200000;
	fVar6 = pMemoryStream->GetF32();
	this->field_0x50 = fVar6;
	fVar6 = pMemoryStream->GetF32();
	this->field_0x54 = fVar6;
	fVar6 = pMemoryStream->GetF32();
	this->field_0x58 = fVar6;
	this->field_0x5c = 0;
	if ((this->flags_0xc & 2) == 0) {
		this->field_0x8c = 0.0f;
		this->field_0x90 = 5;
		this->field_0x98 = 0.8f;
		this->field_0x94 = 0;
		this->field_0x9c = 0.8f;
	}
	else {
		pEVar5 = (EventChunk_24*)pMemoryStream->GetS32();
		//this->field_0x80 = pEVar5;
		//pCVar1 = (CheckpointManagerSubObjB*)pMemoryStream->currentSeekPos;
		//pMemoryStream->currentSeekPos = (char*)&pCVar1->field_0x4;
		//if (pCVar1->pActor_0x0 != (AActorBase*)0x0) {
		//	pMemoryStream->currentSeekPos = pMemoryStream->currentSeekPos + (int)pCVar1->pActor_0x0 * 4;
		//}
		//this->field_0x84 = pCVar1;
		//piVar2 = (int*)pMemoryStream->currentSeekPos;
		//pMemoryStream->currentSeekPos = (char*)(piVar2 + 1);
		//if (*piVar2 != 0) {
		//	pMemoryStream->currentSeekPos = pMemoryStream->currentSeekPos + *piVar2 * 4;
		//}
		//this->field_0x88 = (float)piVar2;
		fVar6 = pMemoryStream->GetF32();
		this->field_0x8c = fVar6;
		iVar3 = pMemoryStream->GetS32();
		this->field_0x90 = iVar3;
		fVar6 = pMemoryStream->GetF32();
		this->field_0x98 = fVar6;
		iVar3 = pMemoryStream->GetS32();
		this->field_0x94 = iVar3;
		fVar6 = pMemoryStream->GetF32();
		this->field_0x9c = fVar6;
		if (this->field_0x80 == (EventChunk_24*)0xffffffff) {
			this->flags_0xc = this->flags_0xc & 0xfffffffd;
		}
	}
	piVar2 = (int*)pMemoryStream->currentSeekPos;
	pMemoryStream->currentSeekPos = (char*)(piVar2 + 1);
	if (*piVar2 != 0) {
		pMemoryStream->currentSeekPos = pMemoryStream->currentSeekPos + *piVar2 * 0x10;
	}
	this->field_0x78 = (char*)piVar2;
	return;
}

ECameraType CameraViewBase::GetCameraType()
{
	return CT_Frontend;
}

void CameraViewBase::Func_0x34(undefined4 param_2)
{
	this->field_0x70 = param_2;
}

Vector Vector3_00472250 = { 0 };

CameraView::CameraView(struct ByteCode* pMemoryStream)
	: CameraViewBase(pMemoryStream)
{
	float fVar1;
	float fVar2;

	this->field_0xb0 = 0;
	this->field_0xc0 = 0;
	fVar2 = Vector3_00472250.z;
	fVar1 = Vector3_00472250.y;
	this->field_0xb4 = Vector3_00472250.x;
	this->field_0xb8 = fVar1;
	this->field_0xbc = fVar2;
	return;
}

MainCamera::MainCamera(ECameraType type, ByteCode* pMemoryStream)
	: CameraView(pMemoryStream)
{
	ECameraType EVar1;
	long lVar2;
	astruct_8* puVar3;

	this->field_0x340 = 0;
	subObj_8.Init(pMemoryStream);
	lVar2 = subObj_12.Init(pMemoryStream);
	puVar3 = &this->subObj_8;
	if (lVar2 == 0) {
		puVar3->field_0x0 = puVar3->field_0x0 & 0xfffdffff;
	}
	else {
		puVar3->field_0x0 = puVar3->field_0x0 | 0x20000;
	}
	SetCameraType(type);
	EVar1 = GetCameraType();
	if (((EVar1 == 3) && ((this->subObj_8).field_0x2c < 1.22173f)) && (1.22173f < (this->subObj_8).field_0x30)) {
		(this->subObj_8).field_0x30 = 1.22173f;
	}
	flags_0xc = flags_0xc | 0x20000;
	return;
}

void MainCamera::SetCameraType(ECameraType type)
{
	uint uVar1;
	bool bVar2;
	bool bVar3;
	bool bVar4;
	bool bVar5;
	bool bVar6;
	bool bVar7;
	bool bVar8;

	if (type == CT_MainCamera) {
		uVar1 = flags_0xc;
		bVar2 = (uVar1 & 0x40) != 0;
		bVar3 = (uVar1 & 4) != 0;
		bVar4 = (uVar1 & 8) != 0;
		bVar5 = (uVar1 & 0x10) != 0;
		bVar6 = (uVar1 & 0x20) != 0;
		bVar7 = (uVar1 & 0x100) != 0;
		bVar8 = (uVar1 & 0x200) != 0;
		if ((byte)(bVar3 + bVar2 + bVar4 + bVar5 + bVar6 + bVar7 + bVar8) < 2) {
			if (bVar3) {
				type = (ECameraType)3;
			}
			else {
				if (bVar4) {
					type = CT_KyaJamgut;
				}
				else {
					if (bVar5) {
						type = (ECameraType)8;
					}
					else {
						if (bVar6) {
							type = (ECameraType)9;
						}
						else {
							if (bVar2) {
								type = (ECameraType)4;
							}
							else {
								if (bVar7) {
									type = CT_KyaWindWall;
								}
								else {
									type = (ECameraType)6;
									if (!bVar8) {
										type = (ECameraType)3;
									}
								}
							}
						}
					}
				}
			}
		}
		else {
			type = (ECameraType)3;
		}
	}
	this->cameraType_0xd4 = type;
	return;
}

ECameraType MainCamera::GetCameraType()
{
	return this->cameraType_0xd4;
}

void astruct_8::Init(struct ByteCode* pMemoryStream)
{
	uint uVar1;
	int iVar2;
	float fVar3;

	uVar1 = pMemoryStream->GetU32();
	this->field_0x0 = uVar1;
	uVar1 = pMemoryStream->GetS32();
	this->field_0x4 = uVar1;
	uVar1 = pMemoryStream->GetS32();
	this->field_0x8 = uVar1;
	fVar3 = pMemoryStream->GetF32();
	this->field_0x18 = fVar3;
	fVar3 = this->field_0x18 * 0.01745329 * 0.5;
	this->field_0x18 =
		sinf(fVar3) /
		cosf(fVar3);
	iVar2 = pMemoryStream->GetS32();
	if (iVar2 == 0) {
		this->field_0x20 = 2.0;
		this->field_0xc = 0.25;
		this->field_0x10 = 0.25;
		this->field_0x14 = 0.0;
		this->field_0x1c = 0.0;
	}
	else {
		if (iVar2 == 1) {
			fVar3 = pMemoryStream->GetF32();
			this->field_0x20 = fVar3;
			fVar3 = pMemoryStream->GetF32();
			this->field_0xc = fVar3;
			fVar3 = pMemoryStream->GetF32();
			this->field_0x10 = fVar3;
			fVar3 = pMemoryStream->GetF32();
			this->field_0x14 = fVar3;
			fVar3 = pMemoryStream->GetF32();
			this->field_0x1c = fVar3 * 0.01745329;
		}
	}
	fVar3 = pMemoryStream->GetF32();
	this->field_0x28 = fVar3 * 0.01745329;
	fVar3 = pMemoryStream->GetF32();
	this->field_0x2c = fVar3 * 0.01745329;
	fVar3 = pMemoryStream->GetF32();
	this->field_0x30 = fVar3 * 0.01745329;
	this->field_0x34 = 0.8726646;
	this->field_0x38 = 2.443461;
	this->field_0x3c = 2.443461;
	fVar3 = pMemoryStream->GetF32();
	this->field_0x40 = fVar3 * 0.01745329;
	fVar3 = pMemoryStream->GetF32();
	this->field_0x44 = fVar3 * 0.01745329;
	fVar3 = pMemoryStream->GetF32();
	this->field_0x48 = fVar3 * 0.01745329;
	this->field_0x4c = 1.047198;
	this->field_0x50 = 3.490659;
	this->field_0x54 = 3.490659;
	fVar3 = pMemoryStream->GetF32();
	this->field_0x58 = fVar3;
	fVar3 = pMemoryStream->GetF32();
	this->field_0x5c = fVar3;
	fVar3 = pMemoryStream->GetF32();
	this->field_0x60 = fVar3;
	this->field_0x64 = 4.0;
	this->field_0x68 = 8.0;
	this->field_0x6c = 10.0;
	fVar3 = this->field_0x5c;
	if (fVar3 < 2.5) {
		this->field_0x24 = 2.5;
	}
	else {
		this->field_0x24 = fVar3 + (this->field_0x58 - fVar3) * 0.5;
	}
	return;
}

bool astruct_12::Init(struct ByteCode* pMemoryStream)
{
	uint uVar1;
	float fVar2;

	uVar1 = pMemoryStream->GetU32();
	if (uVar1 == 1) {
		fVar2 = pMemoryStream->GetF32();
		this->field_0x0 = fVar2;
		pMemoryStream->GetF32();
		this->field_0x4 = 0.0;
		fVar2 = pMemoryStream->GetF32();
		this->field_0x8 = fVar2;
		pMemoryStream->GetF32();
		pMemoryStream->GetF32();
		this->field_0xc = 0.1;
		fVar2 = pMemoryStream->GetF32();
		this->field_0x10 = fVar2 * 0.01745329;
		pMemoryStream->GetF32();
		this->field_0x14 = 0.0;
		fVar2 = pMemoryStream->GetF32();
		this->field_0x18 = fVar2 * 0.01745329;
		fVar2 = pMemoryStream->GetF32();
		this->field_0x1c = fVar2;
		fVar2 = pMemoryStream->GetF32();
		this->field_0x20 = fVar2;
		pMemoryStream->GetF32();
		pMemoryStream->GetF32();
	}
	else {
		this->field_0x0 = 0.5;
		this->field_0x4 = 0.0;
		this->field_0x8 = 4.0;
		this->field_0xc = 0.1;
		this->field_0x10 = 0.0;
		this->field_0x14 = 0.0;
		this->field_0x18 = 3.141593;
		this->field_0x1c = 0.0;
		this->field_0x20 = 8.0;
	}
	return uVar1 == 1;
}
