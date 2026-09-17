#include "Light.h"
#include "MathOps.h"
#include "LightManager.h"
#include "EventManager.h"
#include "MemoryStream.h"
#include "TimeController.h"

CLight::CLight()
{
	this->pZoneHolderA = (S_ZONE_STREAM_REF*)0x0;
	this->pZoneHolderB = (S_ZONE_STREAM_REF*)0x0;

	return;
}

// Should be in: D:/Projects/b-witch/light.cpp
void CLight::Init()
{
	bool bVar1;
	uint uVar2;
	S_ZONE_STREAM_REF* pSVar3;
	int iVar4;
	int iVar5;
	int iVar6;
	CLightManager* pLightManager;

	if (this->pZoneHolderA == (S_ZONE_STREAM_REF*)0x0) {
		iVar4 = 0;
	}
	else {
		iVar4 = this->pZoneHolderA->entryCount;
	}

	bVar1 = iVar4 != 0;
	if (!bVar1) {
		if (this->pZoneHolderB == (S_ZONE_STREAM_REF*)0x0) {
			iVar4 = 0;
		}
		else {
			iVar4 = this->pZoneHolderB->entryCount;
		}
		bVar1 = iVar4 != 0;
	}

	if (bVar1) {
		pSVar3 = this->pZoneHolderA;

		S_STREAM_REF<ed_zone_3d>* pCurRef = pSVar3->aEntries;

		for (iVar4 = pSVar3->entryCount; iVar4 != 0; iVar4 = iVar4 + -1) {
			pSVar3 = (S_ZONE_STREAM_REF*)pSVar3->aEntries;
			pCurRef->Init();
			pCurRef = pCurRef + 1;
		}

		pSVar3 = this->pZoneHolderB;
		pLightManager = CScene::ptable.g_LightManager_004516b0;
		pCurRef = pSVar3->aEntries;
		for (iVar4 = pSVar3->entryCount; CScene::ptable.g_LightManager_004516b0 = pLightManager, iVar4 != 0; iVar4 = iVar4 + -1) {
			pCurRef->Init();
			pCurRef = pCurRef + 1;
			pLightManager = CScene::ptable.g_LightManager_004516b0;
		}

		if (this->pZoneHolderA == (S_ZONE_STREAM_REF*)0x0) {
			iVar4 = 0;
		}
		else {
			iVar4 = this->pZoneHolderA->entryCount;
		}

		iVar6 = 0;
		if (0 < iVar4) {
			do {
				pLightManager->ReferenceZone(this->pZoneHolderA->aEntries[iVar6].Get());
				iVar6 = iVar6 + 1;
				IMPLEMENTATION_GUARD(
					//*(uint*)((int)&this->pZoneHolderA->aEntries[0].pZone + iVar5) = uVar2;
				)
			} while (iVar6 < iVar4);
		}

		iVar4 = 0;
		if (this->pZoneHolderB != (S_ZONE_STREAM_REF*)0x0) {
			iVar4 = this->pZoneHolderB->entryCount;
		}

		iVar6 = 0;
		if (0 < iVar4) {
			do {
				pLightManager->ReferenceZone(this->pZoneHolderB->aEntries[iVar6].Get());
				iVar6 = iVar6 + 1;
				IMPLEMENTATION_GUARD(
					//*(uint*)((int)&this->pZoneHolderB->aEntries[0].pZone + iVar5) = uVar2;
				)
			} while (iVar6 < iVar4);
		}
	}

	return;
}

void CLight::Manage()
{
	return;
}

void CLight::Term()
{
	return;
}

// Should be in: D:/Projects/b-witch/light.cpp
void CLight::Activate()
{
	CScene::ptable.g_LightManager_004516b0->Activate(this, -2);

	return;
}

void CLight::Inactivate()
{
	_rgba inactiveColour;
	CLightManager* pLightManager;

	pLightManager = CScene::ptable.g_LightManager_004516b0;
	inactiveColour.rgba = this->colour_0x4.rgba & 0x7f0fffff | 0x700000;
	this->colour_0x4 = inactiveColour;
	pLightManager->bActiveListDirty = 1;

	return;
}

bool CLight::DoLighting(LightingContext* pContext)
{
	return false;
}

int CLight::GetBaseShape(BaseShape** ppBaseShape)
{
	return 0;
}

void CLight::SetPosition(edF32VECTOR4* pPosition)
{
	*GetPosition() = *pPosition;

	return;
}

