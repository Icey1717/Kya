#include "edParticles/edParticles.h"
#include "DlistManager.h"
#include "ed3D.h"
#include "ed3D/ed3DG2D.h"
#include "MathOps.h"
#include "profile.h"
#include "CameraViewManager.h"

#define PARTICLE_LOG(level, format, ...) MY_LOG_CATEGORY("Particle", level, format, ##__VA_ARGS__)

uint gPartProfIdShapers = 0;
uint gPartProfIdEffectors = 0;
uint gPartProfIdSelectors = 0;
uint gPartProfIdGenerators = 0;
uint gPartProfIdEffGrav = 0;
uint gPartProfIdEffDot = 0;
uint gPartProfIdEffAxe = 0;
uint gPartProfIdEffRot = 0;
uint gPartProfIdEffCol = 0;
uint gPartProfIdEffWind = 0;
uint gPartProfIdEffVisu = 0;
uint gPartProfIdEffIntr = 0;
uint gPartProfIdEffUser = 0;
uint gPartProfIdTotal = 0;
uint gPartProfIdVU = 0;

static float FLOAT_00448580 = 0.15f;
static float FLOAT_00448584 = 0.9f;
static float FLOAT_00448588 = 0.100000024f;
static float FLOAT_0044858c = 1.5259022E-5f;
static float FLOAT_00448590 = 1.0E-4f;
static float FLOAT_00448594 = 14.999999f;
static float gParticleSizeScale = 0.003921569f;

float GenRandom01(_ed_particle_generator_param* pParam)
{
	pParam->field_0x21c = pParam->field_0x21c * 0x19660d + 0x3c6ef35f;
	const uint value = (pParam->field_0x21c & 0x7fffff) | 0x3f800000;
	return *reinterpret_cast<const float*>(&value) - 1.0f;
}

float GenRandomCentered01(_ed_particle_generator_param* pParam)
{
	pParam->field_0x21c = pParam->field_0x21c * 0x19660d + 0x3c6ef35f;
	const uint value = ((((uint64_t)pParam->field_0x21c) << 0x29) >> 0x29) | 0x3F800000;
	return *reinterpret_cast<const float*>(&value) - 1.0f;
}

void edPartGeneratorShapeSet(_ed_particle_generator_param* pParam, byte shape)
{
	pParam->shape = shape;
	switch (shape) {
	case 1:
		//(pParam->speedFunc).pPosFunc = edPartGenBoxPosition;
		//(pParam->posFunc).pPosFunc = edPartGenBoxSpeed;
		break;
	case 2:
		//(pParam->speedFunc).pPosFunc = edPartGenSpherePosition;
		//(pParam->posFunc).pPosFunc = edPartGenSphereSpeed;
		break;
	case 3:
		//(pParam->speedFunc).pPosFunc = edPartGenCylinderPosition;
		//(pParam->posFunc).pPosFunc = edPartGenCylinderSpeed;
		break;
	case 4:
		//(pParam->speedFunc).pPosFunc = edPartGenCylinderPosition;
		//(pParam->posFunc).pPosFunc = edPartGenBoxSpeed;
		break;
	case 5:
		//(pParam->speedFunc).pPosFunc = edPartGenConePosition;
		//(pParam->posFunc).pPosFunc = edPartGenConeSpeed;
		break;
	case 6:
		//(pParam->speedFunc).pPosFunc = edPartGenMeshePosition;
		//(pParam->posFunc).pPosFunc = edPartGenZeroSpeed;
		break;
	case 8:
		//(pParam->speedFunc).pPosFunc = edPartGenDotPosition;
		//(pParam->posFunc).pPosFunc = edPartGenBoxSpeed;
	}
	return;
}

static _ed_particle_manager* p_current_particle_manager = (_ed_particle_manager*)0x0;
ed_part_config* p_current_particle_config;
ed_particle_system* array_particle_system;
ed_part_system_config g_particle_system_config = { 0x0, 0x1, 0x0, H_MAIN };

void edPartInit()
{
	_ed_particle* pCurParticle;
	int curIndex;

	p_current_particle_manager->nbParticles = p_current_particle_config->nbParticles;
	p_current_particle_manager->nbParticleVectors = p_current_particle_config->nbParticles;
	p_current_particle_manager->nbVectors = p_current_particle_config->nbParticles;
	p_current_particle_manager->nbGroups = p_current_particle_config->nbGroups;
	p_current_particle_manager->nbConfigGeneratorParams = p_current_particle_config->nbGeneratorParams;
	p_current_particle_manager->nbConfigEffectorParams = p_current_particle_config->nbEffectorParams;
	p_current_particle_manager->nbConfigSelectorParams = p_current_particle_config->nbSelectorParams;
	p_current_particle_manager->nbConfigShaperParams = p_current_particle_config->nbShaperParams;

	p_current_particle_manager->aParticles.pData = reinterpret_cast<_ed_particle*>(edMemAlloc((EHeap)p_current_particle_config->heapId,
			p_current_particle_manager->nbParticles * sizeof(_ed_particle)));
	memset(p_current_particle_manager->aParticles.pData, 0, p_current_particle_manager->nbParticles * sizeof(_ed_particle));

	p_current_particle_manager->aParticleVectors.pData = reinterpret_cast<_ed_particle_vectors*>(edMemAlloc((EHeap)p_current_particle_config->heapId,
			p_current_particle_manager->nbParticleVectors * sizeof(_ed_particle_vectors)));
	memset(p_current_particle_manager->aParticleVectors.pData, 0, p_current_particle_manager->nbParticleVectors * sizeof(_ed_particle_vectors));

	p_current_particle_manager->aVectors.pData = reinterpret_cast<edF32VECTOR4*>(edMemAlloc((EHeap)p_current_particle_config->heapId,
			p_current_particle_manager->nbVectors * sizeof(edF32VECTOR4)));
	memset(p_current_particle_manager->aVectors.pData, 0, p_current_particle_manager->nbVectors * sizeof(edF32VECTOR4));

	p_current_particle_manager->aGroups.pData = reinterpret_cast<_ed_particle_group*>(edMemAlloc((EHeap)p_current_particle_config->heapId,
			p_current_particle_manager->nbGroups * sizeof(_ed_particle_group)));
	memset(p_current_particle_manager->aGroups.pData, 0, p_current_particle_manager->nbGroups * sizeof(_ed_particle_group));

	p_current_particle_manager->aGeneratorParams.pData = reinterpret_cast<_ed_particle_generator_param*>(edMemAlloc((EHeap)p_current_particle_config->heapId,
			p_current_particle_manager->nbConfigGeneratorParams * sizeof(_ed_particle_generator_param)));
	memset(p_current_particle_manager->aGeneratorParams.pData, 0, p_current_particle_manager->nbConfigGeneratorParams * sizeof(_ed_particle_generator_param));

	p_current_particle_manager->aEffectorParams.pData = reinterpret_cast<_ed_particle_effector_param*>(edMemAlloc((EHeap)p_current_particle_config->heapId,
			p_current_particle_manager->nbConfigEffectorParams * sizeof(_ed_particle_effector_param)));
	memset(p_current_particle_manager->aEffectorParams.pData, 0, p_current_particle_manager->nbConfigEffectorParams * sizeof(_ed_particle_effector_param));

	p_current_particle_manager->aSelectorParams.pData = reinterpret_cast<_ed_particle_selector_param*>(edMemAlloc((EHeap)p_current_particle_config->heapId,
			p_current_particle_manager->nbConfigSelectorParams * sizeof(_ed_particle_selector_param)));
	memset(p_current_particle_manager->aSelectorParams.pData, 0, p_current_particle_manager->nbConfigSelectorParams * sizeof(_ed_particle_selector_param));

	p_current_particle_manager->aShaperParams.pData = reinterpret_cast<_ed_particle_shaper_param*>(edMemAlloc((EHeap)p_current_particle_config->heapId,
			p_current_particle_manager->nbConfigShaperParams * sizeof(_ed_particle_shaper_param)));
	memset(p_current_particle_manager->aShaperParams.pData, 0, p_current_particle_manager->nbConfigShaperParams * sizeof(_ed_particle_shaper_param));

	pCurParticle = p_current_particle_manager->aParticles.pData;
	for (curIndex = 0; curIndex < p_current_particle_manager->nbParticles; curIndex = curIndex + 1) {
		pCurParticle->field_0x6 = (short)curIndex;
		pCurParticle = pCurParticle + 1;
	}

	p_current_particle_manager->bSetup = 1;
	if (g_particle_system_config.bProfile != 0) {
		gPartProfIdTotal = edProfileNew(1, 0, 0x7f, 0);
		gPartProfIdGenerators = edProfileNew(1, 0x7f, 0, 0);
		gPartProfIdVU = edProfileNew(1, 0x20, 0x40, 0x7f);
		gPartProfIdEffectors = edProfileNew(1, 0x7f, 0x7f, 0);
		gPartProfIdEffGrav = edProfileNew(1, 0, 0, 0);
		gPartProfIdEffDot = edProfileNew(1, 0, 0x7f, 0x7f);
		gPartProfIdEffAxe = edProfileNew(1, 0x7f, 0x40, 0x40);
		gPartProfIdEffRot = edProfileNew(1, 0x7f, 0, 0x7f);
		gPartProfIdEffCol = edProfileNew(1, 0x7f, 0x7f, 0x7f);
		gPartProfIdEffWind = edProfileNew(1, 0, 0x7f, 0);
		gPartProfIdEffVisu = edProfileNew(1, 0, 0, 0x7f);
		gPartProfIdEffIntr = edProfileNew(1, 0, 0x40, 0x40);
		gPartProfIdEffUser = edProfileNew(1, 0, 0x40, 0x7f);
		gPartProfIdSelectors = edProfileNew(1, 0x5a, 0x19, 0x5a);
		gPartProfIdShapers = edProfileNew(1, 0x32, 0x32, 0x32);
	}

	return;
}

ed_part_config* edParticlesGetConfig()
{
	return p_current_particle_config;
}

void edPartSystemInit()
{
	int curSystemIndex;

	array_particle_system = (ed_particle_system*)edMemAlloc((EHeap)g_particle_system_config.heapId, g_particle_system_config.nbEffectObj * sizeof(ed_particle_system));
	memset(array_particle_system, 0, g_particle_system_config.nbEffectObj * sizeof(ed_particle_system));

	for (curSystemIndex = 0; curSystemIndex < g_particle_system_config.nbEffectObj;
		curSystemIndex = curSystemIndex + 1) {
		array_particle_system[curSystemIndex].config.nbParticles = 0x400;
		array_particle_system[curSystemIndex].config.nbGroups = 0x10;
		array_particle_system[curSystemIndex].config.field_0x8 = 6;
		array_particle_system[curSystemIndex].config.nbGeneratorParams = 0x10;
		array_particle_system[curSystemIndex].config.field_0x10 = 0x10;
		array_particle_system[curSystemIndex].config.nbEffectorParams = 0x20;
		array_particle_system[curSystemIndex].config.field_0x18 = 4;
		array_particle_system[curSystemIndex].config.nbSelectorParams = 0x10;
		array_particle_system[curSystemIndex].config.field_0x20 = 4;
		array_particle_system[curSystemIndex].config.nbShaperParams = 0x10;
		array_particle_system[curSystemIndex].config.heapId = g_particle_system_config.heapId;
	}

	g_particle_system_config.bObjectsAllocated = 1;
	p_current_particle_manager = &array_particle_system->manager;
	p_current_particle_config = &array_particle_system->config;

	return;
}

ed_part_system_config* edParticlesGetSystemConfig()
{
	return &g_particle_system_config;
}

_ed_particle_manager* edParticlesInstall(ParticleFileData* pFileData, ed_3D_Scene* pScene, ed_g2d_manager* param_3, edDList_material** ppMaterials, ulong* pHashes, int materialIndex, ed_g3d_manager* p3dManager, bool param_8)
{
	edF32VECTOR4* peVar2;
	undefined4 uVar3;
	ed_g2d_manager* peVar4;
	float fVar5;
	bool bVar6;
	edNODE* peVar7;
	ed_hash_code* peVar8;
	DisplayList** ppDVar9;
	DisplayList* pDVar10;
	char** ppcVar11;
	int iVar12;
	char* pcVar13;
	int iVar15;
	float* pfVar16;
	int iVar17;
	edF32MATRIX4* peVar18;
	undefined4* puVar19;
	undefined4* puVar20;
	int nbMatrices;
	_ed_particle_generator_param** pp_Var21;
	int iVar22;
	_ed_particle_generator_param* pCurParam;
	int iVar24;
	_ed_particle_group* pCurGroup;
	undefined* puVar26;
	uint flags;
	ulong hash;
	int local_60;
	ed_g2d_manager* local_34;
	int iStack4;

	bVar6 = false;
	local_34 = (ed_g2d_manager*)0x0;

	int offset = (pFileData->manager).aParticles.offset;
	if (offset != 0x0) {
		(pFileData->manager).aParticles.pData = reinterpret_cast<_ed_particle*>(reinterpret_cast<char*>(pFileData) + offset);
	}

	offset = (pFileData->manager).aParticleVectors.offset;
	if (offset != 0x0) {
		(pFileData->manager).aParticleVectors.pData = reinterpret_cast<_ed_particle_vectors*>(reinterpret_cast<char*>(pFileData) + offset);
	}

	offset = (pFileData->manager).aVectors.offset;
	if (offset != 0x0) {
		(pFileData->manager).aVectors.pData = reinterpret_cast<edF32VECTOR4*>(reinterpret_cast<char*>(pFileData) + offset);
	}

	offset = (pFileData->manager).aGroups.offset;
	if (offset != 0x0) {
		(pFileData->manager).aGroups.pData = reinterpret_cast<_ed_particle_group*>(reinterpret_cast<char*>(pFileData) + offset);
	}

	offset = (pFileData->manager).aGeneratorParams.offset;
	if (offset != 0x0) {
		(pFileData->manager).aGeneratorParams.pData = reinterpret_cast<_ed_particle_generator_param*>(reinterpret_cast<char*>(pFileData) + offset);
	}

	offset = (pFileData->manager).aEffectorParams.offset;
	if (offset != 0x0) {
		(pFileData->manager).aEffectorParams.pData = reinterpret_cast<_ed_particle_effector_param*>(reinterpret_cast<char*>(pFileData) + offset);
	}

	offset = (pFileData->manager).aSelectorParams.offset;
	if (offset != 0x0) {
		(pFileData->manager).aSelectorParams.pData = reinterpret_cast<_ed_particle_selector_param*>(reinterpret_cast<char*>(pFileData) + offset);
	}

	offset = (pFileData->manager).aShaperParams.offset;
	if (offset != 0x0) {
		(pFileData->manager).aShaperParams.pData = reinterpret_cast<_ed_particle_shaper_param*>(reinterpret_cast<char*>(pFileData) + offset);
	}

	offset = (pFileData->manager).field_0xc.offset;
	if (offset != 0x0) {
		(pFileData->manager).field_0xc.pData = reinterpret_cast<undefined*>(reinterpret_cast<char*>(pFileData) + offset);
	}

	_ed_particle* pCurParticle = (pFileData->manager).aParticles.pData;
	for (iVar17 = 0; iVar17 < (pFileData->manager).nbParticles; iVar17 = iVar17 + 1) {
		pCurParticle->field_0x6 = (short)iVar17;
		pCurParticle->field_0x0 = 1;
		pCurParticle->field_0xd = 1;
		pCurParticle = pCurParticle + 1;
	}

	pCurGroup = (pFileData->manager).aGroups.pData;
	iVar17 = 0;
	do {
		if ((pFileData->manager).nbTotalGroups <= iVar17) {
			pCurParam = (pFileData->manager).aGeneratorParams.pData;

			for (iVar17 = 0; iVar17 < (pFileData->manager).nbGeneratorParams; iVar17 = iVar17 + 1) {
				pCurParam->field_0x0 = 1;
				pCurParam->field_0x2 = 1;
				pCurParam->field_0x228 = pCurParam->field_0x178;
				edPartGeneratorShapeSet(pCurParam, pCurParam->shape);
				(pCurParam->speedFunc).field_0x12 = 0xff;
				(pCurParam->speedFunc).field_0x13 = 0xff;
				(pCurParam->speedFunc).field_0x16 = 0xff;
				(pCurParam->posFunc).field_0x12 = 0;
				(pCurParam->posFunc).field_0x13 = 0;
				(pCurParam->posFunc).field_0x16 = 0;
				(pCurParam->speedFunc).field_0x8 = 0xff;
				(pCurParam->posFunc).field_0x8 = 0;
				pCurParam = pCurParam + 1;
			}

			_ed_particle_effector_param* pGenParam = (pFileData->manager).aEffectorParams.pData;
			for (iVar17 = 0; iVar17 < (pFileData->manager).nbEffectorParams; iVar17 = iVar17 + 1) {
				pGenParam->field_0x0 = 1;
				pGenParam->field_0x2 = 1;
				pGenParam = pGenParam + 1;
			}

			_ed_particle_selector_param* pSelectorParam = (pFileData->manager).aSelectorParams.pData;
			for (iVar17 = 0; iVar17 < (pFileData->manager).nbSelectorParams; iVar17 = iVar17 + 1) {
				pSelectorParam->field_0x0 = 1;
				pSelectorParam->field_0x2 = 1;
				pSelectorParam = pSelectorParam + 1;
			}

			_ed_particle_shaper_param* pParticle_1a0 = (pFileData->manager).aShaperParams.pData;
			for (iVar17 = 0; iVar17 < (pFileData->manager).nbShaperParams; iVar17 = iVar17 + 1) {
				pParticle_1a0->field_0x0 = 1;
				pParticle_1a0->field_0x2 = 1;
				pParticle_1a0 = pParticle_1a0 + 1;
			}

			(pFileData->manager).bSetup = 2;

			return &pFileData->manager;
		}

		pCurGroup->field_0x0 = 1;
		pCurGroup->field_0x2 = 1;
		if (pCurGroup->field_0x24.offset != 0x0) {
			pCurGroup->field_0x24.pData = reinterpret_cast<OffsetPointer<_ed_particle_generator_param*>*>(reinterpret_cast<char*>(pFileData) + pCurGroup->field_0x24.offset);
		}

		if (pCurGroup->field_0x30.offset != 0x0) {
			pCurGroup->field_0x30.pData = reinterpret_cast<OffsetPointer<_ed_particle_effector_param*>*>(reinterpret_cast<char*>(pFileData) + pCurGroup->field_0x30.offset);
		}

		if (pCurGroup->field_0x3c.offset != 0x0) {
			pCurGroup->field_0x3c.pData = reinterpret_cast<OffsetPointer<_ed_particle_selector_param*>*>(reinterpret_cast<char*>(pFileData) + pCurGroup->field_0x3c.offset);
		}

		if (pCurGroup->field_0x48.offset != 0x0) {
			pCurGroup->field_0x48.pData = reinterpret_cast<OffsetPointer<_ed_particle_shaper_param*>*>(reinterpret_cast<char*>(pFileData) + pCurGroup->field_0x48.offset);
		}

		pCurGroup->pParticle.pData = (pFileData->manager).aParticles.pData + pCurGroup->pParticle.offset;
		pCurGroup->field_0x6 = pCurGroup->pParticle.pData->field_0x6;

		for (iVar22 = 0; iVar15 = 0, iVar22 < pCurGroup->field_0x20; iVar22 = iVar22 + 1) {
			pCurGroup->field_0x24.pData[iVar22].pData = (pFileData->manager).aGeneratorParams.pData + pCurGroup->field_0x24.pData[iVar22].offset;

			iVar15 = pCurGroup->field_0x80;
			if (iVar15 == 0) {
				iVar15 = rand();
			}

			pCurGroup->field_0x24.pData[iVar22].pData->field_0x21c = iVar15;
			pCurGroup->field_0x24.pData[iVar22].pData->field_0x80 = pCurGroup->field_0x24.pData[iVar22].pData->field_0x40;
			pCurGroup->field_0x24.pData[iVar22].pData->field_0x100 = 1.0f;
		}

		iVar22 = pCurGroup->field_0x2c;
		for (; iVar12 = 0, iVar15 < iVar22; iVar15 = iVar15 + 1) {
			pCurGroup->field_0x30.pData[iVar15].pData = (pFileData->manager).aEffectorParams.pData + pCurGroup->field_0x30.pData[iVar15].offset;

			pCurGroup->field_0x30.pData[iVar15].pData->field_0xc0 = pCurGroup->field_0x30.pData[iVar15].pData->field_0x40;
			
			_ed_particle_effector_param_10* pSub = pCurGroup->field_0x30.pData[iVar15].pData->aSubParams;
			for (iVar12 = 0; iVar12 < pCurGroup->field_0x30.pData[iVar15].pData->nbSubParams; iVar12 = iVar12 + 1) {
				pSub->field_0xc.pData = reinterpret_cast<float*>(reinterpret_cast<char*>(pFileData) + pSub->field_0xc.offset);
				pSub = pSub + 1;
			}

			iVar22 = pCurGroup->field_0x2c;
		}

		for (; iVar22 = 0, iVar12 < pCurGroup->field_0x38; iVar12 = iVar12 + 1) {
			iVar15 = 0;
			pCurGroup->field_0x3c.pData[iVar12].pData = (pFileData->manager).aSelectorParams.pData + pCurGroup->field_0x3c.pData[iVar12].offset;

			_ed_particle_selector_param* pSelectorParam = pCurGroup->field_0x3c.pData[iVar12].pData;

			pSelectorParam->field_0x14.pData = reinterpret_cast<OffsetPointer<void*>*>(reinterpret_cast<char*>(pFileData) + pSelectorParam->field_0x14.offset);
			
			while (iVar15 < pSelectorParam->field_0x10) {
				if (pSelectorParam->field_0x8 == 2) {
					pSelectorParam->field_0x14.pData[iVar15].pData = (pFileData->manager).aEffectorParams.pData + pSelectorParam->field_0x14.pData[iVar15].offset;
				LAB_0027d4d4:
					iVar15 = iVar15 + 1;
				}
				else {
					if (pSelectorParam->field_0x8 == 1) {
						pSelectorParam->field_0x14.pData[iVar15].pData = (pFileData->manager).aGeneratorParams.pData + pSelectorParam->field_0x14.pData[iVar15].offset;
						goto LAB_0027d4d4;
					}

					iVar15 = iVar15 + 1;
				}
			}
		}

		iVar15 = pCurGroup->field_0x44;
		for (; iVar22 < iVar15; iVar22 = iVar22 + 1) {
			pCurGroup->field_0x48.pData[iVar22].pData = (pFileData->manager).aShaperParams.pData + pCurGroup->field_0x48.pData[iVar22].offset;

			_ed_particle_shaper_param* pShaperParam = pCurGroup->field_0x48.pData[iVar22].pData;
			pShaperParam->field_0x198 = pScene;
			pShaperParam->field_0x18d = 0;
			pShaperParam->field_0x18e = 0;
			pShaperParam->field_0x18f = 0;
			if (pShaperParam->field_0x4 == 3) {
				pShaperParam->field_0x194.pData = reinterpret_cast<void*>(reinterpret_cast<char*>(pFileData) + pShaperParam->field_0x194.offset);
				if (p3dManager != (ed_g3d_manager*)0x0) {
					iVar12 = pCurGroup->field_0x10;
					_ed_particle* pParticle = pCurGroup->pParticle.pData;
					for (iVar24 = 0; iVar24 < iVar12; iVar24 = iVar24 + 1) {
						pParticle->pNode = ed3DHierarchyAddToScene(pScene, p3dManager, (char*)0x0);
						pParticle = pParticle + 1;
					}
				}
			}
			else {
				if (param_3 == (ed_g2d_manager*)0x0) {
					if (pShaperParam->field_0x40.offset != 0) {
						pcVar13 = reinterpret_cast<char*>(pFileData) + pShaperParam->field_0x40.offset;
						if (bVar6) {
							pShaperParam->field_0x40.pData = local_34;
						}
						else {
							local_34 = ed3DInstallG2D(pcVar13, *(int*)(pcVar13 + 8), &iStack4, (ed_g2d_manager*)0x0, 1);
							pShaperParam->field_0x40.pData = local_34;
							bVar6 = true;
						}
					}
				}
				else {
					pShaperParam->field_0x40.pData = param_3;
				}

				if (pShaperParam->field_0x48.offset == 0) {
					edDebugPrintf("»>>>>>>>>>>>>>>>>>>>>>>>>>>>>> edPARTICLE_INSTALL, no material haschode found ! «");
				}
				
				pShaperParam->field_0x48.pData = reinterpret_cast<ulong*>(reinterpret_cast<char*>(pFileData) + pShaperParam->field_0x48.offset);

				if (ppMaterials == (edDList_material**)0x0) {
					iVar12 = 0;

#ifdef PLATFORM_WIN
					pShaperParam->field_0x44.pData = new edDList_material[pShaperParam->field_0xb];
#else
					pShaperParam->field_0x44.pData = reinterpret_cast<edDList_material*>(reinterpret_cast<char*>(pFileData) + pShaperParam->field_0x44.offset);
#endif
					pShaperParam->field_0x18c = 0;

					while (iVar12 < pShaperParam->field_0xb) {
						pShaperParam->field_0x44.pData[iVar12].pManager = (ed_g2d_manager*)0x0;
						hash = pShaperParam->field_0x48.pData[iVar12];
						peVar8 = ed3DG2DGetMaterial(pShaperParam->field_0x40.pData, hash);
						if (peVar8 == (ed_hash_code*)0x0) {
							edDebugPrintf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>> edPARTICLE_INSTALL, No material found for this particle effect !\n");
							iVar12 = iVar12 + 1;
						}
						else {
							edDListCreatMaterialFromHashCode(&pShaperParam->field_0x44.pData[iVar12], hash, pShaperParam->field_0x40.pData, 2);
							iVar12 = iVar12 + 1;
						}
					}
				}
				else {
#ifdef PLATFORM_WIN
					pShaperParam->field_0x44.pData = new edDList_material[pShaperParam->field_0xb];
#else
					pShaperParam->field_0x44.pData = reinterpret_cast<edDList_material*>(reinterpret_cast<char*>(pFileData) + pShaperParam->field_0x44.offset);
#endif

					for (iVar12 = 0; iVar12 < pShaperParam->field_0xb; iVar12 = iVar12 + 1) {
						for (iVar24 = 0; iVar24 < materialIndex; iVar24 = iVar24 + 1) {
							if (pShaperParam->field_0x48.pData[iVar12] == pHashes[iVar24]) {
								pShaperParam->field_0x44.pData[iVar12] = *ppMaterials[iVar24];
								break;
							}
						}

						if (iVar24 == materialIndex) {
							edDebugPrintf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>  Material not found\n");
							pShaperParam->field_0x44.pData[iVar12].pManager = (ed_g2d_manager*)0x0;
						}
					}
					
					pShaperParam->field_0x18c = 1;
				}
			}

			pShaperParam->field_0x114 = -1.0f;
			pShaperParam->field_0xd0 = gF32Matrix4Unit;			

			if (pShaperParam->field_0x4 != 3) {
				pShaperParam->field_0x11c = 6;
				pShaperParam->field_0x120 = (uint)pShaperParam->field_0xb << 1;
				pShaperParam->field_0xc = 1;

				local_60 = pShaperParam->field_0x120;
				flags = 0x11;
				if (pShaperParam->field_0x4 == 2) {
					flags = 0x111;
					local_60 = 2;
					pShaperParam->field_0xc = (uint)pShaperParam->field_0xb;
					pShaperParam->field_0x118 = 2;
				}

				iVar12 = 0;
				while (iVar12 < pShaperParam->field_0xc) {
					if (param_8 == false) {
						nbMatrices = 0;
						iVar24 = pShaperParam->field_0x11c * pCurGroup->field_0x10;
						if (pShaperParam->field_0x4 != 1) {
							nbMatrices = iVar24 * 4;
							iVar24 = 0;
						}

						pShaperParam->field_0x124[iVar12] = edDListNew(TO_HEAP(H_MAIN), flags, local_60, iVar24, nbMatrices, 0, (DisplayList*)0x0);
						edDListSetSceneUsed(pShaperParam->field_0x124[iVar12], pScene);
						iVar12 = iVar12 + 1;
					}
					else {
						pShaperParam->field_0x124[iVar12] = (DisplayList*)0x0;
						iVar12 = iVar12 + 1;
					}
				}
			}

			iVar15 = pCurGroup->field_0x44;
		}

		pCurGroup = pCurGroup + 1;
		iVar17 = iVar17 + 1;
	} while (true);
}

#pragma clang optimize on

void edParticlesSetSystem(_ed_particle_manager* pManager)
{
	p_current_particle_manager = pManager;
	return;
}

edF32VECTOR4 edF32VECTOR4_0041ed30 = { 0.0f, 0.0f, 0.0f, -1.0f };

