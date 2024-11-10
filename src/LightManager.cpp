#include "LightManager.h"
#include "Actor.h"
#include "MathOps.h"
#include "TimeController.h"
#include "MemoryStream.h"
#include "ActorCheckpointManager.h"

#include <string.h>
#include <math.h>
#include "CollisionManager.h"
#include "EventManager.h"

#define LIGHT_MANAGER_LOG(level, format, ...) MY_LOG_CATEGORY("LightManager", level, format, ##__VA_ARGS__)

edF32VECTOR4 gF32Vector4UnitY = { 0.0f, 1.0f, 0.0f, 0.0f };

CLightManager::CLightManager()
{
	uint count;
	float fVar1;
	float fVar2;
	float fVar3;
	CLight** ppCVar4;
	int* pBase;
	CLightConfig* pCVar5;
	undefined* puVar6;
	int iVar7;
	edF32MATRIX4* peVar8;
	edF32MATRIX4* peVar9;

	this->referencedLightsCount = 0;
	this->lightCount = 0;
	this->sectorLightCount = 0;
	this->activeLightCount = 0;
	
	this->nextFreeLightConfig = 0;
	this->field_0xe4 = 0;
	this->bSectorListDirty = 0;
	this->bActiveListDirty = 0;
	this->field_0x104 = 0;
	this->field_0x10c = 0;
	this->field_0x108 = 0;
	this->field_0x110 = 0;

	(this->vector_0xf0) = gF32Vector4UnitY;

	this->totalLightCount = 0x100;
	this->aLights = new CLight*[this->totalLightCount * 3];
	this->aSectorLights = this->aLights + this->totalLightCount;
	this->aActiveLights = this->aSectorLights + this->totalLightCount;

	memset(this->aLights, 0, this->totalLightCount * sizeof(CLight*));
	memset(this->aActiveLights, 0, this->totalLightCount * sizeof(CLight*));
	memset(this->aSectorLights, 0, this->totalLightCount * sizeof(CLight*));

	this->totalLightConfigs = 0x180;
	count = this->totalLightConfigs;
	this->aLightConfigs = new CLightConfig[count];

	this->field_0xe8 = 0x40;
	IMPLEMENTATION_GUARD_LOG(
	puVar6 = (undefined*)operator.new.array((long)(this->field_0xe8 * 0xc));
	this->field_0xe0 = puVar6;)

	this->lightAmbient = gF32Vector4Zero;
	this->lightDirections = gF32Matrix4Unit;
	this->lightColorMatrix = gF32Matrix4Unit;

	(this->lightConfig).pLightAmbient = &this->lightAmbient;
	(this->lightConfig).pLightDirections = &this->lightDirections;
	(this->lightConfig).pLightColorMatrix = &this->lightColorMatrix;
	return;
}

void CLightManager::Level_AddAll(ByteCode* pByteCode)
{
	int lightCount;
	char* pSeekPos;
	int* pOffsets;
	int index;

	lightCount = pByteCode->GetS32();

	LIGHT_MANAGER_LOG(LogLevel::Info, "CLightManager::Level_AddAll Creating {0} (0x{0:x}) lights", lightCount);

	pSeekPos = pByteCode->GetPosition();
	index = 0;
	pOffsets = (int*)pSeekPos;
	if (0 < lightCount) {
		do {
			pByteCode->SetPosition(pSeekPos + *pOffsets);
			AddLight(pByteCode, (CLight**)0x0);
			index = index + 1;
			pOffsets = pOffsets + 1;
		} while (index < lightCount);
	}
	return;
}

void CLightManager::Level_Manage()
{
	edF32VECTOR4* peVar1;
	float fVar2;
	Timer* pTVar3;
	edF32MATRIX4* peVar4;
	int iVar5;
	edF32MATRIX4* peVar6;
	float(*pafVar7)[4];
	float fVar8;
	float fVar9;

	*(this->lightConfig).pLightAmbient = gF32Vector4Zero;

	this->nextFreeLightConfig = 0;

	*(this->lightConfig).pLightColorMatrix = gF32Matrix4Zero;
	*(this->lightConfig).pLightDirections = gF32Matrix4Zero;

	this->field_0x104 = 0;
	this->field_0x10c = 0;
	this->field_0x110 = 0;
	this->field_0x108 = 0;

	if (this->bSectorListDirty != 0) {
		BuildSectorList();
	}

	if (this->bActiveListDirty != 0) {
		BuildActiveList();
	}

	for (iVar5 = 0; iVar5 < this->activeLightCount; iVar5 = iVar5 + 1) {
		this->aActiveLights[iVar5]->Manage();
	}

	CLightConfig::Validate(&this->lightConfig, true);

	if (((this->activeLightCount == 0) && (fVar8 < 1e-06f)) &&
		(peVar1 = (this->lightConfig).pLightAmbient,
			peVar1->x * peVar1->x + peVar1->y * peVar1->y + peVar1->z * peVar1->z < 1e-06f)) {
		IMPLEMENTATION_GUARD();
		pTVar3 = Timer::GetTimer();
		fVar9 = pTVar3->totalTime;
		peVar4 = (this->lightConfig).pLightDirections;
		iVar5 = 8;
		peVar6 = &gF32Matrix4Unit;
		do {
			iVar5 = iVar5 + -1;
			fVar2 = peVar6->ab;
			peVar4->aa = peVar6->aa;
			peVar6 = (edF32MATRIX4*)&peVar6->ac;
			peVar4->ab = fVar2;
			peVar4 = (edF32MATRIX4*)&peVar4->ac;
		} while (0 < iVar5);
		fVar9 = fmodf(fVar9, 0.5f);
		fVar9 = (fVar9 / 0.5f) * (fVar9 / 0.5f) * 255.0f;
		((this->lightConfig).pLightColorMatrix)->aa = fVar9;
		((this->lightConfig).pLightColorMatrix)->bb = 255.0f - fVar9;
		((this->lightConfig).pLightColorMatrix)->ba = 255.0f - fVar9;
		((this->lightConfig).pLightColorMatrix)->cc = fVar9;
	}

	fVar9 = CLightConfig::ComputeShadow(&this->lightConfig, &this->vector_0xf0);
	this->shadowValue = fVar9;
	return;
}

void CLightManager::Level_SectorChange(int oldSectorId, int newSectorId)
{
	LIGHT_MANAGER_LOG(LogLevel::Info, "CLightManager::Level_SectorChange Going from {} to {}", oldSectorId, newSectorId);

	if (newSectorId != -1) {
		this->sectorId = newSectorId;
		this->bSectorListDirty = 1;
		this->bActiveListDirty = 1;
	}
	return;
}

void CLightManager::ComputeLighting(float multiplier, CActor* pActor, uint flags, ed_3D_Light_Config* pConfig)
{
	edColPRIM_OBJECT* pObbPrim;
	float fVar2;
	edF32VECTOR4 location;
	CCollision* pCollision;

	location = pActor->currentLocation;

	pCollision = pActor->pCollisionData;
	if ((pCollision != (CCollision*)0x0) && (pObbPrim = pCollision->pObbPrim, pObbPrim != 0)) {
		location.y = location.y + pObbPrim->field_0xb0.y;
	}

	if (((flags & 4) != 0) && (((pActor->flags & 0x1000000) == 0 || (4.0f <= pActor->distanceToGround)))) {
		flags = flags & 0xfffffffb;
	}

	LIGHT_MANAGER_LOG(LogLevel::VeryVerbose, "CLightManager::ComputeLighting {} flags: {:x} location: {}", pActor->name, flags, location.ToString());

	ComputeLighting(multiplier, (ed_3d_hierarchy*)pActor->p3DHierNode, &location, flags, pConfig, (uint)pActor->field_0xf4);

	if (pActor->typeID == ACTOR_HERO_PRIVATE) {
		fVar2 = CLightConfig::ComputeShadow((((pActor->p3DHierNode)->base).pHierarchySetup)->pLightData, &this->vector_0xf0);
		this->shadowValue = fVar2;
	}
	return;
}