void CLight::SetColour(edF32VECTOR4* pColour)
{
	*GetColour() = *pColour;

	return;
}

bool CLight::TestIlluminationZones(edF32VECTOR4* pLocation, int id)
{
	ed_zone_3d* peVar1;
	ed_zone_3d** ppeVar2;
	int iVar3;
	int iVar4;
	int iVar5;
	CEventManager* pEventManager;
	CLightManager* pLightManager;

	pLightManager = CScene::ptable.g_LightManager_004516b0;
	pEventManager = CScene::ptable.g_EventManager_006f5080;

	if ((int*)this->pZoneHolderB == (int*)0x0) {
		iVar3 = 0;
	}
	else {
		iVar3 = *(int*)this->pZoneHolderB;
	}

	LIGHT_MANAGER_LOG(LogLevel::VeryVerbose, "CLight::TestIlluminationZones count: {}", iVar3);

	iVar4 = 0;
	if (0 < iVar3) {
		iVar5 = 0;
		IMPLEMENTATION_GUARD(
			do {
				ppeVar2 = (ed_zone_3d**)(pLightManager->field_0xe0 + *(int*)(this->pZoneHolderB + iVar5 + 4) * 0xc);
				if ((id == -1) || (ppeVar2[2] != (ed_zone_3d*)id)) {
					peVar1 = (ed_zone_3d*)
						edEventComputeZoneAgainstVertex(pEventManager->activeEventChunkID_0x8, *ppeVar2, pLocation, 0);
					ppeVar2[1] = peVar1;
					ppeVar2[2] = (ed_zone_3d*)id;
				}
				if (ppeVar2[1] == (ed_zone_3d*)&DAT_00000001) {
					return false;
				}
				iVar4 = iVar4 + 1;
				iVar5 = iVar5 + 4;
			} while (iVar4 < iVar3);)
	}

	iVar3 = 0;
	if (this->pZoneHolderA->entryCount != 0) {
		iVar3 = this->pZoneHolderA->entryCount;
	}

	LIGHT_MANAGER_LOG(LogLevel::VeryVerbose, "CLight::TestIlluminationZones count: {}", iVar3);

	iVar4 = 0;
	if (0 < iVar3) {
		iVar5 = 0;
		IMPLEMENTATION_GUARD(
			do {
				ppeVar2 = (ed_zone_3d**)(pLightManager->field_0xe0 + *(int*)(this->pZoneHolderA + iVar5 + 4) * 0xc);
				if ((id == -1) || (ppeVar2[2] != (ed_zone_3d*)id)) {
					peVar1 = (ed_zone_3d*)
						edEventComputeZoneAgainstVertex(pEventManager->activeEventChunkID_0x8, *ppeVar2, pLocation, 0);
					ppeVar2[1] = peVar1;
					ppeVar2[2] = (ed_zone_3d*)id;
				}
				if (ppeVar2[1] == (ed_zone_3d*)&DAT_00000002) {
					return false;
				}
				iVar4 = iVar4 + 1;
				iVar5 = iVar5 + 4;
			} while (iVar4 < iVar3);)
	}
	return true;
}

bool CLight::IsInCluster(ed_zone_3d* pZone)
{
	CEventManager* pEventManager;
	edF32VECTOR4* pPosition;
	int iVar3;

	pEventManager = CScene::ptable.g_EventManager_006f5080;
	pPosition = GetPosition();
	iVar3 = edEventComputeZoneAgainstVertex(pEventManager->activeChunkId, pZone, pPosition, 0);
	return iVar3 != 2;
}

CLightSun::CLightSun()
	: CLight()
{
	this->referencedLightIndex = -1;
	this->field_0x8 = 0;

	this->colorModel.field_0x20 = gF32Vector4Zero;
	this->colorModel.color = this->colorModel.field_0x20;
	this->colorModel.ambientColor = this->colorModel.color;

	this->baseShape.position = gF32Vector4Zero;

	this->baseShape.fallout1 = 1.0f;
	this->baseShape.fallout0 = 1.0f;

	this->baseShape.direction = gF32Vector4Zero;
}