void edPartGenNewPosAndSpeed(_ed_particle_generator_param* pGeneratorParam, edF32VECTOR4* param_2, edF32VECTOR4* param_3)
{
	byte bVar1;
	uint uVar2;
	int iVar3;
	int iVar4;
	float puVar9;
	int iVar5;
	uint uVar6;
	int iVar7;
	byte bVar8;
	float fVar9;
	float puVar10;
	float fVar11;
	float fVar12;
	float puVar13;
	float fVar14;
	float fVar15;
	float fVar16;
	float fVar17;
	float fVar18;
	edF32MATRIX4 eStack608;
	edF32VECTOR4 local_220;
	float local_210;
	float local_20c;
	float local_208;
	undefined4 local_204;
	edF32MATRIX4 eStack512;
	edF32VECTOR4 local_1c0;
	float local_1b0;
	float local_1ac;
	float local_1a8;
	undefined4 local_1a4;
	float local_1a0;
	float fStack412;
	float fStack408;
	edF32MATRIX4 eStack400;
	edF32VECTOR4 local_150;
	float local_140;
	float local_13c;
	float local_138;
	undefined4 local_134;
	float local_130;
	float fStack300;
	float fStack296;
	float local_120;
	float fStack284;
	float fStack280;
	float local_110;
	float fStack268;
	float local_108;
	float fStack260;
	float local_100;
	float fStack252;
	float local_f8;
	float fStack244;
	float local_f0;
	float local_ec;
	float local_e8;
	undefined4 uStack228;
	edF32VECTOR4 local_e0;
	edF32VECTOR4 local_d0;

	iVar7 = 2;
	pGeneratorParam->field_0x21c = pGeneratorParam->field_0x21c >> 1;
	fVar11 = gF32Vertex4Zero.w;
	fVar15 = gF32Vertex4Zero.z;
	fVar14 = gF32Vertex4Zero.y;
	bVar1 = pGeneratorParam->shape;
	iVar5 = (int)(uint)pGeneratorParam->field_0xf >> 4;
	bVar8 = pGeneratorParam->field_0xf & 0xf;

	if (bVar1 == PARTICLE_SHAPE_SPHERE) {
		if ((iVar5 != 0) && (iVar7 = iVar5, iVar5 == 1)) {
			iVar7 = 2;
		}

		fVar11 = GenRandomCentered01(pGeneratorParam);
		fVar14 = GenRandomCentered01(pGeneratorParam) - 0.5f;

		const float theta = fVar14 * M_PI;
		const float phi = fVar11 * M_2_PI;

		local_e0.x = cosf(phi) * cosf(theta);
		local_e0.y = sinf(theta);
		local_e0.z = cosf(theta) * sinf(phi);
		local_e0.w = 0.0f;

		if (iVar7 == 3) {
			fVar14 = GenRandom01(pGeneratorParam) * 0.5f;
			param_2->x = local_e0.x * fVar14;
			param_2->y = local_e0.y * fVar14;
			param_2->z = local_e0.z * fVar14;
			param_2->w = fVar14 * 0.0f;
		}
		else {
			if (iVar7 == 2) {
				param_2->x = local_e0.x * 0.5f;
				param_2->y = local_e0.y * 0.5f;
				param_2->z = local_e0.z * 0.5f;
				param_2->w = 0.0f;
			}
		}

		param_2->w = 1.0f;

		edF32Matrix4MulF32Vector4Hard(param_2, &pGeneratorParam->field_0x40, param_2);
	LAB_0027a3b0:
		bVar1 = pGeneratorParam->field_0x7;
	}
	else {
		if (bVar1 == PARTICLE_SHAPE_CYLINDER) {
			if ((iVar5 != 0) && (iVar7 = iVar5, iVar5 == 1)) {
				iVar7 = 2;
			}

			fVar14 = GenRandomCentered01(pGeneratorParam);
			fVar15 = M_2_PI * fVar14;
			puVar10 = 0.5f;
			if (fVar15 < 0.0f) {
				puVar10 = -0.5f;
			}
			local_e0.x = cosf(fVar15 + puVar10);
			fVar14 = M_2_PI * fVar14;
			local_e0.y = 0.0f;
			if (fVar14 < 0.0f) {
				puVar9 = -0.5f;
			}
			else {
				puVar9 = 0.5f;
			}

			local_e0.z = cosf(fVar14 + puVar9);
			local_e0.w = 0.0f;

			// Generate position based on distribution type
			if (iVar7 == 3) {
				// Volume distribution: random radius scaling
				float radiusScale = GenRandomCentered01(pGeneratorParam) - 0.5f;
				param_2->x = local_e0.x * radiusScale;
				param_2->y = radiusScale * 0.0f;
				param_2->z = local_e0.z * radiusScale;
				param_2->w = radiusScale * 0.0f;
				param_2->y = GenRandom01(pGeneratorParam) - 0.5f;
			}
			else if (iVar7 == 2) {
				// Surface distribution: fixed radius
				param_2->x = local_e0.x * 0.5f;
				param_2->y = 0.0f;
				param_2->z = local_e0.z * 0.5f;
				param_2->w = 0.0f;
				param_2->y = GenRandom01(pGeneratorParam) - 0.5f;
			}

			param_2->w = 1.0f;
			edF32Matrix4MulF32Vector4Hard(param_2, &pGeneratorParam->field_0x40, param_2);
			goto LAB_0027a3b0;
		}

		if (bVar1 == 10) {
			if (iVar5 == 0) {
				iVar5 = 1;
			}
			else {
				if (iVar5 == 3) {
					iVar5 = iVar7;
				}
			}

			puVar10 = cosf(M_2_PI * GenRandomCentered01(pGeneratorParam));
			puVar13 = cosf(M_2_PI * GenRandomCentered01(pGeneratorParam));

			if (bVar8 == 1) {
				local_e0.y = 0.0f;
				local_e0.w = 0.0f;
				local_e0.x = puVar10;
				local_e0.z = puVar13;
			}
			if (iVar5 == 2) {
				fVar14 = GenRandom01(pGeneratorParam) - 0.5f * 0.5f;
				param_2->x = fVar14 * puVar10;
				param_2->y = 0.0;
				param_2->z = fVar14 * puVar13;
				param_2->w = 1.0;
			}
			else {
				if (iVar5 == 1) {
					param_2->x = puVar10 * 0.5f;
					param_2->y = 0.0f;
					param_2->z = puVar13 * 0.5f;
					param_2->w = 1.0f;
				}
			}

			edF32Matrix4MulF32Vector4Hard(param_2, &pGeneratorParam->field_0x40, param_2);
			goto LAB_0027a3b0;
		}

		if (bVar1 == 9) {
			if ((iVar5 == 2) || (iVar5 == 3)) {
				iVar5 = 1;
			}
			if (iVar5 == 1) {
				param_2->x = gF32Vertex4Zero.x;
				param_2->y = fVar14;
				param_2->z = fVar15;
				param_2->w = fVar11;
				param_2->x = GenRandom01(pGeneratorParam) - 0.5f;
				if (bVar8 == 1) {
					local_e0.x = 0.0f;
					local_e0.y = GenRandomCentered01(pGeneratorParam) - 0.5f;
					local_e0.w = 0.0f;
					local_e0.z = GenRandom01(pGeneratorParam) - 0.5f;
				}
			}
			else {
				if (iVar5 == 0) {
					pGeneratorParam->field_0x21c = pGeneratorParam->field_0x21c * 0x19660d + 0x3c6ef35f;
					uVar6 = pGeneratorParam->field_0x21c & 1;
					*param_2 = gF32Vertex4Zero;
					if (uVar6 == 0) {
						puVar10 = 0.5f;
					}
					else {
						puVar10 = -0.5f;
					}

					param_2->x = (float)puVar10;
					if (bVar8 == 1) {
						local_e0.y = gF32Vector4Zero.y;
						local_e0.z = gF32Vector4Zero.z;
						local_e0.w = gF32Vector4Zero.w;
						if (uVar6 == 0) {
							local_e0.x = 1.0f;
						}
						else {
							local_e0.x = -1.0f;
						}
					}
				}
			}

			edF32Matrix4MulF32Vector4Hard(param_2, &pGeneratorParam->field_0x40, param_2);
			goto LAB_0027a3b0;
		}

		if (bVar1 == 7) {
			if (iVar5 == 3) {
				iVar5 = iVar7;
			}

			if (iVar5 == 2) {
				param_2->x = GenRandomCentered01(pGeneratorParam) - 0.5f;
				param_2->y = 0.0f;
				param_2->z = GenRandom01(pGeneratorParam) - 0.5f;
				param_2->w = 1.0f;
				if (bVar8 == 1) {
					bVar8 = 0;
				}
			}
			else {
				if (iVar5 == 1) {
					IMPLEMENTATION_GUARD(
					pGeneratorParam->field_0x21c = pGeneratorParam->field_0x21c * 0x19660d + 0x3c6ef35f;
					uVar6 = pGeneratorParam->field_0x21c & 3;
					iVar5 = uVar6 * 8;
					pGeneratorParam->field_0x21c = pGeneratorParam->field_0x21c * 0x19660d + 0x3c6ef35f;
					fVar16 = (float)((uint)((ulong)((long)(int)pGeneratorParam->field_0x21c << 0x29) >> 0x29) | 0x3f800000) - 1.0;
					fStack252 = edF32VECTOR4_0041ed10.y;
					fStack244 = edF32VECTOR4_0041ed10.w;
					local_100 = *(float*)(&DAT_00431990 + *(int*)(&DAT_004319b0 + uVar6 * 8) * 8);
					local_f8 = *(float*)(&DAT_00431994 + *(int*)(&DAT_004319b0 + uVar6 * 8) * 8);
					fStack268 = edF32VECTOR4_0041ed20.y;
					fStack260 = edF32VECTOR4_0041ed20.w;
					local_110 = *(float*)(&DAT_00431990 + *(int*)(&DAT_004319b4 + iVar5) * 8);
					local_108 = *(float*)(&DAT_00431994 + *(int*)(&DAT_004319b4 + iVar5) * 8);
					fVar14 = 1.0 - fVar16;
					fVar15 = edF32VECTOR4_0041ed20.y * fVar16;
					fVar12 = edF32VECTOR4_0041ed20.w * fVar16;
					fVar11 = edF32VECTOR4_0041ed10.y * fVar14;
					fVar9 = edF32VECTOR4_0041ed10.w * fVar14;
					param_2->x = local_110 * fVar16 + local_100 * fVar14;
					param_2->y = fVar15 + fVar11;
					param_2->z = local_108 * fVar16 + local_f8 * fVar14;
					param_2->w = fVar12 + fVar9;
					param_2->w = 1.0;
					if (bVar8 == 1) {
						local_e0.x = *(float*)(&DAT_004319d0 + iVar5);
						local_e0.y = 0.0;
						local_e0.z = *(float*)(&DAT_004319d4 + iVar5);
						local_e0.w = 0.0;
					})
				}
				else {
					if (iVar5 == 0) {
						static float FLOAT_ARRAY_ARRAY_00431990[4][2] = {
							{ 0.5f, 0.5f },
							{ 0.5f, -0.5f },
							{ -0.5f, -0.5f },
							{ -0.5f, 0.5f }
						};

						pGeneratorParam->field_0x21c = pGeneratorParam->field_0x21c * 0x19660d + 0x3c6ef35f;
						iVar5 = pGeneratorParam->field_0x21c & 3;
						param_2->x = FLOAT_ARRAY_ARRAY_00431990[iVar5][0];
						param_2->y = 0.0f;
						param_2->z = FLOAT_ARRAY_ARRAY_00431990[iVar5][1];
						param_2->w = 1.0f;
						if (bVar8 == 1) {
							local_e0.x = param_2->x;
							local_e0.y = param_2->y;
							local_e0.z = param_2->z;
							local_e0.w = 0.0f;
						}
					}
				}
			}

			edF32Matrix4MulF32Vector4Hard(param_2, &pGeneratorParam->field_0x40, param_2);
			goto LAB_0027a3b0;
		}

		if (bVar1 == PARTICLE_SHAPE_BOX) {
			if (iVar5 == 3) {
				IMPLEMENTATION_GUARD(
				param_2->x = GenRandomCentered01(pGeneratorParam) - 0.5;
				param_2->y = GenRandomCentered01(pGeneratorParam) - 0.5;
				param_2->z = GenRandom01(pGeneratorParam) - 0.5;
				param_2->w = 1.0;
				if (bVar8 == 1) {
					fVar14 = param_2->x;
					fVar15 = param_2->y;
					local_e8 = param_2->z;
					fStack284 = fVar15 * fVar15;
					fStack280 = local_e8 * local_e8;
					local_120 = SQRT(fVar14 * fVar14 + fStack284 + fStack280) + 0.0;
					fVar11 = 1.0 / local_120;
					fVar14 = fVar14 * fVar11;
					fVar15 = fVar15 * fVar11;
					local_e8 = local_e8 * fVar11;
					uStack228 = 0;
					local_f0 = -fVar14;
					if (0.0 <= fVar14) {
						local_f0 = fVar14;
					}
					local_ec = -fVar15;
					if (0.0 <= fVar15) {
						local_ec = fVar15;
					}
					if (local_e8 < 0.0) {
						local_e8 = -local_e8;
					}
					local_e0.x = gF32Vector4Zero.x;
					local_e0.y = gF32Vector4Zero.y;
					local_e0.z = gF32Vector4Zero.z;
					local_e0.w = gF32Vector4Zero.w;
					if (local_e8 < local_f0) {
						if (local_ec < local_f0) {
							if (param_2->x < 0.0) {
								local_e0.x = (float)&DAT_bf800000;
							}
							else {
								local_e0.x = 1.0;
							}
						}
						else {
							if (param_2->y < 0.0) {
								local_e0.y = (float)&DAT_bf800000;
							}
							else {
								local_e0.y = 1.0;
							}
						}
					}
					else {
						if (local_ec < local_e8) {
							if (param_2->z < 0.0) {
								local_e0.z = (float)&DAT_bf800000;
							}
							else {
								local_e0.z = 1.0;
							}
						}
						else {
							if (param_2->y < 0.0) {
								local_e0.y = (float)&DAT_bf800000;
							}
							else {
								local_e0.y = 1.0;
							}
						}
					}
				})
			}
			else {
				if (iVar5 == 2) {
					pGeneratorParam->field_0x21c = pGeneratorParam->field_0x21c * 0x19660d + 0x3c6ef35f;
					uVar6 = pGeneratorParam->field_0x21c;
					iVar5 = (int)uVar6 % 6;
					if (iVar5 < 2) {
						if (iVar5 == 0) {
							param_2->x = 0.5f;
						}
						else {
							param_2->x = -0.5f;
						}

						param_2->y = GenRandomCentered01(pGeneratorParam) - 0.5f;
						param_2->z = GenRandom01(pGeneratorParam) - 0.5f;
						param_2->w = 1.0;
						local_e0.y = gF32Vector4Zero.y;
						local_e0.z = gF32Vector4Zero.z;
						local_e0.w = gF32Vector4Zero.w;
						if (iVar5 == 0) {
							local_e0.x = 1.0f;
						}
						else {
							local_e0.x = -1.0f;
						}
					}
					else {
						if (iVar5 < 4) {
							param_2->x = GenRandom01(pGeneratorParam) - 0.5f;
							if (iVar5 == 2) {
								param_2->y = 0.5f;
							}
							else {
								param_2->y = -0.5f;
							}
							param_2->z = GenRandom01(pGeneratorParam) - 0.5f;
							param_2->w = 1.0f;
							local_e0.x = gF32Vector4Zero.x;
							local_e0.z = gF32Vector4Zero.z;
							local_e0.w = gF32Vector4Zero.w;
							if (iVar5 == 2) {
								local_e0.y = 1.0f;
							}
							else {
								local_e0.y = -1.0f;
							}
						}
						else {
							puVar10 = 0.5f;
							pGeneratorParam->field_0x21c = uVar6 * 0x19660d + 0x3c6ef35f;

							param_2->x = GenRandomCentered01(pGeneratorParam) - 0.5f;
							param_2->y = GenRandomCentered01(pGeneratorParam) - 0.5f;

							if (iVar5 != 4) {
								puVar10 = -0.5f;
							}
							param_2->z = (float)puVar10;
							param_2->w = 1.0f;
							local_e0.x = gF32Vector4Zero.x;
							local_e0.y = gF32Vector4Zero.y;
							local_e0.w = gF32Vector4Zero.w;
							if (iVar5 == 4) {
								local_e0.z = 1.0f;
							}
							else {
								local_e0.z = -1.0f;
							}
						}
					}
				}
				else {
					if (iVar5 == 1) {
						IMPLEMENTATION_GUARD(
						pGeneratorParam->field_0x21c = pGeneratorParam->field_0x21c * 0x19660d + 0x3c6ef35f;
						iVar7 = ((int)pGeneratorParam->field_0x21c % 0xc) * 8;
						iVar5 = *(int*)(&DAT_00431930 + iVar7);
						iVar7 = *(int*)(&DAT_00431934 + iVar7);
						pGeneratorParam->field_0x21c = pGeneratorParam->field_0x21c * 0x19660d + 0x3c6ef35f;
						iVar3 = iVar5 * 0x10;
						iVar4 = iVar7 * 0x10;
						fVar15 = (float)(pGeneratorParam->field_0x21c & 0x7fffff | 0x3f800000) - 1.0;
						fVar11 = *(float*)(&DAT_004318b4 + iVar3);
						fVar12 = *(float*)(&DAT_004318b8 + iVar3);
						fVar9 = *(float*)(&DAT_004318bc + iVar3);
						fVar16 = *(float*)(&DAT_004318b4 + iVar4);
						fVar17 = *(float*)(&DAT_004318b8 + iVar4);
						fVar18 = *(float*)(&DAT_004318bc + iVar4);
						fVar14 = 1.0 - fVar15;
						param_2->x = *(float*)(&DAT_004318b0 + iVar4) * fVar15 + *(float*)(&DAT_004318b0 + iVar3) * fVar14;
						param_2->y = fVar16 * fVar15 + fVar11 * fVar14;
						param_2->z = fVar17 * fVar15 + fVar12 * fVar14;
						param_2->w = fVar18 * fVar15 + fVar9 * fVar14;
						if (bVar8 == 1) {
							iVar5 = iVar5 * 0x10;
							iVar7 = iVar7 * 0x10;
							local_e0.x = *(float*)(&DAT_004318b0 + iVar7) * 0.5 + *(float*)(&DAT_004318b0 + iVar5) * 0.5;
							local_e0.y = *(float*)(&DAT_004318b4 + iVar7) * 0.5 + *(float*)(&DAT_004318b4 + iVar5) * 0.5;
							local_e0.z = *(float*)(&DAT_004318b8 + iVar7) * 0.5 + *(float*)(&DAT_004318b8 + iVar5) * 0.5;
							local_e0.w = 0.0;
						})
					}
					else {
						if (iVar5 == 0) {
							IMPLEMENTATION_GUARD(
							pGeneratorParam->field_0x21c = pGeneratorParam->field_0x21c * 0x19660d + 0x3c6ef35f;
							uVar6 = pGeneratorParam->field_0x21c;
							if ((uVar6 & 1) == 0) {
								puVar10 = 0.5f;
							}
							else {
								puVar10 = -0.5f;
							}
							param_2->x = (float)puVar10;
							if ((uVar6 & 2) == 0) {
								puVar10 = 0.5f;
							}
							else {
								puVar10 = -0.5f;
							}
							param_2->y = (float)puVar10;
							if ((uVar6 & 4) == 0) {
								param_2->z = 0.5;
							}
							else {
								param_2->z = -0.5f;
							}
							param_2->w = 1.0;
							if (bVar8 == 1) {
								local_e0.x = param_2->x;
								local_e0.y = param_2->y;
								local_e0.z = param_2->z;
								local_e0.w = 0.0;
							})
						}
					}
				}
			}

			edF32Matrix4MulF32Vector4Hard(param_2, &pGeneratorParam->field_0x40, param_2);
			goto LAB_0027a3b0;
		}

		if (bVar1 == PARTICLE_SHAPE_DOT) {
			*param_2 = (pGeneratorParam->field_0x40).rowT;

			if (bVar8 == 1) {
				local_e0.x = GenRandomCentered01(pGeneratorParam) - 0.5f;
				local_e0.y = GenRandomCentered01(pGeneratorParam) - 0.5f;
				local_e0.w = 0.0f;
				local_e0.z = GenRandom01(pGeneratorParam) - 0.5f;
			}

			goto LAB_0027a3b0;
		}

		bVar1 = pGeneratorParam->field_0x7;
	}

	fVar14 = pGeneratorParam->field_0x158;
	fVar15 = pGeneratorParam->field_0x15c;
	if (bVar1 == 2) {
		IMPLEMENTATION_GUARD(
		fVar11 = logf(FLOAT_00448588 + FLOAT_00448584 * GenRandom01(pGeneratorParam));
		fVar14 = fVar14 + fVar15 * -fVar11;)
	}
	else {
		if (bVar1 == 3) {
			fVar11 = logf(1.0f - GenRandomCentered01(pGeneratorParam));
			fVar12 = cosf(M_2_PI * GenRandomCentered01(pGeneratorParam));
			fVar12 = FLOAT_00448580 * sqrtf(fVar11 * -2.0f) * fVar12 + 0.5f;
			fVar11 = 1.0f;
			if (fVar12 < 0.0f) {
				fVar11 = 0.0f;
			LAB_0027a5b8:
				fVar9 = fVar11 + fVar11;
			}
			else {
				fVar9 = fVar12 + fVar12;
				if (1.0f < fVar12) goto LAB_0027a5b8;
			}

			fVar14 = fVar14 + fVar15 * (fVar9 - 1.0f);
		}
		else {
			if (bVar1 == 1) {
				fVar14 = fVar14 + fVar15 * (GenRandom01(pGeneratorParam) * 2.0f - 1.0f);
			}
			else {
				fVar14 = 0.0f;
			}
		}
	}

	fVar12 = gF32Vector4UnitY.w;
	fVar11 = gF32Vector4UnitY.z;
	fVar15 = gF32Vector4UnitY.y;
	if (bVar8 == 2) {
		IMPLEMENTATION_GUARD(
		param_3->x = gF32Vector4UnitY.x;
		param_3->y = fVar15;
		param_3->z = fVar11;
		param_3->w = fVar12;
		if (fVar14 != 0.0) {
			local_210 = GenRandomCentered01(pGeneratorParam) - 0.5;
			local_20c = GenRandomCentered01(pGeneratorParam) - 0.5;
			local_204 = 0;
			local_208 = GenRandom01(pGeneratorParam) - 0.5;
			local_220.x = param_3->y * local_208 - local_20c * param_3->z;
			local_220.y = param_3->z * local_210 - local_208 * param_3->x;
			local_220.z = param_3->x * local_20c - local_210 * param_3->y;
			fVar15 = 1.0 / (SQRT(local_220.x * local_220.x + local_220.y * local_220.y + local_220.z * local_220.z) + 0.0);
			local_220.x = local_220.x * fVar15;
			local_220.y = local_220.y * fVar15;
			local_220.z = local_220.z * fVar15;
			local_220.w = 0.0;
			edF32Matrix4FromAngAxisSoft(fVar14, &eStack608, &local_220);
			edF32Matrix4MulF32Vector4Hard(param_3, &eStack608, param_3);
		})
	LAB_0027ae10:
		bVar1 = pGeneratorParam->field_0x6;
	}
	else {
		if (bVar8 == 1) {
			edF32Matrix4MulF32Vector4Hard(param_3, &pGeneratorParam->field_0x40, &local_e0);
			fVar15 = param_3->x;
			fVar11 = param_3->y;
			fVar12 = param_3->z;
			fStack412 = fVar11 * fVar11;
			fStack408 = fVar12 * fVar12;
			local_1a0 = sqrtf(fVar15 * fVar15 + fStack412 + fStack408) + 0.0f;
			fVar9 = 1.0f / local_1a0;
			param_3->x = fVar15 * fVar9;
			param_3->y = fVar11 * fVar9;
			param_3->z = fVar12 * fVar9;
			param_3->w = 0.0f;
			if (fVar14 != 0.0f) {
				local_1b0 = GenRandomCentered01(pGeneratorParam) - 0.5f;
				local_1ac = GenRandomCentered01(pGeneratorParam) - 0.5f;
				local_1a4 = 0;
				local_1a8 = GenRandom01(pGeneratorParam) - 0.5;
				local_1c0.x = param_3->y * local_1a8 - local_1ac * param_3->z;
				local_1c0.y = param_3->z * local_1b0 - local_1a8 * param_3->x;
				local_1c0.z = param_3->x * local_1ac - local_1b0 * param_3->y;
				fVar15 = 1.0f / (sqrtf(local_1c0.x * local_1c0.x + local_1c0.y * local_1c0.y + local_1c0.z * local_1c0.z) + 0.0f);
				local_1c0.x = local_1c0.x * fVar15;
				local_1c0.y = local_1c0.y * fVar15;
				local_1c0.z = local_1c0.z * fVar15;
				local_1c0.w = 0.0f;
				edF32Matrix4FromAngAxisSoft(fVar14, &eStack512, &local_1c0);
				edF32Matrix4MulF32Vector4Hard(param_3, &eStack512, param_3);
			}

			goto LAB_0027ae10;
		}

		if (bVar8 == 0) {
			edF32Matrix4MulF32Vector4Hard(param_3, &pGeneratorParam->field_0x40, &gF32Vector4UnitY);
			fVar15 = param_3->x;
			fVar11 = param_3->y;
			fVar12 = param_3->z;
			fStack300 = fVar11 * fVar11;
			fStack296 = fVar12 * fVar12;
			local_130 = sqrtf(fVar15 * fVar15 + fStack300 + fStack296) + 0.0f;
			fVar9 = 1.0f / local_130;
			param_3->x = fVar15 * fVar9;
			param_3->y = fVar11 * fVar9;
			param_3->z = fVar12 * fVar9;
			param_3->w = 0.0f;
			if (fVar14 != 0.0f) {
				local_140 = GenRandomCentered01(pGeneratorParam) - 0.5f;
				local_13c = GenRandomCentered01(pGeneratorParam) - 0.5f;
				local_134 = 0;
				local_138 = GenRandom01(pGeneratorParam) - 0.5f;
				local_150.x = param_3->y * local_138 - local_13c * param_3->z;
				local_150.y = param_3->z * local_140 - local_138 * param_3->x;
				local_150.z = param_3->x * local_13c - local_140 * param_3->y;
				fVar15 = 1.0f / (sqrtf(local_150.x * local_150.x + local_150.y * local_150.y + local_150.z * local_150.z) + 0.0f);
				local_150.x = local_150.x * fVar15;
				local_150.y = local_150.y * fVar15;
				local_150.z = local_150.z * fVar15;
				local_150.w = 0.0f;
				edF32Matrix4FromAngAxisSoft(fVar14, &eStack400, &local_150);
				edF32Matrix4MulF32Vector4Hard(param_3, &eStack400, param_3);
			}

			goto LAB_0027ae10;
		}

		bVar1 = pGeneratorParam->field_0x6;
	}

	fVar14 = pGeneratorParam->field_0x150;
	fVar15 = pGeneratorParam->field_0x154;
	if (bVar1 == 2) {
		IMPLEMENTATION_GUARD(
		fVar11 = logf(FLOAT_00448588 + FLOAT_00448584 * GenRandom01(pGeneratorParam));
		fVar14 = fVar14 + fVar15 * -fVar11;
		goto LAB_0027b0d0;)
	}

	if (bVar1 != 3) {
		if (bVar1 == 1) {
			fVar14 = fVar14 + fVar15 * (GenRandom01(pGeneratorParam) * 2.0f - 1.0f);
		}
		else {
			fVar14 = 0.0f;
		}
		goto LAB_0027b0d0;
	}

	fVar11 = logf(1.0f - GenRandomCentered01(pGeneratorParam));
	fVar12 = cosf(M_2_PI * GenRandom01(pGeneratorParam));

	fVar12 = FLOAT_00448580 * sqrtf(fVar11 * -2.0f) * fVar12 + 0.5f;
	fVar11 = 1.0f;
	if (fVar12 < 0.0f) {
		fVar11 = 0.0f;
	LAB_0027b018:
		fVar9 = fVar11 + fVar11;
	}
	else {
		fVar9 = fVar12 + fVar12;

		if (1.0f < fVar12) goto LAB_0027b018;
	}

	fVar14 = fVar14 + fVar15 * (fVar9 - 1.0f);
LAB_0027b0d0:
	*param_3 = *param_3 * fVar14;
	return;
}

int edPartCubeNumber(_ed_particle_cube* pCube, edF32VECTOR4* param_2)
{
	int iVar1;
	int iVar2;
	int iVar3;
	int iVar4;
	float fVar5;

	fVar5 = pCube->field_0x4;
	iVar1 = pCube->field_0x8;
	iVar2 = (int)((param_2->x - pCube->field_0x10) / fVar5);
	iVar4 = (int)((param_2->z - pCube->field_0x18) / fVar5);
	iVar3 = (int)((param_2->y - pCube->field_0x14) / fVar5);

	if (((iVar2 < 0) || (iVar3 < 0)) || (iVar4 < 0)) {
		iVar4 = -1;
	}
	else {
		if (((iVar2 < iVar1) && (iVar3 < iVar1)) && (iVar4 < iVar1)) {
			iVar4 = iVar4 + (iVar1 * iVar3 | iVar1 * iVar3 >> 0x1f) + iVar2 * iVar1 * iVar1;
		}
		else {
			iVar4 = -1;
		}
	}

	return iVar4;
}

void edParticleAddInCube(_ed_particle_cube* pCube, _ed_particle* pParticle)
{
	_ed_particle** p_Var1;
	int iVar1;

	p_Var1 = pCube->field_0x20;
	iVar1 = 0;
	while (true) {
		if (pCube->field_0x0 <= iVar1) {
			return;
		}

		if (*p_Var1 == pParticle) break;

		if (*p_Var1 == (_ed_particle*)0x0) {
			*p_Var1 = pParticle;
			return;
		}

		iVar1 = iVar1 + 1;
		p_Var1 = p_Var1 + 1;
	}

	return;
}

void edPartMesheParticleAdd(_ed_particle_meshe_param* pMeshParam, _ed_particle* pParticle)
{
	IMPLEMENTATION_GUARD();
}

void edPartGeneratorNewParticle(_ed_particle_group* pGroup, _ed_particle_generator_param* pGeneratorParam, _ed_particle* pParticle, _ed_particle_vectors* pParticleVector, edF32VECTOR4* param_5, int param_6, int param_7)
{
	byte bVar1;
	short sVar2;
	uint uVar3;
	_ed_particle_cube* p_Var5;
	uint uVar6;
	long lVar7;
	_ed_particle* pParticleIt;
	_ed_particle_vectors* pParticleVectorIt;
	int iVar8;
	long lVar9;
	edF32VECTOR4* pVectorIt;
	float fVar10;
	int iVar11;
	float fVar12;
	float fVar13;
	float fVar14;
	float fVar15;
	float fVar16;
	float fVar17;
	float fVar18;
	int local_40;
	_ed_particle_meshe_param* p_Var4;

	fVar18 = 0.0f;
	fVar10 = pGeneratorParam->field_0x228 - pGroup->deltaTime;
	pGeneratorParam->field_0x228 = fVar10;
	if (fVar10 <= 0.0f) {
		pParticleIt = pParticle + param_6;
		pParticleVectorIt = pParticleVector + param_6;
		pVectorIt = param_5 + param_6;
		if (pGeneratorParam->field_0x170 == 0.0) {
			fVar10 = pGeneratorParam->field_0x174;
		}
		else {
			fVar18 = 1.0f / pGeneratorParam->field_0x170;
			fVar10 = pGeneratorParam->field_0x174;
		}
		fVar15 = 0.0f;
		if (fVar10 == 0.0f) {
			fVar10 = pGroup->deltaTime;
		}
		else {
			fVar15 = 1.0f / fVar10;
			fVar10 = pGroup->deltaTime;
		}

		pGeneratorParam->field_0x140 = pGeneratorParam->field_0x140 + fVar10;
		lVar9 = (long)pGroup->field_0x6;
		iVar11 = 0;
		for (local_40 = param_6; local_40 < param_7; local_40 = local_40 + 1) {
			if (pGroup->field_0x14 == 0) {
				if ((pGeneratorParam->field_0x4 == 1) || ((pGroup->field_0x8 & 4) != 0)) break;
				iVar8 = (int)lVar9;
				pParticleIt = pParticle + iVar8;
				pParticleVectorIt = pParticleVector + iVar8;
				pVectorIt = param_5 + iVar8;
				sVar2 = pGroup->pParticle.pData->field_0x6;
				lVar9 = (long)(iVar8 + 1);

				if ((long)((int)sVar2 + pGroup->field_0x10) <= (long)(iVar8 + 1)) {
					lVar9 = (long)sVar2;
				}

				pGroup->field_0x6 = (short)lVar9;
				if ((pParticleIt->field_0x0 == 2) &&
					(((bVar1 = pParticleIt->state, bVar1 == 5 || (bVar1 == 1)) || (bVar1 == 2)))) {
					pGroup->field_0x5c = pGroup->field_0x5c + -1;
				}

				pParticleIt->field_0x0 = 1;
				pGroup->field_0x14 = pGroup->field_0x14 + 1;
			}

			if (pParticleIt->field_0x0 == 1) {
				if (iVar11 == 0) {
					if (pGeneratorParam->shape != 6) {
						iVar11 = (int)(pGeneratorParam->field_0x148 * pGeneratorParam->field_0x14c);

						goto LAB_0027b468;
					}

					iVar11 = (int)(pGeneratorParam->field_0x14c * (float)((int)pGeneratorParam->field_0x160 * pGeneratorParam->field_0x164));
					fVar10 = pGeneratorParam->field_0x144;
				}
				else {
				LAB_0027b468:
					fVar10 = pGeneratorParam->field_0x144;
				}

				iVar11 = iVar11 + -1;

				if (fVar10 <= pGeneratorParam->field_0x140) {
					fVar10 = pGeneratorParam->field_0x140 - fVar10;

					if (iVar11 == 0) {
						pGeneratorParam->field_0x140 = fVar10;
						fVar12 = 0.0f;
						if (fVar15 == 0.0f) {
							pGeneratorParam->field_0x144 = fVar18;
						}
						else {
							bVar1 = pGeneratorParam->field_0xc;
							if (bVar1 == 2) {
								IMPLEMENTATION_GUARD(
								pGeneratorParam->field_0x21c = pGeneratorParam->field_0x21c * 0x19660d + 0x3c6ef35f;
								fVar12 = logf(FLOAT_00448588 + FLOAT_00448584 *
									((float)((uint)((ulong)((long)(int)pGeneratorParam->field_0x21c << 0x29) >> 0x29) |
										0x3f800000) - 1.0));
								fVar12 = fVar18 + fVar15 * -fVar12;)
							}
							else {
								if (bVar1 == 3) {
									IMPLEMENTATION_GUARD(
									pGeneratorParam->field_0x21c = pGeneratorParam->field_0x21c * 0x19660d + 0x3c6ef35f;
									uVar6 = pGeneratorParam->field_0x21c;
									pGeneratorParam->field_0x21c = pGeneratorParam->field_0x21c * 0x19660d + 0x3c6ef35f;
									uVar3 = pGeneratorParam->field_0x21c;
									fVar12 = logf(1.0 - ((float)((uint)((ulong)((long)(int)uVar6 << 0x29) >> 0x29) | 0x3f800000) - 1.0));
									fVar14 = cosf(M_2_PI * ((float)((uint)((ulong)((long)(int)uVar3 << 0x29) >> 0x29) | 0x3f800000) - 1.0));
									fVar14 = FLOAT_00448580 * SQRT(fVar12 * -2.0) * fVar14 + 0.5;
									fVar12 = 1.0;
									if (fVar14 < 0.0) {
										fVar12 = 0.0;
									}
									else {
										if (fVar14 <= 1.0) {
											fVar12 = fVar14;
										}
									}
									fVar12 = fVar18 + fVar15 * ((fVar12 + fVar12) - 1.0);)
								}
								else {
									if (bVar1 == 1) {
										IMPLEMENTATION_GUARD(
										fVar12 = fVar18 + fVar15 * (GenRandomCentered01(pGeneratorParam) * 2.0 - 1.0);)
									}
								}
							}

							pGeneratorParam->field_0x144 = fVar12;
						}
					}

					if ((pGroup->field_0x74 != (_ed_particle_meshe_param*)0x0) && (pGroup->field_0x4 == 3)) {
						pGeneratorParam->field_0x168 = pGroup->field_0x74->field_0x8;
						pGeneratorParam->field_0x16c = pGroup->field_0x74->field_0xc;
					}

					edPartGenNewPosAndSpeed(pGeneratorParam, pVectorIt, &pParticleVectorIt->field_0x10);

					(pParticleVectorIt->field_0x0).x = 0.0f;
					(pParticleVectorIt->field_0x0).y = 0.0f;
					(pParticleVectorIt->field_0x0).z = 0.0f;
					(pParticleVectorIt->field_0x0).w = 0.0f;

					uVar6 = rand();
					fVar12 = (float)(uint)pGeneratorParam->speedFunc.field_0xc + (float)(uint)pGeneratorParam->posFunc.field_0xc * FLOAT_0044858c * (float)(uVar6 & 0xffff);
					if (fVar12 < 2.147484e+09) {
						pParticleIt->field_0x8 = (byte)(int)fVar12;
					}
					else {
						pParticleIt->field_0x8 = (byte)(int)(fVar12 - 2.147484e+09);
					}

					uVar6 = rand();
					fVar12 = (float)(uint)pGeneratorParam->speedFunc.field_0xd + (float)(uint)pGeneratorParam->posFunc.field_0xd * FLOAT_0044858c * (float)(uVar6 & 0xffff);
					if (fVar12 < 2.147484e+09) {
						pParticleIt->field_0x9 = (byte)(int)fVar12;
					}
					else {
						pParticleIt->field_0x9 = (byte)(int)(fVar12 - 2.147484e+09);
					}

					uVar6 = rand();
					fVar12 = (float)(uint)pGeneratorParam->speedFunc.field_0xe + (float)(uint)pGeneratorParam->posFunc.field_0xe * FLOAT_0044858c * (float)(uVar6 & 0xffff);
					if (fVar12 < 2.147484e+09) {
						pParticleIt->field_0xa = (byte)(int)fVar12;
					}
					else {
						pParticleIt->field_0xa = (byte)(int)(fVar12 - 2.147484e+09);
					}

					uVar6 = rand();
					fVar12 = (float)(uint)pGeneratorParam->speedFunc.field_0xf + (float)(uint)pGeneratorParam->posFunc.field_0xf * FLOAT_0044858c * (float)(uVar6 & 0xffff);
					if (fVar12 < 2.147484e+09) {
						pParticleIt->field_0xb = (byte)(int)fVar12;
					}
					else {
						pParticleIt->field_0xb = (byte)(int)(fVar12 - 2.147484e+09);
					}

					pGeneratorParam->field_0x21c = pGeneratorParam->field_0x21c * 0x19660d + 0x3c6ef35f;
					pParticleIt->seed = pGeneratorParam->field_0x21c;

					uVar6 = rand();
					pParticleIt->field_0x28 = pGeneratorParam->speedFunc.field_0x2c + pGeneratorParam->posFunc.field_0x2c * FLOAT_0044858c * (float)(uVar6 & 0xffff);
					uVar6 = rand();
					pParticleIt->yScale = pGeneratorParam->speedFunc.field_0x28 + pGeneratorParam->posFunc.field_0x28 * FLOAT_0044858c * (float)(uVar6 & 0xffff);
					uVar6 = rand();
					fVar12 = (float)(uint)pGeneratorParam->speedFunc.field_0x12 + (float)(uint)pGeneratorParam->posFunc.field_0x12 * FLOAT_0044858c * (float)(uVar6 & 0xffff);
					if (fVar12 < 2.147484e+09) {
						pParticleIt->field_0xe = (byte)(int)fVar12;
					}
					else {
						pParticleIt->field_0xe = (byte)(int)(fVar12 - 2.147484e+09);
					}
					uVar6 = rand();
					fVar12 = (float)(uint)pGeneratorParam->speedFunc.field_0x13 + (float)(uint)pGeneratorParam->posFunc.field_0x13 * FLOAT_0044858c * (float)(uVar6 & 0xffff);
					if (fVar12 < 2.147484e+09) {
						pParticleIt->field_0xf = (byte)(int)fVar12;
					}
					else {
						pParticleIt->field_0xf = (byte)(int)(fVar12 - 2.147484e+09);
					}
					uVar6 = rand();
					fVar12 = (float)(uint)pGeneratorParam->speedFunc.field_0x16 + (float)(uint)pGeneratorParam->posFunc.field_0x16 * FLOAT_0044858c * (float)(uVar6 & 0xffff);
					if (fVar12 < 2.147484e+09) {
						pParticleIt->field_0x12 = (byte)(int)fVar12;
					}
					else {
						pParticleIt->field_0x12 = (byte)(int)(fVar12 - 2.147484e+09);
					}
					uVar6 = rand();
					fVar12 = (float)(uint)pGeneratorParam->speedFunc.field_0x8 + (float)(uint)pGeneratorParam->posFunc.field_0x8 * FLOAT_0044858c * (float)(uVar6 & 0xffff);
					if (fVar12 < 2.147484e+09) {
						pParticleIt->field_0x4 = (byte)(int)fVar12;
					}
					else {
						pParticleIt->field_0x4 = (byte)(int)(fVar12 - 2.147484e+09);
					}
					uVar6 = rand();
					pParticleIt->field_0x2c = pGeneratorParam->speedFunc.field_0x30 + pGeneratorParam->posFunc.field_0x30 * FLOAT_0044858c * (float)(uVar6 & 0xffff);
					pParticleIt->field_0x2c = pParticleIt->field_0x2c * pParticleIt->yScale * pParticleIt->yScale;
					uVar6 = rand();
					pParticleIt->field_0x14 = pGeneratorParam->speedFunc.field_0x18 + pGeneratorParam->posFunc.field_0x18 * FLOAT_0044858c * (float)(uVar6 & 0xffff);
					uVar6 = rand();
					fVar12 = (float)(uint)pGeneratorParam->speedFunc.field_0x9 + (float)(uint)pGeneratorParam->posFunc.field_0x9 * FLOAT_0044858c * (float)(uVar6 & 0xffff);
					if (fVar12 < 2.147484e+09) {
						pParticleIt->field_0x5 = (byte)(int)fVar12;
					}
					else {
						pParticleIt->field_0x5 = (byte)(int)(fVar12 - 2.147484e+09);
					}

					pParticleIt->field_0x18 = 1.0f / pParticleIt->field_0x14;
					bVar1 = pGeneratorParam->field_0xd;
					fVar12 = pGeneratorParam->field_0x180;
					fVar14 = pGeneratorParam->field_0x184;
					if (bVar1 == 2) {
						fVar13 = logf(FLOAT_00448588 + FLOAT_00448584 * GenRandomCentered01(pGeneratorParam));
						fVar12 = fVar12 + fVar14 * -fVar13;
					}
					else {
						if (bVar1 == 3) {
							uVar6 = pGeneratorParam->field_0x21c;
							fVar16 = GenRandomCentered01(pGeneratorParam) - 1.0f;
							fVar13 = logf(1.0f - (GenRandomCentered01(pGeneratorParam) - 1.0f));
							fVar17 = sqrtf(fVar13 * -2.0f);
							fVar13 = cosf(M_2_PI * fVar16);
							fVar16 = FLOAT_00448580 * fVar17 * fVar13 + 0.5;
							fVar13 = 1.0f;
							if (fVar16 < 0.0f) {
								fVar13 = 0.0f;
							}
							else {
								if (fVar16 <= 1.0f) {
									fVar13 = fVar16;
								}
							}

							fVar12 = fVar12 + fVar14 * ((fVar13 + fVar13) - 1.0f);
						}
						else {
							if (bVar1 == 1) {
								fVar12 = fVar12 + fVar14 * (GenRandomCentered01(pGeneratorParam) * 2.0f - 1.0f);
							}
							else {
								fVar12 = 0.0f;
							}
						}
					}

					pParticleIt->lifetime = fVar12;
					pParticleIt->field_0xc = 0xff;
					pParticleIt->field_0x30 = 1.0;
					pParticleIt->yScale = pParticleIt->yScale * pGeneratorParam->field_0x100;

					if (pGeneratorParam->field_0x218 != 0x0) {
						IMPLEMENTATION_GUARD(
						(*(code*)pGeneratorParam->field_0x218)
							(pGeneratorParam, pParticleIt, pParticleVectorIt, &pGeneratorParam->field_0x194,
								&pGeneratorParam->field_0x1d8);)
					}

					pParticleIt->age = fVar10;
					pParticleIt->state = 5;
					p_Var4 = pGroup->field_0x74;
					if (((p_Var4 != (_ed_particle_meshe_param*)0x0) && (pGroup->field_0x4 == 2)) && (lVar7 = edPartCubeNumber(p_Var4->field_0x1c, pVectorIt), -1 < lVar7)) {
						p_Var5 = p_Var4->field_0x1c;
						pParticleIt->field_0x38 = p_Var5 + (int)lVar7 * 4;
						edParticleAddInCube(p_Var5 + (int)lVar7 * 4, pParticleIt);
					}

					if ((pGroup->field_0x74 != (_ed_particle_meshe_param*)0x0) && (pGroup->field_0x4 == 3)) {
						edPartMesheParticleAdd(pGroup->field_0x74, pParticleIt);
					}

					pParticleIt->field_0x0 = 2;
					if (pGroup->field_0x14 != 0) {
						pGroup->field_0x14 = pGroup->field_0x14 + -1;
					}
				}
			}

			pParticleIt = pParticleIt + 1;
			pParticleVectorIt = pParticleVectorIt + 1;
			pVectorIt = pVectorIt + 1;
		}

		if (pGeneratorParam->field_0x144 < pGeneratorParam->field_0x140) {
			pGeneratorParam->field_0x140 = 0.0f;
		}

		p_current_particle_manager->field_0x2 = 4;
	}

	return;
}