void CLightManager::ComputeLighting(float multiplier, ed_3d_hierarchy* pHier, edF32VECTOR4* pLocation, uint flags, ed_3D_Light_Config* pConfig, uint param_7)
{
	edF32MATRIX4* peVar6;
	edF32MATRIX4* peVar7;
	ed_3D_Light_Config* pMixLightingA;
	ed_3D_Light_Config* pFinalConfig;
	ed_3D_Light_Config* pMixLightingB;
	edF32VECTOR4 defaultAmbient;
	edF32MATRIX4 defaultDirections;
	edF32MATRIX4 defaultColors;
	ed_3D_Light_Config lightConfig;

	if (((((GameFlags & 0x20) == 0) && ((flags & 8) == 0)) &&
		(pHier->pHierarchySetup->pLightData = &this->lightConfig, flags != 0)) && (this->activeLightCount != 0)) {
		if (pConfig == (ed_3D_Light_Config*)0x0) {
			const int configIndex = this->nextFreeLightConfig;
			flags = flags & 0xfffffffe;
	
			this->nextFreeLightConfig = configIndex + 1;
			CLightConfig* pFullLightConfig = this->aLightConfigs + configIndex;

			pFullLightConfig->lightAmbient = gF32Vector4Zero;
			pFullLightConfig->lightDirections = gF32Matrix4Unit;
			pFullLightConfig->lightColorMatrix = gF32Matrix4Unit;

			pFullLightConfig->config.pLightAmbient = &pFullLightConfig->lightAmbient;
			pFullLightConfig->config.pLightDirections = &pFullLightConfig->lightDirections;
			pFullLightConfig->config.pLightColorMatrix = &pFullLightConfig->lightColorMatrix;

			LIGHT_MANAGER_LOG(LogLevel::VeryVerbose, "CLightManager::ComputeLighting using config: {} flags: {:x}", configIndex, flags);

			pConfig = &pFullLightConfig->config;
		}

		pMixLightingA = pConfig;

		if ((flags & 1) != 0) {
			defaultAmbient = gF32Vector4Zero;
			defaultDirections = gF32Matrix4Unit;
			defaultColors = gF32Matrix4Unit;
		
			lightConfig.pLightAmbient = &defaultAmbient;
			lightConfig.pLightDirections = &defaultDirections;
			lightConfig.pLightColorMatrix = &defaultColors;

			pMixLightingA = &lightConfig;
			pMixLightingB = pConfig;

			LIGHT_MANAGER_LOG(LogLevel::VeryVerbose, "CLightManager::ComputeLighting A = default, B = config");
		}

		if ((flags & 2) == 0) {
			*pMixLightingA->pLightAmbient = *(this->lightConfig).pLightAmbient;
			*pMixLightingA->pLightColorMatrix = *(this->lightConfig).pLightColorMatrix;
			*pMixLightingA->pLightDirections = *(this->lightConfig).pLightDirections;

			LIGHT_MANAGER_LOG(LogLevel::VeryVerbose, "CLightManager::ComputeLighting using LightManager config (A)");
		}
		else {
			LIGHT_MANAGER_LOG(LogLevel::VeryVerbose, "CLightManager::ComputeLighting computing config (A)");
			ComputeLighting(pLocation, pMixLightingA);
		}

		static edF32VECTOR4 edF32VECTOR4_0040f130 = { 8.3007815E-4f, 0.0027945312f, 2.8164062E-4f, 0.0f };
		static edF32VECTOR4 edF32VECTOR4_0040f140 = { 0.01328125f, 0.0447125f, 0.00450625f, 0.0f };

		if ((flags & 4) != 0) {
			const float flagModifier = sqrtf(
				(float)(param_7 >> 8 & 0xf) * edF32VECTOR4_0040f140.z +
				(float)(param_7 & 0xf) * edF32VECTOR4_0040f140.x +
				(float)(param_7 >> 4 & 0xf) * edF32VECTOR4_0040f140.y);

			LIGHT_MANAGER_LOG(LogLevel::VeryVerbose, "CLightManager::ComputeLighting applying flagModifier (A): {}", flagModifier);

			peVar6 = pMixLightingA->pLightColorMatrix;

			peVar6->rowX = peVar6->rowX * flagModifier;
			peVar6->rowY = peVar6->rowY * flagModifier;
			peVar6->rowZ = peVar6->rowZ * flagModifier;
			peVar6->rowT = peVar6->rowT * flagModifier;

			*pMixLightingA->pLightAmbient = (*pMixLightingA->pLightAmbient) * flagModifier;
		}

		if (multiplier != 1.0f) {
			LIGHT_MANAGER_LOG(LogLevel::VeryVerbose, "CLightManager::ComputeLighting applying multiplier (A): {}", multiplier);

			*pMixLightingA->pLightAmbient = (*pMixLightingA->pLightAmbient) * multiplier;
			*pMixLightingA->pLightColorMatrix = (*pMixLightingA->pLightColorMatrix) * multiplier;
		}

		pFinalConfig = pMixLightingA;

		if ((flags & 1) != 0) {
			edF32VECTOR4* pAmbientB = pMixLightingB->pLightAmbient;
			edF32VECTOR4* pAmbientA = pMixLightingA->pLightAmbient;

			float alpha = (pAmbientA->x * edF32VECTOR4_0040f130.x + pAmbientA->y * edF32VECTOR4_0040f130.y + pAmbientA->z * edF32VECTOR4_0040f130.z)
				- (pAmbientB->x * edF32VECTOR4_0040f130.x + pAmbientB->y * edF32VECTOR4_0040f130.y + pAmbientB->z * edF32VECTOR4_0040f130.z);

			if (alpha < 0.0f) {
				alpha = -alpha;
			}

			LIGHT_MANAGER_LOG(LogLevel::VeryVerbose, "CLightManager::ComputeLighting mixing A and B alpha: {}", alpha);

			if (0.1f < alpha) {
				edF32Vector4LERPHard(0.1f / alpha, pMixLightingB->pLightAmbient, pMixLightingB->pLightAmbient, pMixLightingA->pLightAmbient);
			}
			else {
				*pMixLightingB->pLightAmbient = *pMixLightingA->pLightAmbient;
			}

			for (int iVar4 = 0; pFinalConfig = pMixLightingB, iVar4 < 4; iVar4 = iVar4 + 1) {
				edF32VECTOR4* pDirectionB = pMixLightingB->pLightDirections->vector + iVar4;
				edF32Vector4LERPHard(0.5f, pDirectionB, pDirectionB, pMixLightingA->pLightDirections->vector + iVar4);

				edF32VECTOR4* pColorB = pMixLightingB->pLightColorMatrix->vector + iVar4;
				edF32Vector4LERPHard(0.5f, pColorB, pColorB, pMixLightingA->pLightColorMatrix->vector + iVar4);
			}
		}

		CLightConfig::Validate(pFinalConfig, true);

		// log out final config
		LIGHT_MANAGER_LOG(LogLevel::VeryVerbose, "CLightManager::ComputeLighting final config ambient: {}", pFinalConfig->pLightAmbient->ToString());
		LIGHT_MANAGER_LOG(LogLevel::VeryVerbose, "CLightManager::ComputeLighting final config color:   {}", pFinalConfig->pLightColorMatrix->ToString());
		LIGHT_MANAGER_LOG(LogLevel::VeryVerbose, "CLightManager::ComputeLighting final config dir:    {}", pFinalConfig->pLightDirections->ToString());

		pHier->pHierarchySetup->pLightData = pFinalConfig;

		if ((flags & 0x10) != 0) {
			*pFinalConfig->pLightAmbient = gF32Vector4Zero;
			*pFinalConfig->pLightColorMatrix = gF32Matrix4Zero;
			*pFinalConfig->pLightDirections = gF32Matrix4Zero;
		}
	}

	return;
}

int _test_zone_id$10480 = 0;