void CLightSun::Manage()
{
	CLightManager* pCVar1;
	edF32VECTOR4* pfVar2;
	edF32VECTOR4* pfVar3;
	int iVar2;
	float in_f0 = 0.0f;
	float fVar3;
	float fVar4;
	float fVar5;

	pCVar1 = CScene::ptable.g_LightManager_004516b0;
	if ((this->field_0x8 & 1) == 0) {
		iVar2 = (int)(((long long)colour_0x4.b << 0x38) >> 0x3c);
		pfVar2 = &((CScene::ptable.g_LightManager_004516b0)->lightConfig).pLightDirections->vector[iVar2];
		fVar3 = (this->colorModel).ambientColor.x;
		fVar4 = (this->colorModel).ambientColor.y;
		fVar5 = (this->colorModel).ambientColor.z;
		pfVar2->x = pfVar2->x + (this->baseShape).direction.w * -1.0f;
		pfVar2->y = pfVar2->y + fVar3 * -1.0f;
		pfVar2->z = pfVar2->z + fVar4 * -1.0f;
		pfVar2->w = pfVar2->w + fVar5 * -1.0f;
		pfVar3 = &((CScene::ptable.g_LightManager_004516b0)->lightConfig).pLightColorMatrix->vector[iVar2];
		fVar3 = (this->colorModel).field_0x20.x;
		fVar4 = (this->colorModel).field_0x20.y;
		fVar5 = (this->colorModel).field_0x20.z;
		pfVar3->x = pfVar3->x + (this->colorModel).color.w;
		pfVar3->y = pfVar3->y + fVar3;
		pfVar3->z = pfVar3->z + fVar4;
		pfVar3->w = pfVar3->w + fVar5;
		pCVar1->field_0x104 = pCVar1->field_0x104 + 1;
	}
	return;
}

void CLightSun::Activate()
{
	CScene::ptable.g_LightManager_004516b0->Activate(this, -2);

	return;
}

bool CLightSun::DoLighting(LightingContext* pContext)
{
	edF32VECTOR4* peVar1;
	float fVar2;
	float fVar3;
	float fVar4;
	float fVar5;

	*pContext->pLightDirection = (*pContext->pLightDirection) + ((this->baseShape).direction * -1.0f);

	pContext->colorMultiplier = 1.0f;

	if (pContext->colorMultiplier == 1.0f) {
		*pContext->pLightAmbient = (*pContext->pLightAmbient) + this->colorModel.ambientColor;
		*pContext->pLightColor = (*pContext->pLightColor) + this->colorModel.color;
	}
	else {
		*pContext->pLightAmbient = (*pContext->pLightAmbient) + (this->colorModel.ambientColor * pContext->colorMultiplier);
		*pContext->pLightColor = (*pContext->pLightColor) + (this->colorModel.color * pContext->colorMultiplier);
	}
	return true;
}

int CLightSun::GetBaseShape(BaseShape** ppBaseShape)
{
	if (ppBaseShape != (BaseShape**)0x0) {
		*ppBaseShape = (BaseShape*)&(this->baseShape);
	}
	return 1;
}

edF32VECTOR4* CLightSun::GetPosition()
{
	return &this->baseShape.position;
}

edF32VECTOR4* CLightSun::GetColour()
{
	return &this->colorModel.color;
}

CLightSpot::CLightSpot()
	: CLight()
{
	this->referencedLightIndex = -1;
	this->field_0x8 = 0;

	this->colorModel.field_0x20 = gF32Vector4Zero;
	this->colorModel.color = this->colorModel.field_0x20;
	this->colorModel.ambientColor = this->colorModel.color;

	this->baseShape.position = gF32Vector4Zero;

	this->baseShape.fallout1 = 1.0f;
	this->baseShape.fallout0 = 1.0f;

	this->baseShape.direction = gF32Vector4Zero;

	this->baseShape.fov0 = 1.0f;
	this->baseShape.fov1 = 1.0f;
}

void CLightSpot::Manage()
{
	return;
}

bool CLightSpot::DoLighting(LightingContext* pContext)
{
	IMPLEMENTATION_GUARD();
	return false;
}

void CLightSpot::Activate()
{
	CLightManager* pLightManager;

	pLightManager = CScene::ptable.g_LightManager_004516b0;
	CScene::ptable.g_LightManager_004516b0->Activate(this, -2);
	pLightManager->field_0x108 = pLightManager->field_0x108 + 1;

	return;
}

int CLightSpot::GetBaseShape(BaseShape** ppBaseShape)
{
	if (ppBaseShape != (BaseShape**)0x0) {
		*ppBaseShape = &this->baseShape;
	}

	return 4;
}

edF32VECTOR4* CLightSpot::GetPosition()
{
	return &this->baseShape.position;
}

edF32VECTOR4* CLightSpot::GetColour()
{
	return &this->colorModel.color;
}