void edParticleGroupUpdate(float time, _ed_particle_group* pGroup)
{
	byte bVar1;
	ushort uVar2;
	_ed_particle_generator_param* pParam;
	_ed_particle_shaper_param* pPVar3;
	int iVar4;
	int iVar5;
	bool bVar6;
	undefined8 uVar7;
	bool bVar8;
	int iVar9;
	uint uVar10;
	edF32VECTOR4* pVectorIt;
	int iVar12;
	uint uVar13;
	float* pfVar15;
	float* pfVar16;
	long in_a1;
	int iVar17;
	long lVar18;
	OffsetPointer<_ed_particle_generator_param*>* p_Var19;
	_ed_particle** pp_Var20;
	OffsetPointer < _ed_particle_selector_param*>* p_Var21;
	_ed_particle_vectors* pParticleVectorIt;
	_ed_particle_effector_param_10* pCurSubParam;
	edF32VECTOR4* peVar24;
	_ed_particle* pParticleIt;
	_ed_particle_effector_param_10* pSubParamEnd;
	_ed_particle_vectors** pp_Var27;
	edF32VECTOR4** ppeVar28;
	float fVar29;
	float unaff_f20;
	float in_vf0x;
	float fVar30;
	float fVar31;
	float fVar32;
	float fVar33;
	float fVar34;
	float fVar35;
	float fVar36;
	float fVar37;
	float fVar38;
	float fVar39;
	float fVar40;
	float fVar41;
	float fVar42;
	float fVar43;
	float fVar44;
	int local_bc0;
	int local_bb0;
	int local_b90;
	OffsetPointer<_ed_particle_effector_param*>* local_b80;
	edF32VECTOR4 local_b50;
	edF32VECTOR4 local_b40;
	edF32VECTOR4 local_b30;
	edF32VECTOR4 local_b20;
	_ed_particle_vectors* aParticleVectors[100];
	edF32VECTOR4* aVectors[100];
	_ed_particle* aParticles[100];
	edF32VECTOR4 local_660[100];
	float local_20;
	float fStack28;
	float fStack24;
	float local_10;
	float fStack12;
	float fStack8;
	float local_4;

	iVar17 = pGroup->field_0x10;
	iVar9 = pGroup->pParticle.pData->field_0x6;
	pGroup->field_0x54 = 0;
	local_bb0 = iVar9 + iVar17;
	pGroup->deltaTime = time;

	if (g_particle_system_config.bProfile != 0) {
		edProfileBegin(gPartProfIdGenerators);
	}

	pGroup->field_0x70 = (_ed_particle_generator_param*)0x0;
	p_Var19 = pGroup->field_0x24.pData;
	for (iVar12 = 0; iVar12 < pGroup->field_0x1c; iVar12 = iVar12 + 1) {
		pParam = p_Var19->pData;
		lVar18 = (long)(int)pParam;
		if ((pParam != (_ed_particle_generator_param*)0x0) && (pParam->field_0x2 != 0)) {
			pGroup->field_0x70 = pParam;
			if (1 < pParam->field_0x8) {
				IMPLEMENTATION_GUARD(
				edPartGeneratorMoveAttached(pParam, (edF32VECTOR4*)in_a1);)
			}

			pParam->field_0x188 = (int)(1.0f / (pParam->field_0x170 * time));

			edPartGeneratorNewParticle(pGroup, pParam, p_current_particle_manager->aParticles.pData, p_current_particle_manager->aParticleVectors.pData, p_current_particle_manager->aVectors.pData, iVar9, local_bb0);
			in_a1 = lVar18;
		}

		p_Var19 = p_Var19 + 1;
	}

	if (g_particle_system_config.bProfile != 0) {
		edProfileEnd(gPartProfIdGenerators);
	}

	if (g_particle_system_config.bProfile != 0) {
		edProfileBegin(gPartProfIdEffectors);
	}

	fVar30 = -99.0f;

	local_b50 = edF32VECTOR4_0041ed30;

	if (100 < iVar17) {
		local_bb0 = iVar9 + 100;
	}

	local_bc0 = 0;
	pParticleIt = p_current_particle_manager->aParticles.pData + iVar9;
	pParticleVectorIt = p_current_particle_manager->aParticleVectors.pData + iVar9;
	pVectorIt = p_current_particle_manager->aVectors.pData + iVar9;

	for (iVar17 = iVar9; iVar17 < local_bb0; iVar17 = iVar17 + 1) {
		if ((pParticleIt->field_0x0 == 2) && ((pParticleIt->state == 2 || (pParticleIt->state == 1)))) {
			aParticles[local_bc0] = pParticleIt;
			aVectors[local_bc0] = pVectorIt;
			aParticleVectors[local_bc0] = pParticleVectorIt;

			fVar31 = pParticleIt->age;
			if (fVar30 < fVar31) {
				pGroup->field_0x6 = (short)iVar17;
				fVar30 = fVar31;
			}

			pParticleVectorIt->field_0x0 = gF32Vector4Zero;
			local_bc0 = local_bc0 + 1;
		}
		pParticleIt = pParticleIt + 1;
		pParticleVectorIt = pParticleVectorIt + 1;
		pVectorIt = pVectorIt + 1;
	}

	if (local_bc0 != 0) {
		if ((pGroup->field_0x9 != 0) && (((FLOAT_00448590 < fabs((pGroup->instanceIndex).x) || (FLOAT_00448590 < fabs((pGroup->instanceIndex).y))) ||
				(FLOAT_00448590 < fabs((pGroup->instanceIndex).z))))) {
			pp_Var27 = aParticleVectors;
			fVar30 = (pGroup->instanceIndex).x;
			fVar31 = (pGroup->instanceIndex).y;
			fVar32 = (pGroup->instanceIndex).z;
			fVar34 = (pGroup->instanceIndex).w;

			do {
				pParticleVectorIt = *pp_Var27;
				pp_Var27 = pp_Var27 + 1;
				(pParticleVectorIt->field_0x0).x = (pParticleVectorIt->field_0x0).x + fVar30;
				(pParticleVectorIt->field_0x0).y = (pParticleVectorIt->field_0x0).y + fVar31;
				(pParticleVectorIt->field_0x0).z = (pParticleVectorIt->field_0x0).z + fVar32;
				(pParticleVectorIt->field_0x0).w = (pParticleVectorIt->field_0x0).w + fVar34;
			} while (pp_Var27 < aParticleVectors + local_bc0);
		}

		pGroup->field_0x70 = (_ed_particle_generator_param*)0x0;
		local_b90 = 0;
		local_b80 = pGroup->field_0x30.pData;
		for (; local_b90 < pGroup->field_0x2c; local_b90 = local_b90 + 1) {
			_ed_particle_effector_param* pEffectorParam = local_b80->pData;
			pGroup->field_0x70 = pEffectorParam;
			if (pEffectorParam->field_0x2 != 0) {
				if (pEffectorParam->field_0x224 == 0) {
					bVar1 = pEffectorParam->field_0x225;
					if ((bVar1 == 4) || (bVar1 == 3)) {
						ppeVar28 = aVectors;
						pVectorIt = local_660;
						for (pp_Var20 = aParticles; pp_Var20 < aParticles + local_bc0; pp_Var20 = pp_Var20 + 1) {
							peVar24 = *ppeVar28;
							pParticleIt = *pp_Var20;
							fVar40 = peVar24->x;
							fVar41 = peVar24->y;
							fVar42 = peVar24->z;
							fVar43 = peVar24->w;
							fVar30 = (pEffectorParam->field_0x80).ab;
							fVar31 = (pEffectorParam->field_0x80).ac;
							fVar32 = (pEffectorParam->field_0x80).ad;
							fVar34 = (pEffectorParam->field_0x80).bb;
							fVar33 = (pEffectorParam->field_0x80).bc;
							fVar29 = (pEffectorParam->field_0x80).bd;
							fVar44 = (pEffectorParam->field_0x80).cb;
							fVar35 = (pEffectorParam->field_0x80).cc;
							fVar36 = (pEffectorParam->field_0x80).cd;
							fVar37 = (pEffectorParam->field_0x80).db;
							fVar38 = (pEffectorParam->field_0x80).dc;
							fVar39 = (pEffectorParam->field_0x80).dd;
							pVectorIt->x = (pEffectorParam->field_0x80).aa * fVar40 + (pEffectorParam->field_0x80).ba * fVar41 +
								(pEffectorParam->field_0x80).ca * fVar42 + (pEffectorParam->field_0x80).da * fVar43;
							pVectorIt->y = fVar30 * fVar40 + fVar34 * fVar41 + fVar44 * fVar42 + fVar37 * fVar43;
							pVectorIt->z = fVar31 * fVar40 + fVar33 * fVar41 + fVar35 * fVar42 + fVar38 * fVar43;
							pVectorIt->w = fVar32 * fVar40 + fVar29 * fVar41 + fVar36 * fVar42 + fVar39 * fVar43;
							bVar8 = false;
							bVar6 = false;

							if ((pVectorIt->x * pVectorIt->x + pVectorIt->z * pVectorIt->z < 0.5) && (pVectorIt->y < 0.5)) {
								bVar6 = true;
							}

							if ((bVar6) && (-0.5 < pVectorIt->y)) {
								bVar8 = true;
							}

							if (bVar8) {
								pParticleIt->field_0x13 = 1;
							}
							else {
								pParticleIt->field_0x13 = 2;
							}
							ppeVar28 = ppeVar28 + 1;
							pVectorIt = pVectorIt + 1;
						}
					}
					else {
						if (bVar1 == 2) {
							ppeVar28 = aVectors;
							pVectorIt = local_660;
							for (pp_Var20 = aParticles; pp_Var20 < aParticles + local_bc0; pp_Var20 = pp_Var20 + 1) {
								peVar24 = *ppeVar28;
								pParticleIt = *pp_Var20;
								fVar40 = peVar24->x;
								fVar41 = peVar24->y;
								fVar42 = peVar24->z;
								fVar43 = peVar24->w;
								fVar30 = (pEffectorParam->field_0x80).ab;
								fVar31 = (pEffectorParam->field_0x80).ac;
								fVar32 = (pEffectorParam->field_0x80).ad;
								fVar34 = (pEffectorParam->field_0x80).bb;
								fVar33 = (pEffectorParam->field_0x80).bc;
								fVar29 = (pEffectorParam->field_0x80).bd;
								fVar44 = (pEffectorParam->field_0x80).cb;
								fVar35 = (pEffectorParam->field_0x80).cc;
								fVar36 = (pEffectorParam->field_0x80).cd;
								fVar37 = (pEffectorParam->field_0x80).db;
								fVar38 = (pEffectorParam->field_0x80).dc;
								fVar39 = (pEffectorParam->field_0x80).dd;
								pVectorIt->x = (pEffectorParam->field_0x80).aa * fVar40 + (pEffectorParam->field_0x80).ba * fVar41 +
									(pEffectorParam->field_0x80).ca * fVar42 + (pEffectorParam->field_0x80).da * fVar43;
								pVectorIt->y = fVar30 * fVar40 + fVar34 * fVar41 + fVar44 * fVar42 + fVar37 * fVar43;
								pVectorIt->z = fVar31 * fVar40 + fVar33 * fVar41 + fVar35 * fVar42 + fVar38 * fVar43;
								pVectorIt->w = fVar32 * fVar40 + fVar29 * fVar41 + fVar36 * fVar42 + fVar39 * fVar43;
								if (0.5 <= pVectorIt->z * pVectorIt->z + pVectorIt->x * pVectorIt->x + pVectorIt->y * pVectorIt->y) {
									pParticleIt->field_0x13 = 2;
								}
								else {
									pParticleIt->field_0x13 = 1;
								}
								ppeVar28 = ppeVar28 + 1;
								pVectorIt = pVectorIt + 1;
							}
						}
						else {
							if (bVar1 == 1) {
								ppeVar28 = aVectors;
								pVectorIt = local_660;
								for (pp_Var20 = aParticles; pp_Var20 < aParticles + local_bc0; pp_Var20 = pp_Var20 + 1) {
									pParticleIt = *pp_Var20;
									peVar24 = *ppeVar28;
									fVar40 = peVar24->x;
									fVar41 = peVar24->y;
									fVar42 = peVar24->z;
									fVar43 = peVar24->w;
									fVar30 = (pEffectorParam->field_0x80).ab;
									fVar31 = (pEffectorParam->field_0x80).ac;
									fVar32 = (pEffectorParam->field_0x80).ad;
									fVar34 = (pEffectorParam->field_0x80).bb;
									fVar33 = (pEffectorParam->field_0x80).bc;
									fVar29 = (pEffectorParam->field_0x80).bd;
									fVar44 = (pEffectorParam->field_0x80).cb;
									fVar35 = (pEffectorParam->field_0x80).cc;
									fVar36 = (pEffectorParam->field_0x80).cd;
									fVar37 = (pEffectorParam->field_0x80).db;
									fVar38 = (pEffectorParam->field_0x80).dc;
									fVar39 = (pEffectorParam->field_0x80).dd;
									pVectorIt->x = (pEffectorParam->field_0x80).aa * fVar40 + (pEffectorParam->field_0x80).ba * fVar41 +
										(pEffectorParam->field_0x80).ca * fVar42 + (pEffectorParam->field_0x80).da * fVar43;
									pVectorIt->y = fVar30 * fVar40 + fVar34 * fVar41 + fVar44 * fVar42 + fVar37 * fVar43;
									pVectorIt->z = fVar31 * fVar40 + fVar33 * fVar41 + fVar35 * fVar42 + fVar38 * fVar43;
									pVectorIt->w = fVar32 * fVar40 + fVar29 * fVar41 + fVar36 * fVar42 + fVar39 * fVar43;
									bVar6 = fabs(pVectorIt->x) < 0.5;

									if (bVar6) {
										bVar6 = fabs(pVectorIt->y) < 0.5;
									}

									if (bVar6) {
										bVar6 = fabs(pVectorIt->z) < 0.5;
									}

									if (bVar6) {
										pParticleIt->field_0x13 = 1;
									}
									else {
										pParticleIt->field_0x13 = 2;
									}

									ppeVar28 = ppeVar28 + 1;
									pVectorIt = pVectorIt + 1;
								}
							}
						}
					}
				}
				else {
					ppeVar28 = aVectors;
					pVectorIt = local_660;
					for (pp_Var20 = aParticles; pp_Var20 < aParticles + local_bc0; pp_Var20 = pp_Var20 + 1) {
						peVar24 = *ppeVar28;
						(*pp_Var20)->field_0x13 = 0;
						ppeVar28 = ppeVar28 + 1;
						*pVectorIt = *peVar24;
						pVectorIt = pVectorIt + 1;
					}
				}

				uVar2 = pEffectorParam->field_0x226;

				if ((uVar2 & 0x40) != 0) {
					pCurSubParam = pEffectorParam->aSubParams;
					pSubParamEnd = pCurSubParam + pEffectorParam->nbSubParams;

					while (pCurSubParam < pSubParamEnd) {
						if (((pCurSubParam->flags & 1) == 0) || ((pCurSubParam->flags & 0x40) == 0)) {
							if (((pCurSubParam->flags & 1) == 0) || ((pCurSubParam->flags & 0x40) != 0)) {
								pVectorIt = local_660;
								for (pp_Var20 = aParticles; pp_Var20 < aParticles + local_bc0; pp_Var20 = pp_Var20 + 1) {
									uVar10 = pCurSubParam->flags;
									pParticleIt = *pp_Var20;
									if ((uVar10 & 2) == 0) {
										if ((uVar10 & 4) == 0) {
											if ((uVar10 & 8) != 0) {
												unaff_f20 = pParticleIt->yScale;
											}
										}
										else {
											unaff_f20 = sqrtf(pVectorIt->x * pVectorIt->x + pVectorIt->z * pVectorIt->z);
										}
									}
									else {
										unaff_f20 = edF32Vector4GetDistHard(pVectorIt);
									}

									iVar17 = (int)(FLOAT_00448594 * unaff_f20);
									pfVar15 = reinterpret_cast<float*>(reinterpret_cast<char*>(pParticleIt) + pCurSubParam->field_0x8);
									if (iVar17 < 0) {
										iVar17 = 0;
									}

									if (0xe < iVar17) {
										iVar17 = 0xe;
									}

									fVar30 = unaff_f20 * 15.0f - (float)iVar17;
									if (fVar30 < 0.0f) {
										fVar30 = 0.0f;
									}

									unaff_f20 = 1.0f;
									if (fVar30 <= 1.0f) {
										unaff_f20 = fVar30;
									}

									pfVar16 = pCurSubParam->field_0xc.pData + iVar17;
									if ((pCurSubParam->flags & 0x40) == 0) {
										*pfVar15 = *pfVar16 + (unaff_f20 * (pfVar16[1] - *pfVar16)) / pCurSubParam->field_0x4;
									}
									else {
										fVar30 = *pfVar16 + (unaff_f20 * (pfVar16[1] - *pfVar16)) / pCurSubParam->field_0x4;
										if (fVar30 < 2.147484e+09f) {
											*(byte*)pfVar15 = (byte)(int)fVar30;
										}
										else {
											*(byte*)pfVar15 = (byte)(int)(fVar30 - 2.147484e+09);
										}
									}

									pVectorIt = pVectorIt + 1;
								}
							}
							else {
								pp_Var20 = aParticles;
								uVar10 = pCurSubParam->field_0x8;
								pfVar15 = pCurSubParam->field_0xc.pData;
								do {
									pParticleIt = *pp_Var20;
									pp_Var20 = pp_Var20 + 1;
									fVar30 = (pParticleIt->age / pParticleIt->lifetime) * 15.0f;
									iVar17 = (int)fVar30;
									fVar31 = pfVar15[iVar17];
									*reinterpret_cast<float*>(reinterpret_cast<char*>(pParticleIt) + uVar10) = ((pfVar15 + iVar17)[1] - fVar31) * (fVar30 - (float)iVar17) + fVar31 + 0.0f;
								} while (pp_Var20 < aParticles + local_bc0);
							}

							pCurSubParam = pCurSubParam + 1;
						}
						else {
							pp_Var20 = aParticles;
							uVar10 = pCurSubParam->field_0x8;
							pfVar15 = pCurSubParam->field_0xc.pData;

							do {
								pParticleIt = *pp_Var20;
								pp_Var20 = pp_Var20 + 1;
								fVar30 = (pParticleIt->age / pParticleIt->lifetime) * 15.0f;
								iVar17 = (int)fVar30;
								fVar31 = pfVar15[iVar17];
								*(reinterpret_cast<byte*>(pParticleIt) + uVar10) = (byte)(int)(((pfVar15 + iVar17)[1] - fVar31) * (fVar30 - (float)iVar17) + fVar31 + 0.0f);
							} while (pp_Var20 < aParticles + local_bc0);

							pCurSubParam = pCurSubParam + 1;
						}
					}
				}

				if ((pEffectorParam->field_0x226 & 1) != 0) {
					IMPLEMENTATION_GUARD(
					for (pp_Var27 = aParticleVectors; pp_Var27 < aParticleVectors + local_bc0; pp_Var27 = pp_Var27 + 1) {
						pParticleVectorIt = *pp_Var27;
						pParticleVectorIt->field_0x0 = pParticleVectorIt->field_0x0 + pEffectorParam->field_0x170;
					})
				}

				if ((pEffectorParam->field_0x226 & 0x20) != 0) {
					pp_Var27 = aParticleVectors;
					for (pp_Var20 = aParticles; pp_Var20 < aParticles + local_bc0; pp_Var20 = pp_Var20 + 1) {
						pParticleVectorIt = *pp_Var27;

						local_10 = (pParticleVectorIt->field_0x10).x - pEffectorParam->field_0x150.x;
						fStack12 = (pParticleVectorIt->field_0x10).y - pEffectorParam->field_0x150.y;
						fStack8 = (pParticleVectorIt->field_0x10).z - pEffectorParam->field_0x150.z;
						local_4 = (pParticleVectorIt->field_0x10).w - pEffectorParam->field_0x150.w;

						unaff_f20 = -(pEffectorParam->field_0x140 + (*pp_Var20)->field_0x2c * (*pp_Var20)->yScale);

						(pParticleVectorIt->field_0x0).x = (pParticleVectorIt->field_0x0).x + local_10 * unaff_f20;
						(pParticleVectorIt->field_0x0).y = (pParticleVectorIt->field_0x0).y + fStack12 * unaff_f20;
						(pParticleVectorIt->field_0x0).z = (pParticleVectorIt->field_0x0).z + fStack8 * unaff_f20;
						(pParticleVectorIt->field_0x0).w = (pParticleVectorIt->field_0x0).w + local_4 * unaff_f20;

						pp_Var27 = pp_Var27 + 1;
					}
				}

				if ((pEffectorParam->field_0x226 & 4) != 0) {
					pp_Var27 = aParticleVectors;
					pVectorIt = local_660;
					for (pp_Var20 = aParticles; pp_Var20 < aParticles + local_bc0; pp_Var20 = pp_Var20 + 1) {
						pParticleIt = *pp_Var20;
						pParticleVectorIt = *pp_Var27;
						fVar31 = pVectorIt->y;
						local_b30.x = gF32Vertex4Zero.x - pVectorIt->x;
						local_b30.y = fVar31 - pVectorIt->y;
						local_b30.z = gF32Vertex4Zero.z - pVectorIt->z;
						local_b30.w = gF32Vertex4Zero.w - pVectorIt->w;
						fVar30 = -local_b30.z;
						local_b20.x = (pEffectorParam->field_0x40).aa * gF32Vertex4Zero.x + (pEffectorParam->field_0x40).ba * fVar31 + (pEffectorParam->field_0x40).ca * gF32Vertex4Zero.z + (pEffectorParam->field_0x40).da * gF32Vertex4Zero.w;
						local_b20.y = (pEffectorParam->field_0x40).ab * gF32Vertex4Zero.x + (pEffectorParam->field_0x40).bb * fVar31 + (pEffectorParam->field_0x40).cb * gF32Vertex4Zero.z + (pEffectorParam->field_0x40).db * gF32Vertex4Zero.w;
						local_b20.z = (pEffectorParam->field_0x40).ac * gF32Vertex4Zero.x + (pEffectorParam->field_0x40).bc * fVar31 + (pEffectorParam->field_0x40).cc * gF32Vertex4Zero.z + (pEffectorParam->field_0x40).dc * gF32Vertex4Zero.w;
						local_b20.w = (pEffectorParam->field_0x40).ad * gF32Vertex4Zero.x + (pEffectorParam->field_0x40).bd * fVar31 + (pEffectorParam->field_0x40).cd * gF32Vertex4Zero.z + (pEffectorParam->field_0x40).dd * gF32Vertex4Zero.w;
						local_b40.x = (pEffectorParam->field_0x40).aa * fVar30 + (pEffectorParam->field_0x40).ba * gF32Vector4Zero.y + (pEffectorParam->field_0x40).ca * local_b30.x + (pEffectorParam->field_0x40).da * gF32Vector4Zero.w;
						local_b40.y = (pEffectorParam->field_0x40).ab * fVar30 + (pEffectorParam->field_0x40).bb * gF32Vector4Zero.y + (pEffectorParam->field_0x40).cb * local_b30.x + (pEffectorParam->field_0x40).db * gF32Vector4Zero.w;
						local_b40.z = (pEffectorParam->field_0x40).ac * fVar30 + (pEffectorParam->field_0x40).bc * gF32Vector4Zero.y + (pEffectorParam->field_0x40).cc * local_b30.x + (pEffectorParam->field_0x40).dc * gF32Vector4Zero.w;
						local_b40.w = (pEffectorParam->field_0x40).ad * fVar30 + (pEffectorParam->field_0x40).bd * gF32Vector4Zero.y + (pEffectorParam->field_0x40).cd * local_b30.x + (pEffectorParam->field_0x40).dd * gF32Vector4Zero.w;
						edF32Vector4SubHard(&local_b30, &local_b20, pVectorIt);
						fVar30 = edF32Vector4SafeNormalize0Hard(&local_b30, &local_b30);
						edF32Vector4SafeNormalize0Hard(&local_b40, &local_b40);
						fVar31 = (pEffectorParam->field_0x194 * pParticleIt->field_0x14) / (fVar30 * fVar30 + pEffectorParam->field_0x19c);
						pParticleVectorIt->field_0x0 = pParticleVectorIt->field_0x0 + local_b30 * fVar31;
						unaff_f20 = (pEffectorParam->field_0x190 * pParticleIt->field_0x14) / (fVar30 * fVar30 + pEffectorParam->field_0x198);

						pParticleVectorIt->field_0x0 = pParticleVectorIt->field_0x0 + local_b40 * unaff_f20;
						pp_Var27 = pp_Var27 + 1;
						pVectorIt = pVectorIt + 1;
					}
				}

				if ((pEffectorParam->field_0x226 & 2) != 0) {
					IMPLEMENTATION_GUARD(
					pp_Var27 = aParticleVectors;
					pVectorIt = local_660;
					for (pp_Var20 = aParticles; pp_Var20 < aParticles + local_bc0; pp_Var20 = pp_Var20 + 1) {
						pParticleIt = *pp_Var20;
						pParticleVectorIt = *pp_Var27;
						local_b20.x = gF32Vertex4Zero.x;
						local_b20.y = gF32Vertex4Zero.y;
						local_b20.z = gF32Vertex4Zero.z;
						local_b20.w = gF32Vertex4Zero.w;
						local_b30.x = gF32Vector4Zero.x;
						local_b30.y = gF32Vector4Zero.y;
						local_b30.z = gF32Vector4Zero.z;
						local_b30.w = gF32Vector4Zero.w;
						local_b40.x = gF32Vector4Zero.x;
						local_b40.y = gF32Vector4Zero.y;
						local_b40.z = gF32Vector4Zero.z;
						local_b40.w = gF32Vector4Zero.w;
						edF32Vector4SubHard(&local_b30, &local_b20, pVectorIt);
						local_10 = gF32Vector4UnitZ.y * local_b30.z - local_b30.y * gF32Vector4UnitZ.z;
						fStack12 = gF32Vector4UnitZ.z * local_b30.x - local_b30.z * gF32Vector4UnitZ.x;
						fStack8 = gF32Vector4UnitZ.x * local_b30.y - local_b30.x * gF32Vector4UnitZ.y;
						fVar44 = local_b30.y * fStack8 - fStack12 * local_b30.z;
						fVar35 = local_b30.z * local_10 - fStack8 * local_b30.x;
						fVar36 = local_b30.x * fStack12 - local_10 * local_b30.y;
						fVar30 = (pEffectorParam->field_0x40).ab * local_b20.x;
						fVar31 = (pEffectorParam->field_0x40).ac * local_b20.x;
						fVar34 = (pEffectorParam->field_0x40).ad * local_b20.x;
						fVar32 = (pEffectorParam->field_0x40).bc * local_b20.y;
						fVar33 = (pEffectorParam->field_0x40).bd * local_b20.y;
						fVar29 = (pEffectorParam->field_0x40).cd * local_b20.z;
						local_b20.x = (pEffectorParam->field_0x40).aa * local_b20.x + (pEffectorParam->field_0x40).ba * local_b20.y +
							(pEffectorParam->field_0x40).ca * local_b20.z + (pEffectorParam->field_0x40).da * local_b20.w;
						local_b20.y = fVar30 + (pEffectorParam->field_0x40).bb * local_b20.y + (pEffectorParam->field_0x40).cb * local_b20.z +
							(pEffectorParam->field_0x40).db * local_b20.w;
						local_b20.z = fVar31 + fVar32 + (pEffectorParam->field_0x40).cc * local_b20.z +
							(pEffectorParam->field_0x40).dc * local_b20.w;
						local_b20.w = fVar34 + fVar33 + fVar29 + (pEffectorParam->field_0x40).dd * local_b20.w;
						local_b40.x = (pEffectorParam->field_0x40).aa * fVar44 + (pEffectorParam->field_0x40).ba * fVar35 +
							(pEffectorParam->field_0x40).ca * fVar36 + (pEffectorParam->field_0x40).da * in_vf0x;
						local_b40.y = (pEffectorParam->field_0x40).ab * fVar44 + (pEffectorParam->field_0x40).bb * fVar35 +
							(pEffectorParam->field_0x40).cb * fVar36 + (pEffectorParam->field_0x40).db * in_vf0x;
						local_b40.z = (pEffectorParam->field_0x40).ac * fVar44 + (pEffectorParam->field_0x40).bc * fVar35 +
							(pEffectorParam->field_0x40).cc * fVar36 + (pEffectorParam->field_0x40).dc * in_vf0x;
						local_b40.w = (pEffectorParam->field_0x40).ad * fVar44 + (pEffectorParam->field_0x40).bd * fVar35 +
							(pEffectorParam->field_0x40).cd * fVar36 + (pEffectorParam->field_0x40).dd * in_vf0x;
						local_4 = in_vf0x;
						edF32Vector4SubHard(&local_b30, &local_b20, pVectorIt);
						fVar30 = edF32Vector4SafeNormalize0Hard(&local_b30, &local_b30);
						edF32Vector4SafeNormalize0Hard(&local_b40, &local_b40);
						fVar31 = (*(float*)&pEffectorParam->field_0x194 * *(float*)&pParticleIt->field_0x14) /
							(fVar30 * fVar30 + *(float*)&pEffectorParam->field_0x19c);
						(pParticleVectorIt->field_0x0).x = (pParticleVectorIt->field_0x0).x + local_b30.x * fVar31;
						(pParticleVectorIt->field_0x0).y = (pParticleVectorIt->field_0x0).y + local_b30.y * fVar31;
						(pParticleVectorIt->field_0x0).z = (pParticleVectorIt->field_0x0).z + local_b30.z * fVar31;
						(pParticleVectorIt->field_0x0).w = (pParticleVectorIt->field_0x0).w + local_b30.w * fVar31;
						unaff_f20 = ((float)pEffectorParam->pPosFunc * *(float*)&pParticleIt->field_0x14) /
							(fVar30 * fVar30 + *(float*)&pEffectorParam->field_0x198);
						(pParticleVectorIt->field_0x0).x = (pParticleVectorIt->field_0x0).x + local_b40.x * unaff_f20;
						(pParticleVectorIt->field_0x0).y = (pParticleVectorIt->field_0x0).y + local_b40.y * unaff_f20;
						(pParticleVectorIt->field_0x0).z = (pParticleVectorIt->field_0x0).z + local_b40.z * unaff_f20;
						(pParticleVectorIt->field_0x0).w = (pParticleVectorIt->field_0x0).w + local_b40.w * unaff_f20;
						pp_Var27 = pp_Var27 + 1;
						pVectorIt = pVectorIt + 1;
					})
				}

				if ((pEffectorParam->field_0x226 & 0x100) != 0) {
					IMPLEMENTATION_GUARD(
					pp_Var27 = aParticleVectors;
					pVectorIt = local_660;
					for (pp_Var20 = aParticles; pp_Var20 < aParticles + local_bc0; pp_Var20 = pp_Var20 + 1) {
						pParticleIt = *pp_Var20;
						pParticleVectorIt = *pp_Var27;
						fVar32 = pVectorIt->y;
						fVar30 = gF32Vertex4Zero.x - pVectorIt->x;
						fVar31 = -(gF32Vertex4Zero.z - pVectorIt->z);
						local_b20.x = (pEffectorParam->field_0x40).aa * gF32Vertex4Zero.x + (pEffectorParam->field_0x40).ba * fVar32 +
							(pEffectorParam->field_0x40).ca * gF32Vertex4Zero.z + (pEffectorParam->field_0x40).da * gF32Vertex4Zero.w;
						local_b20.y = (pEffectorParam->field_0x40).ab * gF32Vertex4Zero.x + (pEffectorParam->field_0x40).bb * fVar32 +
							(pEffectorParam->field_0x40).cb * gF32Vertex4Zero.z + (pEffectorParam->field_0x40).db * gF32Vertex4Zero.w;
						local_b20.z = (pEffectorParam->field_0x40).ac * gF32Vertex4Zero.x + (pEffectorParam->field_0x40).bc * fVar32 +
							(pEffectorParam->field_0x40).cc * gF32Vertex4Zero.z + (pEffectorParam->field_0x40).dc * gF32Vertex4Zero.w;
						local_b20.w = (pEffectorParam->field_0x40).ad * gF32Vertex4Zero.x + (pEffectorParam->field_0x40).bd * fVar32 +
							(pEffectorParam->field_0x40).cd * gF32Vertex4Zero.z + (pEffectorParam->field_0x40).dd * gF32Vertex4Zero.w;
						local_b40.x = (pEffectorParam->field_0x40).aa * fVar31 + (pEffectorParam->field_0x40).ba * gF32Vector4Zero.y +
							(pEffectorParam->field_0x40).ca * fVar30 + (pEffectorParam->field_0x40).da * gF32Vector4Zero.w;
						local_b40.y = (pEffectorParam->field_0x40).ab * fVar31 + (pEffectorParam->field_0x40).bb * gF32Vector4Zero.y +
							(pEffectorParam->field_0x40).cb * fVar30 + (pEffectorParam->field_0x40).db * gF32Vector4Zero.w;
						local_b40.z = (pEffectorParam->field_0x40).ac * fVar31 + (pEffectorParam->field_0x40).bc * gF32Vector4Zero.y +
							(pEffectorParam->field_0x40).cc * fVar30 + (pEffectorParam->field_0x40).dc * gF32Vector4Zero.w;
						local_b40.w = (pEffectorParam->field_0x40).ad * fVar31 + (pEffectorParam->field_0x40).bd * gF32Vector4Zero.y +
							(pEffectorParam->field_0x40).cd * fVar30 + (pEffectorParam->field_0x40).dd * gF32Vector4Zero.w;
						local_b30.x = local_b20.x - pVectorIt->x;
						local_b30.y = local_b20.y - pVectorIt->y;
						local_b30.z = local_b20.z - pVectorIt->z;
						local_b30.w = local_b20.w - pVectorIt->w;
						fVar31 = edF32Vector4SafeNormalize0Hard(&local_b30, &local_b30);
						edF32Vector4SafeNormalize0Hard(&local_b40, &local_b40);
						fVar30 = (pParticleVectorIt->field_0x10).x * local_b40.x + (pParticleVectorIt->field_0x10).y * local_b40.y +
							(pParticleVectorIt->field_0x10).z * local_b40.z;
						unaff_f20 = (fVar30 * fVar30 * *(float*)&pEffectorParam->field_0x1a8) / fVar31;
						edF32Vector4ScaleHard(*(float*)&pEffectorParam->field_0x1a0, &local_b40, &local_b40);
						local_10 = (pParticleVectorIt->field_0x10).x - local_b40.x;
						fStack12 = (pParticleVectorIt->field_0x10).y - local_b40.y;
						fStack8 = (pParticleVectorIt->field_0x10).z - local_b40.z;
						local_4 = (pParticleVectorIt->field_0x10).w - local_b40.w;
						fVar30 = -(*(float*)&pEffectorParam->field_0x1a4 + *(float*)&pParticleIt->field_0x2c * pParticleIt->yScale);
						(pParticleVectorIt->field_0x0).x = (pParticleVectorIt->field_0x0).x + local_10 * fVar30;
						(pParticleVectorIt->field_0x0).y = (pParticleVectorIt->field_0x0).y + fStack12 * fVar30;
						(pParticleVectorIt->field_0x0).z = (pParticleVectorIt->field_0x0).z + fStack8 * fVar30;
						(pParticleVectorIt->field_0x0).w = (pParticleVectorIt->field_0x0).w + local_4 * fVar30;
						(pParticleVectorIt->field_0x0).x = (pParticleVectorIt->field_0x0).x + local_b30.x * unaff_f20;
						(pParticleVectorIt->field_0x0).y = (pParticleVectorIt->field_0x0).y + local_b30.y * unaff_f20;
						(pParticleVectorIt->field_0x0).z = (pParticleVectorIt->field_0x0).z + local_b30.z * unaff_f20;
						(pParticleVectorIt->field_0x0).w = (pParticleVectorIt->field_0x0).w + local_b30.w * unaff_f20;
						pp_Var27 = pp_Var27 + 1;
						pVectorIt = pVectorIt + 1;
					})
				}

				if ((pEffectorParam->field_0x226 & 8) != 0) {
					for (pp_Var27 = aParticleVectors; pp_Var27 < aParticleVectors + local_bc0; pp_Var27 = pp_Var27 + 1) {
						pParticleVectorIt = *pp_Var27;
						fVar30 = (pParticleVectorIt->field_0x10).x;
						fVar31 = (pParticleVectorIt->field_0x10).y;
						fVar32 = (pParticleVectorIt->field_0x10).z;
						local_20 = fVar31 * pEffectorParam->field_0x160.z - pEffectorParam->field_0x160.y * fVar32;
						fStack28 = fVar32 * pEffectorParam->field_0x160.x - pEffectorParam->field_0x160.z * fVar30;
						fStack24 = fVar30 * pEffectorParam->field_0x160.y - pEffectorParam->field_0x160.x * fVar31;
						(pParticleVectorIt->field_0x0).x = (pParticleVectorIt->field_0x0).x + local_20;
						(pParticleVectorIt->field_0x0).y = (pParticleVectorIt->field_0x0).y + fStack28;
						(pParticleVectorIt->field_0x0).z = (pParticleVectorIt->field_0x0).z + fStack24;
						(pParticleVectorIt->field_0x0).w = (pParticleVectorIt->field_0x0).w + in_vf0x;
					}
				}

				if ((pEffectorParam->field_0x226 & 0x200) != 0) {
					IMPLEMENTATION_GUARD(
					for (pp_Var20 = aParticles; pp_Var20 < aParticles + local_bc0; pp_Var20 = pp_Var20 + 1) {
						(**(code**)&pEffectorParam->field_0x220)(pGroup);
					})
				}
			}

			local_b80 = local_b80 + 1;
		}

		if (pGroup->field_0x9 != 0) {
			pp_Var27 = aParticleVectors;
			ppeVar28 = aVectors;
			for (pp_Var20 = aParticles; pp_Var20 < aParticles + local_bc0; pp_Var20 = pp_Var20 + 1) {
				pParticleIt = *pp_Var20;
				pParticleVectorIt = *pp_Var27;
				pVectorIt = *ppeVar28;
				fVar30 = time * pParticleIt->field_0x30;
				fVar31 = (pParticleVectorIt->field_0x10).y;
				fVar32 = (pParticleVectorIt->field_0x10).z;
				fVar34 = (pParticleVectorIt->field_0x10).w;
				pVectorIt->x = pVectorIt->x + (pParticleVectorIt->field_0x10).x * fVar30;
				pVectorIt->y = pVectorIt->y + fVar31 * fVar30;
				pVectorIt->z = pVectorIt->z + fVar32 * fVar30;
				pVectorIt->w = pVectorIt->w + fVar34 * fVar30;
				fVar30 = time * pParticleIt->field_0x18;
				fVar31 = (pParticleVectorIt->field_0x0).y;
				fVar32 = (pParticleVectorIt->field_0x0).z;
				fVar34 = (pParticleVectorIt->field_0x0).w;
				(pParticleVectorIt->field_0x10).x = (pParticleVectorIt->field_0x10).x + (pParticleVectorIt->field_0x0).x * fVar30;
				(pParticleVectorIt->field_0x10).y = (pParticleVectorIt->field_0x10).y + fVar31 * fVar30;
				(pParticleVectorIt->field_0x10).z = (pParticleVectorIt->field_0x10).z + fVar32 * fVar30;
				(pParticleVectorIt->field_0x10).w = (pParticleVectorIt->field_0x10).w + fVar34 * fVar30;
				pp_Var27 = pp_Var27 + 1;
				ppeVar28 = ppeVar28 + 1;
			}
		}
	}

	pParticleIt = p_current_particle_manager->aParticles.pData + iVar9;
	pParticleVectorIt = p_current_particle_manager->aParticleVectors.pData + iVar9;
	pVectorIt = p_current_particle_manager->aVectors.pData + iVar9;

	for (iVar17 = iVar9; iVar17 < local_bb0; iVar17 = iVar17 + 1) {
		pParticleIt->field_0x13 = 1;

		if (pParticleIt->field_0x0 == 2) {
			bVar1 = pParticleIt->state;
			if (bVar1 == PARTICLE_STATE_DEAD) {
				if ((pGroup->field_0x8 & 4) == 0) {
					pParticleIt->field_0x0 = 1;
					pGroup->field_0x14 = pGroup->field_0x14 + 1;
				}

				pGroup->field_0x5c = pGroup->field_0x5c + -1;
				pParticleIt->state = PARTICLE_STATE_DESTROYED;
			}
			else {
				if (bVar1 == PARTICLE_STATE_ALIVE) {
					pParticleIt->age = pParticleIt->age + pGroup->deltaTime;
					if ((pParticleIt->lifetime < pParticleIt->age) || (((pGroup->field_0x8 & 2) != 0 && (pParticleIt->field_0x13 == 0)))) {
						pParticleIt->state = PARTICLE_STATE_DEAD;
					}

					fVar30 = gParticleSizeScale * pParticleIt->yScale * (float)(uint)pParticleIt->field_0x4 * 0.5f;
					if (local_b50.w == -1.0f) {
						local_b50.x = pVectorIt->x;
						local_b50.y = pVectorIt->y;
						local_b50.z = pVectorIt->z;
						local_b50.w = fVar30;
					}
					else {
						fVar31 = pVectorIt->x - local_b50.x;
						fVar34 = pVectorIt->y - local_b50.y;
						fVar33 = pVectorIt->z - local_b50.z;
						fVar32 = sqrtf(fVar31 * fVar31 + fVar34 * fVar34 + fVar33 * fVar33) + 0.0f;
						fVar44 = 1.0f / fVar32;
						fVar29 = fVar32 + fVar30;
						if (local_b50.w < fVar29) {
							local_b50.w = -local_b50.w;
							if (fVar32 - fVar30 <= local_b50.w) {
								local_b50.w = fVar32 - fVar30;
							}

							fVar30 = (fVar29 + local_b50.w) * 0.5f;
							local_b50.x = local_b50.x + fVar31 * fVar44 * fVar30;
							local_b50.y = local_b50.y + fVar34 * fVar44 * fVar30;
							local_b50.z = local_b50.z + fVar33 * fVar44 * fVar30;
							local_b50.w = (fVar29 - local_b50.w) * 0.5f;
						}
					}
				}
				else {
					if (bVar1 == 1) {
						pParticleIt->state = 2;
					}
					else {
						if (bVar1 == 5) {
							fVar30 = pParticleIt->age;
							*pVectorIt = *pVectorIt + pParticleVectorIt->field_0x10 * fVar30;
							pParticleIt->state = 1;
							pGroup->field_0x5c = pGroup->field_0x5c + 1;
						}
					}
				}
			}
		}

		pParticleIt = pParticleIt + 1;
		pParticleVectorIt = pParticleVectorIt + 1;
		pVectorIt = pVectorIt + 1;
	}

	pGroup->field_0x84 = local_b50.xyz;
	pGroup->field_0x58 = local_b50.w;

	if (g_particle_system_config.bProfile != 0) {
		edProfileEnd(gPartProfIdEffectors);
	}

	if (g_particle_system_config.bProfile != 0) {
		edProfileBegin(gPartProfIdSelectors);
	}

	pGroup->field_0x70 = (_ed_particle_generator_param*)0x0;
	p_Var21 = pGroup->field_0x3c.pData;
	for (iVar17 = 0; iVar17 < pGroup->field_0x38; iVar17 = iVar17 + 1) {
		IMPLEMENTATION_GUARD(
		pPVar3 = *(_ed_particle_shaper_param**)p_Var21;
		if (pPVar3->field_0x2 != 0) {
			pGroup->field_0x70 = pPVar3;
			if (*(byte*)&pPVar3->field_0x8 == 2) {
				p_Var19 = *(_ed_particle_generator_param**)&pPVar3->field_0x14;
				for (iVar12 = *(int*)&pPVar3->field_0x10; iVar12 != 0; iVar12 = iVar12 + -1) {
					iVar4 = *(int*)p_Var19;
					if ((*(int*)(iVar4 + 0x218) != 1) || (*(float*)(iVar4 + 0x21c) <= (float)pPVar3->field_0xc)) {
						iVar4 = *(int*)p_Var19;
						iVar5 = *(int*)(iVar4 + 0x218);
						if (iVar5 == 1) {
							*(float*)(iVar4 + 0x21c) = *(float*)(iVar4 + 0x21c) + pGroup->deltaTime;
						}
						else {
							if (((iVar5 != 0) && (*(char*)(iVar5 + 3) != '\x02')) && (*(char*)(iVar5 + 3) != '\x01')) {
								*(undefined*)(iVar4 + 2) = 0;
								*(undefined4*)(*(int*)p_Var19 + 0x218) = 0;
							}
						}
					}
					else {
						*(undefined*)(iVar4 + 2) = 0;
						*(undefined4*)(*(int*)p_Var19 + 0x218) = 0;
					}
					p_Var19 = (_ed_particle_generator_param*)&p_Var19->field_0x4;
				}
			}
			else {
				if (*(byte*)&pPVar3->field_0x8 == 1) {
					p_Var19 = *(_ed_particle_generator_param**)&pPVar3->field_0x14;
					for (iVar12 = *(int*)&pPVar3->field_0x10; iVar12 != 0; iVar12 = iVar12 + -1) {
						iVar4 = *(int*)p_Var19;
						if ((*(int*)(iVar4 + 8) != 1) || (*(float*)(iVar4 + 0x17c) <= (float)pPVar3->field_0xc)) {
							iVar4 = *(int*)p_Var19;
							iVar5 = *(int*)(iVar4 + 8);
							if (iVar5 == 1) {
								*(float*)(iVar4 + 0x17c) = *(float*)(iVar4 + 0x17c) + pGroup->deltaTime;
							}
							else {
								if (((iVar5 != 0) && (*(char*)(iVar5 + 3) != '\x02')) && (*(char*)(iVar5 + 3) != '\x01')) {
									*(undefined*)(iVar4 + 2) = 0;
									*(undefined4*)(*(int*)p_Var19 + 8) = 0;
								}
							}
						}
						else {
							*(undefined*)(iVar4 + 2) = 0;
							*(undefined4*)(*(int*)p_Var19 + 8) = 0;
						}
						p_Var19 = (_ed_particle_generator_param*)&p_Var19->field_0x4;
					}
				}
			}
			edPartCheckSelector(pGroup, (_ed_particle_selector_param*)pPVar3, p_current_particle_manager->aParticles,
				p_current_particle_manager->aParticleVectors, p_current_particle_manager->aVectors, iVar9,
				local_bb0);
		}

		p_Var21 = (_ed_particle_selector_param*)((int)p_Var21 + 4);)
	}

	if (g_particle_system_config.bProfile != 0) {
		edProfileEnd(gPartProfIdSelectors);
	}

	return;
}