void CLightManager::ComputeLighting(edF32VECTOR4* pLocation, ed_3D_Light_Config* pConfig)
{
	edF32VECTOR4* peVar1;
	edF32VECTOR4* peVar2;
	CLight* pLight;
	bool bValidZones;
	int zoneCount;
	long long lVar6;
	int iVar7;
	int iVar8;
	edF32MATRIX4* peVar9;
	edF32MATRIX4* peVar10;
	CLightConfig* pCVar11;
	CLight** ppActiveLights;
	uint lightIndex;
	float fVar14;
	float fVar15;
	float fVar16;
	LightingContext lightingContext;

	LIGHT_MANAGER_LOG(LogLevel::VeryVerbose, "CLightManager::ComputeLighting location: {}", pLocation->ToString());
	LIGHT_MANAGER_LOG(LogLevel::VeryVerbose, "CLightManager::ComputeLighting ambient:  {}", (this->lightConfig).pLightAmbient->ToString());

	if (pConfig == (ed_3D_Light_Config*)0x0) {
		IMPLEMENTATION_GUARD(
		iVar8 = this->nextFreeLightConfig;
		peVar10 = &gF32Matrix4Unit;
		iVar7 = 8;
		this->nextFreeLightConfig = iVar8 + 1;
		fVar16 = gF32Vector4Zero.w;
		fVar15 = gF32Vector4Zero.z;
		fVar14 = gF32Vector4Zero.y;
		pCVar11 = this->aLightConfigs + iVar8;
		peVar9 = &pCVar11->field_0x10;
		(pCVar11->field_0x0).x = gF32Vector4Zero.x;
		(pCVar11->field_0x0).y = fVar14;
		(pCVar11->field_0x0).z = fVar15;
		(pCVar11->field_0x0).w = fVar16;
		do {
			iVar7 = iVar7 + -1;
			fVar14 = peVar10->ab;
			peVar9->aa = peVar10->aa;
			peVar10 = (edF32MATRIX4*)&peVar10->ac;
			peVar9->ab = fVar14;
			peVar9 = (edF32MATRIX4*)&peVar9->ac;
		} while (0 < iVar7);
		peVar9 = &pCVar11->field_0x50;
		peVar10 = &gF32Matrix4Unit;
		iVar8 = 8;
		do {
			iVar8 = iVar8 + -1;
			fVar14 = peVar10->ab;
			peVar9->aa = peVar10->aa;
			peVar10 = (edF32MATRIX4*)&peVar10->ac;
			peVar9->ab = fVar14;
			peVar9 = (edF32MATRIX4*)&peVar9->ac;
		} while (0 < iVar8);
		(pCVar11->field_0x90).pLightAmbient = (edF32VECTOR4*)pCVar11;
		(pCVar11->field_0x90).pLightDirections = &pCVar11->field_0x10;
		pConfig = &pCVar11->field_0x90;
		(pCVar11->field_0x90).pLightColorMatrix = &pCVar11->field_0x50;)
	}

	*pConfig->pLightAmbient = *(this->lightConfig).pLightAmbient;

	pConfig->pLightColorMatrix->rowX = gF32Vector4Zero;
	pConfig->pLightColorMatrix->rowY = gF32Vector4Zero;
	pConfig->pLightColorMatrix->rowZ = gF32Vector4Zero;
	pConfig->pLightColorMatrix->rowT = gF32Vector4Zero;

	pConfig->pLightDirections->rowX = gF32Vector4Zero;
	pConfig->pLightDirections->rowY = gF32Vector4Zero;
	pConfig->pLightDirections->rowZ = gF32Vector4Zero;
	pConfig->pLightDirections->rowT = gF32Vector4Zero;
	
	lightingContext.position.x = pLocation->x;
	lightingContext.position.y = pLocation->y;
	lightingContext.position.z = pLocation->z;
	lightingContext.position.w = pLocation->w;

	lightingContext.pLightAmbient = pConfig->pLightAmbient;
	lightingContext.field_0x1c = 0;

	LIGHT_MANAGER_LOG(LogLevel::VeryVerbose, "CLightManager::ComputeLighting active lights: {}", this->activeLightCount);

	ppActiveLights = this->aActiveLights;
	lightIndex = 0;
	if (0 < this->activeLightCount) {
		do {
			pLight = *ppActiveLights;

			LIGHT_MANAGER_LOG(LogLevel::VeryVerbose, "CLightManager::ComputeLighting light: {} flags: {:x}", lightIndex, pLight->colour_0x4.rgba);

			if ((pLight->colour_0x4.rgba & 0x8000000) != 0) {
				if (pLight->pZoneHolder == (ZoneHolder*)0x0) {
					zoneCount = 0;
				}
				else {
					zoneCount = pLight->pZoneHolder->zone;
				}
				bValidZones = zoneCount != 0;

				if (!bValidZones) {
					if ((int*)pLight->field_0x10 == (int*)0x0) {
						iVar8 = 0;
					}
					else {
						iVar8 = *(int*)pLight->field_0x10;
					}
					bValidZones = iVar8 != 0;
				}

				LIGHT_MANAGER_LOG(LogLevel::VeryVerbose, "CLightManager::ComputeLighting light: {} valid zones: {} ({} / {})", lightIndex, bValidZones, zoneCount, iVar8);

				bool bIlluminationZoneTestPass;

				if ((!bValidZones) ||
					(bIlluminationZoneTestPass = pLight->TestIlluminationZones(pLocation, _test_zone_id$10480), bIlluminationZoneTestPass != 0)) {
		
					lVar6 = (long long)(char)(pLight->colour_0x4).b << 0x38;
					iVar8 = (int)(lVar6 >> 0x3c);

					LIGHT_MANAGER_LOG(LogLevel::VeryVerbose, "CLightManager::ComputeLighting light: {} test passed! light index: {}", lightIndex, iVar8);

					assert(iVar8 >= 0 && iVar8 < 4);

					if (iVar8 < 4) {
						lightingContext.pLightDirection = &pConfig->pLightDirections->vector[iVar8];
						lightingContext.pLightColor = &pConfig->pLightColorMatrix->vector[iVar8];
					}

					pLight->DoLighting(&lightingContext);

					LIGHT_MANAGER_LOG(LogLevel::VeryVerbose, "CLightManager::ComputeLighting result direction: {}", lightingContext.pLightDirection->ToString());
					LIGHT_MANAGER_LOG(LogLevel::VeryVerbose, "CLightManager::ComputeLighting result color:     {}", lightingContext.pLightColor->ToString());
					LIGHT_MANAGER_LOG(LogLevel::VeryVerbose, "CLightManager::ComputeLighting result ambient:   {}", lightingContext.pLightAmbient->ToString());
				}
			}
			lightIndex = lightIndex + 1 & 0xff;
			ppActiveLights = ppActiveLights + 1;
		} while ((int)lightIndex < this->activeLightCount);
	}
	_test_zone_id$10480 = _test_zone_id$10480 + 1;
	return;
}

struct SimplestColorModel {
	edF32VECTOR4 ambientColor;
};

struct SimpleColorModel : public SimplestColorModel {
	edF32VECTOR4 color;
};

struct FullColorModel : public SimpleColorModel {
	edF32VECTOR4 field_0x20;
	void Read(ByteCode* pByteCode);
};

void FullColorModel::Read(ByteCode* pByteCode)
{
	byte bVar1;
	float fVar2;

	bVar1 = pByteCode->GetU8();
	if ((char)bVar1 < '\0') {
		fVar2 = (float)((uint)(int)(char)bVar1 >> 1 | (int)(char)bVar1 & 1U);
		fVar2 = fVar2 + fVar2;
	}
	else {
		fVar2 = (float)(int)(char)bVar1;
	}
	(this->ambientColor).x = fVar2 * 2.0f;
	bVar1 = pByteCode->GetU8();
	if ((char)bVar1 < '\0') {
		fVar2 = (float)((uint)(int)(char)bVar1 >> 1 | (int)(char)bVar1 & 1U);
		fVar2 = fVar2 + fVar2;
	}
	else {
		fVar2 = (float)(int)(char)bVar1;
	}
	(this->ambientColor).y = fVar2 * 2.0f;
	bVar1 = pByteCode->GetU8();
	if ((char)bVar1 < '\0') {
		fVar2 = (float)((uint)(int)(char)bVar1 >> 1 | (int)(char)bVar1 & 1U);
		fVar2 = fVar2 + fVar2;
	}
	else {
		fVar2 = (float)(int)(char)bVar1;
	}
	(this->ambientColor).z = fVar2 * 2.0f;
	bVar1 = pByteCode->GetU8();
	if ((char)bVar1 < '\0') {
		fVar2 = (float)((uint)(int)(char)bVar1 >> 1 | (int)(char)bVar1 & 1U);
		fVar2 = fVar2 + fVar2;
	}
	else {
		fVar2 = (float)(int)(char)bVar1;
	}
	(this->ambientColor).w = fVar2 * 2.0f;
	bVar1 = pByteCode->GetU8();
	if ((char)bVar1 < '\0') {
		fVar2 = (float)((uint)(int)(char)bVar1 >> 1 | (int)(char)bVar1 & 1U);
		fVar2 = fVar2 + fVar2;
	}
	else {
		fVar2 = (float)(int)(char)bVar1;
	}
	(this->color).x = fVar2 * 2.0f;
	bVar1 = pByteCode->GetU8();
	if ((char)bVar1 < '\0') {
		fVar2 = (float)((uint)(int)(char)bVar1 >> 1 | (int)(char)bVar1 & 1U);
		fVar2 = fVar2 + fVar2;
	}
	else {
		fVar2 = (float)(int)(char)bVar1;
	}
	(this->color).y = fVar2 * 2.0f;
	bVar1 = pByteCode->GetU8();
	if ((char)bVar1 < '\0') {
		fVar2 = (float)((uint)(int)(char)bVar1 >> 1 | (int)(char)bVar1 & 1U);
		fVar2 = fVar2 + fVar2;
	}
	else {
		fVar2 = (float)(int)(char)bVar1;
	}
	(this->color).z = fVar2 * 2.0f;
	bVar1 = pByteCode->GetU8();
	if ((char)bVar1 < '\0') {
		fVar2 = (float)((uint)(int)(char)bVar1 >> 1 | (int)(char)bVar1 & 1U);
		fVar2 = fVar2 + fVar2;
	}
	else {
		fVar2 = (float)(int)(char)bVar1;
	}
	(this->color).w = fVar2 * 2.0f;
	bVar1 = pByteCode->GetU8();
	if ((char)bVar1 < '\0') {
		fVar2 = (float)((uint)(int)(char)bVar1 >> 1 | (int)(char)bVar1 & 1U);
		fVar2 = fVar2 + fVar2;
	}
	else {
		fVar2 = (float)(int)(char)bVar1;
	}
	(this->field_0x20).x = fVar2 * 2.0f;
	bVar1 = pByteCode->GetU8();
	if ((char)bVar1 < '\0') {
		fVar2 = (float)((uint)(int)(char)bVar1 >> 1 | (int)(char)bVar1 & 1U);
		fVar2 = fVar2 + fVar2;
	}
	else {
		fVar2 = (float)(int)(char)bVar1;
	}
	(this->field_0x20).y = fVar2 * 2.0f;
	bVar1 = pByteCode->GetU8();
	if ((char)bVar1 < '\0') {
		fVar2 = (float)((uint)(int)(char)bVar1 >> 1 | (int)(char)bVar1 & 1U);
		fVar2 = fVar2 + fVar2;
	}
	else {
		fVar2 = (float)(int)(char)bVar1;
	}
	(this->field_0x20).z = fVar2 * 2.0f;
	bVar1 = pByteCode->GetU8();
	if ((char)bVar1 < '\0') {
		fVar2 = (float)((uint)(int)(char)bVar1 >> 1 | (int)(char)bVar1 & 1U);
		fVar2 = fVar2 + fVar2;
	}
	else {
		fVar2 = (float)(int)(char)bVar1;
	}
	(this->field_0x20).w = fVar2 * 2.0f;
	fVar2 = pByteCode->GetF32();
	edF32Vector4ScaleHard(fVar2, &this->ambientColor, &this->ambientColor);
	edF32Vector4ScaleHard(fVar2, &this->color, &this->color);
	edF32Vector4ScaleHard(fVar2, &this->field_0x20, &this->field_0x20);

	LIGHT_MANAGER_LOG(LogLevel::Info, "FullColorModel::Read");
	LIGHT_MANAGER_LOG(LogLevel::Info, "FullColorModel::Read color {}", this->ambientColor.ToString());
	LIGHT_MANAGER_LOG(LogLevel::Info, "FullColorModel::Read 0x10 {}", this->color.ToString());
	LIGHT_MANAGER_LOG(LogLevel::Info, "FullColorModel::Read 0x20 {}", this->field_0x20.ToString());

	return;
}