CLightDirectional::CLightDirectional()
	: CLight()
{
	this->referencedLightIndex = -1;
	this->field_0x8 = 0;

	this->colorModel.field_0x20 = gF32Vector4Zero;
	this->colorModel.color = this->colorModel.field_0x20;
	this->colorModel.ambientColor = this->colorModel.color;

	this->baseShape.position = gF32Vector4Zero;

	this->baseShape.fallout1 = 1.0f;
	this->baseShape.fallout0 = 1.0f;

	this->baseShape.direction = gF32Vector4Zero;

	this->baseShape.fov0 = 1.0f;
	this->baseShape.fov1 = 1.0f;
}

void CLightDirectional::Manage()
{
	IMPLEMENTATION_GUARD();
}

void CLightDirectional::Activate()
{
	CLightManager* pLightManager;

	pLightManager = CScene::ptable.g_LightManager_004516b0;
	CScene::ptable.g_LightManager_004516b0->Activate(this, -2);
	pLightManager->field_0x110 = pLightManager->field_0x110 + 1;

	return;
}

void CLightDirectional::Inactivate()
{
	_rgba _Stack4;
	CLightManager* pLightManager;

	pLightManager = CScene::ptable.g_LightManager_004516b0;
	_Stack4.rgba = this->colour_0x4.rgba & 0x7f0fffff | 0x700000;
	this->colour_0x4 = _Stack4;
	pLightManager->bActiveListDirty = 1;

	return;
}

bool CLightDirectional::DoLighting(LightingContext* pContext)
{
	IMPLEMENTATION_GUARD();
	return false;
}

int CLightDirectional::GetBaseShape(BaseShape** ppBaseShape)
{
	if (ppBaseShape != (BaseShape**)0x0) {
		*ppBaseShape = &this->baseShape;
	}
	return 2;
}

edF32VECTOR4* CLightDirectional::GetPosition()
{
	return &this->baseShape.position;
}

edF32VECTOR4* CLightDirectional::GetColour()
{
	return &this->colorModel.color;
}

CLightAmbient::CLightAmbient()
	: CLight()
{
	this->referencedLightIndex = -1;
	this->field_0x8 = 0;

	this->colorModel.ambientColor = gF32Vector4Zero;
	this->baseShape.position = gF32Vertex4Zero;
}

void CLightAmbient::Manage()
{
	edF32VECTOR4* peVar1;

	if ((this->field_0x8 & 1) == 0) {
		peVar1 = ((CScene::ptable.g_LightManager_004516b0)->lightConfig).pLightAmbient;
		*peVar1 = *peVar1 + this->colorModel.ambientColor;
	}
	return;
}

bool CLightAmbient::DoLighting(LightingContext* pContext)
{
	*pContext->pLightAmbient = this->colorModel.ambientColor;
	return true;
}

edF32VECTOR4* CLightAmbient::GetPosition()
{
	return &this->baseShape.position;
}

edF32VECTOR4* CLightAmbient::GetColour()
{
	return &this->colorModel.ambientColor;
}

void CLightOmni::Manage()
{
	return;
}

void CLightOmni::Activate()
{
	CLightManager* pLightManager;

	pLightManager = CScene::ptable.g_LightManager_004516b0;
	CScene::ptable.g_LightManager_004516b0->Activate(this, -2);

	pLightManager->field_0x10c = pLightManager->field_0x10c + 1;

	return;
}

bool CLightOmni::DoLighting(LightingContext* pContext)
{
	IMPLEMENTATION_GUARD();
	return false;
}

int CLightOmni::GetBaseShape(BaseShape** ppBaseShape)
{
	if (ppBaseShape != (BaseShape**)0x0) {
		*ppBaseShape = (BaseShape*)&this->baseShape;
	}

	return 3;
}

edF32VECTOR4* CLightOmni::GetPosition()
{
	return &this->baseShape.position;
}

edF32VECTOR4* CLightOmni::GetColour()
{
	return &this->colorModel.color;
}

void CLightOmni::Create()
{
	this->colorModel.field_0x20 = gF32Vector4Zero;
	this->colorModel.color = this->colorModel.field_0x20;
	this->colorModel.ambientColor = this->colorModel.color;

	this->baseShape.position = gF32Vector4Zero;

	this->baseShape.fallout1 = 1.0f;
	this->baseShape.fallout0 = 1.0f;
}