// Should be in: D:/Projects/EdenLib/edParticles/sources/edParticles2.cpp
void edParticlesUpdate(float time)
{
	int iVar1;
	_ed_particle_group* pGroup;

	pGroup = p_current_particle_manager->aGroups.pData;
	for (iVar1 = 0; iVar1 < p_current_particle_manager->nbGroups; iVar1 = iVar1 + 1) {
		if ((pGroup->field_0x0 != 0) && (pGroup->field_0x2 != 0)) {
			if (0.0f <= time) {
				edParticleGroupUpdate(time * pGroup->field_0x4c, pGroup);
			}
			else {
				edParticleGroupUpdate(p_current_particle_manager->field_0x4 * pGroup->field_0x4c, pGroup);
			}
		}

		pGroup = pGroup + 1;
	}

	return;
}

void edPartSetDisplayMatrix(_ed_particle_manager* pManager, edF32MATRIX4* pMatrix)
{
	int nbMatrix;
	float fVar2;
	int iVar3;
	edF32MATRIX4* peVar4;
	edF32MATRIX4* peVar5;
	int iVar6;
	_ed_particle_shaper_param* pParticle_1a0;

	nbMatrix = pManager->nbShaperParams;
	pParticle_1a0 = pManager->aShaperParams.pData;

	for (iVar6 = 0; iVar6 < nbMatrix; iVar6 = iVar6 + 1) {
		pParticle_1a0->field_0xd0 = *pMatrix;
		pParticle_1a0 = pParticle_1a0 + 1;
	}

	return;
}

struct Vec2 { float x, y; };

static int table_sorted_indices$1918[256];

static float FLOAT_004485e8 = 0.0019607844f;

union Mat2x3
{
	float m[2][3];

	struct
	{
		float m00, m01, m02;
		float m10, m11, m12;
	};
};

static Vec2 FLOAT_00431860[4] =
{
	{ 0.0f, 0.0f },
	{ 1.0f, 0.0f },
	{ 0.0f, 1.0f },
	{ 1.0f, 1.0f }
};

static Vec2 FLOAT_00431880[4] =
{
	{ 0.0f, 0.0f },
	{ 0.0f, 1.0f },
	{ 1.0f, 0.0f },
	{ 1.0f, 1.0f }
};

static Mat2x3 FLOAT_ARRAY_0041e7d0 =
{
	-1.0f, 0.0f, 0.0f,
	 1.0f, 1.0f, 0.0f
};

static Mat2x3 FLOAT_ARRAY_0041e7f0 =
{
	 1.0f,  0.0f, 0.0f,
	-1.0f, 0.0f,  1.0f
};

static Mat2x3 FLOAT_ARRAY_0041e810 =
{
	-1.0f, 0.0f, 0.0f,
	1.0f, 1.0f, 0.0f
};

static Mat2x3 FLOAT_ARRAY_0041e830 =
{
	1.0f, 0.0f, 0.0f,
	-1.0f, 0.0f, 1.0f
};

static Mat2x3 FLOAT_ARRAY_0041e890 =
{
	1.0f, 0.0f, 0.0f,
	1.0f, 0.0f, 0.0f
};

static Mat2x3 FLOAT_ARRAY_0041e8b0 =
{
	1.0f, 0.0f,	0.0f,
	1.0f, 1.0f, 1.0f
};

static Mat2x3 Mat2x3_0041e970 =
{
	-1.0f, 0.0f, 0.0f,
	 1.0f, 1.0f, 0.0f
};

static Mat2x3 Mat2x3_0041e990 =
{
	 1.0f,  0.0f, 0.0f,
	-1.0f, 0.0f,  1.0f
};

static Mat2x3 Mat2x3_0041e9b0 =
{
	1.0f, 0.0f, 0.0f,
	-1.0f, 0.0f, 1.0f
};

static Mat2x3 Mat2x3_0041ea50 =
{
	 1.0f, 0.0f, 0.0f,
	-1.0f, 0.0f, 1.0f
};

static Mat2x3 Mat2x3_0041ead0 =
{
	-1.0f, 0.0f, 0.0f,
	 1.0f, 1.0f, 0.0f
};

static Mat2x3 Mat2x3_0041eaf0 =
{
	-1.0f, 0.0f, 0.0f,
	 1.0f, 1.0f, 0.0f
};

static Mat2x3 FLOAT_ARRAY_0041eb50 =
{
	1.0f, 0.0f, 0.0f, 
	1.0f, 0.0f, 0.0f
};

static Mat2x3 FLOAT_ARRAY_0041eb70 =
{
	-1.0f, 0.0f, 0.0f,
	 1.0f, 1.0f, 0.0f
};

static Mat2x3 FLOAT_ARRAY_0041eb90 =
{
	 1.0f,  0.0f, 0.0f,
	-1.0f, 0.0f,  1.0f
};

static Mat2x3 FLOAT_ARRAY_0041ebb0 =
{
	1.0f, 0.0f, 0.0f,
	1.0f, 1.0f, 1.0f
};

static Mat2x3 FLOAT_ARRAY_0041ebd0 =
{
	1.0f, 0.0f, 0.0f,
	1.0f, 0.0f, 0.0f
};

static Mat2x3 FLOAT_ARRAY_0041ebf0 =
{
	-1.0f, 0.0f, 0.0f,
	 1.0f, 1.0f, 0.0f
};

static Mat2x3 FLOAT_ARRAY_0041ec10 =
{
	 1.0f,  0.0f, 0.0f,
	-1.0f, 0.0f,  1.0f
};

static Mat2x3 FLOAT_ARRAY_0041ec50 =
{
	1.0f, 0.0f, 0.0f,
	1.0f, 0.0f, 0.0f
};

static Mat2x3 FLOAT_ARRAY_0041ec70 =
{
	-1.0f, 0.0f, 0.0f,
	 1.0f, 1.0f, 0.0f
};

static Mat2x3 FLOAT_ARRAY_0041ec90 =
{
	 1.0f,  0.0f, 0.0f,
	-1.0f, 0.0f,  1.0f
};

void TransformQuadUVs(const Vec2 quad[4], const Mat2x3& mat, Vec2 out[4])
{
	for (int i = 0; i < 4; ++i)
	{
		const float x = quad[i].x;
		const float y = quad[i].y;

		out[i].x = mat.m00 * x + mat.m01 * y + mat.m02;
		out[i].y = mat.m10 * x + mat.m11 * y + mat.m12;
	}
}

struct AffineQuad2D
{
	Vec2 xAxis;
	Vec2 yAxis;
	Vec2 origin;
};

AffineQuad2D gDefaultQuad =
{
	{ 1.0f, 0.0f },
	{ 0.0f, 1.0f },
	{ 0.0f, 0.0f }
};

void ApplyAffine(Mat2x3 mat, AffineQuad2D& outQuad)
{
	AffineQuad2D quad = outQuad;

	outQuad.xAxis.x = quad.xAxis.x * mat.m00 + quad.yAxis.x * mat.m10;
	outQuad.xAxis.y = quad.xAxis.y * mat.m00 + quad.yAxis.y * mat.m10;

	outQuad.yAxis.x = quad.xAxis.x * mat.m01 + quad.yAxis.x * mat.m11;
	outQuad.yAxis.y = quad.xAxis.y * mat.m01 + quad.yAxis.y * mat.m11;

	outQuad.origin.x = mat.m02 + quad.origin.x * mat.m00 + quad.origin.y * mat.m10;
	outQuad.origin.y = mat.m12 + quad.origin.x * mat.m01 + quad.origin.y * mat.m11;

	return;
}

void ApplyAffineB(Mat2x3 mat, AffineQuad2D& outQuad)
{
	AffineQuad2D quad = outQuad;

	outQuad.xAxis.x = quad.yAxis.x * mat.m02 + quad.xAxis.x * mat.m00;
	outQuad.xAxis.y = quad.yAxis.y * mat.m02 + quad.xAxis.y * mat.m00;

	outQuad.yAxis.x = quad.yAxis.x * mat.m10 + quad.xAxis.x * mat.m01;
	outQuad.yAxis.y = quad.yAxis.y * mat.m10 + quad.xAxis.y * mat.m01;

	outQuad.origin.x = quad.origin.y * mat.m02 + quad.origin.x * mat.m00 + mat.m11 + 0.0f;
	outQuad.origin.y = quad.origin.y * mat.m10 + quad.origin.x * mat.m01 + mat.m12 + 0.0f;

	return;
}

void ApplyAffineC(Mat2x3 mat, AffineQuad2D& outQuad)
{
	AffineQuad2D quad = outQuad;
	
	outQuad.xAxis.x = quad.xAxis.y * mat.m02 + quad.yAxis.y * mat.m00;
	outQuad.yAxis.x = quad.yAxis.y * mat.m02 + quad.yAxis.x * mat.m00;

	outQuad.xAxis.y = quad.xAxis.y * mat.m10 + quad.yAxis.y * mat.m01;
	outQuad.yAxis.y = quad.yAxis.y * mat.m10 + quad.yAxis.x * mat.m01;

	outQuad.origin.x = quad.origin.y * mat.m02 + quad.origin.x * mat.m00 + mat.m11 + 0.0f;
	outQuad.origin.y = quad.origin.y * mat.m10 + quad.origin.x * mat.m01 + mat.m12 + 0.0f;

	return;
}

inline edF32VECTOR2 TransformUV(const Vec2& local, const AffineQuad2D& affine)
{
	edF32VECTOR2 uv;
	uv.u = local.y * affine.yAxis.x + local.x * affine.xAxis.x + affine.origin.x;
	uv.v = local.y * affine.yAxis.y + local.x * affine.xAxis.y + affine.origin.y;
	return uv;
}

inline void TransformUVs(const Vec2 local[4], const AffineQuad2D& affine, edF32VECTOR2& uv0, edF32VECTOR2& uv1, edF32VECTOR2& uv2, edF32VECTOR2& uv3)
{
	uv0 = TransformUV(local[0], affine);
	uv1 = TransformUV(local[1], affine);
	uv2 = TransformUV(local[2], affine);
	uv3 = TransformUV(local[3], affine);

	return;
}

inline void ParticleA(AffineQuad2D& quad, _ed_particle_vectors* pCurVector, _ed_particle* pCurParticle, edF32VECTOR4& local_220, edF32VECTOR4& local_230, float fVar28, _ed_particle_shaper_param* pDrawData)
{
	float floatA = (pCurVector->field_0x10.x * local_230.x + pCurVector->field_0x10.y * local_230.y + pCurVector->field_0x10.z * local_230.z) * pCurParticle->field_0x30;
	float floatB = (pCurVector->field_0x10.x * local_220.x + pCurVector->field_0x10.y * local_220.y + pCurVector->field_0x10.z * local_220.z) * pCurParticle->field_0x30;
	float fVar30 = sqrtf(floatA * floatA + floatB * floatB);
	float fVar29 = g_TinyFloat_00448548;

	if (fVar30 != 0.0f) {
		fVar29 = fVar30;
	}

	quad.yAxis.x = floatA * (1.0f / fVar29);
	quad.xAxis.x = floatB * (1.0f / fVar29);
	quad.xAxis.y = -quad.yAxis.x;
	quad.origin.x = (quad.xAxis.x + quad.yAxis.x) * -0.5f + 0.5f;
	quad.origin.y = (quad.xAxis.x - quad.yAxis.x) * -0.5f + 0.5f;
	if ((fVar28 != 0.0f) || (quad.yAxis.y = quad.xAxis.x, pDrawData->field_0x94 != 0.0f)) {
		fVar29 = pDrawData->field_0x94 * pCurParticle->age / pCurParticle->lifetime;
		if (0.0f < quad.yAxis.x) {
			fVar29 = -fVar29;
		}

		float fVar32 = sinf(fVar28 + fVar29);
		fVar29 = cosf(fVar28 + fVar29);
		fVar30 = -(fVar29 * 0.5f) + fVar32 * 0.5f + 0.5f;
		float fVar31 = (-(fVar32 * 0.5f) - fVar29 * 0.5f) + 0.5f;
		float fVar33 = -fVar32 * quad.xAxis.x;
		fVar28 = fVar32 * quad.xAxis.x;
		quad.yAxis.y = fVar29 * quad.xAxis.x + -fVar32 * quad.xAxis.y;
		quad.origin.x = quad.origin.x + fVar31 * quad.yAxis.x + fVar30 * quad.xAxis.x;
		quad.origin.y = quad.origin.y + fVar31 * quad.xAxis.x + fVar30 * quad.xAxis.y;
		quad.xAxis.x = fVar32 * quad.yAxis.x + fVar29 * quad.xAxis.x;
		quad.xAxis.y = fVar28 + fVar29 * quad.xAxis.y;
		quad.yAxis.x = fVar29 * quad.yAxis.x + fVar33;
	}

	return;
}

inline void ParticleB(AffineQuad2D& quad, uint uVar2, uint uVar12, _ed_particle_shaper_param* pDrawData, _ed_particle* pCurParticle)
{
	int iVar15;
	long lVar14;

	if ((uVar2 & 0x10) == 0) {
		iVar15 = 0;
	}
	else {
		iVar15 = (int)(((int)uVar12 >> 8 & 0xffffU) * (int)pDrawData->field_0x82) >> 0x10;
	}

	if ((uVar2 & 0x80) == 0) {
		iVar15 = iVar15 + (int)(pCurParticle->age / pCurParticle->lifetime * (float)(int)pDrawData->field_0x82);
		lVar14 = (long)iVar15;

		if (lVar14 < 0) {
			lVar14 = (long)-iVar15;
		}

		if (pDrawData->field_0x82 <= lVar14) {
			lVar14 = (long)(pDrawData->field_0x82 + -1);
		}
	}
	else {
		iVar15 = (iVar15 + (int)(pCurParticle->age * (float)pDrawData->field_0x98)) % (int)pDrawData->field_0x82;
		lVar14 = (long)iVar15;
		if (pDrawData->field_0x82 == 0) {
			trap(7);
		}

		if (lVar14 < 0) {
			lVar14 = (long)-iVar15;
		}
	}

	short sVar3 = pDrawData->field_0x84;
	FLOAT_ARRAY_0041ebb0.m00 = 1.0f / (float)(int)sVar3;
	FLOAT_ARRAY_0041ebb0.m10 = 1.0f / (float)(int)pDrawData->field_0x86;
	if (sVar3 == 0) {
		trap(7);
	}

	FLOAT_ARRAY_0041ebb0.m11 = FLOAT_ARRAY_0041ebb0.m00 * (float)((int)lVar14 % (int)sVar3);
	if (pDrawData->field_0x84 == 0) {
		trap(7);
	}

	FLOAT_ARRAY_0041ebb0.m12 = FLOAT_ARRAY_0041ebb0.m10 * (float)((int)lVar14 / (int)pDrawData->field_0x84);

	ApplyAffineC(FLOAT_ARRAY_0041ebb0, quad);

	return;
}

inline void ParticleC(AffineQuad2D& quad, float fVar28)
{
	quad.xAxis.x = cosf(fVar28);
	quad.xAxis.y = sinf(fVar28);
	quad.yAxis.x = -quad.xAxis.y;
	quad.origin.x = -(quad.xAxis.x * 0.5f) + quad.xAxis.y * 0.5f + 0.5f;
	quad.origin.y = (-(quad.xAxis.y * 0.5f) - quad.xAxis.x * 0.5f) + 0.5f;
	quad.yAxis.y = quad.xAxis.x;
}

void ParticleColorA(_rgba* rgbaColor, _ed_particle* pCurParticle, int alphaMultiplier)
{
	uint32_t baseColor = pCurParticle->field_0x8_uint;
	uint8_t a = ((baseColor >> 24) * pCurParticle->field_0xc * alphaMultiplier) >> 16;
	uint8_t r = ((baseColor >> 16 & 0xFF) * pCurParticle->field_0x12) >> 8;
	uint8_t g = ((baseColor >> 8 & 0xFF) * pCurParticle->field_0xf) >> 8;
	uint8_t b = ((baseColor & 0xFF) * pCurParticle->field_0xe) >> 8;

	rgbaColor[0].rgba = (a << 24) | (r << 16) | (g << 8) | b;

	return;
}

void ParticleColorB(_rgba* rgbaColor, _ed_particle* pCurParticle, int alphaMultiplier, _ed_particle_shaper_param* pDrawData, uint blendAlpha)
{
	uint lerp8 = (int)(pCurParticle->seed >> 3) & 0xff;
	uint oneMinusLerp8 = 0xff - lerp8;
	uint paletteIdx = pCurParticle->seed & 7;

	uint32_t colorA = pDrawData->field_0x9c[paletteIdx];
	uint32_t colorB = pDrawData->field_0x9c[paletteIdx + 1];

	_rgba rb;
	_rgba ga;

	// Interpolate R and B
	rb.rgba = (pCurParticle->field_0x8_uint & 0xff00ff) * (0xff - blendAlpha) +
		((((colorA & 0xff00ff) * oneMinusLerp8 + (colorB & 0xff00ff) * lerp8) >> 8) & 0xff00ff) * blendAlpha;

	// Interpolate G and A
	ga.rgba = ((pCurParticle->field_0x8_uint >> 8) & 0xff00ff) * (0xff - blendAlpha) +
		(((((colorA >> 8) & 0xff00ff) * oneMinusLerp8 + ((colorB >> 8) & 0xff00ff) * lerp8) & 0xff00ff00) >> 8) * blendAlpha;

	_rgba mix;
	mix.rgba = ((rb.rgba >> 8) & 0xff00ff) | (ga.rgba & 0xff00ff00);

	mix.r = ((mix.r & 0xff) * (pCurParticle->field_0xe & 0xff) >> 8) & 0xff;
	mix.g = ((mix.g & 0xff) * (pCurParticle->field_0xf & 0xff) >> 8) & 0xff;
	mix.b = ((mix.b & 0xff) * (pCurParticle->field_0x12 & 0xff) >> 8) & 0xff;
	mix.a = ((mix.a & 0xff) * (pCurParticle->field_0xc & 0xff) >> 8) & 0xff;
	mix.a = ((mix.a & 0xff) * (alphaMultiplier & 0xff) >> 8) & 0xff;

	*rgbaColor = mix;

	return;
}

void ParticleColorC(_rgba* rgbaColor, uint local_250, uint local_24c, uint local_248)
{
	uint32_t baseColor = rgbaColor[0].rgba;
	uint32_t rb = baseColor & 0xFF00FF;
	uint32_t ga = (baseColor >> 8) & 0xFF00FF;

	rgbaColor[1].rgba = (baseColor & 0xFF000000) | ((rb * local_250 >> 8) & 0xFF00FF) | ((ga * local_250) & 0xFF00);
	rgbaColor[2].rgba = (baseColor & 0xFF000000) | ((rb * local_24c >> 8) & 0xFF00FF) | ((ga * local_24c) & 0xFF00);
	rgbaColor[3].rgba = (baseColor & 0xFF000000) | ((rb * local_248 >> 8) & 0xFF00FF) | ((ga * local_248) & 0xFF00);

	return;
}