struct BaseShapeA
{
	edF32VECTOR4 position;
};

struct BaseShapeB : public BaseShapeA
{
	float fallout0;
	float fallout1;
	undefined field_0x18;
	undefined field_0x19;
	undefined field_0x1a;
	undefined field_0x1b;
	undefined field_0x1c;
	undefined field_0x1d;
	undefined field_0x1e;
	undefined field_0x1f;
};

struct BaseShapeC : public BaseShapeB 
{
	edF32VECTOR4 direction;
};

struct BaseShape : public BaseShapeC
{
	float fov0;
	float fov1;
	undefined field_0x58;
	undefined field_0x59;
	undefined field_0x5a;
	undefined field_0x5b;
	undefined field_0x5c;
	undefined field_0x5d;
	undefined field_0x5e;
	undefined field_0x5f;
};

struct SunShape : public BaseShape
{
	void Read(ByteCode* pByteCode);
};

struct LightShape : public BaseShape
{
	void Read(ByteCode* pByteCode);
};

struct SpotShape : public BaseShape
{
	void Read(ByteCode* pByteCode);
};

void SpotShape::Read(ByteCode* pByteCode)
{
	float fVar1;

	fVar1 = pByteCode->GetF32();
	this->position.x = fVar1;
	fVar1 = pByteCode->GetF32();
	this->position.y = fVar1;
	fVar1 = pByteCode->GetF32();
	this->position.z = fVar1;
	this->position.w = 1.0f;

	fVar1 = pByteCode->GetF32();
	this->fallout0 = fVar1;
	fVar1 = pByteCode->GetF32();
	this->fallout1 = fVar1;

	fVar1 = pByteCode->GetF32();
	this->direction.x = fVar1;
	fVar1 = pByteCode->GetF32();
	this->direction.y = fVar1;
	fVar1 = pByteCode->GetF32();
	this->direction.z = fVar1;
	this->direction.w = 1.0f;

	fVar1 = pByteCode->GetF32();
	this->fov0 = cosf(fVar1);

	fVar1 = pByteCode->GetF32();
	this->fov1 = cosf(fVar1);

	LIGHT_MANAGER_LOG(LogLevel::Info, "SpotShape::Read");
	LIGHT_MANAGER_LOG(LogLevel::Info, "SpotShape::Read position: {}", this->position.ToString());
	LIGHT_MANAGER_LOG(LogLevel::Info, "SpotShape::Read fallout0 {}", this->fallout0);
	LIGHT_MANAGER_LOG(LogLevel::Info, "SpotShape::Read fallout1 {}", this->fallout1);
	LIGHT_MANAGER_LOG(LogLevel::Info, "SpotShape::Read direction {}", this->direction.ToString());
	LIGHT_MANAGER_LOG(LogLevel::Info, "SpotShape::Read field_0x30 {}", this->fov0);
	LIGHT_MANAGER_LOG(LogLevel::Info, "SpotShape::Read field_0x34 {}", this->fov1);
}

void LightShape::Read(ByteCode* pByteCode)
{
	float fVar1;

	fVar1 = pByteCode->GetF32();
	this->position.x = fVar1;
	fVar1 = pByteCode->GetF32();
	this->position.y = fVar1;
	fVar1 = pByteCode->GetF32();
	this->position.z = fVar1;
	this->position.w = 1.0f;

	fVar1 = pByteCode->GetF32();
	this->fallout0 = fVar1;
	fVar1 = pByteCode->GetF32();
	this->fallout1 = fVar1;

	LIGHT_MANAGER_LOG(LogLevel::Info, "LightShape::Read");
	LIGHT_MANAGER_LOG(LogLevel::Info, "LightShape::Read position: {}", this->position.ToString());
	LIGHT_MANAGER_LOG(LogLevel::Info, "LightShape::Read fallout0 {}", this->fallout0);
	LIGHT_MANAGER_LOG(LogLevel::Info, "LightShape::Read fallout1 {}", this->fallout1);
}

void SunShape::Read(ByteCode* pByteCode)
{
	float fVar1;

	fVar1 = pByteCode->GetF32();
	this->position.x = fVar1;
	fVar1 = pByteCode->GetF32();
	this->position.y = fVar1;
	fVar1 = pByteCode->GetF32();
	this->position.z = fVar1;
	this->position.w = 1.0f;

	fVar1 = pByteCode->GetF32();
	this->fallout0 = fVar1;
	fVar1 = pByteCode->GetF32();
	this->fallout1 = fVar1;

	fVar1 = pByteCode->GetF32();
	this->direction.x = fVar1;
	fVar1 = pByteCode->GetF32();
	this->direction.y = fVar1;
	fVar1 = pByteCode->GetF32();
	this->direction.z = fVar1;
	this->direction.w = 1.0f;

	LIGHT_MANAGER_LOG(LogLevel::Info, "SunShape::Read");
	LIGHT_MANAGER_LOG(LogLevel::Info, "SunShape::Read position: {}", this->position.ToString());
	LIGHT_MANAGER_LOG(LogLevel::Info, "SunShape::Read fallout0 {}", this->fallout0);
	LIGHT_MANAGER_LOG(LogLevel::Info, "SunShape::Read fallout1 {}", this->fallout1);
	LIGHT_MANAGER_LOG(LogLevel::Info, "SunShape::Read direction {}", this->direction.ToString());
	return;
}

class CLightSun : public CLight 
{
public:
	CLightSun()
		: CLight()
	{
		this->colorModel.field_0x20 = gF32Vector4Zero;
		this->colorModel.color = this->colorModel.field_0x20;
		this->colorModel.ambientColor = this->colorModel.color;

		this->baseShape.position = gF32Vector4Zero;

		this->baseShape.fallout1 = 1.0f;
		this->baseShape.fallout0 = 1.0f;

		this->baseShape.direction = gF32Vector4Zero;
	}

	virtual void Manage();
	virtual bool DoLighting(LightingContext* pContext);
	virtual int GetBaseShape(BaseShape** ppBaseShape);

	BaseShapeC baseShape;
	FullColorModel colorModel;
};

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
		pfVar2->x = pfVar2->x + (this->baseShape).direction.w * -1.0;
		pfVar2->y = pfVar2->y + fVar3 * -1.0;
		pfVar2->z = pfVar2->z + fVar4 * -1.0;
		pfVar2->w = pfVar2->w + fVar5 * -1.0;
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