CLightTorch::CLightTorch(ByteCode* pByteCode)
	: CLight()
	, lightType(-1)
{
	this->referencedLightIndex = -1;
	this->field_0x8 = 0;

	Create(pByteCode);
}

void CLightTorch::Manage()
{
	CLightManager* pCVar1;
	Timer* pTVar2;
	int iVar3;
	float* pfVar4;
	float fVar5;
	float puVar6;
	float fVar6;
	float fVar7;

	pTVar2 = Timer::GetTimer();
	fVar5 = (this->field_0xc0).y - pTVar2->cutsceneDeltaTime;
	(this->field_0xc0).y = fVar5;
	if (fVar5 < 0.0) {
		fVar5 = (this->field_0xc0).w;
		(this->field_0xc0).z = fVar5;
		if (fVar5 < 0.5) {
			iVar3 = rand();
			fVar5 = (this->field_0xc0).z + ((float)iVar3 / 2.147484e+09f) * 0.7f + 0.3f;
			(this->field_0xc0).w = fVar5;
			if (1.0 < fVar5) {
				(this->field_0xc0).w = 1.0;
			}
		}
		else {
			iVar3 = rand();
			fVar5 = ((this->field_0xc0).z - ((float)iVar3 / 2.147484e+09f) * 0.7f) - 0.3f;
			(this->field_0xc0).w = fVar5;
			if (fVar5 < 0.0) {
				(this->field_0xc0).w = 0.0;
			}
		}
		(this->field_0xc0).y = (this->field_0xc0).x;
	}
	fVar6 = (this->field_0xc0).x;
	fVar5 = (this->field_0xc0).z;
	puVar6 = ((fVar6 - (this->field_0xc0).y) * ((this->field_0xc0).w - fVar5)) / fVar6;
	edF32Vector4LERPHard(fVar5 + puVar6, &(this->colorModel).color, &this->angleRotY, &this->field_0xb0);
	pCVar1 = CScene::ptable.g_LightManager_004516b0;
	if (((this->field_0x8 & 1) == 0) && (this->lightType == 1)) {
		IMPLEMENTATION_GUARD(
			iVar3 = (int)(((long)(char)(this->light).colour_0x4.b << 0x38) >> 0x3c);
		pfVar4 = &(((CScene::ptable.g_LightManager_004516b0)->lightConfig).pLightDirections)->aa + iVar3 * 4;
		fVar5 = (this->baseShape).base.direction.y;
		fVar6 = (this->baseShape).base.direction.z;
		fVar7 = (this->baseShape).base.direction.w;
		*pfVar4 = *pfVar4 + (this->baseShape).base.direction.x * -1.0;
		pfVar4[1] = pfVar4[1] + fVar5 * -1.0;
		pfVar4[2] = pfVar4[2] + fVar6 * -1.0;
		pfVar4[3] = pfVar4[3] + fVar7 * -1.0;
		pfVar4 = &((pCVar1->lightConfig).pLightColorMatrix)->aa + iVar3 * 4;
		fVar5 = this->colorModel.color.y;
		fVar6 = this->colorModel.color.z;
		fVar7 = this->colorModel.color.w;
		*pfVar4 = *pfVar4 + this->colorModel.color.x;
		pfVar4[1] = pfVar4[1] + fVar5;
		pfVar4[2] = pfVar4[2] + fVar6;
		pfVar4[3] = pfVar4[3] + fVar7;
		pCVar1->field_0x104 = pCVar1->field_0x104 + 1;)
	}
	return;
}

void CLightTorch::Activate()
{
	CScene::ptable.g_LightManager_004516b0->Activate(this, -2);

	return;
}

bool CLightTorch::DoLighting(LightingContext* pContext)
{
	IMPLEMENTATION_GUARD();
	return false;
}

int CLightTorch::GetBaseShape(BaseShape** ppBaseShape)
{
	int lightType;

	lightType = this->lightType;
	if (lightType == 4) {
		if (ppBaseShape != (BaseShape**)0x0) {
			*ppBaseShape = &this->baseShape;
		}
		lightType = 4;
	}
	else {
		if (lightType == 2) {
			if (ppBaseShape != (BaseShape**)0x0) {
				*ppBaseShape = &this->baseShape;
			}
			lightType = 2;
		}
		else {
			if (lightType == 1) {
				if (ppBaseShape != (BaseShape**)0x0) {
					*ppBaseShape = &this->baseShape;
				}
				lightType = 1;
			}
			else {
				if (lightType == 3) {
					if (ppBaseShape != (BaseShape**)0x0) {
						*ppBaseShape = &this->baseShape;
					}
					lightType = 3;
				}
				else {
					lightType = -1;
				}
			}
		}
	}

	return lightType;
}