void edPartDrawShaper(float alpha, _ed_particle_group* pGroup, _ed_particle_shaper_param* pDrawData, _ed_particle* pParticle, _ed_particle_vectors* pVector, edF32VECTOR4* pRawVectors, int startIndex, int endIndex)
{
	byte bVar1;
	ushort uVar2;
	short sVar3;
	edFCamera* pCamera;
	edNODE* pNode;
	ed_3d_sprite* peVar5;
	undefined8 uVar6;
	edF32VECTOR4* peVar7;
	uint* puVar8;
	_ed_particle_vectors* pCurVector;
	int iVar10;
	uint uVar11;
	uint uVar12;
	uint* puVar13;
	long lVar14;
	int iVar15;
	float puVar16;
	uint* puVar17;
	edF32MATRIX4* peVar18;
	ed_3d_sprite* peVar19;
	undefined2* puVar20;
	int iVar21;
	uint uVar22;
	// Pointer to [4][2] array
	Vec2* pfVar23;
	_ed_particle* pCurParticle;
	uint uVar26;
	float fVar27;
	float fVar28;
	float fVar29;
	float fVar30;
	float fVar31;
	float fVar32;
	float fVar33;
	float fVar34;
	float fVar35;
	int local_900;
	int nbParticlesToDraw;
	int local_8d0;
	AffineQuad2D local_7d0;
	AffineQuad2D local_770;
	AffineQuad2D local_710;
	AffineQuad2D local_690;
	AffineQuad2D local_670;
	AffineQuad2D local_65c;
	AffineQuad2D local_620;
	float local_600;
	float local_5fc;
	AffineQuad2D local_5f0;
	float local_5d0;
	float local_5cc;
	float local_5c8;
	float local_5c4;
	float local_5c0;
	float local_5bc;
	AffineQuad2D local_570;
	float local_510;
	float local_50c;
	float local_508;
	float local_504;
	float local_500;
	float local_4fc;
	float local_490;
	AffineQuad2D local_48c;
	AffineQuad2D local_410;
	float local_3f0;
	float local_3ec;
	float local_3e8;
	float local_3e4;
	float local_3e0;
	float local_3dc;
	AffineQuad2D local_350;
	float local_33c;
	float local_310;
	float local_30c;
	float local_308;
	float local_304;
	float local_300;
	float local_2fc;
	AffineQuad2D local_2f0;
	edF32MATRIX4 eStack720;
	char local_290;
	char local_28f;
	char local_28e;
	edF32VECTOR2 uv0;
	edF32VECTOR2 uv1;
	edF32VECTOR2 uv2;
	edF32VECTOR2 uv3;
	char local_26c;
	bool bSeparateVertexColors;
	int alphaMultiplier;
	_rgba rgbaColor[4];
	uint local_254;
	uint local_250;
	uint local_24c;
	uint local_248;
	char local_244;
	float aspectRatio;
	float width;
	float height;
	edF32VECTOR4 local_230;
	edF32VECTOR4 local_220;
	edF32VECTOR4 eStack528;
	edF32VECTOR4 eStack512;
	edF32MATRIX4 eStack496;
	edF32MATRIX4 local_1b0;
	edF32VECTOR4 local_170[4];
	edF32VECTOR4 aCameraPoints[4];
	edF32MATRIX4 particleToCameraMatrix;
	undefined4 local_98;
	undefined4 local_94;
	edF32VECTOR4* pCurRawVector;
	float local_90;
	float local_8c;
	uint local_88;
	uint uStack132;
	uint local_80;
	uint local_7c;
	uint local_78;
	uint local_74;
	uint local_70;
	uint local_6c;
	uint local_68;
	uint local_64;
	uint local_60;
	uint local_5c;
	uint local_58;
	uint local_54;
	uint local_50;
	uint local_4c;
	uint local_48;
	uint local_44;
	uint local_40;
	uint local_3c;
	uint local_38;
	uint local_34;
	uint local_30;
	uint local_2c;
	uint local_28;
	uint local_24;
	uint local_20;
	uint local_1c;
	uint local_18;
	uint local_14;
	uint local_10;
	uint local_c;
	uint local_8;
	uint local_4;

	PARTICLE_LOG(LogLevel::Info, "edPartDrawShaper: startIndex: 0x{:x} endIndex: 0x{:x} pGroup->field_0x58: {}", startIndex, endIndex, pGroup->field_0x58);

	if (pGroup->field_0x58 != -1.0f) {
		pCamera = pDrawData->field_0x198->pCamera;
		edF32Matrix4MulF32Matrix4Hard(&particleToCameraMatrix, &pDrawData->field_0xd0, &pCamera->worldToCamera);
		nbParticlesToDraw = 0;

		aCameraPoints[0] = {};
		aCameraPoints[1] = {};
		aCameraPoints[2] = {};
		aCameraPoints[3] = {};

		float computedVerticalHalfFOV = pCamera->computedVerticalHalfFOV;
		float negativeComputedVerticalHalfFOV = -pCamera->computedVerticalHalfFOV;

		float baseHorizontalHalfFOV = pCamera->baseHorizontalHalfFOV;
		float finalHorizontalHalfFOV = pCamera->finalHorizontalHalfFOV;

		fVar35 = 1.0f / (sqrtf(aCameraPoints[0].x * aCameraPoints[0].x + computedVerticalHalfFOV * computedVerticalHalfFOV + baseHorizontalHalfFOV * baseHorizontalHalfFOV) + 0.0f);
		aCameraPoints[0].x = aCameraPoints[0].x * fVar35;
		aCameraPoints[0].y = computedVerticalHalfFOV * fVar35;
		aCameraPoints[0].z = baseHorizontalHalfFOV * fVar35;
		aCameraPoints[0].w = 0.0f;

		fVar27 = 1.0f / (sqrtf(negativeComputedVerticalHalfFOV * negativeComputedVerticalHalfFOV + aCameraPoints[1].y * aCameraPoints[1].y + finalHorizontalHalfFOV * finalHorizontalHalfFOV) + 0.0f);
		aCameraPoints[1].x = negativeComputedVerticalHalfFOV * fVar27;
		aCameraPoints[1].y = aCameraPoints[1].y * fVar27;
		aCameraPoints[1].z = finalHorizontalHalfFOV * fVar27;
		aCameraPoints[1].w = 0.0f;

		fVar27 = 1.0f / (sqrtf(aCameraPoints[2].x * aCameraPoints[2].x + negativeComputedVerticalHalfFOV * negativeComputedVerticalHalfFOV + baseHorizontalHalfFOV * baseHorizontalHalfFOV) + 0.0f);
		aCameraPoints[2].x = aCameraPoints[2].x * fVar27;
		aCameraPoints[2].y = negativeComputedVerticalHalfFOV * fVar27;
		aCameraPoints[2].z = baseHorizontalHalfFOV * fVar27;
		aCameraPoints[2].w = 0.0f;

		fVar27 = 1.0f / (sqrtf(computedVerticalHalfFOV * computedVerticalHalfFOV + aCameraPoints[3].y * aCameraPoints[3].y + finalHorizontalHalfFOV * finalHorizontalHalfFOV) + 0.0f);
		aCameraPoints[3].x = computedVerticalHalfFOV * fVar27;
		aCameraPoints[3].y = aCameraPoints[3].y * fVar27;
		aCameraPoints[3].z = finalHorizontalHalfFOV * fVar27;
		aCameraPoints[3].w = 0.0f;

		fVar27 = 1.0f;
		if (1.0f < pDrawData->aspectRatio) {
			fVar27 = pDrawData->aspectRatio;
		}

		const edF32VECTOR3 point = pGroup->field_0x84;

		fVar30 = pGroup->field_0x58 * fVar27;
		fVar28 = particleToCameraMatrix.ac * point.x + particleToCameraMatrix.bc * point.y + particleToCameraMatrix.cc * point.z + particleToCameraMatrix.dc * 1.0f;
		iVar15 = 2;

		if (fVar30 < fVar28) {
			iVar15 = 0;
		}
		else {
			// Check that the particle is within the camera's view frustum.
			for (iVar10 = 0; iVar10 < 4; iVar10 = iVar10 + 1) {
				fVar33 = aCameraPoints[iVar10].x * (particleToCameraMatrix.aa * point.x + particleToCameraMatrix.ba * point.y + particleToCameraMatrix.ca * point.z + particleToCameraMatrix.da * 1.0f) + aCameraPoints[iVar10].y *
					(particleToCameraMatrix.ab * point.x + particleToCameraMatrix.bb * point.y + particleToCameraMatrix.cb * point.z + particleToCameraMatrix.db * 1.0f) + aCameraPoints[iVar10].z * fVar28;

				PARTICLE_LOG(LogLevel::Info, "edPartDrawShaper: iVar21: {} fVar30: {} fVar33: {}", iVar10, fVar33, fVar30);

				if (fVar30 < fVar33) {
					iVar15 = 0;
					break;
				}

				if (fabs(fVar33) < fVar30) {
					iVar15 = 1;
				}
			}
		}

		PARTICLE_LOG(LogLevel::Info, "edPartDrawShaper: iVar15: {}", iVar15);

		if (iVar15 != 0) {
			if (iVar15 == 1) {
				pCurParticle = pParticle + startIndex;
				fVar27 = fVar27 * FLOAT_004485e8;
				pCurRawVector = pRawVectors + startIndex;

				for (iVar15 = startIndex; iVar15 < endIndex; iVar15 = iVar15 + 1) {
					pCurParticle->field_0xd = 0;

					PARTICLE_LOG(LogLevel::Info, "edPartDrawShaper: Particle 0x{:x} state: {}", iVar15, pCurParticle->state);

					if (pCurParticle->state == 2) {
						fVar29 = fVar27 * pCurParticle->yScale * (float)(uint)pCurParticle->field_0x4;
						edF32VECTOR4 pos = *pCurRawVector;

						PARTICLE_LOG(LogLevel::Info, "edPartDrawShaper: pos: ({}, {}, {}, {})", pos.x, pos.y, pos.z, pos.w);

						fVar28 = particleToCameraMatrix.ac * pos.x + particleToCameraMatrix.bc * pos.y + particleToCameraMatrix.cc * pos.z + particleToCameraMatrix.dc * pos.w;
						iVar10 = 2;

						if (fVar29 < fVar28) {
							iVar10 = 0;
						}
						else {
							for (iVar21 = 0; iVar21 < 4; iVar21 = iVar21 + 1) {
								fVar34 = aCameraPoints[iVar21].x * (particleToCameraMatrix.aa * pos.x + particleToCameraMatrix.ba * pos.y + particleToCameraMatrix.ca * pos.z + particleToCameraMatrix.da * pos.w) +
									aCameraPoints[iVar21].y * (particleToCameraMatrix.ab * pos.x + particleToCameraMatrix.bb * pos.y + particleToCameraMatrix.cb * pos.z + particleToCameraMatrix.db * pos.w) +
									aCameraPoints[iVar21].z * fVar28;

								// Check each corner of the particle's bounding box against the camera's view frustum.
								PARTICLE_LOG(LogLevel::Info, "edPartDrawShaper: iVar21: {} fVar30: {} fVar33: {}", iVar21, fVar29, fVar34);

								if (fVar29 < fVar34) {
									iVar10 = 0;
									break;
								}

								if (fabs(fVar34) < fVar29) {
									iVar10 = 1;
								}
							}
						}

						if (iVar10 != 0) {
							pCurParticle->field_0xd = 1;
							nbParticlesToDraw = nbParticlesToDraw + 1;
						}
					}

					pCurParticle = pCurParticle + 1;
					pCurRawVector = pCurRawVector + 1;
				}
			}
			else {
				pCurParticle = pParticle + startIndex;

				for (iVar15 = startIndex; iVar15 < endIndex; iVar15 = iVar15 + 1) {
					if (pCurParticle->state == 2) {
						pCurParticle->field_0xd = 1;
						nbParticlesToDraw = nbParticlesToDraw + 1;
					}
					else {
						pCurParticle->field_0xd = 0;
					}

					pCurParticle = pCurParticle + 1;
				}
			}

			if (nbParticlesToDraw != 0) {
				PARTICLE_LOG(LogLevel::Info, "edPartDrawShaper: nbParticlesToDraw: 0x{:x}", nbParticlesToDraw);

				pCurParticle = pParticle + startIndex;
				pCurVector = pVector + startIndex;
				pCurRawVector = pRawVectors + startIndex;

				bVar1 = pDrawData->field_0x4;

				if (bVar1 == 3) {
					IMPLEMENTATION_GUARD(
					peVar18 = &pDrawData->field_0xd0;
					pCurVector = local_170;
					iVar15 = 4;
					do {
						uVar6 = *(undefined8*)peVar18;
						fVar27 = peVar18->ac;
						fVar28 = peVar18->ad;
						iVar15 = iVar15 + -1;
						pCurVector->x = (float)uVar6;
						pCurVector->y = (float)((ulong)uVar6 >> 0x20);
						pCurVector->z = fVar27;
						pCurVector->w = fVar28;
						peVar18 = (edF32MATRIX4*)&peVar18->ba;
						pCurVector = pCurVector + 1;
					} while (0 < iVar15);
					fVar27 = 1.0 / (SQRT(local_170[0].x * local_170[0].x + local_170[0].y * local_170[0].y +
						local_170[0].z * local_170[0].z) + 0.0);
					local_170[0].x = local_170[0].x * fVar27;
					local_170[0].y = local_170[0].y * fVar27;
					local_170[0].z = local_170[0].z * fVar27;
					local_170[0].w = 0.0;
					fVar27 = 1.0 / (SQRT(local_170[1].x * local_170[1].x + local_170[1].y * local_170[1].y +
						local_170[1].z * local_170[1].z) + 0.0);
					local_170[1].x = local_170[1].x * fVar27;
					local_170[1].y = local_170[1].y * fVar27;
					local_170[1].z = local_170[1].z * fVar27;
					local_170[1].w = 0.0;
					fVar27 = 1.0 / (SQRT(local_170[2].x * local_170[2].x + local_170[2].y * local_170[2].y +
						local_170[2].z * local_170[2].z) + 0.0);
					local_170[2].x = local_170[2].x * fVar27;
					local_170[2].y = local_170[2].y * fVar27;
					local_170[2].z = local_170[2].z * fVar27;
					local_170[2].w = 0.0;
					for (; startIndex < endIndex; startIndex = startIndex + 1) {
						pNode = pCurParticle->pNode;
						if ((pCurParticle->field_0x0 == 2) && (pCurParticle->state == 2)) {
							ed3DHierarchyNodeClrFlag(pNode, 0x40);
							peVar5 = pNode->pData;
							pCurVector = local_170;
							iVar15 = 8;
							peVar19 = peVar5;
							do {
								iVar15 = iVar15 + -1;
								fVar27 = pCurVector->y;
								peVar19->flags_0x0 = (uint)pCurVector->x;
								pCurVector = (edF32VECTOR4*)&pCurVector->z;
								*(float*)&peVar19->materialIndex = fVar27;
								peVar19 = (ed_3d_sprite*)&peVar19->offsetA;
							} while (0 < iVar15);
							edF32Matrix4MulF32Vector4Hard
							((edF32VECTOR4*)&peVar5->bUseShadowMatrix_0x30, &pDrawData->field_0xd0, pCurRawVector);
							*(undefined4*)&peVar5->nbBatches = 0x3f800000;
							fVar27 = gParticleSizeScale * pCurParticle->yScale * (float)(uint)pCurParticle->field_0x4 * 0.5;
							peVar5->flags_0x0 = (uint)((float)peVar5->flags_0x0 * fVar27);
							*(float*)&peVar5->materialIndex = *(float*)&peVar5->materialIndex * fVar27;
							peVar5->offsetA = (int)((float)peVar5->offsetA * fVar27);
							peVar5->pNext = (ed_3d_sprite*)((float)peVar5->pNext * fVar27);
							(peVar5->boundingSphere).x = (peVar5->boundingSphere).x * fVar27;
							(peVar5->boundingSphere).y = (peVar5->boundingSphere).y * fVar27;
							(peVar5->boundingSphere).z = (peVar5->boundingSphere).z * fVar27;
							(peVar5->boundingSphere).w = (peVar5->boundingSphere).w * fVar27;
							peVar5->pSTBuf = (char*)((float)peVar5->pSTBuf * fVar27);
							peVar5->pColorBuf = (char*)((float)peVar5->pColorBuf * fVar27);
							peVar5->pVertexBuf = (edF32VECTOR4*)((float)peVar5->pVertexBuf * fVar27);
							peVar5->pWHBuf = (char*)((float)peVar5->pWHBuf * fVar27);
						}
						else {
							ed3DHierarchyNodeSetFlag(pNode, 0x40);
						}
						pCurParticle = pCurParticle + 1;
						pCurRawVector = pCurRawVector + 1;
					})
				}
				else {
					if ((bVar1 == 1) || (bVar1 == 4)) {
						if (pDrawData->field_0x124[0] != (DisplayList*)0x0) {
							edDListSetCurrent(pDrawData->field_0x124[0]);
						}

						edDListLoadMatrix(&pDrawData->field_0xd0);
						uVar2 = pDrawData->field_0x80;
						fVar27 = pDrawData->field_0xc0 * 255.0f;
						if (2.147484e+09 <= fVar27) {
							fVar27 = fVar27 - 2.147484e+09;
						}

						local_900 = 0;
						uVar26 = (int)fVar27 & 0xff;
						if ((pDrawData->field_0x18d & 0x1f) != 0) {
							IMPLEMENTATION_GUARD(
							ed3DSceneComputeCameraToScreenMatrix(pDrawData->field_0x198, &local_1b0);
							edF32Matrix4MulF32Matrix4Hard(&eStack496, &pDrawData->field_0x198->pCamera->worldToCamera, &local_1b0);
							if ((pDrawData->field_0x18d & 3) != 0) {
								pCurParticle = pParticle + startIndex;
								for (iVar15 = startIndex; iVar15 < endIndex; iVar15 = iVar15 + 1) {
									iVar10 = 0;
									if (pCurParticle->field_0xd != 0) {
										for (; iVar21 = local_900, iVar10 < local_900; iVar10 = iVar10 + 1) {
											if ((pDrawData->field_0x18d & 1) == 0) {
												if (table_sorted_values$1919[iVar10] < pCurParticle->age) break;
											}
											else {
												if (pCurParticle->age < table_sorted_values$1919[iVar10]) break;
											}
										}
										for (; iVar10 < iVar21; iVar21 = iVar21 + -1) {
											table_sorted_values$1919[iVar21] = (float)table_sorted_indices$1918[iVar21 + 0xff];
											table_sorted_indices$1918[iVar21] = *(int*)(&DAT_00482fac + iVar21 * 4);
										}
										local_900 = local_900 + 1;
										table_sorted_values$1919[iVar10] = pCurParticle->age;
										table_sorted_indices$1918[iVar10] = iVar15;
									}
									pCurParticle = pCurParticle + 1;
								}
								table_sorted_indices$1918[local_900] = -1;
								table_sorted_values$1919[local_900] = (float)&DAT_bf800000;
							}

							if (((pDrawData->field_0x18d & 0x10) != 0) && (local_900 != 0)) {
								fVar27 = 0.0;
								pCurParticle = pParticle + startIndex;
								pCurRawVector = pRawVectors + startIndex;
								bVar1 = pDrawData->field_0x18f;
								for (iVar15 = 0; iVar15 < local_900; iVar15 = iVar15 + 1) {
									pCurParticle = pParticle + table_sorted_indices$1918[iVar15];
									pCurRawVector = pRawVectors + table_sorted_indices$1918[iVar15];
									if (pCurParticle->field_0xd != 0) {
										if (fVar27 < (float)(uint)bVar1) {
											edF32Matrix4MulF32Vector4Hard(&eStack512, &eStack496, pCurRawVector);
											eStack512.w = gParticleSizeScale * pCurParticle->yScale * (float)(uint)pCurParticle->field_0x4 * 0.5 *
												ABS(1.0 / eStack512.w) * ABS(local_1b0.aa);
											if (max_size$1966 < eStack512.w) {
												eStack512.w = max_size$1966;
											}
											fVar27 = fVar27 + eStack512.w * eStack512.w;
											if ((float)(uint)bVar1 < fVar27) {
												pCurParticle->field_0xd = 0;
												nbParticlesToDraw = nbParticlesToDraw + -1;
											}
											else {
												pCurParticle->field_0xd = 1;
											}
										}
										else {
											pCurParticle->field_0xd = 0;
											nbParticlesToDraw = nbParticlesToDraw + -1;
										}
									}
								}
							}
							if ((pDrawData->field_0x18d & 0xc) != 0) {
								local_900 = 0;
								pCurParticle = pParticle + startIndex;
								pCurRawVector = pRawVectors + startIndex;
								for (iVar15 = startIndex; iVar15 < endIndex; iVar15 = iVar15 + 1) {
									if (pCurParticle->field_0xd != 0) {
										edF32Matrix4MulF32Vector4Hard(&eStack528, &eStack496, pCurRawVector);
										for (iVar10 = 0; iVar21 = local_900, iVar10 < local_900; iVar10 = iVar10 + 1) {
											if ((pDrawData->field_0x18d & 4) == 0) {
												if (eStack528.z < table_sorted_values$1919[iVar10]) break;
											}
											else {
												if (table_sorted_values$1919[iVar10] < eStack528.z) break;
											}
										}
										for (; iVar10 < iVar21; iVar21 = iVar21 + -1) {
											table_sorted_values$1919[iVar21] = (float)table_sorted_indices$1918[iVar21 + 0xff];
											table_sorted_indices$1918[iVar21] = *(int*)(&DAT_00482fac + iVar21 * 4);
										}
										local_900 = local_900 + 1;
										table_sorted_values$1919[iVar10] = pCurParticle->age;
										table_sorted_indices$1918[iVar10] = iVar15;
									}
									pCurParticle = pCurParticle + 1;
									pCurRawVector = pCurRawVector + 1;
								}
								table_sorted_indices$1918[local_900] = -1;
								table_sorted_values$1919[local_900] = (float)&DAT_bf800000;
							})
						}

						if (pDrawData->field_0x44.pData->pManager == (ed_g2d_manager*)0x0) {
							edDListUseMaterial((edDList_material*)0x0);
						}
						else {
							edDListUseMaterial(pDrawData->field_0x44.pData);
						}

						edDListBlendSet(1);

						if (pDrawData->field_0x4 == 1) {
							edDListBegin(0.0f, 0.0f, 0.0f, 0xb, nbParticlesToDraw);
							local_28e = (uVar2 & 1) != 0;
							local_290 = '\0';
							if ((bool)local_28e) {
								uv0.v = 1.0f;
								uv3.u = 1.0f;
								uv2.u = 0.0f;
								uv2.v = 0.0f;
								uv3.v = 0.0f;
							}
							else {
								uv0.v = 0.0f;
							}

							uv1.v = 1.0f;
							uv1.u = 1.0f;
							uv0.u = 0.0f;
							local_28f = '\x01';
							local_26c = '\x01';
							alphaMultiplier = (int)(alpha * 255.0f);
							bSeparateVertexColors = pDrawData->field_0xc4_uint != -1;
							if (bSeparateVertexColors) {
								local_254 = (uint)pDrawData->field_0xc4;
								local_250 = (uint)pDrawData->field_0xc5;
								local_24c = (uint)pDrawData->field_0xc6;
								local_248 = (uint)pDrawData->field_0xc7;
							}
							local_244 = '\x01';

							aspectRatio = pDrawData->aspectRatio;

							pfVar23 = FLOAT_00431860;

							local_230 = particleToCameraMatrix.rowX;
							local_220 = particleToCameraMatrix.rowY;
						}
						else {
							edDListBegin(0.0f, 0.0f, 0.0f, 8, nbParticlesToDraw << 2);
							uv0.u = 0.0f;
							local_290 = '\x01';
							local_28f = '\x01';
							local_28e = '\x01';
							local_26c = '\x01';
							uv0.v = 1.0;
							alphaMultiplier = (int)(alpha * 255.0);
							uv1.u = 1.0f;
							uv1.v = 1.0f;
							uv3.u = 1.0f;
							uv2.u = 0.0f;
							uv2.v = 0.0f;
							uv3.v = 0.0f;

							bSeparateVertexColors = pDrawData->field_0xc4_uint != -1;
							if (bSeparateVertexColors) {
								local_254 = (uint)pDrawData->field_0xc4;
								local_250 = (uint)pDrawData->field_0xc5;
								local_24c = (uint)pDrawData->field_0xc6;
								local_248 = (uint)pDrawData->field_0xc7;
							}

							aspectRatio = pDrawData->aspectRatio;

							static edF32VECTOR4 edF32VECTOR4_00431720 = { 0.0f, 0.0f, -1.0f, 0.0f };

							pfVar23 = FLOAT_00431880;
							edF32Matrix4FromEulerSoft(&eStack720, &pDrawData->field_0x20, "XYZ");
							local_230 = gF32Vector4UnitX;
							local_220 = edF32VECTOR4_00431720;
						}

						if (((((((uVar2 & 1) == 0) || ((uVar2 & 0x20) == 0)) || ((short)(uVar2 & 0x400) >> 10 != 1)) ||
							(((short)(uVar2 & 0x800) >> 0xb != 0 || ((uVar2 & 8) == 0)))) ||
							((((uVar2 & 0x300) != 0 || (((uVar2 & 2) == 0 || ((uVar2 & 4) == 0)))) || ((uVar2 & 0x40) != 0)))) ||
							((((uVar26 != 0 || (local_290 != '\0')) || (local_28f == '\0')) ||
								(((local_28e == '\0' || (local_26c == '\0')) ||
									((bSeparateVertexColors != false || ((local_244 == '\0' || ((pDrawData->field_0x18d & 0xc) != 0)))))))))) {
							if (((((uVar2 & 1) == 0) ||
								((((uVar2 & 0x20) == 0 || ((short)(uVar2 & 0x400) >> 10 != 0)) ||
									((short)(uVar2 & 0x800) >> 0xb != 0)))) ||
								((((uVar2 & 8) == 0 || ((uVar2 & 0x300) != 0)) ||
									(((uVar2 & 2) == 0 || (((uVar2 & 4) == 0 || ((uVar2 & 0x40) != 0)))))))) ||
								((uVar26 != 0 ||
									(((((local_290 != '\0' || (local_28f == '\0')) || (local_28e == '\0')) ||
										((local_26c == '\0' || (bSeparateVertexColors != false)))) ||
										((local_244 == '\0' || ((pDrawData->field_0x18d & 0xc) != 0)))))))) {
								if (((((uVar2 & 1) == 0) || ((uVar2 & 0x20) == 0)) || ((short)(uVar2 & 0x400) >> 10 != 1)) ||
									((((((((short)(uVar2 & 0x800) >> 0xb != 0 || ((uVar2 & 8) == 0)) || ((uVar2 & 0x300) != 0)) ||
										(((uVar2 & 2) != 0 || ((uVar2 & 4) != 0)))) ||
										(((uVar2 & 0x40) != 0 || ((uVar26 != 0 || (local_290 != '\0')))))) || (local_28f == '\0')) ||
										((((local_28e == '\0' || (local_26c == '\0')) || (bSeparateVertexColors != false)) ||
											((local_244 == '\0' || ((pDrawData->field_0x18d & 0xc) != 0)))))))) {
									if (((((uVar2 & 1) == 0) || (((uVar2 & 0x20) == 0 || ((short)(uVar2 & 0x400) >> 10 != 1)))) ||
										((short)(uVar2 & 0x800) >> 0xb != 0)) ||
										(((((((((uVar2 & 8) == 0 || ((uVar2 & 0x300) != 0)) || ((uVar2 & 2) == 0)) ||
											(((uVar2 & 4) == 0 || ((uVar2 & 0x40) != 0)))) || (uVar26 == 0)) ||
											((local_290 != '\0' || (local_28f == '\0')))) ||
											((local_28e == '\0' || (((local_26c == '\0' || (bSeparateVertexColors != false)) || (local_244 == '\0'))))))
											|| ((pDrawData->field_0x18d & 0xc) != 0)))) {
										if ((((((uVar2 & 1) == 0) || ((uVar2 & 0x20) != 0)) ||
											(((uVar2 & 0x300) == 0 || (((uVar2 & 0x40) == 0 || ((uVar2 & 0x10) != 0)))))) ||
											(((uVar2 & 0x80) == 0 || (((uVar26 == 0 || (local_290 != '\0')) || (local_28f == '\0')))))) ||
											(((local_28e == '\0' || (local_26c == '\0')) ||
												((bSeparateVertexColors != false || ((local_244 == '\0' || ((pDrawData->field_0x18d & 0xc) != 0)))))))) {
											if ((((uVar2 & 1) == 0) ||
												(((((uVar2 & 0x20) == 0 || ((short)(uVar2 & 0x400) >> 10 != 0)) ||
													((short)(uVar2 & 0x800) >> 0xb != 0)) || (((uVar2 & 8) != 0 || ((uVar2 & 0x300) != 0))))))
												|| ((((uVar2 & 2) != 0 || (((uVar2 & 4) != 0 || ((uVar2 & 0x40) != 0)))) ||
													((uVar26 != 0 ||
														((((local_290 != '\0' || (local_28f == '\0')) || (local_28e == '\0')) ||
															(((local_26c == '\0' || (bSeparateVertexColors != false)) ||
																((local_244 == '\0' || ((pDrawData->field_0x18d & 0xc) != 0)))))))))))) {
												if (((((uVar2 & 1) == 0) || ((uVar2 & 0x20) != 0)) ||
													(((uVar2 & 0x300) == 0 ||
														((((uVar2 & 2) != 0 || ((uVar2 & 4) != 0)) || ((uVar2 & 0x40) != 0)))))) ||
													(((((uVar26 != 0 || (local_290 != '\0')) ||
														((local_28f == '\0' || ((local_28e == '\0' || (local_26c == '\0')))))) ||
														(bSeparateVertexColors != false)) || ((local_244 == '\0' || ((pDrawData->field_0x18d & 0xc) != 0)))))) {
													if (((((((uVar2 & 1) == 0) ||
														(((((uVar2 & 0x20) == 0 || ((short)(uVar2 & 0x400) >> 10 != 0)) ||
															((short)(uVar2 & 0x800) >> 0xb != 0)) ||
															(((uVar2 & 8) != 0 || ((uVar2 & 0x300) != 0)))))) || ((uVar2 & 2) != 0)) ||
														((((uVar2 & 4) != 0 || ((uVar2 & 0x40) == 0)) ||
															(((uVar2 & 0x10) == 0 ||
																((((uVar2 & 0x80) == 0 || (uVar26 != 0)) || (local_290 != '\0')))))))) ||
														((local_28f == '\0' || (local_28e == '\0')))) ||
														((local_26c == '\0' ||
															(((bSeparateVertexColors != false || (local_244 == '\0')) || ((pDrawData->field_0x18d & 0xc) != 0))))))
													{
														if (((((((uVar2 & 1) == 0) || ((uVar2 & 0x20) != 0)) || ((uVar2 & 0x300) != 0)) ||
															(((uVar2 & 2) == 0 || ((uVar2 & 4) == 0)))) ||
															((((uVar2 & 0x40) == 0 || (((uVar2 & 0x10) != 0 || ((uVar2 & 0x80) == 0)))) ||
																(uVar26 != 0)))) ||
															((((((local_290 != '\0' || (local_28f == '\0')) || (local_28e == '\0')) ||
																((local_26c == '\0' || (bSeparateVertexColors != false)))) || (local_244 == '\0')) ||
																((pDrawData->field_0x18d & 0xc) != 0)))) {
															if (((((uVar2 & 1) == 0) || ((uVar2 & 0x20) == 0)) ||
																(((short)(uVar2 & 0x400) >> 10 != 0 ||
																	((((short)(uVar2 & 0x800) >> 0xb != 1 || ((uVar2 & 0x300) != 0)) ||
																		((uVar2 & 2) != 0)))))) ||
																((((((uVar2 & 4) != 0 || ((uVar2 & 0x40) != 0)) || (uVar26 != 0)) ||
																	((local_290 != '\0' || (local_28f == '\0')))) ||
																	(((local_28e == '\0' ||
																		(((local_26c == '\0' || (bSeparateVertexColors != false)) || (local_244 == '\0')))) ||
																		((pDrawData->field_0x18d & 0xc) != 0)))))) {
																if (((((uVar2 & 1) == 0) || ((uVar2 & 0x20) == 0)) ||
																	(((short)(uVar2 & 0x400) >> 10 != 0 ||
																		(((short)(uVar2 & 0x800) >> 0xb != 0 || ((uVar2 & 8) == 0)))))) ||
																	(((((uVar2 & 0x300) != 0 ||
																		((((((uVar2 & 2) != 0 || ((uVar2 & 4) != 0)) || ((uVar2 & 0x40) != 0)) ||
																			((uVar26 != 0 || (local_290 != '\0')))) ||
																			((local_28f == '\0' || ((local_28e == '\0' || (local_26c == '\0')))))))) ||
																		(bSeparateVertexColors != false)) ||
																		((local_244 == '\0' || ((pDrawData->field_0x18d & 0xc) != 0)))))) {
																	if (((((uVar2 & 1) == 0) ||
																		((((uVar2 & 0x20) == 0 || ((short)(uVar2 & 0x400) >> 10 != 1)) ||
																			((short)(uVar2 & 0x800) >> 0xb != 1)))) ||
																		((((uVar2 & 0x300) != 0 || ((uVar2 & 2) == 0)) || ((uVar2 & 4) == 0)))) ||
																		(((((uVar2 & 0x40) != 0 || (uVar26 != 0)) ||
																			(((local_290 != '\0' ||
																				(((local_28f == '\0' || (local_28e == '\0')) || (local_26c == '\0')))) ||
																				((bSeparateVertexColors != false || (local_244 == '\0')))))) ||
																			((pDrawData->field_0x18d & 0xc) != 0)))) {
																		if ((((((uVar2 & 1) == 0) || ((uVar2 & 0x20) == 0)) ||
																			(((short)(uVar2 & 0x400) >> 10 != 1 ||
																				(((((short)(uVar2 & 0x800) >> 0xb != 0 || ((uVar2 & 8) != 0)) ||
																					((uVar2 & 0x300) != 0)) || (((uVar2 & 2) != 0 || ((uVar2 & 4) != 0))))))))
																			|| (((uVar2 & 0x40) != 0 || ((uVar26 != 0 || (local_290 != '\0')))))) ||
																			(((local_28f == '\0' ||
																				(((local_28e == '\0' || (local_26c == '\0')) || (bSeparateVertexColors != false)))) ||
																				((local_244 == '\0' || ((pDrawData->field_0x18d & 0xc) != 0)))))) {
																			if (((uVar2 & 1) == 0) ||
																				(((((((uVar2 & 0x20) != 0 || ((uVar2 & 0x300) != 0)) ||
																					(((uVar2 & 2) == 0 ||
																						((((uVar2 & 4) != 0 || ((uVar2 & 0x40) != 0)) || (uVar26 != 0)))))) ||
																					((local_290 != '\0' || (local_28f == '\0')))) || (local_28e == '\0')) ||
																					(((local_26c == '\0' || (bSeparateVertexColors != false)) ||
																						((local_244 == '\0' || ((pDrawData->field_0x18d & 0xc) != 0)))))))) {
																				if (((((((uVar2 & 1) == 0) || ((uVar2 & 0x20) != 0)) || ((uVar2 & 0x300) != 0))
																					|| (((uVar2 & 2) != 0 || ((uVar2 & 4) != 0)))) || ((uVar2 & 0x40) != 0)) ||
																					((((uVar26 == 0 || (local_290 != '\0')) ||
																						((local_28f == '\0' ||
																							(((local_28e == '\0' || (local_26c == '\0')) || (bSeparateVertexColors != false))))))
																						|| ((local_244 == '\0' || ((pDrawData->field_0x18d & 0xc) != 0)))))) {
																					if ((((uVar2 & 1) == 0) || (((uVar2 & 0x20) != 0 || ((uVar2 & 0x300) != 0))))
																						|| (((uVar2 & 2) != 0 ||
																							((((((uVar2 & 4) != 0 || ((uVar2 & 0x40) == 0)) ||
																								((uVar2 & 0x10) == 0)) ||
																								((((uVar2 & 0x80) == 0 || (uVar26 != 0)) ||
																									((local_290 != '\0' || ((local_28f == '\0' || (local_28e == '\0'))))
																										)))) || ((local_26c == '\0' ||
																											(((bSeparateVertexColors != false || (local_244 == '\0')) ||
																												((pDrawData->field_0x18d & 0xc) != 0)))))))))) {
																						if (((((uVar2 & 1) == 0) || ((uVar2 & 0x20) == 0)) ||
																							(((((short)(uVar2 & 0x400) >> 10 != 0 ||
																								(((short)(uVar2 & 0x800) >> 0xb != 1 || ((uVar2 & 0x300) != 0)))) ||
																								((uVar2 & 2) != 0)) ||
																								(((((uVar2 & 4) != 0 || ((uVar2 & 0x40) != 0)) || (uVar26 == 0)) ||
																									((local_290 != '\0' || (local_28f == '\0')))))))) ||
																							(((local_28e == '\0' || ((local_26c == '\0' || (bSeparateVertexColors != false)))) ||
																								((local_244 == '\0' || ((pDrawData->field_0x18d & 0xc) != 0)))))) {
																							if ((((((((uVar2 & 1) == 0) || ((uVar2 & 0x20) != 0)) ||
																								((uVar2 & 0x300) != 0)) ||
																								((((uVar2 & 2) == 0 || ((uVar2 & 4) == 0)) ||
																									(((uVar2 & 0x40) == 0 ||
																										(((uVar2 & 0x10) == 0 || ((uVar2 & 0x80) == 0)))))))) ||
																								(uVar26 != 0)) ||
																								((((local_290 != '\0' || (local_28f == '\0')) || (local_28e == '\0'))
																									|| ((local_26c == '\0' || (bSeparateVertexColors != false)))))) ||
																								((local_244 == '\0' || ((pDrawData->field_0x18d & 0xc) != 0)))) {
																								if ((((((uVar2 & 1) == 0) || ((uVar2 & 0x20) == 0)) ||
																									((short)(uVar2 & 0x400) >> 10 != 1)) ||
																									((((((short)(uVar2 & 0x800) >> 0xb != 0 || ((uVar2 & 8) == 0)) ||
																										((uVar2 & 0x300) != 0)) ||
																										(((uVar2 & 2) != 0 || ((uVar2 & 4) == 0)))) ||
																										(((uVar2 & 0x40) != 0 || ((uVar26 != 0 || (local_290 != '\0')))))))
																									) || ((local_28f == '\0' ||
																										((((local_28e == '\0' || (local_26c == '\0')) ||
																											(bSeparateVertexColors != false)) ||
																											((local_244 == '\0' || ((pDrawData->field_0x18d & 0xc) != 0))
																												)))))) {
																									if ((((((uVar2 & 1) == 0) ||
																										(((uVar2 & 0x20) != 0 || ((uVar2 & 0x300) != 0)))) ||
																										((uVar2 & 2) == 0)) ||
																										((((uVar2 & 4) == 0 || ((uVar2 & 0x40) == 0)) ||
																											((uVar2 & 0x10) != 0)))) ||
																										(((((uVar2 & 0x80) != 0 || (uVar26 != 0)) ||
																											((local_290 != '\0' ||
																												((local_28f == '\0' || (local_28e == '\0')))))) ||
																											((local_26c == '\0' ||
																												(((bSeparateVertexColors != false || (local_244 == '\0')) ||
																													((pDrawData->field_0x18d & 0xc) != 0)))))))) {
																										if ((((((uVar2 & 1) == 0) || ((uVar2 & 0x20) == 0)) ||
																											(((short)(uVar2 & 0x400) >> 10 != 0 ||
																												(((short)(uVar2 & 0x800) >> 0xb != 0 || ((uVar2 & 8) != 0)))))
																											) || (((uVar2 & 0x300) != 0 ||
																												(((((((uVar2 & 2) == 0 || ((uVar2 & 4) != 0)) ||
																													((uVar2 & 0x40) != 0)) ||
																													((uVar26 != 0 || (local_290 != '\0')))) ||
																													(local_28f == '\0')) ||
																													((local_28e == '\0' || (local_26c == '\0')))))))) ||
																											((bSeparateVertexColors != false ||
																												((local_244 == '\0' || ((pDrawData->field_0x18d & 0xc) != 0)))))
																											) {
																											if ((((uVar2 & 1) == 0) ||
																												(((((uVar2 & 0x20) == 0 || ((short)(uVar2 & 0x400) >> 10 != 0)
																													) || ((short)(uVar2 & 0x800) >> 0xb != 0)) ||
																													(((uVar2 & 8) == 0 || ((uVar2 & 0x300) != 0)))))) ||
																												(((((uVar2 & 2) != 0 ||
																													(((uVar2 & 4) != 0 || ((uVar2 & 0x40) != 0)))) ||
																													(uVar26 == 0)) ||
																													((((local_290 != '\0' || (local_28f == '\0')) ||
																														(local_28e == '\0')) ||
																														(((local_26c == '\0' || (bSeparateVertexColors != false)) ||
																															((local_244 == '\0' || ((pDrawData->field_0x18d & 0xc) != 0)
																																))))))))) {
																												if (((((uVar2 & 1) == 0) || ((uVar2 & 0x20) == 0)) ||
																													((((short)(uVar2 & 0x400) >> 10 != 1 ||
																														(((((short)(uVar2 & 0x800) >> 0xb != 0 ||
																															((uVar2 & 8) == 0)) || ((uVar2 & 0x300) != 0)) ||
																															(((uVar2 & 2) == 0 || ((uVar2 & 4) == 0)))))) ||
																														((((uVar2 & 0x40) == 0 ||
																															(((uVar2 & 0x10) == 0 || ((uVar2 & 0x80) == 0)))) ||
																															(uVar26 != 0)))))) ||
																													((((local_290 != '\0' || (local_28f == '\0')) ||
																														(local_28e == '\0')) ||
																														(((local_26c == '\0' || (bSeparateVertexColors != false)) ||
																															((local_244 == '\0' ||
																																((pDrawData->field_0x18d & 0xc) != 0)))))))) {
																													if (((((uVar2 & 1) == 0) || ((uVar2 & 0x20) == 0)) ||
																														(((short)(uVar2 & 0x400) >> 10 != 1 ||
																															((((((short)(uVar2 & 0x800) >> 0xb != 1 ||
																																((uVar2 & 0x300) != 0)) || ((uVar2 & 2) != 0)) ||
																																((((uVar2 & 4) != 0 || ((uVar2 & 0x40) != 0)) ||
																																	(((uVar2 & 0x10) != 0 ||
																																		(((uVar2 & 0x80) != 0 || (uVar26 != 0)))))))) ||
																																(local_290 != '\0')))))) ||
																														(((((local_28f == '\0' || (local_28e == '\0')) ||
																															(local_26c == '\0')) ||
																															((bSeparateVertexColors != false || (local_244 == '\0')))) ||
																															((pDrawData->field_0x18d & 0xc) != 0)))) {
																														if (((((uVar2 & 1) == 0) || ((uVar2 & 0x20) == 0)) ||
																															(((short)(uVar2 & 0x400) >> 10 != 0 ||
																																((((short)(uVar2 & 0x800) >> 0xb != 0 ||
																																	((uVar2 & 8) != 0)) || ((uVar2 & 0x300) != 0)))))) ||
																															((((((uVar2 & 2) != 0 || ((uVar2 & 4) != 0)) ||
																																((uVar2 & 0x40) != 0)) ||
																																(((uVar2 & 0x10) != 0 || ((uVar2 & 0x80) != 0)))) ||
																																(((((uVar26 == 0 ||
																																	(((local_290 != '\0' || (local_28f == '\0')) ||
																																		(local_28e == '\0')))) ||
																																	((local_26c == '\0' || (bSeparateVertexColors != false)))) ||
																																	(local_244 == '\0')) ||
																																	((pDrawData->field_0x18d & 0xc) != 0)))))) {
																															if (((local_290 == '\0') && (local_28f != '\0')) &&
																																((local_28e != '\0' &&
																																	((((local_26c != '\0' && (bSeparateVertexColors == '\0')) &&
																																		(local_244 != '\0')) &&
																																		((pDrawData->field_0x18d & 0xc) == 0)))))) {
																																// Fall_Leaves.g2d
																																pCurParticle = pParticle + startIndex;
																																pCurRawVector = pRawVectors + startIndex;

																																for (; startIndex < endIndex; startIndex = startIndex + 1) {
																																	if (pCurParticle->field_0xd != 0) {
																																		//continue;

																																		if ((pDrawData->field_0x18d & 0xc) != 0) {
																																			iVar15 = table_sorted_indices$1918[startIndex];
																																			pCurParticle = pParticle + iVar15;
																																			pCurRawVector = pRawVectors + iVar15;
																																			pCurVector = pVector + iVar15;
																																		}

																																		uVar12 = pCurParticle->seed;

																																		if ((uVar2 & 1) != 0) {
																																			fVar27 = pCurParticle->age / pCurParticle->lifetime;
																																			if ((uVar2 & 0x20) == 0) {
																																				local_710 = gDefaultQuad;
																																			}
																																			else {
																																				if ((short)(uVar2 & 0x400) >> 10 == 0) {
																																					fVar28 = pDrawData->field_0x90;
																																				}
																																				else {
																																					fVar28 = ((float)(uVar12 & 0xffff) * pDrawData->field_0x90) / 65535.0f;
																																				}

																																				if ((short)(uVar2 & 0x800) >> 0xb == 1) {
																																					ParticleA(local_710, pCurVector, pCurParticle, local_220, local_230, fVar28, pDrawData);
																																				}
																																				else {
																																					fVar29 = pDrawData->field_0x94 * fVar27;
																																					if (((uVar2 & 8) != 0) && ((uVar12 & 0x20) != 0)) {
																																						fVar29 = -fVar29;
																																					}
																																					ParticleC(local_710, fVar29);
																																				}
																																			}

																																			if ((uVar2 & 0x300) != 0) {
																																				FLOAT_ARRAY_0041eb50.m00 = pDrawData->field_0x88;
																																				FLOAT_ARRAY_0041eb50.m10 = pDrawData->field_0x8c;
																																				FLOAT_ARRAY_0041eb50.m11 = FLOAT_ARRAY_0041eb50.m00 * -0.5f + 0.5f;
																																				FLOAT_ARRAY_0041eb50.m12 = FLOAT_ARRAY_0041eb50.m10 * -0.5f + 0.5f;

																																				ApplyAffineC(FLOAT_ARRAY_0041eb50, local_710);
																																			}

																																			if (((uVar2 & 2) != 0) && ((uVar12 & 0x40) != 0)) {
																																				ApplyAffineC(FLOAT_ARRAY_0041eb70, local_710);
																																			}

																																			if (((uVar2 & 4) != 0) && ((uVar12 & 0x80) != 0)) {
																																				ApplyAffineC(FLOAT_ARRAY_0041eb90, local_710);
																																			}

																																			if ((uVar2 & 0x40) != 0) {
																																				ParticleB(local_710, uVar2, uVar12, pDrawData, pCurParticle);
																																			}

																																			TransformUVs(pfVar23, local_710, uv0, uv1, uv2, uv3);
																																		}

																																		if (uVar26 == 0) {
																																			ParticleColorA(rgbaColor, pCurParticle, alphaMultiplier);
																																		}
																																		else {
																																			ParticleColorB(rgbaColor, pCurParticle, alphaMultiplier, pDrawData, uVar26);
																																		}

																																		uVar12 = rgbaColor[0].rgba;
																																		local_58 = 0;
																																		if (bSeparateVertexColors != false) {
																																			ParticleColorC(rgbaColor, local_250, local_24c, local_248);
																																		}

																																		height = gParticleSizeScale * pCurParticle->yScale * (float)(uint)pCurParticle->field_0x4 * 0.5f;
																																		width = height * aspectRatio;
																																		edDlistPartVertex(width, height, &uv0, &uv1, &uv2, &uv3, &rgbaColor[0], pCurRawVector);
																																	}

																																	pCurParticle = pCurParticle + 1;
																																	pCurRawVector = pCurRawVector + 1;
																																	pCurVector = pCurVector + 1;
																																}
																															}
																															else {
																																if ((local_290 == '\0') &&
																																	((pDrawData->field_0x18d & 0xc) == 0)) {
																																	IMPLEMENTATION_GUARD(
																																	pCurParticle = pParticle + startIndex;
																																	pCurRawVector = pRawVectors + startIndex;
																																	for (; startIndex < endIndex; startIndex = startIndex + 1) {
																																		if (pCurParticle->field_0xd != 0) {
																																			if ((pDrawData->field_0x18d & 0xc) != 0) {
																																				iVar15 = table_sorted_indices$1918[startIndex];
																																				pCurParticle = pParticle + iVar15;
																																				pCurRawVector = pRawVectors + iVar15;
																																				pCurVector = &pVector->field_0x0 + iVar15 * 2;
																																			}
																																			uVar12 = pCurParticle->seed;
																																			if ((uVar2 & 1) != 0) {
																																				fVar27 = pCurParticle->age / pCurParticle->lifetime;
																																				if ((uVar2 & 0x20) == 0) {
																																					local_76c = 0.0;
																																					local_770 = 1.0;
																																					local_764 = 1.0;
																																					local_768 = 0.0;
																																					local_760 = 0.0;
																																					fStack1884 = 0.0;
																																				}
																																				else {
																																					if ((short)(uVar2 & 0x400) >> 10 == 0) {
																																						fVar28 = pDrawData->field_0x90;
																																					}
																																					else {
																																						fVar28 = ((float)(uVar12 & 0xffff) *
																																							pDrawData->field_0x90) / 65535.0;
																																					}

																																					if ((short)(uVar2 & 0x800) >> 0xb == 1) {
																																						ParticleA(local_770, pCurVector, pCurParticle, local_220, local_230, fVar28, pDrawData);
																																					}
																																					else {
																																						fVar29 = pDrawData->field_0x94 * fVar27;
																																						if (((uVar2 & 8) != 0) && ((uVar12 & 0x20) != 0)) {
																																							fVar29 = -fVar29;
																																						}

																																						ParticleC(local_770, fVar29);
																																					}
																																				}
																																				if ((uVar2 & 0x300) != 0) {
																																					FLOAT_ARRAY_0041ebd0.m00 = pDrawData->field_0x88;
																																					FLOAT_ARRAY_0041ebd0.m10 = pDrawData->field_0x8c;
																																					FLOAT_ARRAY_0041ebd0.m11 = FLOAT_ARRAY_0041ebd0.m00 * -0.5f + 0.5f;
																																					FLOAT_ARRAY_0041ebd0.m12 = FLOAT_ARRAY_0041ebd0.m10 * -0.5f + 0.5f;

																																					ApplyAffineC(FLOAT_ARRAY_0041ebd0, local_770);
																																				}

																																				if (((uVar2 & 2) != 0) && ((uVar12 & 0x40) != 0)) {
																																					ApplyAffineC(FLOAT_ARRAY_0041ebf0, local_770);
																																				}

																																				if (((uVar2 & 4) != 0) && ((uVar12 & 0x80) != 0)) {
																																					ApplyAffineC(FLOAT_ARRAY_0041ec10, local_770);
																																				}

																																				if ((uVar2 & 0x40) != 0) {
																																					ParticleB(local_770, uVar2, uVar12, pDrawData, pCurParticle);
																																				}

																																				TransformUVs(pfVar23, local_770, uv0, uv1, uv2, uv3);
																																			}

																																			if (uVar26 == 0) {
																																				ParticleColorA(rgbaColor, pCurParticle, alphaMultiplier);
																																			}
																																			else {
																																				ParticleColorB(rgbaColor, pCurParticle, alphaMultiplier, pDrawData, uVar26);
																																			}

																																			uVar12 = rgbaColor[0];
																																			puVar17 = &local_68;
																																			if (bSeparateVertexColors != false) {
																																				ParticleColorC(rgbaColor, local_250, local_24c, local_248);
																																			}

																																			height = gParticleSizeScale * pCurParticle->yScale * (float)(uint)pCurParticle->field_0x4 * 0.5;
																																			width = height * aspectRatio;
																																			if (local_28f != '\0') {
																																				if (local_28e == '\0') {
																																					edDListTexCoo2f(0.0, 0.0);
																																					edDListTexCoo2f(1.0, 1.0);
																																				}
																																				else {
																																					edDListTexCoo2f(uv0.u, uv0.v);
																																					edDListTexCoo2f(uv1.u, uv1.v);
																																					edDListTexCoo2f(uv2.u, uv2.v);
																																					edDListTexCoo2f(uv3.u, uv3.v);
																																				}
																																			}
																																			if (local_26c != '\0') {
																																				edDListColor4u8((byte)rgbaColor[0], rgbaColor[0]._1_1_,
																																					rgbaColor[0]._2_1_, rgbaColor[0]._3_1_);
																																				if (bSeparateVertexColors != false) {
																																					edDListColor4u8((byte)rgbaColor[1], rgbaColor[1]._1_1_,
																																						rgbaColor[1]._2_1_, rgbaColor[1]._3_1_);
																																					edDListColor4u8((byte)rgbaColor[2], rgbaColor[2]._1_1_,
																																						rgbaColor[2]._2_1_, rgbaColor[2]._3_1_);
																																					edDListColor4u8((byte)rgbaColor[3], rgbaColor[3]._1_1_,
																																						rgbaColor[3]._2_1_, rgbaColor[3]._3_1_);
																																				}
																																			}
																																			if (local_244 != '\0') {
																																				edDListWidthHeight2f(width, height);
																																			}
																																			edDListVertex4f(pCurRawVector->x, pCurRawVector->y, pCurRawVector->z, 0.0);
																																		}

																																		pCurParticle = pCurParticle + 1;
																																		pCurRawVector = pCurRawVector + 1;
																																		pCurVector = pCurVector + 1;
																																	})
																																}
																																else {
																																	// Water_rings.g2d

																																	if ((pDrawData->field_0x18d & 0xc) == 0) {
																																		pCurParticle = pParticle + startIndex;
																																		pCurRawVector = pRawVectors + startIndex;
																																		local_8d0 = endIndex;
																																	}
																																	else {
																																		startIndex = 0;
																																		local_8d0 = local_900;
																																	}

																																	for (; startIndex < local_8d0; startIndex = startIndex + 1) {
																																		if (pCurParticle->field_0xd != 0) {
																																			//continue;

																																			if ((pDrawData->field_0x18d & 0xc) != 0) {
																																				iVar15 = table_sorted_indices$1918[startIndex];
																																				pCurParticle = pParticle + iVar15;
																																				pCurRawVector = pRawVectors + iVar15;
																																				pCurVector = pVector + iVar15;
																																			}

																																			uVar12 = pCurParticle->seed;
																																			if ((uVar2 & 1) != 0) {
																																				fVar27 = pCurParticle->age / pCurParticle->lifetime;

																																				if ((uVar2 & 0x20) == 0) {
																																					local_7d0 = gDefaultQuad;
																																				}
																																				else {
																																					if ((short)(uVar2 & 0x400) >> 10 == 0) {
																																						fVar28 = pDrawData->field_0x90;
																																					}
																																					else {
																																						fVar28 = ((float)(uVar12 & 0xffff) * pDrawData->field_0x90) / 65535.0f;
																																					}

																																					if ((short)(uVar2 & 0x800) >> 0xb == 1) {
																																						ParticleA(local_7d0, pCurVector, pCurParticle, local_220, local_230, fVar28, pDrawData);
																																					}
																																					else {
																																						fVar29 = pDrawData->field_0x94 * fVar27;
																																						if (((uVar2 & 8) != 0) && ((uVar12 & 0x20) != 0)) {
																																							fVar29 = -fVar29;
																																						}

																																						local_7d0.xAxis.x = cosf(fVar28 + fVar29);
																																						local_7d0.xAxis.y = sinf(fVar28 + fVar29);

																																						local_7d0.origin.x = -(local_7d0.xAxis.x * 0.5f) + local_7d0.xAxis.y * 0.5f + 0.5f;
																																						local_7d0.origin.y = (-(local_7d0.xAxis.y * 0.5f) - local_7d0.xAxis.x * 0.5f) + 0.5f;
																																						local_7d0.yAxis.x = -local_7d0.xAxis.y;
																																						local_7d0.yAxis.y = local_7d0.xAxis.x;
																																					}
																																				}

																																				if ((uVar2 & 0x300) != 0) {
																																					FLOAT_ARRAY_0041ec50.m00 = pDrawData->field_0x88;
																																					FLOAT_ARRAY_0041ec50.m10 = pDrawData->field_0x8c;
																																					FLOAT_ARRAY_0041ec50.m11 = FLOAT_ARRAY_0041ec50.m00 * -0.5f + 0.5f;
																																					FLOAT_ARRAY_0041ec50.m12 = FLOAT_ARRAY_0041ec50.m10 * -0.5f + 0.5f;

																																					ApplyAffineC(FLOAT_ARRAY_0041ec50, local_7d0);
																																				}

																																				if (((uVar2 & 2) != 0) && ((uVar12 & 0x40) != 0)) {
																																					ApplyAffineC(FLOAT_ARRAY_0041ec70, local_7d0);
																																				}

																																				if (((uVar2 & 4) != 0) && ((uVar12 & 0x80) != 0)) {
																																					ApplyAffineC(FLOAT_ARRAY_0041ec90, local_7d0);
																																				}

																																				if ((uVar2 & 0x40) != 0) {
																																					ParticleB(local_7d0, uVar2, uVar12, pDrawData, pCurParticle);
																																				}

																																				TransformUVs(pfVar23, local_7d0, uv0, uv1, uv2, uv3);
																																			}

																																			if (uVar26 == 0) {
																																				ParticleColorA(rgbaColor, pCurParticle, alphaMultiplier);
																																			}
																																			else {
																																				ParticleColorB(rgbaColor, pCurParticle, alphaMultiplier, pDrawData, uVar26);
																																			}

																																			uVar12 = rgbaColor[0].rgba;
																																			puVar17 = &local_78;
																																			if (bSeparateVertexColors != false) {
																																				ParticleColorC(rgbaColor, local_250, local_24c, local_248);
																																			}

																																			fVar28 = gParticleSizeScale * pCurParticle->yScale * (float)(uint)pCurParticle->field_0x4 * 0.5;
																																			fVar27 = fVar28 * aspectRatio;
																																			width = fVar27;
																																			height = fVar28;

																																			if (local_290 == '\0') {
																																				if (local_28f != '\0') {
																																					if (local_28e == '\0') {
																																						edDListTexCoo2f(0.0f, 0.0f);
																																						edDListTexCoo2f(1.0f, 1.0f);
																																					}
																																					else {
																																						edDListTexCoo2f(uv0.u, uv0.v);
																																						edDListTexCoo2f(uv1.u, uv1.v);
																																						edDListTexCoo2f(uv2.u, uv2.v);
																																						edDListTexCoo2f(uv3.u, uv3.v);
																																					}
																																				}

																																				if (local_26c != '\0') {
																																					edDListColor4u8(rgbaColor[0].r, rgbaColor[0].g, rgbaColor[0].b, rgbaColor[0].a);

																																					if (bSeparateVertexColors != false) {
																																						edDListColor4u8(rgbaColor[1].r, rgbaColor[1].g, rgbaColor[1].b, rgbaColor[1].a);
																																						edDListColor4u8(rgbaColor[2].r, rgbaColor[2].g, rgbaColor[2].b, rgbaColor[2].a);
																																						edDListColor4u8(rgbaColor[3].r, rgbaColor[3].g, rgbaColor[3].b, rgbaColor[3].a);
																																					}
																																				}

																																				if (local_244 != '\0') {
																																					IMPLEMENTATION_GUARD(
																																					edDListWidthHeight2f(width, height);)
																																				}

																																				edDListVertex4f(pCurRawVector->x, pCurRawVector->y, pCurRawVector->z, 0.0);
																																			}
																																			else {
																																				edDListTexCoo2f(uv0.u, uv0.v);
																																				edDListColor4u8(rgbaColor[0].r, rgbaColor[0].g, rgbaColor[0].b, rgbaColor[0].a);
																																				edDListVertex4f(pCurRawVector->x - fVar27, pCurRawVector->y, pCurRawVector->z + fVar28, 0.0f);

																																				edDListTexCoo2f(uv1.u, uv1.v);
																																				if (bSeparateVertexColors != false) {
																																					edDListColor4u8(rgbaColor[1].r, rgbaColor[1].g, rgbaColor[1].b, rgbaColor[1].a);
																																				}
																																				edDListVertex4f(pCurRawVector->x + fVar27, pCurRawVector->y, pCurRawVector->z + fVar28, 0.0f);

																																				edDListTexCoo2f(uv2.u, uv2.v);
																																				if (bSeparateVertexColors != false) {
																																					edDListColor4u8(rgbaColor[2].r, rgbaColor[2].g, rgbaColor[2].b, rgbaColor[2].a);
																																				}
																																				edDListVertex4f(pCurRawVector->x - fVar27, pCurRawVector->y, pCurRawVector->z - fVar28, 0.0f);

																																				edDListTexCoo2f(uv3.u, uv3.v);
																																				if (bSeparateVertexColors != false) {
																																					edDListColor4u8(rgbaColor[3].r, rgbaColor[3].g, rgbaColor[3].b, rgbaColor[3].a);
																																				}
																																				edDListVertex4f(pCurRawVector->x + fVar27, pCurRawVector->y, pCurRawVector->z - fVar28, 0.0f);
																																			}
																																		}

																																		pCurParticle = pCurParticle + 1;
																																		pCurRawVector = pCurRawVector + 1;
																																		pCurVector = pCurVector + 1;
																																	}
																																}
																															}
																														}
																														else {
																															IMPLEMENTATION_GUARD(
																															pCurParticle = pParticle + startIndex;
																															pCurRawVector = pRawVectors + startIndex;
																															for (; startIndex < endIndex; startIndex = startIndex + 1) {
																																if (pCurParticle->field_0xd != 0) {
																																	uVar12 = pCurParticle->seed;
																																	fVar27 = pDrawData->field_0x94 * (pCurParticle->age / pCurParticle->lifetime);
																																	if ((uVar12 & 0x20) != 0) {
																																		fVar27 = -fVar27;
																																	}
																																	fVar27 = pDrawData->field_0x90 + fVar27;

																																	ParticleC(local_770, fVar27);
																																	TransformUVs(pfVar23, local_770, uv0, uv1, uv2, uv3);

																																	uVar11 = (int)uVar12 >> 3;
																																	local_44 = (&pDrawData->field_0x9c)[uVar12 & 7];
																																	iVar15 = 0xff - (uVar11 & 0xff);
																																	local_48 = (&pDrawData->angleRotY)[uVar12 & 7];
																																	uVar22 = (pCurParticle->field_0x8 & 0xff00ff) *
																																		(0xff - uVar26) +
																																		((local_44 & 0xff00ff) * iVar15 +
																																			(local_48 & 0xff00ff) * (uVar11 & 0xff) >> 8
																																			& 0xff00ff) * uVar26;
																																	uVar12 = (pCurParticle->field_0x8 >> 8 & 0xff00ff) *
																																		(0xff - uVar26) +
																																		(((local_44 >> 8 & 0xff00ff) * iVar15 +
																																			(local_48 >> 8 & 0xff00ff) *
																																			(uVar11 & 0xff) & 0xff00ff00) >> 8) *
																																		uVar26;
																																	local_40 = CONCAT13((char)(((uVar12 >> 0x18) *
																																		(uint) * (byte*)&pCurParticle->
																																		field_0xc >> 8) * alphaMultiplier >> 8),
																																		CONCAT12((char)((uVar22 >> 0x18) *
																																			(uint) * (byte*)&pCurParticle->field_0x12 >> 8),
																																			CONCAT11((char)(((uVar12 & 0xff00ff00) >> 8 & 0xff)
																																				* (uint)pCurParticle->field_0xf >> 8),
																																				(char)((uVar22 >> 8 & 0xff) *
																																					(uint) * (byte*)&pCurParticle->field_0xe
																																					>> 8))));
																																	rgbaColor[0] = local_40;
																																	height = gParticleSizeScale *
																																		pCurParticle->yScale *
																																		(float)(uint)pCurParticle->field_0x4 * 0.5;
																																	width = height * aspectRatio;
																																	edDlistPartVertex(width, height, &uv0, &uv1, &uv2, &uv3, &rgbaColor[0], (undefined8*)pCurRawVector);
																																}
																																pCurParticle = pCurParticle + 1;
																																pCurRawVector = pCurRawVector + 1;
																															})
																														}
																													}
																													else {
																														pCurParticle = pParticle + startIndex;
																														pCurRawVector = pRawVectors + startIndex;

																														for (; startIndex < endIndex; startIndex = startIndex + 1) {
																															if (pCurParticle->field_0xd != 0) {
																																// Project particle velocity onto camera axes and scale by lifetime factor
																																float projectedX = (pCurVector->field_0x10.x * local_230.x +
																																	pCurVector->field_0x10.y * local_230.y +
																																	pCurVector->field_0x10.z * local_230.z) * pCurParticle->field_0x30;

																																float projectedY = (pCurVector->field_0x10.x * local_220.x +
																																	pCurVector->field_0x10.y * local_220.y +
																																	pCurVector->field_0x10.z * local_220.z) * pCurParticle->field_0x30;

																																// Calculate magnitude and normalize direction
																																fVar28 = sqrtf(projectedX * projectedX + projectedY * projectedY);
																																fVar27 = g_TinyFloat_00448548;
																																if (fVar28 != 0.0f) {
																																	fVar27 = fVar28;
																																}

																																float normalizedX = projectedX * (1.0f / fVar27);
																																float normalizedY = projectedY * (1.0f / fVar27);
																																float negNormalizedX = -normalizedX;

																																// Calculate initial quad transformation
																																AffineQuad2D quad;
																																quad.xAxis.x = normalizedX;
																																quad.xAxis.y = normalizedY;
																																quad.yAxis.x = negNormalizedX;
																																quad.yAxis.y = normalizedY;
																																quad.origin.x = (normalizedY + normalizedX) * -0.5f + 0.5f;
																																quad.origin.y = (normalizedY - normalizedX) * -0.5f + 0.5f;

																																// Apply rotation if base or lifetime-based rotation is enabled
																																if ((pDrawData->field_0x90 != 0.0f) || (pDrawData->field_0x94 != 0.0f)) {
																																	fVar27 = pDrawData->field_0x94 * (pCurParticle->age / pCurParticle->lifetime);
																																	if (0.0f < normalizedX) {
																																		fVar27 = -fVar27;
																																	}
																																	fVar27 = pDrawData->field_0x90 + fVar27;

																																	// Create rotation transformation
																																	AffineQuad2D rotQuad;
																																	ParticleC(rotQuad, fVar27);

																																	// Apply rotation to base quad
																																	float tempXAxisX = fVar28 * rotQuad.xAxis.x + -rotQuad.xAxis.y * negNormalizedX;
																																	float tempXAxisY = rotQuad.xAxis.y * normalizedX + fVar28 * rotQuad.xAxis.y;
																																	float tempYAxisX = rotQuad.yAxis.y * normalizedX + fVar28 * rotQuad.yAxis.x;
																																	float tempYAxisY = fVar28 * rotQuad.yAxis.y + -rotQuad.xAxis.y * normalizedY;
																																	float tempOriginX = rotQuad.origin.x * normalizedX + rotQuad.origin.y * normalizedY + quad.origin.x + 0.0f;
																																	float tempOriginY = rotQuad.origin.x * normalizedY + rotQuad.origin.y * negNormalizedX + quad.origin.y + 0.0f;

																																	quad.xAxis.x = tempXAxisX;
																																	quad.xAxis.y = tempXAxisY;
																																	quad.yAxis.x = tempYAxisX;
																																	quad.yAxis.y = tempYAxisY;
																																	quad.origin.x = tempOriginX;
																																	quad.origin.y = tempOriginY;
																																}

																																// Transform UV coordinates
																																TransformUVs(pfVar23, quad, uv0, uv1, uv2, uv3);

																																// Calculate particle color with alpha blending
																																ParticleColorA(rgbaColor, pCurParticle, alphaMultiplier);

																																// Calculate particle dimensions
																																height = gParticleSizeScale * pCurParticle->yScale * (float)(uint)pCurParticle->field_0x4 * 0.5f;
																																width = height * aspectRatio;

																																// Submit vertex data for rendering
																																edDlistPartVertex(width, height, &uv0, &uv1, &uv2, &uv3, &rgbaColor[0], pCurRawVector);
																															}

																															pCurParticle = pCurParticle + 1;
																															pCurRawVector = pCurRawVector + 1;
																															pCurVector = pCurVector + 1;
																														}
																													}
																												}
																												else {
																													pCurParticle = pParticle + startIndex;
																													pCurRawVector = pRawVectors + startIndex;
																													for (; startIndex < endIndex; startIndex = startIndex + 1) {
																														if (pCurParticle->field_0xd != 0) {
																															uVar26 = pCurParticle->seed;
																															fVar27 = pDrawData->field_0x94 * (pCurParticle->age / pCurParticle->lifetime);
																															if ((uVar26 & 0x20) != 0) {
																																fVar27 = -fVar27;
																															}

																															fVar27 = pDrawData->field_0x90 + fVar27;

																															ParticleC(local_670, fVar27);

																															if ((uVar26 & 0x40) != 0) {
																																ApplyAffineC(Mat2x3_0041eaf0, local_670);
																															}

																															if ((uVar26 & 0x80) != 0) {
																																fVar27 = local_670.xAxis.y * Mat2x3_0041ea50.m02;
																																fVar28 = local_670.yAxis.y * Mat2x3_0041ea50.m02;
																																local_670.xAxis.y = local_670.xAxis.y * Mat2x3_0041ea50.m10 + local_670.xAxis.x * Mat2x3_0041ea50.m01;
																																local_670.yAxis.y = local_670.yAxis.y * Mat2x3_0041ea50.m10 + local_670.yAxis.x * Mat2x3_0041ea50.m01;
																																fVar29 = local_670.origin.y * Mat2x3_0041ea50.m02;
																																local_670.origin.y = local_670.origin.y * Mat2x3_0041ea50.m10 + local_670.origin.x * Mat2x3_0041ea50.m01 + Mat2x3_0041ea50.m12 + 0.0f;
																																local_670.xAxis.x = fVar27 + local_670.xAxis.x * Mat2x3_0041ea50.m00;
																																local_670.yAxis.x = fVar28 + local_670.yAxis.x * Mat2x3_0041ea50.m00;
																																local_670.origin.x = fVar29 + local_670.origin.x * Mat2x3_0041ea50.m00 + Mat2x3_0041ea50.m11 + 0.0f;
																															}

																															sVar3 = pDrawData->field_0x82;
																															iVar15 = (((int)(((int)uVar26 >> 8 & 0xffffU) * (int)sVar3) >> 0x10) +
																																(int)(pCurParticle->age * (float)pDrawData->field_0x98)) % (int)sVar3;

																															if (sVar3 == 0) {
																																trap(7);
																															}

																															if (iVar15 < 0) {
																																iVar15 = -iVar15;
																															}

																															sVar3 = pDrawData->field_0x84;
																															FLOAT_ARRAY_0041ebb0.m00 = 1.0f / (float)(int)sVar3;
																															FLOAT_ARRAY_0041ebb0.m10 = 1.0f / (float)(int)pDrawData->field_0x86;
																															if (sVar3 == 0) {
																																trap(7);
																															}

																															FLOAT_ARRAY_0041ebb0.m11 = FLOAT_ARRAY_0041ebb0.m00 * (float)(iVar15 % (int)sVar3);
																															if (pDrawData->field_0x84 == 0) {
																																trap(7);
																															}

																															FLOAT_ARRAY_0041ebb0.m12 = FLOAT_ARRAY_0041ebb0.m10 *
																																(float)(iVar15 / (int)pDrawData->field_0x84);

																															ApplyAffineC(FLOAT_ARRAY_0041ebb0, local_670);

																															TransformUVs(pfVar23, local_670, uv0, uv1, uv2, uv3);

																															uVar26 = pCurParticle->field_0x8;

																															ParticleColorA(rgbaColor, pCurParticle, alphaMultiplier);
																															
																															height = gParticleSizeScale * pCurParticle->yScale *
																																(float)(uint)pCurParticle->field_0x4 * 0.5f;
																															width = height * aspectRatio;
																															edDlistPartVertex(width, height, &uv0, &uv1, &uv2, &uv3, &rgbaColor[0], pCurRawVector);
																														}

																														pCurParticle = pCurParticle + 1;
																														pCurRawVector = pCurRawVector + 1;
																													}
																												}
																											}
																											else {
																												pCurParticle = pParticle + startIndex;
																												pCurRawVector = pRawVectors + startIndex;

																												for (; startIndex < endIndex; startIndex = startIndex + 1) {
																													if (pCurParticle->field_0xd != 0) {
																														uVar12 = pCurParticle->seed;

																														// Calculate rotation based on lifetime
																														fVar27 = pDrawData->field_0x94 * (pCurParticle->age / pCurParticle->lifetime);
																														if ((uVar12 & 0x20) != 0) {
																															fVar27 = -fVar27;
																														}
																														fVar27 = pDrawData->field_0x90 + fVar27;

																														// Initialize quad with rotation
																														ParticleC(local_770, fVar27);

																														// Transform UV coordinates
																														TransformUVs(pfVar23, local_770, uv0, uv1, uv2, uv3);

																														// Calculate particle color with palette blending
																														ParticleColorB(rgbaColor, pCurParticle, alphaMultiplier, pDrawData, uVar26);

																														// Calculate particle dimensions
																														height = gParticleSizeScale * pCurParticle->yScale * (float)(uint)pCurParticle->field_0x4 * 0.5f;
																														width = height * aspectRatio;

																														// Submit vertex data for rendering
																														edDlistPartVertex(width, height, &uv0, &uv1, &uv2, &uv3, &rgbaColor[0], pCurRawVector);
																													}

																													pCurParticle = pCurParticle + 1;
																													pCurRawVector = pCurRawVector + 1;
																												}
																											}
																										}
																										else {
																											pCurParticle = pParticle + startIndex;
																											pCurRawVector = pRawVectors + startIndex;

																											for (; startIndex < endIndex; startIndex = startIndex + 1) {
																												if (pCurParticle->field_0xd != 0) {
																													// Calculate rotation based on lifetime
																													fVar28 = pDrawData->field_0x90 + pDrawData->field_0x94 * (pCurParticle->age / pCurParticle->lifetime);

																													// Initialize quad with rotation
																													ParticleC(local_65c, fVar28);

																													// Apply horizontal flip transformation if flag is set
																													if ((pCurParticle->seed & 0x40) != 0) {
																														ApplyAffineC(Mat2x3_0041ead0, local_65c);
																													}

																													// Transform UV coordinates
																													TransformUVs(pfVar23, local_65c, uv0, uv1, uv2, uv3);

																													// Calculate particle color with alpha blending
																													ParticleColorA(rgbaColor, pCurParticle, alphaMultiplier);

																													// Calculate particle dimensions
																													height = gParticleSizeScale * pCurParticle->yScale * (float)(uint)pCurParticle->field_0x4 * 0.5f;
																													width = height * aspectRatio;

																													// Submit vertex data for rendering
																													edDlistPartVertex(width, height, &uv0, &uv1, &uv2, &uv3, &rgbaColor[0], pCurRawVector);
																												}

																												pCurParticle = pCurParticle + 1;
																												pCurRawVector = pCurRawVector + 1;
																											}
																										}
																									}
																									else {
																										IMPLEMENTATION_GUARD(
																										pCurParticle = pParticle + startIndex;
																										pCurRawVector = pRawVectors + startIndex;
																										for (; startIndex < endIndex; startIndex = startIndex + 1) {
																											if (pCurParticle->field_0xd != 0) {
																												local_610 = 1.0;
																												local_604 = 1.0;
																												local_60c = 0.0;
																												local_608 = 0.0;
																												local_600 = 0.0;
																												local_5fc = 0.0;
																												if ((pCurParticle->seed & 0x40) != 0) {
																													local_610 = DAT_0041ea78 * 0.0 + DAT_0041ea70 * 1.0;
																													local_608 = DAT_0041ea78 * 1.0 + DAT_0041ea70 * 0.0;
																													local_60c = DAT_0041ea7c * 0.0 + DAT_0041ea74 * 1.0;
																													local_604 = DAT_0041ea7c * 1.0 + DAT_0041ea74 * 0.0;
																													local_600 = DAT_0041ea78 * 0.0 +
																														DAT_0041ea70 * 0.0 + DAT_0041ea80 + 0.0;
																													local_5fc = DAT_0041ea7c * 0.0 +
																														DAT_0041ea74 * 0.0 + DAT_0041ea84 + 0.0;
																												}
																												if ((pCurParticle->seed & 0x80) != 0) {
																													fVar27 = local_60c * DAT_0041ea98;
																													fVar28 = local_604 * DAT_0041ea98;
																													local_60c = local_60c * DAT_0041ea9c +
																														local_610 * DAT_0041ea94;
																													local_604 = local_604 * DAT_0041ea9c +
																														local_608 * DAT_0041ea94;
																													fVar29 = local_5fc * DAT_0041ea98;
																													local_5fc = local_5fc * DAT_0041ea9c +
																														local_600 * DAT_0041ea94 + DAT_0041eaa4 + 0.0;
																													local_610 = fVar27 + local_610 * DAT_0041ea90;
																													local_608 = fVar28 + local_608 * DAT_0041ea90;
																													local_600 = fVar29 + local_600 * DAT_0041ea90 +
																														DAT_0041eaa0 + 0.0;
																												}
																												iVar15 = (int)((pCurParticle->age / pCurParticle->lifetime) *
																													(float)(int)pDrawData->field_0x82);
																												lVar14 = (long)iVar15;
																												if (lVar14 < 0) {
																													lVar14 = (long)-iVar15;
																												}
																												if (pDrawData->field_0x82 <= lVar14) {
																													lVar14 = (long)(pDrawData->field_0x82 + -1);
																												}
																												sVar3 = pDrawData->field_0x84;
																												DAT_0041eab0 = 1.0 / (float)(int)sVar3;
																												DAT_0041eabc = 1.0 / (float)(int)pDrawData->field_0x86;
																												if (sVar3 == 0) {
																													trap(7);
																												}
																												DAT_0041eac0 = DAT_0041eab0 * (float)((int)lVar14 % (int)sVar3);
																												if (pDrawData->field_0x84 == 0) {
																													trap(7);
																												}
																												DAT_0041eac4 = DAT_0041eabc *
																													(float)((int)lVar14 / (int)pDrawData->field_0x84)
																													;
																												fVar27 = local_60c * DAT_0041eab8 + local_610 * DAT_0041eab0;
																												fVar28 = local_604 * DAT_0041eab8 + local_608 * DAT_0041eab0;
																												fVar29 = local_60c * DAT_0041eabc + local_610 * DAT_0041eab4;
																												fVar31 = local_604 * DAT_0041eabc + local_608 * DAT_0041eab4;
																												fVar30 = local_5fc * DAT_0041eab8 +
																													local_600 * DAT_0041eab0 + DAT_0041eac0 + 0.0;
																												fVar32 = local_5fc * DAT_0041eabc +
																													local_600 * DAT_0041eab4 + DAT_0041eac4 + 0.0;
																												uv0.u = pfVar23[0][1] * fVar28 +
																													pfVar23[0][0] * fVar27 + fVar30 + 0.0;
																												uv0.v = pfVar23[0][1] * fVar31 +
																													pfVar23[0][0] * fVar29 + fVar32 + 0.0;
																												uv1.u = pfVar23[1][1] * fVar28 +
																													pfVar23[1][0] * fVar27 + fVar30 + 0.0;
																												uv1.v = pfVar23[1][1] * fVar31 +
																													pfVar23[1][0] * fVar29 + fVar32 + 0.0;
																												uv2.u = pfVar23[2][1] * fVar28 +
																													pfVar23[2][0] * fVar27 + fVar30 + 0.0;
																												uv2.v = pfVar23[2][1] * fVar31 +
																													pfVar23[2][0] * fVar29 + fVar32 + 0.0;
																												uv3.u = pfVar23[3][1] * fVar28 +
																													pfVar23[3][0] * fVar27 + fVar30 + 0.0;
																												uv3.v = pfVar23[3][1] * fVar31 +
																													pfVar23[3][0] * fVar29 + fVar32 + 0.0;
																												uVar26 = pCurParticle->field_0x8;
																												rgbaColor[0] = (((uVar26 >> 0x18) *
																													(uint) * (byte*)&pCurParticle->field_0xc >> 8) *
																													alphaMultiplier >> 8) << 0x18;
																												rgbaColor[0] = rgbaColor[0] |
																													CONCAT12((char)((uVar26 >> 0x10 & 0xff) *
																														(uint) * (byte*)&pCurParticle->field_0x12
																														>> 8),
																														CONCAT11((char)((uVar26 >> 8 & 0xff) *
																															(uint)pCurParticle->field_0xf >>
																															8), (char)((uVar26 & 0xff) *
																																(uint) * (byte*)&
																																pCurParticle->field_0xe >> 8)));
																												height = gParticleSizeScale *
																													pCurParticle->field_0x24 *
																													(float)(uint)pCurParticle->field_0x4 * 0.5;
																												width = height * aspectRatio;
																												edDlistPartVertex(width, height, &uv0, &uv1, &uv2, &uv3, &rgbaColor[0],
																													(undefined8*)pCurRawVector);
																											}
																											pCurParticle = pCurParticle + 1;
																											pCurRawVector = pCurRawVector + 1;
																										})
																									}
																								}
																								else {
																									pCurParticle = pParticle + startIndex;
																									pCurRawVector = pRawVectors + startIndex;

																									for (; startIndex < endIndex; startIndex = startIndex + 1) {
																										if (pCurParticle->field_0xd != 0) {
																											uVar26 = pCurParticle->seed;
																											fVar27 = pDrawData->field_0x94 * (pCurParticle->age / pCurParticle->lifetime);
																											if ((uVar26 & 0x20) != 0) {
																												fVar27 = -fVar27;
																											}

																											fVar27 = ((float)(uVar26 & 0xffff) * pDrawData->field_0x90) / 65535.0f + fVar27;

																											local_5f0.yAxis.y = cosf(fVar27);
																											local_5f0.xAxis.y = sinf(fVar27);

																											local_5f0.xAxis.x = local_5f0.yAxis.y;
																											local_5f0.yAxis.x = -local_5f0.xAxis.y;

																											local_5f0.origin.x = -(local_5f0.yAxis.y * 0.5f) + local_5f0.xAxis.y * 0.5f + 0.5f;
																											local_5f0.origin.y = (-(local_5f0.xAxis.y * 0.5f) - local_5f0.yAxis.y * 0.5f) + 0.5f;

																											if ((uVar26 & 0x80) != 0) {
																												ApplyAffineC(Mat2x3_0041ea50, local_5f0);
																											}

																											TransformUVs(pfVar23, local_5f0, uv0, uv1, uv2, uv3);

																											ParticleColorA(rgbaColor, pCurParticle, alphaMultiplier);

																											height = gParticleSizeScale * pCurParticle->yScale * (float)(uint)pCurParticle->field_0x4 * 0.5f;
																											width = height * aspectRatio;
																											edDlistPartVertex(width, height, &uv0, &uv1, &uv2, &uv3, &rgbaColor[0], pCurRawVector);
																										}

																										pCurParticle = pCurParticle + 1;
																										pCurRawVector = pCurRawVector + 1;
																									}
																								}
																							}
																							else {
																								IMPLEMENTATION_GUARD(
																								pCurParticle = pParticle + startIndex;
																								pCurRawVector = pRawVectors + startIndex;
																								for (; startIndex < endIndex; startIndex = startIndex + 1) {
																									if (pCurParticle->field_0xd != 0) {
																										uVar26 = pCurParticle->seed;
																										local_5d0 = 1.0;
																										local_5c4 = 1.0;
																										local_5cc = 0.0;
																										local_5c8 = 0.0;
																										local_5c0 = 0.0;
																										local_5bc = 0.0;
																										if ((uVar26 & 0x40) != 0) {
																											local_5d0 = DAT_0041e9f8 * 0.0 + DAT_0041e9f0 * 1.0;
																											local_5c8 = DAT_0041e9f8 * 1.0 + DAT_0041e9f0 * 0.0;
																											local_5cc = DAT_0041e9fc * 0.0 + DAT_0041e9f4 * 1.0;
																											local_5c4 = DAT_0041e9fc * 1.0 + DAT_0041e9f4 * 0.0;
																											local_5c0 = DAT_0041e9f8 * 0.0 +
																												DAT_0041e9f0 * 0.0 + DAT_0041ea00 + 0.0;
																											local_5bc = DAT_0041e9fc * 0.0 +
																												DAT_0041e9f4 * 0.0 + DAT_0041ea04 + 0.0;
																										}
																										if ((uVar26 & 0x80) != 0) {
																											fVar27 = local_5cc * DAT_0041ea18;
																											fVar28 = local_5c4 * DAT_0041ea18;
																											local_5cc = local_5cc * DAT_0041ea1c + local_5d0 * DAT_0041ea14;
																											local_5c4 = local_5c4 * DAT_0041ea1c + local_5c8 * DAT_0041ea14;
																											fVar29 = local_5bc * DAT_0041ea18;
																											local_5bc = local_5bc * DAT_0041ea1c +
																												local_5c0 * DAT_0041ea14 + DAT_0041ea24 + 0.0;
																											local_5d0 = fVar27 + local_5d0 * DAT_0041ea10;
																											local_5c8 = fVar28 + local_5c8 * DAT_0041ea10;
																											local_5c0 = fVar29 + local_5c0 * DAT_0041ea10 + DAT_0041ea20 + 0.0
																												;
																										}
																										sVar3 = pDrawData->field_0x82;
																										iVar15 = (((int)(((int)uVar26 >> 8 & 0xffffU) * (int)sVar3) >> 0x10)
																											+ (int)(pCurParticle->age *
																												(float)*(int*)&pDrawData->field_0x98)) % (int)sVar3
																											;
																										if (sVar3 == 0) {
																											trap(7);
																										}
																										if (iVar15 < 0) {
																											iVar15 = -iVar15;
																										}
																										sVar3 = pDrawData->field_0x84;
																										DAT_0041ea30 = 1.0 / (float)(int)sVar3;
																										DAT_0041ea3c = 1.0 / (float)(int)pDrawData->field_0x86;
																										if (sVar3 == 0) {
																											trap(7);
																										}
																										DAT_0041ea40 = DAT_0041ea30 * (float)(iVar15 % (int)sVar3);
																										if (pDrawData->field_0x84 == 0) {
																											trap(7);
																										}
																										DAT_0041ea44 = DAT_0041ea3c *
																											(float)(iVar15 / (int)pDrawData->field_0x84);
																										fVar27 = local_5cc * DAT_0041ea38 + local_5d0 * DAT_0041ea30;
																										fVar28 = local_5c4 * DAT_0041ea38 + local_5c8 * DAT_0041ea30;
																										fVar29 = local_5cc * DAT_0041ea3c + local_5d0 * DAT_0041ea34;
																										fVar31 = local_5c4 * DAT_0041ea3c + local_5c8 * DAT_0041ea34;
																										fVar30 = local_5bc * DAT_0041ea38 +
																											local_5c0 * DAT_0041ea30 + DAT_0041ea40 + 0.0;
																										fVar32 = local_5bc * DAT_0041ea3c +
																											local_5c0 * DAT_0041ea34 + DAT_0041ea44 + 0.0;
																										uv0.u = pfVar23[0][1] * fVar28 + pfVar23[0][0] * fVar27 + fVar30 + 0.0;
																										uv0.v = pfVar23[0][1] * fVar31 + pfVar23[0][0] * fVar29 + fVar32 + 0.0;
																										uv1.u = pfVar23[1][1] * fVar28 + pfVar23[1][0] * fVar27 + fVar30 + 0.0
																											;
																										uv1.v = pfVar23[1][1] * fVar31 + pfVar23[1][0] * fVar29 + fVar32 + 0.0
																											;
																										uv2.u = pfVar23[2][1] * fVar28 + pfVar23[2][0] * fVar27 + fVar30 + 0.0
																											;
																										uv2.v = pfVar23[2][1] * fVar31 + pfVar23[2][0] * fVar29 + fVar32 + 0.0
																											;
																										uv3.u = pfVar23[3][1] * fVar28 + pfVar23[3][0] * fVar27 + fVar30 + 0.0
																											;
																										uv3.v = pfVar23[3][1] * fVar31 + pfVar23[3][0] * fVar29 + fVar32 + 0.0
																											;
																										uVar26 = pCurParticle->field_0x8;
																										rgbaColor[0] = (((uVar26 >> 0x18) * (uint) * (byte*)&pCurParticle->field_0xc
																											>> 8) * alphaMultiplier >> 8) << 0x18;
																										rgbaColor[0] = rgbaColor[0] |
																											CONCAT12((char)((uVar26 >> 0x10 & 0xff) *
																												(uint) * (byte*)&pCurParticle->field_0x12 >> 8
																												), CONCAT11((char)((uVar26 >> 8 & 0xff) *
																													(uint)pCurParticle->
																													field_0xf >> 8),
																													(char)((uVar26 & 0xff) * (uint) * (byte*)&pCurParticle->field_0xe
																														>> 8)));
																										height = gParticleSizeScale *
																											pCurParticle->field_0x24 * (float)(uint)pCurParticle->field_0x4 *
																											0.5;
																										width = height * aspectRatio;
																										edDlistPartVertex(width, height, &uv0, &uv1, &uv2, &uv3, &rgbaColor[0], (undefined8*)pCurRawVector);
																									}
																									pCurParticle = pCurParticle + 1;
																									pCurRawVector = pCurRawVector + 1;
																								})
																							}
																						}
																						else {
																							pCurParticle = pParticle + startIndex;
																							pCurRawVector = pRawVectors + startIndex;

																							for (; startIndex < endIndex; startIndex = startIndex + 1) {
																								if (pCurParticle->field_0xd != 0) {
																									float a = (pCurVector->field_0x10.x * local_230.x + pCurVector->field_0x10.y * local_230.y +
																										pCurVector->field_0x10.z * local_230.z) * pCurParticle->field_0x30;

																									float b = (pCurVector->field_0x10.x * local_220.x + pCurVector->field_0x10.y * local_220.y +
																										pCurVector->field_0x10.z * local_220.z) * pCurParticle->field_0x30;

																									fVar28 = sqrtf(a * a + b * b);
																									fVar27 = g_TinyFloat_00448548;
																									if (fVar28 != 0.0f) {
																										fVar27 = fVar28;
																									}

																									local_570.yAxis.x = a * (1.0 / fVar27);
																									local_570.xAxis.x = b * (1.0 / fVar27);

																									local_570.xAxis.y = -local_570.yAxis.x;
																									local_570.origin.x = (local_570.xAxis.x + local_570.yAxis.x) * -0.5f + 0.5f;
																									local_570.origin.y = (local_570.xAxis.x - local_570.yAxis.x) * -0.5f + 0.5f;

																									if ((pDrawData->field_0x90 != 0.0f) || (local_570.yAxis.y = local_570.xAxis.x, pDrawData->field_0x94 != 0.0f)) {
																										fVar27 = pDrawData->field_0x94 * (pCurParticle->age / pCurParticle->lifetime);
																										if (0.0f < local_570.yAxis.x) {
																											fVar27 = -fVar27;
																										}
																										fVar27 = pDrawData->field_0x90 + fVar27;

																										fVar31 = sinf(fVar27);

																										fVar29 = -(fVar28 * 0.5f) + fVar31 * 0.5f + 0.5f;
																										fVar30 = (-(fVar31 * 0.5f) - fVar28 * 0.5f) + 0.5f;
																										fVar32 = -fVar31 * local_570.xAxis.x;
																										fVar27 = fVar31 * local_570.xAxis.x;
																										local_570.yAxis.y = fVar28 * local_570.xAxis.x + -fVar31 * local_570.xAxis.y;
																										local_570.origin.x = fVar30 * local_570.yAxis.x +
																											fVar29 * local_570.xAxis.x + local_570.origin.x + 0.0f;
																										local_570.origin.y = fVar30 * local_570.xAxis.x +
																											fVar29 * local_570.xAxis.y + local_570.origin.y + 0.0f;
																										local_570.xAxis.x = fVar31 * local_570.yAxis.x + fVar28 * local_570.xAxis.x;
																										local_570.xAxis.y = fVar27 + fVar28 * local_570.xAxis.y;
																										local_570.yAxis.x = fVar28 * local_570.yAxis.x + fVar32;
																									}

																									TransformUVs(pfVar23, local_570, uv0, uv1, uv2, uv3);

																									ParticleColorB(rgbaColor, pCurParticle, alphaMultiplier, pDrawData, uVar26);
																									height = gParticleSizeScale * pCurParticle->yScale * (float)(uint)pCurParticle->field_0x4 * 0.5f;
																									width = height * aspectRatio;
																									edDlistPartVertex(width, height, &uv0, &uv1, &uv2, &uv3, &rgbaColor[0], pCurRawVector);
																								}

																								pCurParticle = pCurParticle + 1;
																								pCurRawVector = pCurRawVector + 1;
																								pCurVector = pCurVector + 1;
																							}
																						}
																					}
																					else {
																						IMPLEMENTATION_GUARD(
																						pCurParticle = pParticle + startIndex;
																						pCurRawVector = pRawVectors + startIndex;
																						for (; startIndex < endIndex; startIndex = startIndex + 1) {
																							if (pCurParticle->field_0xd != 0) {
																								sVar3 = pDrawData->field_0x82;
																								iVar15 = (((int)((*(int*)&pCurParticle->seed >> 8 & 0xffffU) *
																									(int)sVar3) >> 0x10) +
																									(int)(pCurParticle->age *
																										(float)*(int*)&pDrawData->field_0x98)) % (int)sVar3;
																								if (sVar3 == 0) {
																									trap(7);
																								}
																								if (iVar15 < 0) {
																									iVar15 = -iVar15;
																								}
																								sVar3 = pDrawData->field_0x84;
																								DAT_0041e9d0 = 1.0 / (float)(int)sVar3;
																								DAT_0041e9dc = 1.0 / (float)(int)pDrawData->field_0x86;
																								if (sVar3 == 0) {
																									trap(7);
																								}
																								DAT_0041e9e0 = DAT_0041e9d0 * (float)(iVar15 % (int)sVar3);
																								if (pDrawData->field_0x84 == 0) {
																									trap(7);
																								}
																								DAT_0041e9e4 = DAT_0041e9dc *
																									(float)(iVar15 / (int)pDrawData->field_0x84);
																								fVar27 = DAT_0041e9d8 * 0.0 + DAT_0041e9d0 * 1.0;
																								fVar28 = DAT_0041e9d8 * 1.0 + DAT_0041e9d0 * 0.0;
																								fVar29 = DAT_0041e9dc * 0.0 + DAT_0041e9d4 * 1.0;
																								fVar31 = DAT_0041e9dc * 1.0 + DAT_0041e9d4 * 0.0;
																								fVar30 = DAT_0041e9d8 * 0.0 + DAT_0041e9d0 * 0.0 + DAT_0041e9e0 + 0.0;
																								fVar32 = DAT_0041e9dc * 0.0 + DAT_0041e9d4 * 0.0 + DAT_0041e9e4 + 0.0;
																								uv0.u = pfVar23[0][1] * fVar28 + pfVar23[0][0] * fVar27 + fVar30 + 0.0;
																								uv0.v = pfVar23[0][1] * fVar31 + pfVar23[0][0] * fVar29 + fVar32 + 0.0;
																								uv1.u = pfVar23[1][1] * fVar28 + pfVar23[1][0] * fVar27 + fVar30 + 0.0;
																								uv1.v = pfVar23[1][1] * fVar31 + pfVar23[1][0] * fVar29 + fVar32 + 0.0;
																								uv2.u = pfVar23[2][1] * fVar28 + pfVar23[2][0] * fVar27 + fVar30 + 0.0;
																								uv2.v = pfVar23[2][1] * fVar31 + pfVar23[2][0] * fVar29 + fVar32 + 0.0;
																								uv3.u = pfVar23[3][1] * fVar28 + pfVar23[3][0] * fVar27 + fVar30 + 0.0;
																								uv3.v = pfVar23[3][1] * fVar31 + pfVar23[3][0] * fVar29 + fVar32 + 0.0;
																								uVar26 = pCurParticle->field_0x8;
																								rgbaColor[0] = (((uVar26 >> 0x18) * (uint) * (byte*)&pCurParticle->field_0xc >> 8
																									) * alphaMultiplier >> 8) << 0x18;
																								rgbaColor[0] = rgbaColor[0] |
																									CONCAT12((char)((uVar26 >> 0x10 & 0xff) *
																										(uint) * (byte*)&pCurParticle->field_0x12 >> 8),
																										CONCAT11((char)((uVar26 >> 8 & 0xff) *
																											(uint)pCurParticle->field_0xf >> 8),
																											(char)((uVar26 & 0xff) *
																												(uint) * (byte*)&pCurParticle->field_0xe
																												>> 8)));
																								height = gParticleSizeScale *
																									pCurParticle->field_0x24 * (float)(uint)pCurParticle->field_0x4 * 0.5;
																								width = height * aspectRatio;
																								edDlistPartVertex(width, height, &uv0, &uv1, &uv2, &uv3, &rgbaColor[0], (undefined8*)pCurRawVector);
																							}

																							pCurParticle = pCurParticle + 1;
																							pCurRawVector = pCurRawVector + 1;
																						})
																					}
																				}
																				else {
																					// Simple case.
																					// lucioles_group.g2d
																					pCurParticle = pParticle + startIndex;
																					pCurRawVector = pRawVectors + startIndex;

																					for (; startIndex < endIndex; startIndex = startIndex + 1) {
																						if (pCurParticle->field_0xd != 0) {
																							//continue;

																							uv0.u = pfVar23[0].x * 0.0f + pfVar23[0].y * 1.0f + 0.0f;
																							uv0.v = pfVar23[0].x * 1.0f + pfVar23[0].y * 0.0f + 0.0f;
																							uv1.u = pfVar23[1].x * 0.0f + pfVar23[1].y * 1.0f + 0.0f;
																							uv1.v = pfVar23[1].x * 1.0f + pfVar23[1].y * 0.0f + 0.0f;
																							uv2.u = pfVar23[2].x * 0.0f + pfVar23[2].y * 1.0f + 0.0f;
																							uv2.v = pfVar23[2].x * 1.0f + pfVar23[2].y * 0.0f + 0.0f;
																							uv3.u = pfVar23[3].x * 0.0f + pfVar23[3].y * 1.0f + 0.0f;
																							uv3.v = pfVar23[3].x * 1.0f + pfVar23[3].y * 0.0f + 0.0f;

																							uVar12 = (int)pCurParticle->seed >> 3;
																							uVar11 = pCurParticle->seed & 7;
																							local_20 = pDrawData->field_0x9c[uVar11];
																							iVar15 = 0xff - (uVar12 & 0xff);
																							local_24 = pDrawData->field_0x9c[uVar11 + 1];
																							uVar11 = (pCurParticle->field_0x8 & 0xff00ff) * (0xff - uVar26) + ((local_20 & 0xff00ff) * iVar15 + (local_24 & 0xff00ff) * (uVar12 & 0xff) >> 8 & 0xff00ff) * uVar26;
																							uVar12 = (pCurParticle->field_0x8 >> 8 & 0xff00ff) * (0xff - uVar26) + (((local_20 >> 8 & 0xff00ff) * iVar15 + (local_24 >> 8 & 0xff00ff) * (uVar12 & 0xff) & 0xff00ff00) >> 8 ) * uVar26;

																							uint8_t a_src = (uVar12 >> 24) & 0xFF;
																							uint8_t r_src = (uVar11 >> 24) & 0xFF;
																							uint8_t g_src = (uVar12 >> 8) & 0xFF;
																							uint8_t b_src = (uVar11 >> 8) & 0xFF;

																							uint8_t a_final = (a_src * (uint8_t)pCurParticle->field_0xc * alphaMultiplier) >> 16;
																							uint8_t r_final = (r_src * (uint8_t)pCurParticle->field_0x12) >> 8;
																							uint8_t g_final = (g_src * (uint8_t)pCurParticle->field_0xf) >> 8;
																							uint8_t b_final = (b_src * (uint8_t)pCurParticle->field_0xe) >> 8;

																							rgbaColor[0].rgba = (a_final << 24) | (r_final << 16) | (g_final << 8) | b_final;


																							height = gParticleSizeScale * pCurParticle->yScale * (float)(uint)pCurParticle->field_0x4 * 0.5f;
																							width = height * aspectRatio;
																							edDlistPartVertex(width, height, &uv0, &uv1, &uv2, &uv3, &rgbaColor[0], pCurRawVector);
																						}

																						pCurParticle = pCurParticle + 1;
																						pCurRawVector = pCurRawVector + 1;
																					}
																				}
																			}
																			else {
																				pCurParticle = pParticle + startIndex;
																				pCurRawVector = pRawVectors + startIndex;

																				for (; startIndex < endIndex; startIndex = startIndex + 1) {
																					if (pCurParticle->field_0xd != 0) {
																						// Apply vertical flip transformation if flag is set
																						if ((pCurParticle->seed & 0x80) != 0) {
																							const Mat2x3& flipTransform = Mat2x3_0041e9b0; // Assuming this is a Mat2x3 array

																							fVar27 = local_50c * flipTransform.m02;
																							fVar28 = local_504 * flipTransform.m02;

																							local_50c = local_50c * flipTransform.m10 + local_510 * flipTransform.m01;
																							local_504 = local_504 * flipTransform.m10 + local_508 * flipTransform.m01;

																							fVar29 = local_4fc * flipTransform.m02;
																							local_4fc = local_4fc * flipTransform.m10 + local_500 * flipTransform.m01 + flipTransform.m12 + 0.0f;

																							local_510 = fVar27 + local_510 * flipTransform.m00;
																							local_508 = fVar28 + local_508 * flipTransform.m00;
																							local_500 = fVar29 + local_500 * flipTransform.m00 + flipTransform.m11 + 0.0f;
																						}

																						// Transform UV coordinates using the affine transformation
																						uv0.u = pfVar23[0].y * local_508 + pfVar23[0].x * local_510 + local_500 + 0.0f;
																						uv0.v = pfVar23[0].y * local_504 + pfVar23[0].x * local_50c + local_4fc + 0.0f;
																						uv1.u = pfVar23[1].y * local_508 + pfVar23[1].x * local_510 + local_500 + 0.0f;
																						uv1.v = pfVar23[1].y * local_504 + pfVar23[1].x * local_50c + local_4fc + 0.0f;
																						uv2.u = pfVar23[2].y * local_508 + pfVar23[2].x * local_510 + local_500 + 0.0f;
																						uv2.v = pfVar23[2].y * local_504 + pfVar23[2].x * local_50c + local_4fc + 0.0f;
																						uv3.u = pfVar23[3].y * local_508 + pfVar23[3].x * local_510 + local_500 + 0.0f;
																						uv3.v = pfVar23[3].y * local_504 + pfVar23[3].x * local_50c + local_4fc + 0.0f;

																						// Calculate particle color with alpha blending
																						uVar26 = pCurParticle->field_0x8;
																						uint8_t alpha = ((uVar26 >> 24) * pCurParticle->field_0xc * alphaMultiplier) >> 16;
																						uint8_t red = ((uVar26 >> 16 & 0xFF) * pCurParticle->field_0x12) >> 8;
																						uint8_t green = ((uVar26 >> 8 & 0xFF) * pCurParticle->field_0xf) >> 8;
																						uint8_t blue = ((uVar26 & 0xFF) * pCurParticle->field_0xe) >> 8;

																						rgbaColor[0].rgba = (alpha << 24) | (red << 16) | (green << 8) | blue;

																						// Calculate particle dimensions
																						height = gParticleSizeScale * pCurParticle->yScale * (float)(uint)pCurParticle->field_0x4 * 0.5f;
																						width = height * aspectRatio;

																						// Submit vertex data for rendering
																						edDlistPartVertex(width, height, &uv0, &uv1, &uv2, &uv3, &rgbaColor[0], pCurRawVector);
																					}

																					pCurParticle = pCurParticle + 1;
																					pCurRawVector = pCurRawVector + 1;
																				}
																			}
																		}
																		else {
																			pCurParticle = pParticle + startIndex;
																			pCurRawVector = pRawVectors + startIndex;

																			for (; startIndex < endIndex; startIndex = startIndex + 1) {
																				if (pCurParticle->field_0xd != 0) {
																					// Calculate rotation angle based on particle seed and lifetime
																					fVar28 = ((float)(uint)pCurParticle->seed * pDrawData->field_0x90) / 65535.0f +
																						pDrawData->field_0x94 * (pCurParticle->age / pCurParticle->lifetime);

																					// Initialize base quad transformation with rotation
																					ParticleC(local_770, fVar28);

																					// Transform UV coordinates
																					TransformUVs(pfVar23, local_770, uv0, uv1, uv2, uv3);

																					// Calculate particle color with alpha blending
																					uVar26 = pCurParticle->field_0x8;
																					uint8_t alpha = ((uVar26 >> 24) * pCurParticle->field_0xc * alphaMultiplier) >> 16;
																					uint8_t red = ((uVar26 >> 16 & 0xFF) * pCurParticle->field_0x12) >> 8;
																					uint8_t green = ((uVar26 >> 8 & 0xFF) * pCurParticle->field_0xf) >> 8;
																					uint8_t blue = ((uVar26 & 0xFF) * pCurParticle->field_0xe) >> 8;

																					rgbaColor[0].rgba = (alpha << 24) | (red << 16) | (green << 8) | blue;

																					// Calculate particle dimensions
																					height = gParticleSizeScale * pCurParticle->yScale * (float)(uint)pCurParticle->field_0x4 * 0.5f;
																					width = height * aspectRatio;

																					// Submit vertex data for rendering
																					edDlistPartVertex(width, height, &uv0, &uv1, &uv2, &uv3, &rgbaColor[0], pCurRawVector);
																				}

																				pCurParticle = pCurParticle + 1;
																				pCurRawVector = pCurRawVector + 1;
																			}
																		}
																	}
																	else {
																		pCurParticle = pParticle + startIndex;
																		pCurRawVector = pRawVectors + startIndex;

																		for (; startIndex < endIndex; startIndex = startIndex + 1) {
																			if (pCurParticle->field_0xd != 0) {
																				uVar26 = pCurParticle->seed;

																				// Calculate base rotation from seed
																				fVar28 = ((float)(uVar26 & 0xffff) * pDrawData->field_0x90) / 65535.0f;

																				// Project particle velocity onto camera axes and scale by lifetime factor
																				float projectedX = (pCurVector->field_0x10.x * local_230.x +
																					pCurVector->field_0x10.y * local_230.y +
																					pCurVector->field_0x10.z * local_230.z) * pCurParticle->field_0x30;

																				float projectedY = (pCurVector->field_0x10.x * local_220.x +
																					pCurVector->field_0x10.y * local_220.y +
																					pCurVector->field_0x10.z * local_220.z) * pCurParticle->field_0x30;

																				// Calculate magnitude and normalize direction
																				fVar29 = sqrtf(projectedX * projectedX + projectedY * projectedY);
																				fVar27 = g_TinyFloat_00448548;
																				if (fVar29 != 0.0f) {
																					fVar27 = fVar29;
																				}

																				float normalizedX = projectedX * (1.0f / fVar27);
																				float normalizedY = projectedY * (1.0f / fVar27);
																				float negNormalizedX = -normalizedX;

																				// Calculate initial quad transformation offsets
																				float originOffsetX = (normalizedY + normalizedX) * -0.5f + 0.5f;
																				float originOffsetY = (normalizedY - normalizedX) * -0.5f + 0.5f;

																				// Apply rotation if base or lifetime-based rotation is enabled
																				AffineQuad2D quad;
																				if ((fVar28 != 0.0f) || (quad.yAxis.y = normalizedY, pDrawData->field_0x94 != 0.0f)) {
																					fVar27 = pDrawData->field_0x94 * (pCurParticle->age / pCurParticle->lifetime);
																					if (0.0f < normalizedX) {
																						fVar27 = -fVar27;
																					}
																					fVar28 = fVar28 + fVar27;

																					// Initialize base quad transformation with rotation
																					ParticleC(quad, fVar28);

																					// Apply the normalized direction vectors
																					quad.yAxis.y = fVar27 * normalizedY + -fVar31 * negNormalizedX;
																					quad.origin.x = fVar30 * normalizedX + fVar29 * normalizedY + originOffsetX + 0.0f;
																					quad.origin.y = fVar30 * normalizedY + fVar29 * negNormalizedX + originOffsetY + 0.0f;
																					quad.yAxis.x = fVar31 * normalizedX + fVar27 * normalizedY;
																					quad.xAxis.x = fVar28 + fVar27 * negNormalizedX;
																					quad.xAxis.y = fVar27 * normalizedX + fVar32;
																				}
																				else {
																					quad.xAxis.x = normalizedX;
																					quad.xAxis.y = normalizedY;
																					quad.yAxis.x = negNormalizedX;
																					quad.yAxis.y = normalizedY;
																					quad.origin.x = originOffsetX;
																					quad.origin.y = originOffsetY;
																				}

																				// Apply horizontal flip transformation if flag is set
																				if ((uVar26 & 0x40) != 0) {
																					const Mat2x3& flipTransform = Mat2x3_0041e970; // Assuming DAT values are Mat2x3

																					fVar27 = quad.xAxis.x * flipTransform.m02;
																					fVar28 = quad.yAxis.y * flipTransform.m02;
																					quad.xAxis.x = quad.xAxis.x * flipTransform.m10 + quad.yAxis.x * flipTransform.m01;
																					quad.yAxis.y = quad.yAxis.y * flipTransform.m10 + quad.xAxis.y * flipTransform.m01;
																					fVar29 = quad.origin.y * flipTransform.m02;
																					quad.origin.y = quad.origin.y * flipTransform.m10 + quad.origin.x * flipTransform.m01 + flipTransform.m12 + 0.0f;
																					quad.yAxis.x = fVar27 + quad.yAxis.x * flipTransform.m00;
																					quad.xAxis.y = fVar28 + quad.xAxis.y * flipTransform.m00;
																					quad.origin.x = fVar29 + quad.origin.x * flipTransform.m00 + flipTransform.m11 + 0.0f;
																				}

																				// Apply vertical flip transformation if flag is set
																				if ((uVar26 & 0x80) != 0) {
																					const Mat2x3& flipTransform2 = Mat2x3_0041e990; // Assuming DAT values are Mat2x3

																					fVar27 = quad.xAxis.x * flipTransform2.m02;
																					fVar28 = quad.yAxis.y * flipTransform2.m02;
																					quad.xAxis.x = quad.xAxis.x * flipTransform2.m10 + quad.yAxis.x * flipTransform2.m01;
																					quad.yAxis.y = quad.yAxis.y * flipTransform2.m10 + quad.xAxis.y * flipTransform2.m01;
																					fVar29 = quad.origin.y * flipTransform2.m02;
																					quad.origin.y = quad.origin.y * flipTransform2.m10 + quad.origin.x * flipTransform2.m01 + flipTransform2.m12 + 0.0f;
																					quad.yAxis.x = fVar27 + quad.yAxis.x * flipTransform2.m00;
																					quad.xAxis.y = fVar28 + quad.xAxis.y * flipTransform2.m00;
																					quad.origin.x = fVar29 + quad.origin.x * flipTransform2.m00 + flipTransform2.m11 + 0.0f;
																				}

																				// Transform UV coordinates using the final transformation matrix
																				TransformUVs(pfVar23, quad, uv0, uv1, uv2, uv3);

																				// Calculate particle color with alpha blending
																				uVar26 = pCurParticle->field_0x8;
																				uint8_t alpha = ((uVar26 >> 24) * pCurParticle->field_0xc * alphaMultiplier) >> 16;
																				uint8_t red = ((uVar26 >> 16 & 0xFF) * pCurParticle->field_0x12) >> 8;
																				uint8_t green = ((uVar26 >> 8 & 0xFF) * pCurParticle->field_0xf) >> 8;
																				uint8_t blue = ((uVar26 & 0xFF) * pCurParticle->field_0xe) >> 8;

																				rgbaColor[0].rgba = (alpha << 24) | (red << 16) | (green << 8) | blue;

																				// Calculate particle dimensions
																				height = gParticleSizeScale * pCurParticle->yScale * (float)(uint)pCurParticle->field_0x4 * 0.5f;
																				width = height * aspectRatio;

																				// Submit vertex data for rendering
																				edDlistPartVertex(width, height, &uv0, &uv1, &uv2, &uv3, &rgbaColor[0], pCurRawVector);
																			}

																			pCurParticle = pCurParticle + 1;
																			pCurRawVector = pCurRawVector + 1;
																			pCurVector = pCurVector + 1;
																		}
																	}
																}
																else {
																	IMPLEMENTATION_GUARD(
																	pCurParticle = pParticle + startIndex;
																	pCurRawVector = pRawVectors + startIndex;
																	for (; startIndex < endIndex; startIndex = startIndex + 1) {
																		if (pCurParticle->field_0xd != 0) {
																			fVar27 = pDrawData->field_0x94 * (pCurParticle->age / pCurParticle->lifetime);
																			if ((pCurParticle->seed & 0x20) != 0) {
																				fVar27 = -fVar27;
																			}
																			fVar27 = pDrawData->field_0x90 + fVar27;
																			ParticleC(local_770, fVar27);
																			TransformUVs(pfVar23, local_770, uv0, uv1, uv2, uv3);

																			uVar26 = pCurParticle->field_0x8;
																			rgbaColor[0] = (((uVar26 >> 0x18) * (uint) * (byte*)&pCurParticle->field_0xc >> 8) *
																				alphaMultiplier >> 8) << 0x18;
																			rgbaColor[0] = rgbaColor[0] |
																				CONCAT12((char)((uVar26 >> 0x10 & 0xff) *
																					(uint) * (byte*)&pCurParticle->field_0x12 >> 8),
																					CONCAT11((char)((uVar26 >> 8 & 0xff) *
																						(uint)pCurParticle->field_0xf >> 8),
																						(char)((uVar26 & 0xff) *
																							(uint) * (byte*)&pCurParticle->field_0xe >> 8)));
																			height = gParticleSizeScale * pCurParticle->field_0x24 * (float)(uint)pCurParticle->field_0x4 *
																				0.5;
																			width = height * aspectRatio;
																			edDlistPartVertex(width, height, &uv0, &uv1, &uv2, &uv3,
																				&rgbaColor[0], (undefined8*)pCurRawVector);
																		}
																		pCurParticle = pCurParticle + 1;
																		pCurRawVector = pCurRawVector + 1;
																	})
																}
															}
															else {
																pCurParticle = pParticle + startIndex;
																pCurRawVector = pRawVectors + startIndex;
																for (; startIndex < endIndex; startIndex = startIndex + 1) {
																	if (pCurParticle->field_0xd != 0) {
																		ParticleA(local_410, pCurVector, pCurParticle, local_220, local_230, pDrawData->field_0x90, pDrawData);

																		TransformUVs(pfVar23, local_410, uv0, uv1, uv2, uv3);

																		ParticleColorA(rgbaColor, pCurParticle, alphaMultiplier);
																		
																		height = gParticleSizeScale * pCurParticle->yScale * (float)(uint)pCurParticle->field_0x4 * 0.5f;
																		width = height * aspectRatio;
																		edDlistPartVertex(width, height, &uv0, &uv1, &uv2, &uv3, rgbaColor, pCurRawVector);
																	}
																	pCurParticle = pCurParticle + 1;
																	pCurRawVector = pCurRawVector + 1;
																	pCurVector = pCurVector + 1;
																}
															}
														}
														else {
															IMPLEMENTATION_GUARD(
															pCurParticle = pParticle + startIndex;
															pCurRawVector = pRawVectors + startIndex;
															for (; startIndex < endIndex; startIndex = startIndex + 1) {
																if (pCurParticle->field_0xd != 0) {
																	local_3f0 = 1.0;
																	local_3e4 = 1.0;
																	local_3ec = 0.0;
																	local_3e8 = 0.0;
																	local_3e0 = 0.0;
																	local_3dc = 0.0;
																	if ((pCurParticle->seed & 0x40) != 0) {
																		local_3f0 = DAT_0041e918 * 0.0 + DAT_0041e910 * 1.0;
																		local_3e8 = DAT_0041e918 * 1.0 + DAT_0041e910 * 0.0;
																		local_3ec = DAT_0041e91c * 0.0 + DAT_0041e914 * 1.0;
																		local_3e4 = DAT_0041e91c * 1.0 + DAT_0041e914 * 0.0;
																		local_3e0 = DAT_0041e918 * 0.0 + DAT_0041e910 * 0.0 + DAT_0041e920 + 0.0;
																		local_3dc = DAT_0041e91c * 0.0 + DAT_0041e914 * 0.0 + DAT_0041e924 + 0.0;
																	}
																	if ((pCurParticle->seed & 0x80) != 0) {
																		fVar27 = local_3ec * DAT_0041e938;
																		fVar28 = local_3e4 * DAT_0041e938;
																		local_3ec = local_3ec * DAT_0041e93c + local_3f0 * DAT_0041e934;
																		local_3e4 = local_3e4 * DAT_0041e93c + local_3e8 * DAT_0041e934;
																		fVar29 = local_3dc * DAT_0041e938;
																		local_3dc = local_3dc * DAT_0041e93c + local_3e0 * DAT_0041e934 + DAT_0041e944 + 0.0
																			;
																		local_3f0 = fVar27 + local_3f0 * DAT_0041e930;
																		local_3e8 = fVar28 + local_3e8 * DAT_0041e930;
																		local_3e0 = fVar29 + local_3e0 * DAT_0041e930 + DAT_0041e940 + 0.0;
																	}
																	iVar15 = (int)(pCurParticle->age * (float)*(int*)&pDrawData->field_0x98) %
																		(int)pDrawData->field_0x82;
																	if (pDrawData->field_0x82 == 0) {
																		trap(7);
																	}
																	if (iVar15 < 0) {
																		iVar15 = -iVar15;
																	}
																	sVar3 = pDrawData->field_0x84;
																	DAT_0041e950 = 1.0 / (float)(int)sVar3;
																	DAT_0041e95c = 1.0 / (float)(int)pDrawData->field_0x86;
																	if (sVar3 == 0) {
																		trap(7);
																	}
																	DAT_0041e960 = DAT_0041e950 * (float)(iVar15 % (int)sVar3);
																	if (pDrawData->field_0x84 == 0) {
																		trap(7);
																	}
																	DAT_0041e964 = DAT_0041e95c * (float)(iVar15 / (int)pDrawData->field_0x84);
																	fVar27 = local_3ec * DAT_0041e958 + local_3f0 * DAT_0041e950;
																	fVar28 = local_3e4 * DAT_0041e958 + local_3e8 * DAT_0041e950;
																	fVar29 = local_3ec * DAT_0041e95c + local_3f0 * DAT_0041e954;
																	fVar31 = local_3e4 * DAT_0041e95c + local_3e8 * DAT_0041e954;
																	fVar30 = local_3dc * DAT_0041e958 + local_3e0 * DAT_0041e950 + DAT_0041e960 + 0.0;
																	fVar32 = local_3dc * DAT_0041e95c + local_3e0 * DAT_0041e954 + DAT_0041e964 + 0.0;
																	uv0.u = pfVar23[0][1] * fVar28 + pfVar23[0][0] * fVar27 + fVar30 + 0.0;
																	uv0.v = pfVar23[0][1] * fVar31 + pfVar23[0][0] * fVar29 + fVar32 + 0.0;
																	uv1.u = pfVar23[1][1] * fVar28 + pfVar23[1][0] * fVar27 + fVar30 + 0.0;
																	uv1.v = pfVar23[1][1] * fVar31 + pfVar23[1][0] * fVar29 + fVar32 + 0.0;
																	uv2.u = pfVar23[2][1] * fVar28 + pfVar23[2][0] * fVar27 + fVar30 + 0.0;
																	uv2.v = pfVar23[2][1] * fVar31 + pfVar23[2][0] * fVar29 + fVar32 + 0.0;
																	uv3.u = pfVar23[3][1] * fVar28 + pfVar23[3][0] * fVar27 + fVar30 + 0.0;
																	uv3.v = pfVar23[3][1] * fVar31 + pfVar23[3][0] * fVar29 + fVar32 + 0.0;
																	uVar26 = pCurParticle->field_0x8;
																	rgbaColor[0] = (((uVar26 >> 0x18) * (uint) * (byte*)&pCurParticle->field_0xc >> 8) * alphaMultiplier
																		>> 8) << 0x18;
																	rgbaColor[0] = rgbaColor[0] |
																		CONCAT12((char)((uVar26 >> 0x10 & 0xff) *
																			(uint) * (byte*)&pCurParticle->field_0x12 >> 8),
																			CONCAT11((char)((uVar26 >> 8 & 0xff) * (uint)pCurParticle->field_0xf
																				>> 8),
																				(char)((uVar26 & 0xff) *
																					(uint) * (byte*)&pCurParticle->field_0xe >> 8)));
																	height = gParticleSizeScale * pCurParticle->field_0x24 * (float)(uint)pCurParticle->field_0x4 * 0.5
																		;
																	width = height * aspectRatio;
																	edDlistPartVertex(width, height, &uv0, &uv1, &uv2, &uv3,
																		&rgbaColor[0], (undefined8*)pCurRawVector);
																}
																pCurParticle = pCurParticle + 1;
																pCurRawVector = pCurRawVector + 1;
															})
														}
													}
													else {
														IMPLEMENTATION_GUARD(
														pCurParticle = pParticle + startIndex;
														pCurRawVector = pRawVectors + startIndex;
														for (; startIndex < endIndex; startIndex = startIndex + 1) {
															if (pCurParticle->field_0xd != 0) {
																fVar28 = pDrawData->field_0x90 + pDrawData->field_0x94 * (pCurParticle->age / pCurParticle->lifetime);
																
																ParticleC(local_770, fVar28);

																sVar3 = pDrawData->field_0x82;
																iVar15 = (((int)((*(int*)&pCurParticle->seed >> 8 & 0xffffU) * (int)sVar3) >> 0x10) +
																	(int)(pCurParticle->age * (float)*(int*)&pDrawData->field_0x98)) %
																	(int)sVar3;
																if (sVar3 == 0) {
																	trap(7);
																}
																if (iVar15 < 0) {
																	iVar15 = -iVar15;
																}
																sVar3 = pDrawData->field_0x84;
																DAT_0041e8f0 = 1.0 / (float)(int)sVar3;
																DAT_0041e8fc = 1.0 / (float)(int)pDrawData->field_0x86;
																if (sVar3 == 0) {
																	trap(7);
																}
																DAT_0041e900 = DAT_0041e8f0 * (float)(iVar15 % (int)sVar3);
																if (pDrawData->field_0x84 == 0) {
																	trap(7);
																}
																DAT_0041e904 = DAT_0041e8fc * (float)(iVar15 / (int)pDrawData->field_0x84);
																fVar31 = fVar28 * DAT_0041e8f8 + fVar27 * DAT_0041e8f0;
																fVar32 = fVar27 * DAT_0041e8f8 + -fVar28 * DAT_0041e8f0;
																fVar33 = fVar28 * DAT_0041e8fc + fVar27 * DAT_0041e8f4;
																fVar28 = fVar27 * DAT_0041e8fc + -fVar28 * DAT_0041e8f4;
																fVar27 = fVar30 * DAT_0041e8f8 + fVar29 * DAT_0041e8f0 + DAT_0041e900 + 0.0;
																fVar29 = fVar30 * DAT_0041e8fc + fVar29 * DAT_0041e8f4 + DAT_0041e904 + 0.0;
																uv0.u = pfVar23[0][1] * fVar32 + pfVar23[0][0] * fVar31 + fVar27 + 0.0;
																uv0.v = pfVar23[0][1] * fVar28 + pfVar23[0][0] * fVar33 + fVar29 + 0.0;
																uv1.u = pfVar23[1][1] * fVar32 + pfVar23[1][0] * fVar31 + fVar27 + 0.0;
																uv1.v = pfVar23[1][1] * fVar28 + pfVar23[1][0] * fVar33 + fVar29 + 0.0;
																uv2.u = pfVar23[2][1] * fVar32 + pfVar23[2][0] * fVar31 + fVar27 + 0.0;
																uv2.v = pfVar23[2][1] * fVar28 + pfVar23[2][0] * fVar33 + fVar29 + 0.0;
																uv3.u = pfVar23[3][1] * fVar32 + pfVar23[3][0] * fVar31 + fVar27 + 0.0;
																uv3.v = pfVar23[3][1] * fVar28 + pfVar23[3][0] * fVar33 + fVar29 + 0.0;
																uVar26 = pCurParticle->field_0x8;
																rgbaColor[0] = (((uVar26 >> 0x18) * (uint) * (byte*)&pCurParticle->field_0xc >> 8) * alphaMultiplier >>
																	8) << 0x18;
																rgbaColor[0] = rgbaColor[0] |
																	CONCAT12((char)((uVar26 >> 0x10 & 0xff) *
																		(uint) * (byte*)&pCurParticle->field_0x12 >> 8),
																		CONCAT11((char)((uVar26 >> 8 & 0xff) * (uint)pCurParticle->field_0xf >>
																			8), (char)((uVar26 & 0xff) *
																				(uint) * (byte*)&pCurParticle->field_0xe >> 8)))
																	;
																height = gParticleSizeScale * pCurParticle->field_0x24 * (float)(uint)pCurParticle->field_0x4 * 0.5;
																width = height * aspectRatio;
																edDlistPartVertex(width, height, &uv0, &uv1, &uv2, &uv3, &rgbaColor[0],
																	(undefined8*)pCurRawVector);
															}
															pCurParticle = pCurParticle + 1;
															pCurRawVector = pCurRawVector + 1;
														})
													}
												}
												else {
													IMPLEMENTATION_GUARD(
													pCurParticle = pParticle + startIndex;
													pCurRawVector = pRawVectors + startIndex;
													for (; startIndex < endIndex; startIndex = startIndex + 1) {
														if (pCurParticle->field_0xd != 0) {
															FLOAT_ARRAY_0041e8d0[0] = pDrawData->field_0x88;
															FLOAT_ARRAY_0041e8d0[3] = pDrawData->field_0x8c;
															FLOAT_ARRAY_0041e8d0[4] = FLOAT_ARRAY_0041e8d0[0] * -0.5 + 0.5;
															FLOAT_ARRAY_0041e8d0[5] = FLOAT_ARRAY_0041e8d0[3] * -0.5 + 0.5;
															fVar27 = FLOAT_ARRAY_0041e8d0[2] * 0.0 + FLOAT_ARRAY_0041e8d0[0] * 1.0;
															fVar28 = FLOAT_ARRAY_0041e8d0[2] * 1.0 + FLOAT_ARRAY_0041e8d0[0] * 0.0;
															fVar29 = FLOAT_ARRAY_0041e8d0[3] * 0.0 + FLOAT_ARRAY_0041e8d0[1] * 1.0;
															fVar31 = FLOAT_ARRAY_0041e8d0[3] * 1.0 + FLOAT_ARRAY_0041e8d0[1] * 0.0;
															fVar30 = FLOAT_ARRAY_0041e8d0[2] * 0.0 +
																FLOAT_ARRAY_0041e8d0[0] * 0.0 + FLOAT_ARRAY_0041e8d0[4] + 0.0;
															fVar32 = FLOAT_ARRAY_0041e8d0[3] * 0.0 +
																FLOAT_ARRAY_0041e8d0[1] * 0.0 + FLOAT_ARRAY_0041e8d0[5] + 0.0;
															uv0.u = pfVar23[0][1] * fVar28 + pfVar23[0][0] * fVar27 + fVar30 + 0.0;
															uv0.v = pfVar23[0][1] * fVar31 + pfVar23[0][0] * fVar29 + fVar32 + 0.0;
															uv1.u = pfVar23[1][1] * fVar28 + pfVar23[1][0] * fVar27 + fVar30 + 0.0;
															uv1.v = pfVar23[1][1] * fVar31 + pfVar23[1][0] * fVar29 + fVar32 + 0.0;
															uv2.u = pfVar23[2][1] * fVar28 + pfVar23[2][0] * fVar27 + fVar30 + 0.0;
															uv2.v = pfVar23[2][1] * fVar31 + pfVar23[2][0] * fVar29 + fVar32 + 0.0;
															uv3.u = pfVar23[3][1] * fVar28 + pfVar23[3][0] * fVar27 + fVar30 + 0.0;
															uv3.v = pfVar23[3][1] * fVar31 + pfVar23[3][0] * fVar29 + fVar32 + 0.0;
															uVar26 = pCurParticle->field_0x8;
															rgbaColor[0] = (((uVar26 >> 0x18) * (uint) * (byte*)&pCurParticle->field_0xc >> 8) * alphaMultiplier >> 8
																) << 0x18;
															rgbaColor[0] = rgbaColor[0] |
																CONCAT12((char)((uVar26 >> 0x10 & 0xff) * (uint) * (byte*)&pCurParticle->field_0x12
																	>> 8),
																	CONCAT11((char)((uVar26 >> 8 & 0xff) * (uint)pCurParticle->field_0xf >> 8)
																		, (char)((uVar26 & 0xff) * (uint) * (byte*)&pCurParticle->field_0xe
																			>> 8)));
															height = gParticleSizeScale * pCurParticle->field_0x24 * (float)(uint)pCurParticle->field_0x4 * 0.5;
															width = height * aspectRatio;
															edDlistPartVertex(width, height, &uv0, &uv1, &uv2, &uv3, &rgbaColor[0],
																(undefined8*)pCurRawVector);
														}
														pCurParticle = pCurParticle + 1;
														pCurRawVector = pCurRawVector + 1;
													})
												}
											}
											else {
												pCurParticle = pParticle + startIndex;
												pCurRawVector = pRawVectors + startIndex;
												for (; startIndex < endIndex; startIndex = startIndex + 1) {
													if (pCurParticle->field_0xd != 0) {
														fVar28 = pDrawData->field_0x90 + pDrawData->field_0x94 * (pCurParticle->age / pCurParticle->lifetime);
														ParticleC(local_710, fVar28);
														TransformUVs(pfVar23, local_710, uv0, uv1, uv2, uv3);

														ParticleColorA(rgbaColor, pCurParticle, alphaMultiplier);
														height = gParticleSizeScale * pCurParticle->yScale * (float)(uint)pCurParticle->field_0x4 * 0.5f;
														width = height * aspectRatio;
														edDlistPartVertex(width, height, &uv0, &uv1, &uv2, &uv3, rgbaColor, pCurRawVector);
													}
													pCurParticle = pCurParticle + 1;
													pCurRawVector = pCurRawVector + 1;
												}
											}
										}
										else {
											// Butterfly.g2d
											pCurParticle = pParticle + startIndex;
											pCurRawVector = pRawVectors + startIndex;

											for (; startIndex < endIndex; startIndex = startIndex + 1) {
												if (pCurParticle->field_0xd != 0) {
													AffineQuad2D fVar277;

													FLOAT_ARRAY_0041e890.m00 = pDrawData->field_0x88;
													FLOAT_ARRAY_0041e890.m10 = pDrawData->field_0x8c;
													FLOAT_ARRAY_0041e890.m11 = FLOAT_ARRAY_0041e890.m00 * -0.5f + 0.5f;
													FLOAT_ARRAY_0041e890.m12 = FLOAT_ARRAY_0041e890.m10 * -0.5f + 0.5f;

													fVar277.xAxis.x = FLOAT_ARRAY_0041e890.m02 * 0.0f + FLOAT_ARRAY_0041e890.m00 * 1.0f;
													fVar277.xAxis.y = FLOAT_ARRAY_0041e890.m02 * 1.0f + FLOAT_ARRAY_0041e890.m00 * 0.0f;
													fVar277.yAxis.x = FLOAT_ARRAY_0041e890.m10 * 0.0f + FLOAT_ARRAY_0041e890.m01 * 1.0f;
													fVar277.yAxis.y = FLOAT_ARRAY_0041e890.m10 * 1.0f + FLOAT_ARRAY_0041e890.m01 * 0.0f;
													fVar277.origin.x = FLOAT_ARRAY_0041e890.m02 * 0.0f + FLOAT_ARRAY_0041e890.m00 * 0.0f + FLOAT_ARRAY_0041e890.m11 + 0.0f;
													fVar277.origin.y = FLOAT_ARRAY_0041e890.m10 * 0.0f + FLOAT_ARRAY_0041e890.m01 * 0.0f + FLOAT_ARRAY_0041e890.m12 + 0.0f;

													// Hack
													//pCurParticle->age = 3.087498f;
													//pCurParticle->seed = 0x692BB87;

													iVar15 = (int)(pCurParticle->age * (float)pDrawData->field_0x98) % pDrawData->field_0x82;
													if (pDrawData->field_0x82 == 0) {
														trap(7);
													}

													if (iVar15 < 0) {
														iVar15 = -iVar15;
													}

													sVar3 = pDrawData->field_0x84;
													FLOAT_ARRAY_0041e8b0.m00 = 1.0f / (float)(int)sVar3;
													FLOAT_ARRAY_0041e8b0.m10 = 1.0f / (float)(int)pDrawData->field_0x86;
													if (sVar3 == 0) {
														trap(7);
													}

													FLOAT_ARRAY_0041e8b0.m11 = FLOAT_ARRAY_0041e8b0.m00 * (float)(iVar15 % (int)sVar3);

													if (pDrawData->field_0x84 == 0) {
														trap(7);
													}

													FLOAT_ARRAY_0041e8b0.m12 = FLOAT_ARRAY_0041e8b0.m10 * (float)(iVar15 / (int)pDrawData->field_0x84);

													AffineQuad2D b;
													ApplyAffineB(FLOAT_ARRAY_0041e8b0, b);

													TransformUVs(pfVar23, b, uv0, uv1, uv2, uv3);

													ParticleColorB(rgbaColor, pCurParticle, alphaMultiplier, pDrawData, uVar26);

													height = gParticleSizeScale * pCurParticle->yScale * (float)(uint)pCurParticle->field_0x4 * 0.5f;
													width = height * aspectRatio;

													edDlistPartVertex(width, height, &uv0, &uv1, &uv2, &uv3, &rgbaColor[0], pCurRawVector);
												}

												pCurParticle = pCurParticle + 1;
												pCurRawVector = pCurRawVector + 1;
											}
										}
									}
									else {
										pCurParticle = pParticle + startIndex;
										pCurRawVector = pRawVectors + startIndex;

										for (; startIndex < endIndex; startIndex = startIndex + 1) {
											if (pCurParticle->field_0xd != 0) {
												uVar12 = pCurParticle->seed;

												// Calculate rotation angle based on seed and lifetime
												fVar27 = pDrawData->field_0x94 * (pCurParticle->age / pCurParticle->lifetime);
												if ((uVar12 & 0x20) != 0) {
													fVar27 = -fVar27;
												}
												fVar27 = ((float)(uVar12 & 0xffff) * pDrawData->field_0x90) / 65535.0f + fVar27;

												// Initialize base quad transformation with rotation
												ParticleC(local_350, fVar27);

												// Apply horizontal flip transformation if flag is set
												if ((uVar12 & 0x40) != 0) {
													ApplyAffineB(FLOAT_ARRAY_0041e810, local_350);
												}

												// Apply vertical flip transformation if flag is set
												if ((uVar12 & 0x80) != 0) {
													ApplyAffineC(FLOAT_ARRAY_0041e830, local_350);
												}

												// Transform UV coordinates using the final transformation matrix
												TransformUVs(pfVar23, local_350, uv0, uv1, uv2, uv3);

												// Calculate particle color with palette blending
												uVar11 = (int)uVar12 >> 3;
												uint paletteIdx = uVar12 & 7;
												local_8 = pDrawData->field_0x9c[paletteIdx];
												iVar15 = 0xff - (uVar11 & 0xff);
												local_c = pDrawData->field_0x9c[paletteIdx + 1];

												// Interpolate between palette colors for R and B channels
												uVar22 = (pCurParticle->field_0x8 & 0xff00ff) * (0xff - uVar26) +
													((local_8 & 0xff00ff) * iVar15 + (local_c & 0xff00ff) * (uVar11 & 0xff) >> 8 & 0xff00ff) * uVar26;

												// Interpolate between palette colors for G and A channels
												uVar12 = (pCurParticle->field_0x8 >> 8 & 0xff00ff) * (0xff - uVar26) +
													(((local_8 >> 8 & 0xff00ff) * iVar15 + (local_c >> 8 & 0xff00ff) * (uVar11 & 0xff) & 0xff00ff00) >> 8) * uVar26;

												// Pack interpolated color components with particle modifiers
												uint8_t alpha = ((uVar12 >> 24) * pCurParticle->field_0xc * alphaMultiplier) >> 16;
												uint8_t red = ((uVar22 >> 24) * pCurParticle->field_0x12) >> 8;
												uint8_t green = (((uVar12 & 0xff00ff00) >> 8 & 0xff) * pCurParticle->field_0xf) >> 8;
												uint8_t blue = ((uVar22 >> 8 & 0xff) * pCurParticle->field_0xe) >> 8;

												rgbaColor[0].rgba = (alpha << 24) | (red << 16) | (green << 8) | blue;

												// Calculate particle dimensions
												height = gParticleSizeScale * pCurParticle->yScale * (float)(uint)pCurParticle->field_0x4 * 0.5f;
												width = height * aspectRatio;

												// Submit vertex data for rendering
												edDlistPartVertex(width, height, &uv0, &uv1, &uv2, &uv3, &rgbaColor[0], pCurRawVector);
											}

											pCurParticle = pCurParticle + 1;
											pCurRawVector = pCurRawVector + 1;
										}
									}
								}
								else {
									pCurParticle = pParticle + startIndex;
									pCurRawVector = pRawVectors + startIndex;

									for (; startIndex < endIndex; startIndex = startIndex + 1) {
										if (pCurParticle->field_0xd != 0) {
											fVar27 = pDrawData->field_0x94 * (pCurParticle->age / pCurParticle->lifetime);
											if ((pCurParticle->seed & 0x20) != 0) {
												fVar27 = -fVar27;
											}

											fVar27 = ((float)(pCurParticle->seed & 0xffff) * pDrawData->field_0x90) / 65535.0f + fVar27;
											local_2f0.yAxis.y = cosf(fVar27);
											local_2f0.xAxis.y = sinf(fVar27);

											local_2f0.xAxis.x = local_2f0.yAxis.y;
											local_2f0.yAxis.x = -local_2f0.xAxis.y;

											local_2f0.origin.x = -(local_2f0.yAxis.y * 0.5f) + local_2f0.xAxis.y * 0.5f + 0.5f;
											local_2f0.origin.y = (-(local_2f0.xAxis.y * 0.5f) - local_2f0.yAxis.y * 0.5f) + 0.5f;

											TransformUVs(pfVar23, local_2f0, uv0, uv1, uv2, uv3);

											ParticleColorA(rgbaColor, pCurParticle, alphaMultiplier);

											height = gParticleSizeScale * pCurParticle->yScale * (float)(uint)pCurParticle->field_0x4 * 0.5f;
											width = height * aspectRatio;
											edDlistPartVertex(width, height, &uv0, &uv1, &uv2, &uv3, &rgbaColor[0], pCurRawVector);
										}

										pCurParticle = pCurParticle + 1;
										pCurRawVector = pCurRawVector + 1;
									}
								}
							}
							else {
								pCurParticle = pParticle + startIndex;
								pCurRawVector = pRawVectors + startIndex;

								for (; startIndex < endIndex; startIndex = startIndex + 1) {
									if (pCurParticle->field_0xd != 0) {
										uVar26 = pCurParticle->seed;

										// Calculate rotation angle based on particle lifetime
										fVar27 = pDrawData->field_0x94 * (pCurParticle->age / pCurParticle->lifetime);
										if ((uVar26 & 0x20) != 0) {
											fVar27 = -fVar27;
										}
										fVar27 = pDrawData->field_0x90 + fVar27;

										// Initialize base quad transformation
										ParticleC(local_2f0, fVar27);

										// Apply horizontal flip transformation if flag is set
										if ((uVar26 & 0x40) != 0) {
											const Mat2x3& flipTransform = FLOAT_ARRAY_0041e810;

											local_310 = local_30c * flipTransform.m02 + local_304 * flipTransform.m00;
											local_308 = local_304 * flipTransform.m02 + fVar27 * flipTransform.m00;
											local_30c = local_30c * flipTransform.m10 + local_304 * flipTransform.m01;
											local_304 = local_304 * flipTransform.m10 + fVar27 * flipTransform.m01;
											local_300 = local_2fc * flipTransform.m02 + fVar28 * flipTransform.m00 + flipTransform.m11 + 0.0f;
											local_2fc = local_2fc * flipTransform.m10 + fVar28 * flipTransform.m01 + flipTransform.m12 + 0.0f;
										}

										// Apply vertical flip transformation if flag is set
										if ((uVar26 & 0x80) != 0) {
											const Mat2x3& flipTransform = FLOAT_ARRAY_0041e830;

											fVar27 = local_30c * flipTransform.m02;
											fVar28 = local_304 * flipTransform.m02;
											local_30c = local_30c * flipTransform.m10 + local_310 * flipTransform.m01;
											local_304 = local_304 * flipTransform.m10 + local_308 * flipTransform.m01;
											fVar29 = local_2fc * flipTransform.m02;
											local_2fc = local_2fc * flipTransform.m10 + local_300 * flipTransform.m01 + flipTransform.m12 + 0.0f;
											local_310 = fVar27 + local_310 * flipTransform.m00;
											local_308 = fVar28 + local_308 * flipTransform.m00;
											local_300 = fVar29 + local_300 * flipTransform.m00 + flipTransform.m11 + 0.0f;
										}

										// Transform UV coordinates using the final transformation matrix
										uv0.u = pfVar23[0].y * local_308 + pfVar23[0].x * local_310 + local_300 + 0.0f;
										uv0.v = pfVar23[0].y * local_304 + pfVar23[0].x * local_30c + local_2fc + 0.0f;
										uv1.u = pfVar23[1].y * local_308 + pfVar23[1].x * local_310 + local_300 + 0.0f;
										uv1.v = pfVar23[1].y * local_304 + pfVar23[1].x * local_30c + local_2fc + 0.0f;
										uv2.u = pfVar23[2].y * local_308 + pfVar23[2].x * local_310 + local_300 + 0.0f;
										uv2.v = pfVar23[2].y * local_304 + pfVar23[2].x * local_30c + local_2fc + 0.0f;
										uv3.u = pfVar23[3].y * local_308 + pfVar23[3].x * local_310 + local_300 + 0.0f;
										uv3.v = pfVar23[3].y * local_304 + pfVar23[3].x * local_30c + local_2fc + 0.0f;

										// Calculate particle color with alpha blending
										uVar26 = pCurParticle->field_0x8_uint;
										uint8_t alpha = ((uVar26 >> 24) * pCurParticle->field_0xc * alphaMultiplier) >> 16;
										uint8_t red = ((uVar26 >> 16 & 0xFF) * pCurParticle->field_0x12) >> 8;
										uint8_t green = ((uVar26 >> 8 & 0xFF) * pCurParticle->field_0xf) >> 8;
										uint8_t blue = ((uVar26 & 0xFF) * pCurParticle->field_0xe) >> 8;

										rgbaColor[0].rgba = (alpha << 24) | (red << 16) | (green << 8) | blue;

										// Calculate particle dimensions
										height = gParticleSizeScale * pCurParticle->yScale * (float)(uint)pCurParticle->field_0x4 * 0.5f;
										width = height * aspectRatio;

										// Submit vertex data for rendering
										edDlistPartVertex(width, height, &uv0, &uv1, &uv2, &uv3, &rgbaColor[0], pCurRawVector);
									}

									pCurParticle = pCurParticle + 1;
									pCurRawVector = pCurRawVector + 1;
								}
							}
						}
						else {
							// Gaz_toxik.g2d
							pCurParticle = pParticle + startIndex;
							pCurRawVector = pRawVectors + startIndex;

							for (; startIndex < endIndex; startIndex = startIndex + 1) {
								if (pCurParticle->field_0xd != 0) {
									//continue;
									//pCurParticle->seed = 0xD3570283;
									uVar26 = pCurParticle->seed;
									fVar27 = pDrawData->field_0x94 * (pCurParticle->age / pCurParticle->lifetime);
									if ((uVar26 & 0x20) != 0) {
										fVar27 = -fVar27;
									}

									fVar27 = ((float)(uVar26 & 0xffff) * pDrawData->field_0x90) / 65535.0f + fVar27;

									local_2f0.yAxis.y = cosf(fVar27);
									local_2f0.xAxis.y = sinf(fVar27);

									fVar28 = -(local_2f0.yAxis.y * 0.5f) + local_2f0.xAxis.y * 0.5f + 0.5f;
									local_2f0.origin.y = (-(local_2f0.xAxis.y * 0.5f) - local_2f0.yAxis.y * 0.5f) + 0.5f;
									fVar27 = -local_2f0.xAxis.y;
									local_2f0.xAxis.x = local_2f0.yAxis.y;
									local_2f0.yAxis.x = fVar27;
									local_2f0.origin.x = fVar28;

									if ((uVar26 & 0x40) != 0) {
										ApplyAffineC(FLOAT_ARRAY_0041e7d0, local_2f0);
									}

									if ((uVar26 & 0x80) != 0) {
										fVar27 = local_2f0.xAxis.y * FLOAT_ARRAY_0041e7f0.m02;
										fVar28 = local_2f0.yAxis.y * FLOAT_ARRAY_0041e7f0.m02;

										local_2f0.xAxis.y = local_2f0.xAxis.y * FLOAT_ARRAY_0041e7f0.m10 + local_2f0.xAxis.x * FLOAT_ARRAY_0041e7f0.m01;
										local_2f0.yAxis.y = local_2f0.yAxis.y * FLOAT_ARRAY_0041e7f0.m10 + local_2f0.yAxis.x * FLOAT_ARRAY_0041e7f0.m01;
										fVar29 = local_2f0.origin.y * FLOAT_ARRAY_0041e7f0.m02;
										local_2f0.origin.y = local_2f0.origin.y * FLOAT_ARRAY_0041e7f0.m10 + local_2f0.origin.x * FLOAT_ARRAY_0041e7f0.m01 + FLOAT_ARRAY_0041e7f0.m12 + 0.0f;
										local_2f0.xAxis.x = fVar27 + local_2f0.xAxis.x * FLOAT_ARRAY_0041e7f0.m00;
										local_2f0.yAxis.x = fVar28 + local_2f0.yAxis.x * FLOAT_ARRAY_0041e7f0.m00;
										local_2f0.origin.x = fVar29 + local_2f0.origin.x * FLOAT_ARRAY_0041e7f0.m00 + FLOAT_ARRAY_0041e7f0.m11 + 0.0f;
									}

									TransformUVs(pfVar23, local_2f0, uv0, uv1, uv2, uv3);

									ParticleColorA(rgbaColor, pCurParticle, alphaMultiplier);

									height = gParticleSizeScale * pCurParticle->yScale * (float)(uint)pCurParticle->field_0x4 * 0.5f;
									width = height * aspectRatio;
									edDlistPartVertex(width, height, &uv0, &uv1, &uv2, &uv3, &rgbaColor[0], pCurRawVector);
								}

								pCurParticle = pCurParticle + 1;
								pCurRawVector = pCurRawVector + 1;
							}
						}

						edDListEnd();

						if (pDrawData->field_0x124[0] != (DisplayList*)0x0) {
							edDlistAddtoView(pDrawData->field_0x124[0]);
						}
					}
					else {
						if (bVar1 == 2) {
							IMPLEMENTATION_GUARD(
							iVar15 = 0;
							if (pDrawData->field_0x118 == 0) {
								if (pDrawData->field_0x124[0] != (DisplayList*)0x0) {
									edDListSetCurrent(pDrawData->field_0x124[0]);
								}
								uVar26 = pGroup->field_0x10;
								edDListPatchableInfo(&local_7c, &local_80, &uStack132, &local_88, uVar26, 0);
								pCurParticle = pParticle + startIndex;
								pCurRawVector = pRawVectors + startIndex;
								for (; startIndex < (int)uVar26; startIndex = startIndex + 1) {
									if (pCurParticle->field_0xd == 0) {
										local_94 = 0;
										local_98 = 0;
										iVar15 = startIndex;
										if (0x48 < (uint)(startIndex << 2)) {
											iVar15 = startIndex + ((startIndex << 2) / 0x48) * 2;
										}
										puVar20 = (undefined2*)(local_88 + iVar15 * 4);
										*puVar20 = 0;
										puVar20[1] = 0;
									}
									else {
										if (pDrawData->field_0x6f != 0) {
											local_90 = pCurParticle->field_0x24;
											iVar15 = startIndex;
											if (0x48 < (uint)(startIndex << 2)) {
												iVar15 = startIndex + ((startIndex << 2) / 0x48) * 2;
											}
											puVar20 = (undefined2*)(local_88 + iVar15 * 4);
											*puVar20 = (short)(int)(local_90 * 4096.0);
											puVar20[1] = (short)(int)(local_90 * 4096.0);
											local_8c = local_90;
										}
										if (pDrawData->field_0x6e != 0) {
											puVar17 = (uint*)(local_80 + startIndex * 0x10);
											*puVar17 = pCurParticle->field_0x8;
											puVar17[1] = pCurParticle->field_0x8;
											puVar17[2] = pCurParticle->field_0x8;
											puVar17[3] = pCurParticle->field_0x8;
										}
										if (pDrawData->field_0x6c != 0) {
											fVar28 = pCurRawVector->y;
											fVar27 = pCurRawVector->z;
											pfVar23 = (float*)(local_7c + startIndex * 0x40);
											pfVar23[0][0] = pCurRawVector->x;
											pfVar23[0][1] = fVar28;
											pfVar23[1][0] = fVar27;
										}
									}
									pCurParticle = pCurParticle + 1;
									pCurRawVector = pCurRawVector + 1;
								}
								edDListPatcheEnd(uVar26, 1);
								if (pDrawData->field_0x124[0] != (DisplayList*)0x0) {
									edDlistAddtoView(pDrawData->field_0x124[0]);
								}
							}
							else {
								iVar10 = pDrawData->field_0xc;
								for (; iVar15 < iVar10; iVar15 = iVar15 + 1) {
									if (pDrawData->field_0x124[iVar15] != (DisplayList*)0x0) {
										edDListSetCurrent(pDrawData->field_0x124[iVar15]);
									}
									edDListLoadMatrix(&pDrawData->field_0xd0);
									edDListUseMaterial(pDrawData->field_0x44 + iVar15);
									edDListBegin(0.0f, 0.0f, 0.0f, 0xb, pGroup->field_0x10);
									for (iVar10 = 0; iVar10 < pGroup->field_0x10; iVar10 = iVar10 + 1) {
										edDListColor4u8((pDrawData->field_0x50).r, (pDrawData->field_0x50).g, (pDrawData->field_0x50).b, (pDrawData->field_0x50).a);
										edDListTexCoo2f(pDrawData->field_0x54, pDrawData->field_0x58);
										edDListTexCoo2f(pDrawData->field_0x5c, pDrawData->instanceIndex);
										edDListWidthHeight2f(pDrawData->field_0x64, pDrawData->field_0x68);
										edDListVertex4f(0.0f, 0.0f, 0.0f, 0.0f);
									}
									edDListEnd();
									if (pDrawData->field_0x124[iVar15] != (DisplayList*)0x0) {
										edDlistAddtoView(pDrawData->field_0x124[iVar15]);
									}
									iVar10 = pDrawData->field_0xc;
								}
								pDrawData->field_0x118 = pDrawData->field_0x118 + -1;
							})
						}
					}
				}
			}
		}
	}

	return;
}