class CLightSpot : public CLight
{
public:
	CLightSpot()
		: CLight()
	{
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

	virtual void Manage() { return; }
	virtual int GetBaseShape(BaseShape** ppBaseShape);
	virtual edF32VECTOR4* GetPosition() { return &this->baseShape.position; }

	BaseShape baseShape;
	FullColorModel colorModel;
};

int CLightSpot::GetBaseShape(BaseShape** ppBaseShape)
{
	if (ppBaseShape != (BaseShape**)0x0) {
		*ppBaseShape = &this->baseShape;
	}
	return 4;
}

class CLightDirectional : public CLight
{
public:
	CLightDirectional()
		: CLight()
	{
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

	virtual void Manage() { return; }
	virtual int GetBaseShape(BaseShape** ppBaseShape);
	virtual edF32VECTOR4* GetPosition() { return &this->baseShape.position; }

	BaseShape baseShape;
	FullColorModel colorModel;
};

int CLightDirectional::GetBaseShape(BaseShape** ppBaseShape)
{
	if (ppBaseShape != (BaseShape**)0x0) {
		*ppBaseShape = &this->baseShape;
	}
	return 2;
}

class CLightAmbient : public CLight
{
public:
	CLightAmbient()
		: CLight()
	{
		this->colorModel.ambientColor = gF32Vector4Zero;
		this->baseShape.position = gF32Vertex4Zero;
	}

	virtual void Manage();
	virtual bool DoLighting(LightingContext* pContext);
	virtual edF32VECTOR4* GetPosition() { return &this->baseShape.position; }

	BaseShapeA baseShape;
	SimplestColorModel colorModel;
};

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

class CLightOmni : public CLight
{
public:
	CLightOmni()
	{}

	virtual void Manage() { return; }
	virtual int GetBaseShape(BaseShape** ppBaseShape);
	virtual edF32VECTOR4* GetPosition() { return &this->baseShape.position; }
	virtual void Create(ByteCode* pByteCode);

	BaseShapeB baseShape;
	FullColorModel colorModel;
};

int CLightOmni::GetBaseShape(BaseShape** ppBaseShape)
{
	if (ppBaseShape != (BaseShape**)0x0) {
		*ppBaseShape = (BaseShape*)&this->baseShape;
	}
	return 3;
}

void CLightOmni::Create(ByteCode* pByteCode)
{
	CLight();

	this->colorModel.field_0x20 = gF32Vector4Zero;
	this->colorModel.color = this->colorModel.field_0x20;
	this->colorModel.ambientColor = this->colorModel.color;

	this->baseShape.position = gF32Vector4Zero;

	this->baseShape.fallout1 = 1.0f;
	this->baseShape.fallout0 = 1.0f;
}

class CLightTorch : public CLight
{
public:
	CLightTorch()
		: CLight()
		, lightType(-1)
	{}

	virtual void Manage();
	virtual int GetBaseShape(BaseShape** ppBaseShape);
	virtual edF32VECTOR4* GetPosition() { return &this->baseShape.position; }
	virtual void Create(ByteCode* pByteCode);

	FullColorModel colorModel;
	int lightType;
	BaseShape baseShape;

	edF32VECTOR4 field_0xa0;
	edF32VECTOR4 field_0xb0;
	edF32VECTOR4 field_0xc0;
};

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
	edF32Vector4LERPHard(fVar5 + puVar6, &(this->colorModel).color, &this->field_0xa0, &this->field_0xb0);
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

	this->pZoneHolder = reinterpret_cast<ZoneHolder*>(pSVar1);