edF32VECTOR4* CLightTorch::GetPosition()
{
	return &this->baseShape.position;
}

edF32VECTOR4* CLightTorch::GetColour()
{
	return &this->colorModel.color;
}

void CLightTorch::Create(ByteCode* pByteCode)
{
	int* pSVar1;
	int* piVar2;
	byte bVar3;
	int lightType;
	int sectorId;
	int iVar6;
	float fVar7;
	float fVar8;
	float fVar9;

	lightType = pByteCode->GetS32();
	this->lightType = lightType;
	this->colorModel.Read(pByteCode);
	if (lightType == 4) {
		reinterpret_cast<SpotShape*>(&this->baseShape)->Read(pByteCode);
	}
	else {
		if (lightType == 1) {
			reinterpret_cast<SunShape*>(&this->baseShape)->Read(pByteCode);
		}
		else {
			if (lightType == 2) {
				IMPLEMENTATION_GUARD(
					DirectionalShape::Read(&this->baseShape, pByteCode);)
			}
			else {
				if (lightType == 3) {
					reinterpret_cast<LightShape*>(&this->baseShape)->Read(pByteCode);
				}
				else {
					if (lightType == -1) {
						reinterpret_cast<LightShape*>(&this->baseShape)->Read(pByteCode);
					}
				}
			}
		}
	}

	sectorId = pByteCode->GetS32();
	iVar6 = pByteCode->GetS32();
	uint uVar7 = pByteCode->GetU32();

	LIGHT_MANAGER_LOG(LogLevel::Info, "CLightTorch::Create Extra params: sector Id: {} {} {}", sectorId, iVar6, uVar7);

	this->field_0x8 = (ushort)uVar7;
	pSVar1 = (int*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = pByteCode->currentSeekPos + 4;
	if (*pSVar1 != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + *pSVar1 * 4;
	}
	this->pZoneHolderA = reinterpret_cast<S_ZONE_STREAM_REF*>(pSVar1);

	piVar2 = (int*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = pByteCode->currentSeekPos + 4;
	if (*piVar2 != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + *piVar2 * 4;
	}
	this->pZoneHolderB = reinterpret_cast<S_ZONE_STREAM_REF*>(piVar2);

	(this->angleRotY) = this->colorModel.color;
	bVar3 = pByteCode->GetU8();
	if ((char)bVar3 < '\0') {
		fVar7 = (float)((uint)(int)(char)bVar3 >> 1 | (int)(char)bVar3 & 1U);
		fVar7 = fVar7 + fVar7;
	}
	else {
		fVar7 = (float)(int)(char)bVar3;
	}
	(this->field_0xb0).x = fVar7 * 2.0f;
	bVar3 = pByteCode->GetU8();
	if ((char)bVar3 < '\0') {
		fVar7 = (float)((uint)(int)(char)bVar3 >> 1 | (int)(char)bVar3 & 1U);
		fVar7 = fVar7 + fVar7;
	}
	else {
		fVar7 = (float)(int)(char)bVar3;
	}
	(this->field_0xb0).y = fVar7 * 2.0f;
	bVar3 = pByteCode->GetU8();
	if ((char)bVar3 < '\0') {
		fVar7 = (float)((uint)(int)(char)bVar3 >> 1 | (int)(char)bVar3 & 1U);
		fVar7 = fVar7 + fVar7;
	}
	else {
		fVar7 = (float)(int)(char)bVar3;
	}
	(this->field_0xb0).z = fVar7 * 2.0f;
	bVar3 = pByteCode->GetU8();
	if ((char)bVar3 < '\0') {
		fVar7 = (float)((uint)(int)(char)bVar3 >> 1 | (int)(char)bVar3 & 1U);
		fVar7 = fVar7 + fVar7;
	}
	else {
		fVar7 = (float)(int)(char)bVar3;
	}
	(this->field_0xb0).w = fVar7 * 2.0f;

	fVar7 = pByteCode->GetF32();
	(this->field_0xc0).x = fVar7;
	(this->field_0xc0).y = 0.0f;
	(this->field_0xc0).w = 0.0f;
	(this->field_0xc0).z = 0.0f;
	CScene::ptable.g_LightManager_004516b0->Reference(this, sectorId, true, false, iVar6);
	return;
}