void edParticlesDraw(_ed_particle_manager* pManager, float time)
{
	_ed_particle_shaper_param* pDrawData;
	uint uVar2;
	uint uVar4;
	int iVar5;
	OffsetPointer<_ed_particle_shaper_param*>* ppPVar6;
	int iVar7;
	_ed_particle_group* pGroup;

	PARTICLE_LOG(LogLevel::Info, "\nedParticlesDraw Drawing manager with {} groups, and delta time: {}", p_current_particle_manager->nbGroups, time);

	if (g_particle_system_config.bProfile != 0) {
		edProfileBegin(gPartProfIdShapers);
	}

	pGroup = p_current_particle_manager->aGroups.pData;
	for (iVar7 = 0; iVar7 < p_current_particle_manager->nbGroups; iVar7 = iVar7 + 1) {
		if (pGroup->field_0x0 != 0) {
			int nbInGroup = pGroup->field_0x10;
			short groupStart = pGroup->pParticle.pData->field_0x6;

			PARTICLE_LOG(LogLevel::Info, "edParticlesDraw Drawing group 0x{:x} with 0x{:x} particles", iVar7, nbInGroup);

			pGroup->field_0x70 = (_ed_particle_generator_param*)0x0;
			ppPVar6 = pGroup->field_0x48.pData;
			for (iVar5 = 0; iVar5 < pGroup->field_0x40; iVar5 = iVar5 + 1) {
				pDrawData = ppPVar6->pData;
				if ((pDrawData != (_ed_particle_shaper_param*)0x0) && (pDrawData->field_0x2 != 0)) {
					pGroup->field_0x70 = pDrawData;
					edPartDrawShaper(time, pGroup, pDrawData, p_current_particle_manager->aParticles.pData,
						p_current_particle_manager->aParticleVectors.pData, p_current_particle_manager->aVectors.pData,
						(int)groupStart, groupStart + nbInGroup);
				}

				ppPVar6 = ppPVar6 + 1;
			}
		}

		pGroup = pGroup + 1;
	}

	if (g_particle_system_config.bProfile != 0) {
		edProfileEnd(gPartProfIdShapers);
	}

	return;
}