	piVar2 = (int*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = pByteCode->currentSeekPos + 4;
	if (*piVar2 != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + *piVar2 * 4;
	}

	this->field_0x10 = (undefined*)piVar2;

	(this->field_0xa0) = this->colorModel.color;
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

void CLightManager::AddLight(ByteCode* pByteCode, CLight** ppOutLight)
{
	uint lightType;
	CLight* pLight;
	//Light_Torch* ppLVar3;

	lightType = pByteCode->GetU32();

	LIGHT_MANAGER_LOG(LogLevel::Info, "CLightManager::AddLight Creating light of type: {}", lightType);

	if (lightType == 2) {
		pLight = new CLightTorch;
		if (pLight != (CLight*)0x0) {
			CLightTorch* pLightTorch = reinterpret_cast<CLightTorch*>(pLight);
			pLightTorch->Create(pByteCode);
		}
	}
	else {
		if ((lightType != 1) && (lightType != 0)) {
			return;
		}
		pLight = CreateSimpleLight(pByteCode);
	}

	(pLight->colour_0x4).a = (pLight->colour_0x4).a | 0x10;

	LIGHT_MANAGER_LOG(LogLevel::VeryVerbose, "CLightManager::AddLight Set flags light: {} flags: {:x}", pLight->referencedLightIndex, pLight->colour_0x4.rgba);

	pLight->Activate();
	if (ppOutLight != (CLight**)0x0) {
		*ppOutLight = pLight;
	}
	return;
}

CLight* CLightManager::CreateSimpleLight(ByteCode* pByteCode)
{
	int* piVar1;
	float fVar2;
	float fVar3;
	float fVar4;
	int lightType;
	int sectorId;
	int iVar6;
	uint uVar7;
	//Light_Ambient* pNewAmbient;
	//Light_Sun* pNewSun;
	//Light_Omni* pNewOmni;
	//Light_Directional* pNewDirectional;
	//Light_Spot* pNewSpot;
	CLight* pNewLight;
	ushort uVar8;
	_rgba _Var10;
	//LightVTable* pLVar11;
	_rgba _Var12;
	ZoneHolder* pSVar13;
	FullColorModel fullColorModel;
	BaseShape baseShape;

	lightType = pByteCode->GetS32();

	LIGHT_MANAGER_LOG(LogLevel::Info, "CLightManager::CreateSimpleLight Creating light of type: {}", lightType);

	fullColorModel.Read(pByteCode);
	if (lightType == 4) {
		reinterpret_cast<SpotShape*>(&baseShape)->Read(pByteCode);
	}
	else {
		if (lightType == 1) {
			reinterpret_cast<SunShape*>(&baseShape)->Read(pByteCode);
		}
		else {
			if (lightType == 2) {
				IMPLEMENTATION_GUARD(
				DirectionalShape::Read(&baseShape, pByteCode);)
			}
			else {
				if (lightType == 3) {
					reinterpret_cast<LightShape*>(&baseShape)->Read(pByteCode);
				}
				else {
					if (lightType == -1) {
						reinterpret_cast<LightShape*>(&baseShape)->Read(pByteCode);
					}
				}
			}
		}
	}
	sectorId = pByteCode->GetS32();
	iVar6 = pByteCode->GetS32();
	uVar7 = pByteCode->GetU32();

	LIGHT_MANAGER_LOG(LogLevel::Info, "CLightManager::CreateSimpleLight Extra params: sector Id: {} {} {}", sectorId, iVar6, uVar7);

	uVar8 = (ushort)uVar7;
	if (((1e-06f <= fabs(fullColorModel.color.x)) || (1e-06f <= fabs(fullColorModel.color.y))) ||
		(1e-06f <= fabs(fullColorModel.color.z))) {
		if (lightType == 4) {
			uVar8 = uVar8 | 1;
			pNewLight = new CLightSpot;
			if (pNewLight != (CLight*)0x0) {
				CLightSpot* pLightSpot = reinterpret_cast<CLightSpot*>(pNewLight);

				pLightSpot->baseShape = baseShape;
				pLightSpot->colorModel = fullColorModel;
			}
		}
		else {
			if (lightType == 2) {
				uVar8 = uVar8 | 1;
				pNewLight = new CLightDirectional;
				if (pNewLight != (CLight*)0x0) {
					CLightDirectional* pLightDirectional = reinterpret_cast<CLightDirectional*>(pNewLight);

					pLightDirectional->baseShape = baseShape;
					pLightDirectional->colorModel = fullColorModel;
				}
			}
			else {
				if (lightType == 3) {
					uVar8 = uVar8 | 1;
					pNewLight = new CLightOmni;
					if (pNewLight != (CLight*)0x0) {
						CLightOmni* pLightOmni = reinterpret_cast<CLightOmni*>(pNewLight);

						pLightOmni->Create(pByteCode);

						pLightOmni->baseShape.position = baseShape.position;

						pLightOmni->baseShape.fallout0 = baseShape.fallout0;
						pLightOmni->baseShape.fallout1 = baseShape.fallout1;

						pLightOmni->colorModel = fullColorModel;
					}
				}
				else {
					if ((lightType == 1) && (pNewLight = new CLightSun, pNewLight != (CLight*)0x0)) {
						CLightSun* pLightSun = reinterpret_cast<CLightSun*>(pNewLight);

						pLightSun->baseShape.position = baseShape.position;

						pLightSun->baseShape.fallout0 = baseShape.fallout0;
						pLightSun->baseShape.fallout1 = baseShape.fallout1;

						pLightSun->baseShape.direction = baseShape.direction;

						pLightSun->colorModel = fullColorModel;
					}
				}
			}
		}
	}
	else {
		pNewLight = new CLightAmbient;
		if (pNewLight != (CLight*)0x0) {
			CLightAmbient* pLightAmbient = reinterpret_cast<CLightAmbient*>(pNewLight);

			pLightAmbient->colorModel.ambientColor = fullColorModel.ambientColor;
			pLightAmbient->baseShape.position = baseShape.position;
		}
		iVar6 = -3;
	}

	pNewLight->field_0x8 = uVar8;

	pSVar13 = (ZoneHolder*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = pByteCode->currentSeekPos + 4;

	const int checkpointDataSize = *reinterpret_cast<int*>(pSVar13);

	if (checkpointDataSize != 0x0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + (checkpointDataSize * 4);
	}

	pNewLight->pZoneHolder = pSVar13;
	piVar1 = (int*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)(piVar1 + 1);
	if (*piVar1 != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + *piVar1 * 4;
	}
	pNewLight->field_0x10 = (undefined*)piVar1;

	Reference(pNewLight, sectorId, true, true, iVar6);
	return pNewLight;
}

void CLightManager::Activate(CLight* pLight, int param_3)
{
	long long lVar1;
	_rgba color;
	_rgba local_4;

	lVar1 = param_3;
	color = pLight->colour_0x4;
	if (lVar1 == -2) {
		local_4.b = color.b;
		lVar1 = ((long long)(char)local_4.b << 0x3c) >> 0x3c;
	}
	if (lVar1 == -3) {
		local_4.rgba = color.rgba & 0xf7ffffff;
	}
	else {
		if (lVar1 == -1) {
			local_4.rgba = (color.rgba | 0x28000000);
		}
		else {
			local_4.rgba = (color.rgba & 0xdfffffff | 0x8000000);
		}
	}

	long long shifted = (long long)lVar1 << 0x38;
	shifted = shifted >> 0x38;
	byte a = (byte)((shifted & 0xf) << 4);

	local_4.a = local_4.a | 0x80;
	local_4.b = (0xffffff0f & local_4.b) | a;

	pLight->colour_0x4 = local_4;

	LIGHT_MANAGER_LOG(LogLevel::VeryVerbose, "CLightManager::Activate Set flags light: {} flags: {:x}", pLight->referencedLightIndex, pLight->colour_0x4.rgba);

	this->bActiveListDirty = 1;
	return;
}

void CLightManager::Reference(CLight* pLight, int sectorId, bool param_4, bool param_5, int param_6)
{
	int iVar1;
	CLight* pCVar2;
	_rgba uVar3;
	uint uVar4;
	CLight** ppLights;
	byte bVar6;
	int iVar7;
	CLight** pCVar8;
	_rgba local_4;

	pLight->referencedLightIndex = (short)this->referencedLightsCount;
	bVar6 = 1;
	if (param_4 != false) {
		bVar6 = 0x41;
	}
	if (param_5 != false) {
		bVar6 = bVar6 | 0x10;
	}
	local_4.rgba = (uint)bVar6 << 0x18;
	uVar4 = local_4.rgba;

	uVar3.r = sectorId;
	uVar3.g = 0;
	uVar3.b = param_6;
	uVar3.a = 0;

	local_4.rgba = (local_4.rgba | uVar3.rgba & 0xff0fffff);
	local_4.rgba = (local_4.rgba | 0x700000);
	if (param_6 < 0) {
		local_4.rgba = uVar4 | uVar3.rgba & 0xfffff | 0x700000 | 0x20000000;
	}

	iVar7 = 1;
	pLight->colour_0x4 = local_4;

	LIGHT_MANAGER_LOG(LogLevel::VeryVerbose, "CLightManager::Reference Set flags light: {} flags: {:x}", pLight->referencedLightIndex, pLight->colour_0x4.rgba);

	iVar1 = this->referencedLightsCount;
	ppLights = this->aLights;
	ppLights[iVar1] = pLight;
	pCVar8 = ppLights + iVar1 + 1;
	this->lightCount = this->lightCount + 1;
	iVar1 = this->totalLightCount;
	pCVar2 = (CLight*)this->aLights;

	if (1 < iVar1) {
		do {
			if (*pCVar8 == (CLight*)0x0) break;
			pCVar8 = pCVar8 + 1;
			iVar7 = iVar7 + 1;
			IMPLEMENTATION_GUARD(
			if ((CLight**)((int)&pCVar2->pVTable + iVar1 * 4) <= pCVar8) {
				pCVar8 = pCVar2;
			})
		} while (iVar7 < iVar1);
	}

	int diff = (char*)pCVar8 - (char*)pCVar2;
	if (diff < 0) {
		diff = diff + 3;
	}

	this->referencedLightsCount = diff / sizeof(CLight*);
	this->bSectorListDirty = 1;
	this->bActiveListDirty = 1;
	return;
}

void CLightManager::BuildSectorList()
{
	CLight* pLight;
	CLight** ppSectorLights;
	CLight** ppLights;
	int lightIndex;
	char local_4;

	LIGHT_MANAGER_LOG(LogLevel::VeryVerbose, "CLightManager::BuildSectorList Checking {} lights", this->lightCount);

	ppLights = this->aLights;
	ppSectorLights = this->aSectorLights;
	this->sectorLightCount = 0;
	lightIndex = 0;
	if (0 < this->lightCount) {
		do {
			pLight = *ppLights;
			if (pLight != (CLight*)0x0) {

				LIGHT_MANAGER_LOG(LogLevel::VeryVerbose, "CLightManager::BuildSectorList Light {} a: {:x}", lightIndex, pLight->colour_0x4.a);

				if ((pLight->colour_0x4.a & 0x1) != 0) {
					local_4 = pLight->colour_0x4.r;

					LIGHT_MANAGER_LOG(LogLevel::VeryVerbose, "CLightManager::BuildSectorList Light {} is active, sector id: {}", lightIndex, pLight->colour_0x4.r);

					if (((long)local_4 == -1) || ((long)local_4 == (long)this->sectorId)) {
						LIGHT_MANAGER_LOG(LogLevel::VeryVerbose, "CLightManager::BuildSectorList Light {} is inside active sector ({})", lightIndex, this->sectorId);
						*ppSectorLights = pLight;
						ppSectorLights = ppSectorLights + 1;
						this->sectorLightCount = this->sectorLightCount + 1;
					}
				}
				lightIndex = lightIndex + 1;
			}
			ppLights = ppLights + 1;
		} while (lightIndex < this->lightCount);
	}
	this->bSectorListDirty = 0;
	this->bActiveListDirty = 1;
	return;
}

void CLightManager::BuildActiveList()
{
	CLight* pLightA;
	_rgba _Var2;
	bool bVar3;
	long long lVar4;
	ulong uVar5;
	CLight** ppSectorLights;
	CLight** ppActiveLights;
	int iVar7;
	int iVar8;
	int iVar9;
	long long lVar10;
	int iVar11;
	float fVar12;
	float local_20[6];
	_rgba local_8;
	_rgba local_4;

	bVar3 = false;
	iVar7 = 0;
	this->activeLightCount = 0;
	ppSectorLights = this->aSectorLights;
	ppActiveLights = this->aActiveLights;
	if (0 < this->sectorLightCount) {
		do {
			pLightA = *ppSectorLights;
			_Var2 = pLightA->colour_0x4;
			local_4.rgba = _Var2.rgba & 0xfdffffff;
			if ((((_Var2.rgba & 0x1000000) != 0) && ((_Var2.rgba & 0x80000000) != 0)) &&
				(((_Var2.rgba & 0x4000000) == 0 || ('\0' < local_4.g)))) {
				if ((_Var2.rgba & 0x20000000) != 0) {
					bVar3 = true;
					local_4.rgba = _Var2.rgba & 0xfd0fffff | 0xf00000;
				}
				local_4.rgba = local_4.rgba | 0x2000000;
				*ppActiveLights = pLightA;
				ppActiveLights = ppActiveLights + 1;
				this->activeLightCount = this->activeLightCount + 1;
			}
			iVar7 = iVar7 + 1;
			pLightA->colour_0x4 = local_4;

			LIGHT_MANAGER_LOG(LogLevel::VeryVerbose, "CLightManager::BuildActiveList A Set flags light: {} flags: {:x}", pLightA->referencedLightIndex, pLightA->colour_0x4.rgba);

			ppSectorLights = ppSectorLights + 1;
		} while (iVar7 < this->sectorLightCount);
	}

	if (bVar3) {
		iVar7 = this->activeLightCount;
		ppSectorLights = this->aActiveLights;
		iVar11 = 0;
		if (0 < iVar7) {
			do {
				local_8 = (*ppSectorLights)->colour_0x4;
				if ((((local_8.rgba & 0x8000000) != 0) &&
					(local_8.b = (byte)((uint)local_8.rgba >> 0x10), ((long long)(char)local_8.b << 0x38) >> 0x3c == -1)) &&
					(lVar4 = (*ppSectorLights)->GetBaseShape((BaseShape**)0x0), lVar4 == 1)) {
		
					local_20[3] = 0.0f;
					local_20[2] = 0.0f;
					local_20[1] = 0.0f;
					local_20[0] = 0.0f;

					iVar7 = 0;
					if (0 < this->activeLightCount) {
						do {
							if ((iVar7 != iVar11) &&
								(pLightA = this->aActiveLights[iVar7], ((pLightA->colour_0x4).a & 8) != 0)) {
	
								lVar4 = (long long)(char)(pLightA->colour_0x4).b << 0x38;
								lVar10 = lVar4 >> 0x3c;
								if ((lVar10 != -1) && (lVar10 < 4)) {
	
									fVar12 = ComputeCollision(pLightA, *ppSectorLights);
									iVar9 = (int)(lVar4 >> 0x3c);
									local_20[iVar9] = local_20[iVar9] + fVar12;
								}
							}
							iVar7 = iVar7 + 1;
						} while (iVar7 < this->activeLightCount);
					}

					uVar5 = (ulong)(local_20[1] < local_20[0]);
					if (local_20[2] < local_20[local_20[1] < local_20[0]]) {
						uVar5 = 2;
					}
					if (local_20[3] < local_20[(int)uVar5]) {
						uVar5 = 3;
					}

					IMPLEMENTATION_GUARD_LOG();
					long long shifted = (long long)uVar5 << 0x38;
					shifted = shifted >> 0x38;
					byte a = (byte)((shifted & 0xf) << 4);

					local_8.a = local_4.a | 0x80;
					local_8.b = (0xffffff0f & local_4.b) | a;

					(*ppSectorLights)->colour_0x4 = local_8;

					LIGHT_MANAGER_LOG(LogLevel::VeryVerbose, "CLightManager::BuildActiveList B Set flags light: {} flags: {:x}", (*ppSectorLights)->referencedLightIndex, (*ppSectorLights)->colour_0x4.rgba);
				}
				iVar7 = this->activeLightCount;
				iVar11 = iVar11 + 1;
				ppSectorLights = ppSectorLights + 1;
			} while (iVar11 < iVar7);
		}
		ppSectorLights = this->aActiveLights;
		iVar11 = 0;
		if (0 < iVar7) {
			do {
				local_8 = (*ppSectorLights)->colour_0x4;
				if (((local_8.rgba & 0x8000000) != 0) &&
					(local_8.b = (byte)(local_8.rgba >> 0x10), ((long long)(char)local_8.b << 0x38) >> 0x3c == -1)) {
					local_20[3] = 0.0;
					local_20[2] = 0.0;
					local_20[1] = 0.0;
					local_20[0] = 0.0;
					iVar7 = 0;
					if (0 < this->activeLightCount) {
						do {
							if ((iVar7 != iVar11) &&
								(pLightA = this->aActiveLights[iVar7], ((pLightA->colour_0x4).a & 8) != 0)) {
								lVar4 = (long long)(char)(pLightA->colour_0x4).b << 0x38;
								lVar10 = lVar4 >> 0x3c;
								if ((lVar10 != -1) && (lVar10 < 4)) {
									fVar12 = ComputeCollision(pLightA, *ppSectorLights);
									iVar9 = (int)(lVar4 >> 0x3c);
									local_20[iVar9] = local_20[iVar9] + fVar12;
								}
							}
							iVar7 = iVar7 + 1;
						} while (iVar7 < this->activeLightCount);
					}
					uVar5 = (ulong)(local_20[1] < local_20[0]);
					if (local_20[2] < local_20[local_20[1] < local_20[0]]) {
						uVar5 = 2;
					}
					if (local_20[3] < local_20[(int)uVar5]) {
						uVar5 = 3;
					}
					IMPLEMENTATION_GUARD_LOG();
					long long shifted = (long long)uVar5 << 0x38;
					shifted = shifted >> 0x38;
					byte a = (byte)((shifted & 0xf) << 4);

					local_8.a = local_4.a | 0x80;
					local_8.b = (0xffffff0f & local_4.b) | a;

					(*ppSectorLights)->colour_0x4 = local_8;
					LIGHT_MANAGER_LOG(LogLevel::VeryVerbose, "CLightManager::BuildActiveList C Set flags light: {} flags: {:x}", (*ppSectorLights)->referencedLightIndex, (*ppSectorLights)->colour_0x4.rgba);

				}
				iVar11 = iVar11 + 1;
				ppSectorLights = ppSectorLights + 1;
			} while (iVar11 < this->activeLightCount);
		}
	}
	this->bActiveListDirty = 0;
	return;
}

float CLightManager::ComputeCollision(CLight* pLightA, CLight* pLightB)
{
	bool bVar1;
	int shapeTypeA;
	int shapeTypeB;
	int zoneCount;
	int iVar5;
	BaseShape* pBVar6;
	int iVar7;
	int iVar8;
	float fVar9;
	edF32VECTOR4 local_20;
	BaseShape* pBaseShapeB;
	BaseShape* pBaseShapeA;

	shapeTypeA = pLightA->GetBaseShape(&pBaseShapeA);
	shapeTypeB = pLightB->GetBaseShape(&pBaseShapeB);
	if ((shapeTypeA == 1) || (shapeTypeB == 1)) {
		pBVar6 = pBaseShapeB;
		if (shapeTypeA == 1) {
			pBVar6 = pBaseShapeA;
			shapeTypeA = shapeTypeB;
		}
		if (shapeTypeA == 3) {
			fVar9 = 1.0f;
		}
		else {
			fVar9 = (1.0f - (pBVar6->direction.x * pBaseShapeB->direction.x +
				pBVar6->direction.y * pBaseShapeB->direction.y +
				pBVar6->direction.z * pBaseShapeB->direction.z)) / 2.0;
		}
	}
	else {
		if (pLightA->pZoneHolder == (ZoneHolder*)0x0) {
			zoneCount = 0;
		}
		else {
			zoneCount = pLightA->pZoneHolder->zone;
		}
		bVar1 = zoneCount != 0;
		if (!bVar1) {
			if ((int*)pLightA->field_0x10 == (int*)0x0) {
				iVar5 = 0;
			}
			else {
				iVar5 = *(int*)pLightA->field_0x10;
			}
			bVar1 = iVar5 != 0;
		}
		if (bVar1) {
			if (pLightB->pZoneHolder == (ZoneHolder*)0x0) {
				zoneCount = 0;
			}
			else {
				zoneCount = pLightB->pZoneHolder->zone;
			}
			bVar1 = zoneCount != 0;
			if (!bVar1) {
				if ((int*)pLightB->field_0x10 == (int*)0x0) {
					iVar5 = 0;
				}
				else {
					iVar5 = *(int*)pLightB->field_0x10;
				}
				bVar1 = iVar5 != 0;
			}
			if (bVar1) {
				if (pLightA->pZoneHolder == (ZoneHolder*)0x0) {
					zoneCount = 0;
				}
				else {
					zoneCount = pLightA->pZoneHolder->zone;
				}
				if ((int*)pLightB->field_0x10 == (int*)0x0) {
					iVar5 = 0;
				}
				else {
					iVar5 = *(int*)pLightB->field_0x10;
				}
				if ((0 < zoneCount) && (0 < iVar5)) {
					for (iVar8 = 0; iVar8 < zoneCount; iVar8 = iVar8 + 1) {
						for (iVar7 = 0; iVar7 < iVar5; iVar7 = iVar7 + 1) {
							IMPLEMENTATION_GUARD(
							if ((&pLightA->pZoneHolder->pManagerC24_0x4)[iVar8] ==
								*(CWayPoint**)(pLightB->field_0x10 + iVar7 * 4 + 4)) {
								return 0.0;
							})
						}
					}
				}
				if (pLightB->pZoneHolder == (ZoneHolder*)0x0) {
					zoneCount = 0;
				}
				else {
					zoneCount = pLightB->pZoneHolder->zone;
				}
				if ((int*)pLightA->field_0x10 == (int*)0x0) {
					iVar5 = 0;
				}
				else {
					iVar5 = *(int*)pLightA->field_0x10;
				}
				if ((0 < zoneCount) && (0 < iVar5)) {
					for (iVar8 = 0; iVar8 < zoneCount; iVar8 = iVar8 + 1) {
						for (iVar7 = 0; iVar7 < iVar5; iVar7 = iVar7 + 1) {
							IMPLEMENTATION_GUARD(
							if ((&pLightB->pZoneHolder->pManagerC24_0x4)[iVar8] ==
								*(CWayPoint**)(pLightA->field_0x10 + iVar7 * 4 + 4)) {
								return 0.0;
							})
						}
					}
				}
			}
		}

		edF32Vector4SubHard(&local_20, &pBaseShapeB->position, &pBaseShapeA->position);

		if (pBaseShapeA->fallout1 + pBaseShapeB->fallout1 <
			sqrtf(local_20.x * local_20.x + local_20.y * local_20.y + local_20.z * local_20.z)) {
			fVar9 = 0.0f;
		}
		else {
			if ((shapeTypeA == 3) || (shapeTypeB == 3)) {
				if (shapeTypeA == 3) {
					pBaseShapeA = pBaseShapeB;
					shapeTypeA = shapeTypeB;
				}
				if (shapeTypeA == 3) {
					fVar9 = 1.0f;
				}
				else {
					edF32Vector4SafeNormalize0Hard(&local_20, &local_20);
					fVar9 = (1.0f - (local_20.x * pBaseShapeA->direction.x + local_20.y * pBaseShapeA->direction.y +
						local_20.z * pBaseShapeA->direction.z)) / 2.0f;
				}
			}
			else {
				fVar9 = (1.0f - (pBaseShapeA->direction.x * pBaseShapeB->direction.x +
					pBaseShapeA->direction.y * pBaseShapeB->direction.y +
					pBaseShapeA->direction.z * pBaseShapeB->direction.z)) / 2.0f;
			}
		}
	}
	return fVar9;
}

void CLightConfig::Validate(ed_3D_Light_Config* pConfig, bool bDoColor)
{
	edF32VECTOR4* pfVar1;
	edF32VECTOR4* peVar2;
	int iVar3;
	int iVar4;

	iVar4 = 0;
	do {
		peVar2 = &pConfig->pLightDirections->vector[iVar4];
		peVar2->w = 0.0f;
		edF32Vector4SafeNormalize0Hard(peVar2, peVar2);
		iVar4 = iVar4 + 1;
	} while (iVar4 < 4);

	iVar3 = 0;
	if (bDoColor != false) {
		do {
			pfVar1 = &pConfig->pLightColorMatrix->vector[iVar3];
			if (pfVar1->x < 0.0f) {
				pfVar1->x = 0.0f;
			}
			else {
				if (255.9f < pfVar1->x) {
					pfVar1->x = 255.9f;
				}
			}
			if (pfVar1->y < 0.0f) {
				pfVar1->y = 0.0f;
			}
			else {
				if (255.9f < pfVar1->y) {
					pfVar1->y = 255.9f;
				}
			}
			if (pfVar1->z < 0.0f) {
				pfVar1->z = 0.0f;
			}
			else {
				if (255.9f < pfVar1->z) {
					pfVar1->z = 255.9f;
				}
			}
			pfVar1->w = 0.0f;
			iVar3 = iVar3 + 1;
		} while (iVar3 < 4);

		peVar2 = pConfig->pLightAmbient;
		if (peVar2->x < 0.0f) {
			peVar2->x = 0.0f;
		}
		else {
			if (255.9f < peVar2->x) {
				peVar2->x = 255.9f;
			}
		}
		if (peVar2->y < 0.0f) {
			peVar2->y = 0.0f;
		}
		else {
			if (255.9f < peVar2->y) {
				peVar2->y = 255.9f;
			}
		}
		if (peVar2->z < 0.0f) {
			peVar2->z = 0.0f;
		}
		else {
			if (255.9f < peVar2->z) {
				peVar2->z = 255.9f;
			}
		}

		peVar2->w = 0.0f;
	}

	return;
}

float CLightConfig::ComputeShadow(ed_3D_Light_Config* pConfig, edF32VECTOR4* param_2)
{
	edF32VECTOR4* pAmbient;
	edF32VECTOR4* pColor;
	edF32VECTOR4* pDirection;
	float fVar5;
	int iVar6;
	float fVar7;
	float fVar8;
	float fVar9;
	float fVar10;
	float absY;

	fVar8 = 1e-06f;
	*param_2 = g_xVector;

	fVar9 = fVar8;

	for (iVar6 = 0; iVar6 < 4; iVar6 = iVar6 + 1) {
		pColor = &pConfig->pLightColorMatrix->vector[iVar6];
		pColor->w = 0.0f;
		fVar10 = pColor->x * pColor->x + pColor->y * pColor->y + pColor->z * pColor->z;

		fVar7 = fVar9;

		if (1e-06f < fVar10) {
			fVar7 = sqrtf(fVar10);
			pDirection = &pConfig->pLightDirections->vector[iVar6];

			absY = pDirection->y;
			if (absY < 0.0f) {
				absY = -absY;
			}

			param_2->x = param_2->x + pDirection->x * fVar7;
			param_2->y = param_2->y + absY * fVar7;
			param_2->z = param_2->z + pDirection->y * fVar7;
			param_2->w = param_2->w + pDirection->z * fVar7;

			fVar8 = fVar8 + fVar7;
			if (fVar7 <= fVar9) {
				fVar7 = fVar9;
			}
		}
		fVar9 = fVar7;
	}

	edF32Vector4GetNegHard(param_2, param_2);
	edF32Vector4ScaleHard(1.0f / fVar8, param_2, param_2);
	edF32Vector4SafeNormalize1Hard(param_2, param_2);

	pAmbient = pConfig->pLightAmbient;
	fVar9 = 1.0f - sqrtf(pAmbient->x * pAmbient->x + pAmbient->y * pAmbient->y + pAmbient->z * pAmbient->z) / fVar9;
	fVar9 = (1.0f - fVar9 * fVar9) * 128.0f;
	if (fVar9 < 64.0f) {
		fVar10 = 64.0f;
	}
	else {
		fVar10 = 127.0f;
		if (fVar9 <= 127.0f) {
			fVar10 = fVar9;
		}
	}
	return fVar10;
}

CLight::CLight()
{
	this->pZoneHolder = (ZoneHolder*)0x0;
	this->field_0x10 = (undefined*)0x0;
	this->referencedLightIndex = -1;
	this->field_0x8 = 0;
}

void CLight::Activate()
{
	CScene::ptable.g_LightManager_004516b0->Activate(this, -2);
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

	if ((int*)this->field_0x10 == (int*)0x0) {
		iVar3 = 0;
	}
	else {
		iVar3 = *(int*)this->field_0x10;
	}

	LIGHT_MANAGER_LOG(LogLevel::VeryVerbose, "CLight::TestIlluminationZones count: {}", iVar3);

	iVar4 = 0;
	if (0 < iVar3) {
		iVar5 = 0;
		IMPLEMENTATION_GUARD(
		do {
			ppeVar2 = (ed_zone_3d**)(pLightManager->field_0xe0 + *(int*)(this->field_0x10 + iVar5 + 4) * 0xc);
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
	if (this->pZoneHolder->zone != 0) {
		iVar3 = this->pZoneHolder->zone;
	}

	LIGHT_MANAGER_LOG(LogLevel::VeryVerbose, "CLight::TestIlluminationZones count: {}", iVar3);

	iVar4 = 0;
	if (0 < iVar3) {
		iVar5 = 0;
		IMPLEMENTATION_GUARD(
		do {
			ppeVar2 = (ed_zone_3d**)(pLightManager->field_0xe0 + *(int*)(this->pZoneHolder + iVar5 + 4) * 0xc);
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

void CLightManager::SetManagedByCluster(CLight* pLight)
{
	(pLight->colour_0x4).a = (pLight->colour_0x4).a | 4;
	return;
}

void CLightManager::EnterManagedCluster(CLight* pLight)
{
	if ((pLight->colour_0x4).g == 0) {
		this->bActiveListDirty = 1;
	}

	(pLight->colour_0x4).g = (pLight->colour_0x4).g + 1;

	return;
}

void CLightManager::LeaveManagedCluster(CLight* pLight)
{
	(pLight->colour_0x4).g = (pLight->colour_0x4).g - 1;

	if ((pLight->colour_0x4).g == 0) {
		this->bActiveListDirty = 1;
	}

	return;
}

template<>
void S_STREAM_REF<CLight>::Init()
{
	this->pObj = STORE_SECTION(&CScene::ptable.g_LightManager_004516b0->aLights[this->index]);
	return;
}