void edParticlesUnInstall(_ed_particle_manager* pManager, ed_3D_Scene* pScene)
{
	byte bVar1;
	int iVar2;
	_ed_particle* pParticle;
	int iVar3;
	int iVar4;
	_ed_particle_group* pGroup;
	int iVar5;
	_ed_particle_shaper_param* pShaperParam;

	pGroup = pManager->aGroups.pData;

	for (iVar4 = 0; iVar4 < pManager->nbTotalGroups; iVar4 = iVar4 + 1) {
		iVar5 = 0;

		while (iVar5 < pGroup->field_0x44) {
			pShaperParam = pGroup->field_0x48.pData[iVar5].pData;
			iVar3 = 0;

			if (pShaperParam->field_0x4 == 3) {
				pParticle = pGroup->pParticle.pData;
				iVar3 = pGroup->field_0x10;
				for (iVar2 = 0; iVar2 < iVar3; iVar2 = iVar2 + 1) {
					if (pParticle->pNode != (edNODE*)0x0) {
						ed3DHierarchyRemoveFromScene(pScene, pParticle->pNode);
						pParticle->pNode = (edNODE*)0x0;
					}

					pParticle = pParticle + 1;
				}

				iVar5 = iVar5 + 1;
			}
			else {
				bVar1 = pShaperParam->field_0xb;
				for (; iVar3 < (int)(uint)bVar1; iVar3 = iVar3 + 1) {
					if ((pShaperParam->field_0x44.pData[iVar3].pManager != (ed_g2d_manager*)0x0) && (pShaperParam->field_0x18c == 0)) {
						edDListTermMaterial(pShaperParam->field_0x44.pData + iVar3);
					}

					bVar1 = pShaperParam->field_0xb;
				}

				for (iVar3 = 0; iVar3 < pShaperParam->field_0xc; iVar3 = iVar3 + 1) {
					if (pShaperParam->field_0x124[iVar3].Get() != 0) {
						edDListDelete(pShaperParam->field_0x124[iVar3]);
					}
				}

				iVar5 = iVar5 + 1;
			}
		}

		pGroup = pGroup + 1;
	}

	pManager->bSetup = 0;

	return;
}

// Should be in: D:/Projects/EdenLib/edParticles/sources/edParticleGenerator.cpp
void edPartGeneratorComputeMatrices(_ed_particle_generator_param* pParam)
{
	edF32Matrix4SetIdentityHard(&pParam->field_0x40);
	edF32Matrix4ScaleHard(&pParam->field_0x40, &pParam->field_0x40, &pParam->scale);
	edF32Matrix4RotateXYZHard(&pParam->field_0x40, &pParam->field_0x40, &pParam->rotationEuler);
	edF32Matrix4TranslateHard(&pParam->field_0x40, &pParam->field_0x40, &pParam->position);

	return;
}

// Should be in: D:/Projects/EdenLib/edParticles/sources/edParticleEffector.cpp
void edPartEffectorComputeMatrices(_ed_particle_effector_param* pParam)
{
	edF32VECTOR4 local_30;
	edF32VECTOR4 local_20;
	edF32VECTOR4 local_10;

	local_10.x = 1.0f / (pParam->field_0x10).x;
	local_10.y = 1.0f / (pParam->field_0x10).y;
	local_10.z = 1.0f / (pParam->field_0x10).z;
	local_10.w = 0.0f;

	local_20.x = -(pParam->field_0x20).x;
	local_20.y = -(pParam->field_0x20).y;
	local_20.z = -(pParam->field_0x20).z;
	local_20.w = 0.0f;

	local_30.x = -(pParam->position).x;
	local_30.y = -(pParam->position).y;
	local_30.z = -(pParam->position).z;
	local_30.w = 0.0f;

	edF32Matrix4SetIdentityHard(&pParam->field_0x40);
	edF32Matrix4ScaleHard(&pParam->field_0x40, &pParam->field_0x40, &pParam->field_0x10);
	edF32Matrix4RotateXYZHard(&pParam->field_0x40, &pParam->field_0x40, &pParam->field_0x20);
	edF32Matrix4TranslateHard(&pParam->field_0x40, &pParam->field_0x40, &pParam->position);
	edF32Matrix4SetIdentityHard(&pParam->field_0x80);
	edF32Matrix4TranslateHard(&pParam->field_0x80, &pParam->field_0x80, &local_30);
	edF32Matrix4RotateZYXHard(&pParam->field_0x80, &pParam->field_0x80, &local_20);
	edF32Matrix4ScaleHard(&pParam->field_0x80, &pParam->field_0x80, &local_10);

	return;
}

_ed_particle_effector_param* edPartGetEffector(_ed_particle_manager* pManager, char* szName)
{
	ulong hash = ed3DComputeHashCode(szName);
	return _edPartGetEntityByHashcode<_ed_particle_effector_param>(pManager->aEffectorParams.pData.Get(), pManager->nbEffectorParams, &hash);
}

void edPartSetRespawning(_ed_particle_manager* pManager, int bRespawning)
{
	int iVar1;
	int iVar2;
	_ed_particle_group* p_Var3;

	p_Var3 = pManager->aGroups.pData;
	iVar1 = pManager->nbTotalGroups;

	if (bRespawning == 0) {
		for (iVar2 = 0; iVar2 < iVar1; iVar2 = iVar2 + 1) {
			p_Var3->field_0x8 = p_Var3->field_0x8 | 4;
			p_Var3 = p_Var3 + 1;
		}
	}
	else {
		for (iVar2 = 0; iVar2 < iVar1; iVar2 = iVar2 + 1) {
			p_Var3->field_0x8 = p_Var3->field_0x8 & 0xfb;
			p_Var3 = p_Var3 + 1;
		}
	}

	return;
}

void edPart_00277f20(_ed_particle_generator_param* pParam, uint param_2)
{
	pParam->field_0x140 = 0.0f;
	pParam->field_0x144 = 0.0f;
	pParam->field_0x228 = pParam->field_0x178;
	pParam->field_0x21c = param_2;
	return;
}


void edPart_0027cd80(int param_1)
{
	int iVar1;
	_ed_particle_group* pCurGroup;
	_ed_particle* pCurParticle;
	_ed_particle_generator_param* pCurGenerator;

	pCurGroup = p_current_particle_manager->aGroups.pData;
	for (iVar1 = 0; iVar1 < p_current_particle_manager->nbGroups; iVar1 = iVar1 + 1) {
		pCurGroup->field_0x14 = pCurGroup->field_0x10;
		pCurGroup->field_0x5c = 0;
		if (pCurGroup->pParticle.pData != (_ed_particle*)0x0) {
			pCurGroup->field_0x6 = pCurGroup->pParticle.pData->field_0x6;
		}
		pCurGroup = pCurGroup + 1;
	}

	pCurGenerator = p_current_particle_manager->aGeneratorParams.pData;
	for (iVar1 = 0; iVar1 < p_current_particle_manager->nbConfigGeneratorParams; iVar1 = iVar1 + 1) {
		edPart_00277f20(pCurGenerator, param_1);
		pCurGenerator = pCurGenerator + 1;
	}

	pCurParticle = p_current_particle_manager->aParticles.pData;
	for (iVar1 = 0; iVar1 < p_current_particle_manager->nbParticles; iVar1 = iVar1 + 1) {
		pCurParticle->field_0x0 = 1;
		pCurParticle = pCurParticle + 1;
	}

	return;
}

