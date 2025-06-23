#include "edParticles.h"
#include "Rendering/DisplayList.h"
#include "ed3D.h"
#include "MathOps.h"
#include "profile.h"
#include "../../../CameraViewManager.h"

_ed_particle_manager* edParticlesInstall(ParticleFileData* pFileData, ed_3D_Scene* pScene, ed_g2d_manager* param_3, edDList_material** ppMaterials, ulong* pHashes, int materialIndex, ed_g3d_manager* p3dManager, bool param_8)
{
	_ed_particle_vectors* p_Var1;
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
	if (p_Var1 != 0x0) {
		(pFileData->manager).aParticleVectors.pData = reinterpret_cast<_ed_particle_vectors*>(reinterpret_cast<char*>(pFileData) + offset);
	}

	offset = (pFileData->manager).field_0x2c.offset;
	if (offset != 0x0) {
		(pFileData->manager).field_0x2c.pData = reinterpret_cast<edF32VECTOR4*>(reinterpret_cast<char*>(pFileData) + offset);
	}

	offset = (pFileData->manager).aGroups.offset;
	if (offset != 0x0) {
		(pFileData->manager).aGroups.pData = reinterpret_cast<_ed_particle_group*>(reinterpret_cast<char*>(pFileData) + offset);
	}

	offset = (pFileData->manager).aParams.offset;
	if (offset != 0x0) {
		(pFileData->manager).aParams.pData = reinterpret_cast<_ed_particle_generator_param*>(reinterpret_cast<char*>(pFileData) + offset);
	}

	offset = (pFileData->manager).field_0x50.offset;
	if (offset != 0x0) {
		(pFileData->manager).field_0x50.pData = reinterpret_cast<_ed_particle_generator_param*>(reinterpret_cast<char*>(pFileData) + offset);
	}

	offset = (pFileData->manager).field_0x5c.offset;
	if (offset != 0x0) {
		(pFileData->manager).field_0x5c.pData = reinterpret_cast<Particle_0x20*>(reinterpret_cast<char*>(pFileData) + offset);
	}

	offset = (pFileData->manager).field_0x68.offset;
	if (offset != 0x0) {
		(pFileData->manager).field_0x68.pData = reinterpret_cast<Particle_0x1a0*>(reinterpret_cast<char*>(pFileData) + offset);
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
		if ((pFileData->manager).nbParams <= iVar17) {
			pCurParam = (pFileData->manager).aParams.pData;

			for (iVar17 = 0; iVar17 < (pFileData->manager).field_0x4c; iVar17 = iVar17 + 1) {
				//pCurParam->field_0x0 = 1;
				//pCurParam->field_0x2 = 1;
				//pCurParam->field_0x228 = pCurParam->field_0x178;
				//edPartGeneratorShapeSet(pCurParam, pCurParam->shape);
				//pCurParam->field_0x1a2 = 0xff;
				//pCurParam->field_0x1a3 = 0xff;
				//pCurParam->field_0x1a6 = 0xff;
				//pCurParam->field_0x1e6 = 0;
				//pCurParam->field_0x1e7 = 0;
				//pCurParam->field_0x1ea = 0;
				//pCurParam->field_0x198 = 0xff;
				//pCurParam->field_0x1dc = 0;
				pCurParam = pCurParam + 1;
			}

			_ed_particle_generator_param* pGenParam = (pFileData->manager).field_0x50.pData;
			for (iVar17 = 0; iVar17 < (pFileData->manager).field_0x58; iVar17 = iVar17 + 1) {
				pGenParam->field_0x0 = 1;
				pGenParam->field_0x2 = 1;
				pGenParam = pGenParam + 1;
			}

			Particle_0x20* pParticle_20 = (pFileData->manager).field_0x5c.pData;
			for (iVar17 = 0; iVar17 < (pFileData->manager).field_0x64; iVar17 = iVar17 + 1) {
				pParticle_20->field_0x0 = 1;
				pParticle_20->field_0x2 = 1;
				pParticle_20 = pParticle_20 + 1;
			}

			Particle_0x1a0* pParticle_1a0 = (pFileData->manager).field_0x68.pData;
			for (iVar17 = 0; iVar17 < (pFileData->manager).field_0x70; iVar17 = iVar17 + 1) {
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
			pCurGroup->field_0x30.pData = reinterpret_cast<OffsetPointer<_ed_particle_generator_param*>*>(reinterpret_cast<char*>(pFileData) + pCurGroup->field_0x30.offset);
		}

		if (pCurGroup->field_0x3c.offset != 0x0) {
			pCurGroup->field_0x3c.pData = reinterpret_cast<OffsetPointer<Particle_0x20*>*>(reinterpret_cast<char*>(pFileData) + pCurGroup->field_0x3c.offset);
		}

		if (pCurGroup->field_0x48.offset != 0x0) {
			pCurGroup->field_0x48.pData = reinterpret_cast<OffsetPointer<Particle_0x1a0*>*>(reinterpret_cast<char*>(pFileData) + pCurGroup->field_0x48.offset);
		}

		pCurGroup->pParticle.pData = (pFileData->manager).aParticles.pData + pCurGroup->pParticle.offset;
		pCurGroup->field_0x6 = pCurGroup->pParticle.pData->field_0x6;

		for (iVar22 = 0; iVar15 = 0, iVar22 < pCurGroup->field_0x20; iVar22 = iVar22 + 1) {
			pCurGroup->field_0x24.pData[iVar22].pData = (pFileData->manager).aParams.pData + pCurGroup->field_0x24.pData[iVar22].offset;

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
			pCurGroup->field_0x30.pData[iVar15].pData = (pFileData->manager).field_0x50.pData + pCurGroup->field_0x30.pData[iVar15].offset;

			pCurGroup->field_0x30.pData[iVar15].pData->field_0xc0 = pCurGroup->field_0x30.pData[iVar15].pData->field_0x40;
			
			//iVar24 = *(int*)(pCurGroup->field_0x30 + iVar22) + 0x1b0;
			for (iVar12 = 0; iVar12 < pCurGroup->field_0x30.pData[iVar15].pData->field_0x214; iVar12 = iVar12 + 1) {
				//*(int*)(iVar24 + 0xc) = &pFileData->field_0x0 + *(int*)(iVar24 + 0xc);
				//iVar24 = iVar24 + 0x10;
			}

			iVar22 = pCurGroup->field_0x2c;
		}

		for (; iVar22 = 0, iVar12 < pCurGroup->field_0x38; iVar12 = iVar12 + 1) {
			iVar15 = 0;
			pCurGroup->field_0x3c.pData[iVar12].pData = (pFileData->manager).field_0x5c.pData + pCurGroup->field_0x3c.pData[iVar12].offset;

			Particle_0x20* pParticle20 = pCurGroup->field_0x3c.pData[iVar12].pData;

			pParticle20->field_0x14.pData = reinterpret_cast<OffsetPointer<_ed_particle_generator_param*>*>(reinterpret_cast<char*>(pFileData) + pParticle20->field_0x14.offset);
			
			while (iVar15 < pParticle20->field_0x10) {
				if (pParticle20->field_0x8 == 2) {
					pParticle20->field_0x14.pData[iVar15].pData = (pFileData->manager).field_0x50.pData + pParticle20->field_0x14.pData[iVar15].offset;
				LAB_0027d4d4:
					iVar15 = iVar15 + 1;
				}
				else {
					if (pParticle20->field_0x8 == 1) {
						pParticle20->field_0x14.pData[iVar15].pData = (pFileData->manager).aParams.pData + pParticle20->field_0x14.pData[iVar15].offset;
						goto LAB_0027d4d4;
					}

					iVar15 = iVar15 + 1;
				}
			}
		}

		iVar15 = pCurGroup->field_0x44;
		for (; iVar22 < iVar15; iVar22 = iVar22 + 1) {
			pCurGroup->field_0x48.pData[iVar22].pData = (pFileData->manager).field_0x68.pData + pCurGroup->field_0x48.pData[iVar22].offset;

			Particle_0x1a0* pParticle_1a0 = pCurGroup->field_0x48.pData[iVar22].pData;
			pParticle_1a0->field_0x198 = pScene;
			pParticle_1a0->field_0x18d = 0;
			pParticle_1a0->field_0x18e = 0;
			pParticle_1a0->field_0x18f = 0;
			if (pParticle_1a0->field_0x4 == 3) {
				pParticle_1a0->field_0x194.pData = reinterpret_cast<void*>(reinterpret_cast<char*>(pFileData) + pParticle_1a0->field_0x194.offset);
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
					if (pParticle_1a0->field_0x40.offset != 0) {
						pcVar13 = reinterpret_cast<char*>(pFileData) + pParticle_1a0->field_0x40.offset;
						if (bVar6) {
							pParticle_1a0->field_0x40.pData = local_34;
						}
						else {
							local_34 = ed3DInstallG2D(pcVar13, *(int*)(pcVar13 + 8), &iStack4, (ed_g2d_manager*)0x0, 1);
							pParticle_1a0->field_0x40.pData = local_34;
							bVar6 = true;
						}
					}
				}
				else {
					pParticle_1a0->field_0x40.pData = param_3;
				}

				if (pParticle_1a0->field_0x48.offset == 0) {
					edDebugPrintf("»>>>>>>>>>>>>>>>>>>>>>>>>>>>>> edPARTICLE_INSTALL, no material haschode found ! «");
				}
				
				pParticle_1a0->field_0x48.pData = reinterpret_cast<ulong*>(reinterpret_cast<char*>(pFileData) + pParticle_1a0->field_0x48.offset);

				if (ppMaterials == (edDList_material**)0x0) {
					iVar12 = 0;
					pParticle_1a0->field_0x44.pData = reinterpret_cast<OffsetPointer<edDList_material*>*>(reinterpret_cast<char*>(pFileData) + pParticle_1a0->field_0x44.offset);
					pParticle_1a0->field_0x18c = 0;

					while (iVar12 < pParticle_1a0->field_0xb) {
						pParticle_1a0->field_0x44.pData[iVar12].pData->pManager = 0;
						hash = pParticle_1a0->field_0x48.pData[iVar12];
						peVar8 = ed3DG2DGetMaterial(pParticle_1a0->field_0x40.pData, hash);
						if (peVar8 == (ed_hash_code*)0x0) {
							edDebugPrintf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>> edPARTICLE_INSTALL, No material found for this particle effect !\n");
							iVar12 = iVar12 + 1;
						}
						else {
							edDListCreatMaterialFromHashCode(pParticle_1a0->field_0x44.pData[iVar12].pData, hash, pParticle_1a0->field_0x40.pData, 2);
							iVar12 = iVar12 + 1;
						}
					}
				}
				else {
					pParticle_1a0->field_0x44.pData = reinterpret_cast<OffsetPointer<edDList_material*>*>(reinterpret_cast<char*>(pFileData) + pParticle_1a0->field_0x44.offset);

					for (iVar12 = 0; iVar12 < pParticle_1a0->field_0xb; iVar12 = iVar12 + 1) {
						for (iVar24 = 0; iVar24 < materialIndex; iVar24 = iVar24 + 1) {
							if (pParticle_1a0->field_0x48.pData[iVar12] == pHashes[iVar24]) {
								pParticle_1a0->field_0x44.pData[iVar12].pData = ppMaterials[iVar24];
								break;
							}
						}

						if (iVar24 == materialIndex) {
							edDebugPrintf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>  Material not found\n");
							pParticle_1a0->field_0x44.pData[iVar12].pData->pManager = (ed_g2d_manager*)0x0;
						}
					}
					
					pParticle_1a0->field_0x18c = 1;
				}
			}

			pParticle_1a0->field_0x114 = -1.0f;
			pParticle_1a0->field_0xd0 = gF32Matrix4Unit;			

			if (pParticle_1a0->field_0x4 != 3) {
				pParticle_1a0->field_0x11c = 6;
				pParticle_1a0->field_0x120 = (uint)pParticle_1a0->field_0xb << 1;
				pParticle_1a0->field_0xc = 1;

				local_60 = pParticle_1a0->field_0x120;
				flags = 0x11;
				if (pParticle_1a0->field_0x4 == 2) {
					flags = 0x111;
					local_60 = 2;
					pParticle_1a0->field_0xc = (uint)pParticle_1a0->field_0xb;
					pParticle_1a0->field_0x118 = 2;
				}

				iVar12 = 0;
				while (iVar12 < pParticle_1a0->field_0xc) {
					if (param_8 == false) {
						nbMatrices = 0;
						iVar24 = pParticle_1a0->field_0x11c * pCurGroup->field_0x10;
						if (pParticle_1a0->field_0x4 != 1) {
							nbMatrices = iVar24 * 4;
							iVar24 = 0;
						}

						pParticle_1a0->field_0x124[iVar12] = edDListNew(TO_HEAP(H_MAIN), flags, local_60, iVar24, nbMatrices, 0, (DisplayList*)0x0);
						edDListSetSceneUsed(pParticle_1a0->field_0x124[iVar12], pScene);
						iVar12 = iVar12 + 1;
					}
					else {
						pParticle_1a0->field_0x124[iVar12] = (DisplayList*)0x0;
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

static _ed_particle_manager* p_current_particle_manager = (_ed_particle_manager*)0x0;

void edParticlesSetSystem(_ed_particle_manager* pManager)
{
	p_current_particle_manager = pManager;
	return;
}

void edParticleGroupUpdate(float time, _ed_particle_group* pGroup)
{

}

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
	Particle_0x1a0* pParticle_1a0;

	nbMatrix = pManager->field_0x70;
	pParticle_1a0 = pManager->field_0x68.pData;

	for (iVar6 = 0; iVar6 < nbMatrix; iVar6 = iVar6 + 1) {
		pParticle_1a0->field_0xd0 = *pMatrix;
		pParticle_1a0 = pParticle_1a0 + 1;
	}

	return;
}

uint gPartProfIdShapers = 0;

struct EffectsManagerProps
{
	int bProfile = 0;
} g_particle_system_config;

static float FLOAT_004485e8 = 0.0019607844f;

void edPartDrawShaper(float param_1, _ed_particle_group* pGroup, Particle_0x1a0* pDrawData, _ed_particle* pParticle, _ed_particle_vectors* pVector, edF32VECTOR4* param_6, int param_7, int param_8)
{
	byte bVar1;
	ushort uVar2;
	short sVar3;
	edFCamera* peVar4;
	edNODE* pNode;
	ed_3d_sprite* peVar5;
	undefined8 uVar6;
	edF32VECTOR4* peVar7;
	uint* puVar8;
	edF32VECTOR4* peVar9;
	int iVar10;
	uint uVar11;
	uint uVar12;
	uint* puVar13;
	long lVar14;
	int iVar15;
	undefined* puVar16;
	uint* puVar17;
	edF32MATRIX4* peVar18;
	ed_3d_sprite* peVar19;
	undefined2* puVar20;
	int iVar21;
	uint uVar22;
	float* pfVar23;
	edF32VECTOR4* peVar24;
	_ed_particle* p_Var25;
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
	int local_8e0;
	int local_8d0;
	float local_7d0;
	float local_7cc;
	float local_7c8;
	float local_7c4;
	float local_7c0;
	float fStack1980;
	float local_770;
	float local_76c;
	float local_768;
	float local_764;
	float local_760;
	float fStack1884;
	float local_710;
	float local_70c;
	float local_708;
	float local_704;
	float local_700;
	float fStack1788;
	float local_690;
	float local_68c;
	float local_688;
	float local_684;
	float local_680;
	float fStack1660;
	float local_670;
	float local_66c;
	float local_668;
	float local_664;
	float local_660;
	float local_65c;
	float local_630;
	float local_62c;
	float local_628;
	float local_624;
	float local_620;
	float local_61c;
	float local_610;
	float local_60c;
	float local_608;
	float local_604;
	float local_600;
	float local_5fc;
	float local_5f0;
	float local_5ec;
	float local_5e8;
	float local_5e4;
	float local_5e0;
	float local_5dc;
	float local_5d0;
	float local_5cc;
	float local_5c8;
	float local_5c4;
	float local_5c0;
	float local_5bc;
	float local_570;
	float local_56c;
	float local_568;
	float local_564;
	float local_560;
	float fStack1372;
	float local_510;
	float local_50c;
	float local_508;
	float local_504;
	float local_500;
	float local_4fc;
	float local_490;
	float local_48c;
	float local_488;
	float local_484;
	float local_480;
	float fStack1148;
	float local_410;
	float local_40c;
	float local_408;
	float local_404;
	float local_400;
	float fStack1020;
	float local_3f0;
	float local_3ec;
	float local_3e8;
	float local_3e4;
	float local_3e0;
	float local_3dc;
	float local_350;
	float local_34c;
	float local_348;
	float local_344;
	float local_340;
	float local_33c;
	float local_310;
	float local_30c;
	float local_308;
	float local_304;
	float local_300;
	float local_2fc;
	float local_2f0;
	float local_2ec;
	float local_2e8;
	float local_2e4;
	float local_2e0;
	float local_2dc;
	edF32MATRIX4 eStack720;
	char local_290;
	char local_28f;
	char local_28e;
	float local_28c;
	float local_288;
	float local_284;
	float local_280;
	float local_27c;
	float local_278;
	float local_274;
	float local_270;
	char local_26c;
	char local_26b;
	int local_268;
	undefined4 local_264;
	undefined4 local_260;
	undefined4 local_25c;
	undefined4 local_258;
	uint local_254;
	uint local_250;
	uint local_24c;
	uint local_248;
	char local_244;
	float local_240;
	float local_23c;
	float local_238;
	float local_230;
	float local_22c;
	float local_228;
	float local_224;
	float local_220;
	float local_21c;
	float local_218;
	float local_214;
	edF32VECTOR4 eStack528;
	edF32VECTOR4 eStack512;
	edF32MATRIX4 eStack496;
	edF32MATRIX4 local_1b0;
	edF32VECTOR4 local_170[4];
	edF32VECTOR4 local_120[4];
	edF32MATRIX4 local_e0;
	undefined4 local_98;
	undefined4 local_94;
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

	if (pGroup->field_0x58 != -1.0f) {
		peVar4 = pDrawData->field_0x198->pCamera;
		edF32Matrix4MulF32Matrix4Hard(&local_e0, &pDrawData->field_0xd0, &peVar4->worldToCamera);
		local_8e0 = 0;
		local_120[0] = {};
		local_120[1] = {};
		local_120[2] = {};
		local_120[3] = {};

		fVar27 = peVar4->computedVerticalHalfFOV;
		fVar28 = peVar4->baseHorizontalHalfFOV;
		fVar29 = -peVar4->computedVerticalHalfFOV;
		fVar30 = peVar4->finalHorizontalHalfFOV;
		fVar31 = -peVar4->computedVerticalHalfFOV;
		fVar32 = peVar4->baseHorizontalHalfFOV;
		fVar33 = peVar4->computedVerticalHalfFOV;
		fVar34 = peVar4->finalHorizontalHalfFOV;

		fVar35 = 1.0 / (sqrtf(local_120[0].x * local_120[0].x + fVar27 * fVar27 + fVar28 * fVar28) + 0.0);
		local_120[0].x = local_120[0].x * fVar35;
		local_120[0].y = fVar27 * fVar35;
		local_120[0].z = fVar28 * fVar35;
		local_120[0].w = 0.0;
		fVar27 = 1.0 / (sqrtf(fVar29 * fVar29 + local_120[1].y * local_120[1].y + fVar30 * fVar30) + 0.0);
		local_120[1].x = fVar29 * fVar27;
		local_120[1].y = local_120[1].y * fVar27;
		local_120[1].z = fVar30 * fVar27;
		local_120[1].w = 0.0;
		fVar27 = 1.0 / (sqrtf(local_120[2].x * local_120[2].x + fVar31 * fVar31 + fVar32 * fVar32) + 0.0);
		local_120[2].x = local_120[2].x * fVar27;
		local_120[2].y = fVar31 * fVar27;
		local_120[2].z = fVar32 * fVar27;
		local_120[2].w = 0.0;
		fVar27 = 1.0 / (sqrtf(fVar33 * fVar33 + local_120[3].y * local_120[3].y + fVar34 * fVar34) + 0.0);
		local_120[3].x = fVar33 * fVar27;
		local_120[3].y = local_120[3].y * fVar27;
		local_120[3].z = fVar34 * fVar27;
		local_120[3].w = 0.0;
		fVar27 = 1.0;

		if (1.0 < pDrawData->field_0x70) {
			fVar27 = pDrawData->field_0x70;
		}

		fVar31 = pGroup->field_0x84;
		fVar32 = pGroup->field_0x88;
		fVar29 = pGroup->field_0x8c;
		fVar30 = pGroup->field_0x58 * fVar27;
		fVar28 = local_e0.ac * fVar31 + local_e0.bc * fVar32 + local_e0.cc * fVar29 + local_e0.dc * 1.0;
		iVar15 = 2;

		if (fVar30 < fVar28) {
			iVar15 = 0;
		}
		else {
			for (iVar10 = 0; iVar10 < 4; iVar10 = iVar10 + 1) {
				fVar33 = local_120[iVar10].x * (local_e0.aa * fVar31 + local_e0.ba * fVar32 + local_e0.ca * fVar29 + local_e0.da * 1.0) +
					local_120[iVar10].y *
					(local_e0.ab * fVar31 + local_e0.bb * fVar32 + local_e0.cb * fVar29 + local_e0.db * 1.0) + local_120[iVar10].z * fVar28;

				if (fVar30 < fVar33) {
					iVar15 = 0;
					break;
				}

				if (fabs(fVar33) < fVar30) {
					iVar15 = 1;
				}
			}
		}

		if (iVar15 != 0) {
			if (iVar15 == 1) {
				p_Var25 = pParticle + param_7;
				fVar27 = fVar27 * FLOAT_004485e8;
				peVar24 = param_6 + param_7;

				for (iVar15 = param_7; iVar15 < param_8; iVar15 = iVar15 + 1) {
					p_Var25->field_0xd = 0;
					if (p_Var25->field_0x3 == 2) {
						fVar29 = fVar27 * p_Var25->field_0x24 * (float)(uint)p_Var25->field_0x4;
						fVar30 = peVar24->x;
						fVar31 = peVar24->y;
						fVar32 = peVar24->z;
						fVar33 = peVar24->w;
						fVar28 = local_e0.ac * fVar30 + local_e0.bc * fVar31 + local_e0.cc * fVar32 + local_e0.dc * fVar33;
						iVar10 = 2;

						if (fVar29 < fVar28) {
							iVar10 = 0;
						}
						else {
							for (iVar21 = 0; iVar21 < 4; iVar21 = iVar21 + 1) {
								fVar34 = local_120[iVar21].x *
									(local_e0.aa * fVar30 + local_e0.ba * fVar31 + local_e0.ca * fVar32 + local_e0.da * fVar33) +
									local_120[iVar21].y *
									(local_e0.ab * fVar30 + local_e0.bb * fVar31 + local_e0.cb * fVar32 + local_e0.db * fVar33) +
									local_120[iVar21].z * fVar28;

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
							p_Var25->field_0xd = 1;
							local_8e0 = local_8e0 + 1;
						}
					}
					p_Var25 = p_Var25 + 1;
					peVar24 = peVar24 + 1;
				}
			}
			else {
				p_Var25 = pParticle + param_7;

				for (iVar15 = param_7; iVar15 < param_8; iVar15 = iVar15 + 1) {
					if (p_Var25->field_0x3 == 2) {
						p_Var25->field_0xd = 1;
						local_8e0 = local_8e0 + 1;
					}
					else {
						p_Var25->field_0xd = 0;
					}

					p_Var25 = p_Var25 + 1;
				}
			}

			if (local_8e0 != 0) {
				p_Var25 = pParticle + param_7;
				//peVar9 = &pVector->field_0x0 + param_7 * 2;
				peVar24 = param_6 + param_7;
				bVar1 = pDrawData->field_0x4;
				if (bVar1 == 3) {
					IMPLEMENTATION_GUARD(
					peVar18 = &pDrawData->field_0xd0;
					peVar9 = local_170;
					iVar15 = 4;
					do {
						uVar6 = *(undefined8*)peVar18;
						fVar27 = peVar18->ac;
						fVar28 = peVar18->ad;
						iVar15 = iVar15 + -1;
						peVar9->x = (float)uVar6;
						peVar9->y = (float)((ulong)uVar6 >> 0x20);
						peVar9->z = fVar27;
						peVar9->w = fVar28;
						peVar18 = (edF32MATRIX4*)&peVar18->ba;
						peVar9 = peVar9 + 1;
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
					for (; param_7 < param_8; param_7 = param_7 + 1) {
						pNode = p_Var25->pNode;
						if ((p_Var25->field_0x0 == 2) && (p_Var25->field_0x3 == 2)) {
							ed3DHierarchyNodeClrFlag(pNode, 0x40);
							peVar5 = pNode->pData;
							peVar9 = local_170;
							iVar15 = 8;
							peVar19 = peVar5;
							do {
								iVar15 = iVar15 + -1;
								fVar27 = peVar9->y;
								peVar19->flags_0x0 = (uint)peVar9->x;
								peVar9 = (edF32VECTOR4*)&peVar9->z;
								*(float*)&peVar19->materialIndex = fVar27;
								peVar19 = (ed_3d_sprite*)&peVar19->offsetA;
							} while (0 < iVar15);
							edF32Matrix4MulF32Vector4Hard
							((edF32VECTOR4*)&peVar5->bUseShadowMatrix_0x30, &pDrawData->field_0xd0, peVar24);
							*(undefined4*)&peVar5->pTextureDataMystery = 0x3f800000;
							fVar27 = DAT_00448598 * p_Var25->field_0x24 * (float)(uint)p_Var25->field_0x4 * 0.5;
							peVar5->flags_0x0 = (uint)((float)peVar5->flags_0x0 * fVar27);
							*(float*)&peVar5->materialIndex = *(float*)&peVar5->materialIndex * fVar27;
							peVar5->offsetA = (int)((float)peVar5->offsetA * fVar27);
							peVar5->pNext = (ed_3d_sprite*)((float)peVar5->pNext * fVar27);
							(peVar5->vector_0x10).x = (peVar5->vector_0x10).x * fVar27;
							(peVar5->vector_0x10).y = (peVar5->vector_0x10).y * fVar27;
							(peVar5->vector_0x10).z = (peVar5->vector_0x10).z * fVar27;
							(peVar5->vector_0x10).w = (peVar5->vector_0x10).w * fVar27;
							peVar5->pSTBuf = (char*)((float)peVar5->pSTBuf * fVar27);
							peVar5->pColorBuf = (char*)((float)peVar5->pColorBuf * fVar27);
							peVar5->pVertexBuf = (edF32VECTOR4*)((float)peVar5->pVertexBuf * fVar27);
							peVar5->field_0x2c = (char*)((float)peVar5->field_0x2c * fVar27);
						}
						else {
							ed3DHierarchyNodeSetFlag(pNode, 0x40);
						}
						p_Var25 = p_Var25 + 1;
						peVar24 = peVar24 + 1;
					})
				}
				else {
					if ((bVar1 == 1) || (bVar1 == 4)) {
						IMPLEMENTATION_GUARD(
						if (pDrawData->field_0x124[0] != (DisplayList*)0x0) {
							edDListSetCurrent(pDrawData->field_0x124[0]);
						}
						edDListLoadMatrix(&pDrawData->field_0xd0);
						uVar2 = pDrawData->field_0x80;
						fVar27 = pDrawData->field_0xc0 * 255.0;
						if (2.147484e+09 <= fVar27) {
							fVar27 = fVar27 - 2.147484e+09;
						}
						local_900 = 0;
						uVar26 = (int)fVar27 & 0xff;
						if ((pDrawData->field_0x18d & 0x1f) != 0) {
							ed3DSceneComputeCameraToScreenMatrix(pDrawData->field_0x198, &local_1b0);
							edF32Matrix4MulF32Matrix4Hard(&eStack496, &pDrawData->field_0x198->pCamera->worldToCamera, &local_1b0);
							if ((pDrawData->field_0x18d & 3) != 0) {
								p_Var25 = pParticle + param_7;
								for (iVar15 = param_7; iVar15 < param_8; iVar15 = iVar15 + 1) {
									iVar10 = 0;
									if (p_Var25->field_0xd != 0) {
										for (; iVar21 = local_900, iVar10 < local_900; iVar10 = iVar10 + 1) {
											if ((pDrawData->field_0x18d & 1) == 0) {
												if (table_sorted_values$1919[iVar10] < p_Var25->field_0x1c) break;
											}
											else {
												if (p_Var25->field_0x1c < table_sorted_values$1919[iVar10]) break;
											}
										}
										for (; iVar10 < iVar21; iVar21 = iVar21 + -1) {
											table_sorted_values$1919[iVar21] = (float)table_sorted_indices$1918[iVar21 + 0xff];
											table_sorted_indices$1918[iVar21] = *(int*)(&DAT_00482fac + iVar21 * 4);
										}
										local_900 = local_900 + 1;
										table_sorted_values$1919[iVar10] = p_Var25->field_0x1c;
										table_sorted_indices$1918[iVar10] = iVar15;
									}
									p_Var25 = p_Var25 + 1;
								}
								table_sorted_indices$1918[local_900] = -1;
								table_sorted_values$1919[local_900] = (float)&DAT_bf800000;
							}
							if (((pDrawData->field_0x18d & 0x10) != 0) && (local_900 != 0)) {
								fVar27 = 0.0;
								p_Var25 = pParticle + param_7;
								peVar24 = param_6 + param_7;
								bVar1 = pDrawData->field_0x18f;
								for (iVar15 = 0; iVar15 < local_900; iVar15 = iVar15 + 1) {
									p_Var25 = pParticle + table_sorted_indices$1918[iVar15];
									peVar24 = param_6 + table_sorted_indices$1918[iVar15];
									if (p_Var25->field_0xd != 0) {
										if (fVar27 < (float)(uint)bVar1) {
											edF32Matrix4MulF32Vector4Hard(&eStack512, &eStack496, peVar24);
											eStack512.w = DAT_00448598 * p_Var25->field_0x24 * (float)(uint)p_Var25->field_0x4 * 0.5 *
												ABS(1.0 / eStack512.w) * ABS(local_1b0.aa);
											if (max_size$1966 < eStack512.w) {
												eStack512.w = max_size$1966;
											}
											fVar27 = fVar27 + eStack512.w * eStack512.w;
											if ((float)(uint)bVar1 < fVar27) {
												p_Var25->field_0xd = 0;
												local_8e0 = local_8e0 + -1;
											}
											else {
												p_Var25->field_0xd = 1;
											}
										}
										else {
											p_Var25->field_0xd = 0;
											local_8e0 = local_8e0 + -1;
										}
									}
								}
							}
							if ((pDrawData->field_0x18d & 0xc) != 0) {
								local_900 = 0;
								p_Var25 = pParticle + param_7;
								peVar24 = param_6 + param_7;
								for (iVar15 = param_7; iVar15 < param_8; iVar15 = iVar15 + 1) {
									if (p_Var25->field_0xd != 0) {
										edF32Matrix4MulF32Vector4Hard(&eStack528, &eStack496, peVar24);
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
										table_sorted_values$1919[iVar10] = p_Var25->field_0x1c;
										table_sorted_indices$1918[iVar10] = iVar15;
									}
									p_Var25 = p_Var25 + 1;
									peVar24 = peVar24 + 1;
								}
								table_sorted_indices$1918[local_900] = -1;
								table_sorted_values$1919[local_900] = (float)&DAT_bf800000;
							}
						}
						if (pDrawData->field_0x44->pManager == (ed_g2d_manager*)0x0) {
							edDListUseMaterial((edDList_material*)0x0);
						}
						else {
							edDListUseMaterial(pDrawData->field_0x44);
						}
						edDListBlendSet(1);
						if (pDrawData->field_0x4 == 1) {
							edDListBegin(0.0, 0.0, 0.0, 0xb, local_8e0);
							local_28e = (uVar2 & 1) != 0;
							local_290 = '\0';
							if ((bool)local_28e) {
								local_288 = 1.0;
								local_274 = 1.0;
								local_27c = 0.0;
								local_278 = 0.0;
								local_270 = 0.0;
							}
							else {
								local_288 = 0.0;
							}
							local_280 = 1.0;
							local_284 = 1.0;
							local_28c = 0.0;
							local_28f = '\x01';
							local_26c = '\x01';
							local_268 = (int)(param_1 * 255.0);
							local_26b = *(int*)&pDrawData->field_0xc4 != -1;
							if ((bool)local_26b) {
								local_254 = (uint)pDrawData->field_0xc4;
								local_250 = (uint)pDrawData->field_0xc5;
								local_24c = (uint)pDrawData->field_0xc6;
								local_248 = (uint)pDrawData->field_0xc7;
							}
							local_244 = '\x01';
							local_240 = pDrawData->field_0x70;
							pfVar23 = (float*)&DAT_00431860;
							local_230 = local_e0.aa;
							local_22c = local_e0.ab;
							local_228 = local_e0.ac;
							local_224 = local_e0.ad;
							local_220 = local_e0.ba;
							local_21c = local_e0.bb;
							local_218 = local_e0.bc;
							local_214 = local_e0.bd;
						}
						else {
							edDListBegin(0.0, 0.0, 0.0, 8, local_8e0 << 2);
							local_28c = 0.0;
							local_290 = '\x01';
							local_28f = '\x01';
							local_28e = '\x01';
							local_26c = '\x01';
							local_288 = 1.0;
							local_268 = (int)(param_1 * 255.0);
							local_284 = 1.0;
							local_280 = 1.0;
							local_274 = 1.0;
							local_27c = 0.0;
							local_278 = 0.0;
							local_270 = 0.0;
							local_26b = *(int*)&pDrawData->field_0xc4 != -1;
							if ((bool)local_26b) {
								local_254 = (uint)pDrawData->field_0xc4;
								local_250 = (uint)pDrawData->field_0xc5;
								local_24c = (uint)pDrawData->field_0xc6;
								local_248 = (uint)pDrawData->field_0xc7;
							}
							local_240 = pDrawData->field_0x70;
							pfVar23 = (float*)&DAT_00431880;
							edF32Matrix4FromEulerSoft(&eStack720, &pDrawData->field_0x20, (char*)&PTR_DAT_004318a0);
							local_230 = gF32Vector4UnitX.x;
							local_22c = gF32Vector4UnitX.y;
							local_228 = gF32Vector4UnitX.z;
							local_224 = gF32Vector4UnitX.w;
							local_220 = DAT_00431720;
							local_21c = DAT_00431724;
							local_218 = DAT_00431728;
							local_214 = DAT_0043172c;
						}
						if (((((((uVar2 & 1) == 0) || ((uVar2 & 0x20) == 0)) || ((short)(uVar2 & 0x400) >> 10 != 1)) ||
							(((short)(uVar2 & 0x800) >> 0xb != 0 || ((uVar2 & 8) == 0)))) ||
							((((uVar2 & 0x300) != 0 || (((uVar2 & 2) == 0 || ((uVar2 & 4) == 0)))) || ((uVar2 & 0x40) != 0)))) ||
							((((uVar26 != 0 || (local_290 != '\0')) || (local_28f == '\0')) ||
								(((local_28e == '\0' || (local_26c == '\0')) ||
									((local_26b != '\0' || ((local_244 == '\0' || ((pDrawData->field_0x18d & 0xc) != 0)))))))))) {
							if (((((uVar2 & 1) == 0) ||
								((((uVar2 & 0x20) == 0 || ((short)(uVar2 & 0x400) >> 10 != 0)) ||
									((short)(uVar2 & 0x800) >> 0xb != 0)))) ||
								((((uVar2 & 8) == 0 || ((uVar2 & 0x300) != 0)) ||
									(((uVar2 & 2) == 0 || (((uVar2 & 4) == 0 || ((uVar2 & 0x40) != 0)))))))) ||
								((uVar26 != 0 ||
									(((((local_290 != '\0' || (local_28f == '\0')) || (local_28e == '\0')) ||
										((local_26c == '\0' || (local_26b != '\0')))) ||
										((local_244 == '\0' || ((pDrawData->field_0x18d & 0xc) != 0)))))))) {
								if (((((uVar2 & 1) == 0) || ((uVar2 & 0x20) == 0)) || ((short)(uVar2 & 0x400) >> 10 != 1)) ||
									((((((((short)(uVar2 & 0x800) >> 0xb != 0 || ((uVar2 & 8) == 0)) || ((uVar2 & 0x300) != 0)) ||
										(((uVar2 & 2) != 0 || ((uVar2 & 4) != 0)))) ||
										(((uVar2 & 0x40) != 0 || ((uVar26 != 0 || (local_290 != '\0')))))) || (local_28f == '\0')) ||
										((((local_28e == '\0' || (local_26c == '\0')) || (local_26b != '\0')) ||
											((local_244 == '\0' || ((pDrawData->field_0x18d & 0xc) != 0)))))))) {
									if (((((uVar2 & 1) == 0) || (((uVar2 & 0x20) == 0 || ((short)(uVar2 & 0x400) >> 10 != 1)))) ||
										((short)(uVar2 & 0x800) >> 0xb != 0)) ||
										(((((((((uVar2 & 8) == 0 || ((uVar2 & 0x300) != 0)) || ((uVar2 & 2) == 0)) ||
											(((uVar2 & 4) == 0 || ((uVar2 & 0x40) != 0)))) || (uVar26 == 0)) ||
											((local_290 != '\0' || (local_28f == '\0')))) ||
											((local_28e == '\0' || (((local_26c == '\0' || (local_26b != '\0')) || (local_244 == '\0'))))))
											|| ((pDrawData->field_0x18d & 0xc) != 0)))) {
										if ((((((uVar2 & 1) == 0) || ((uVar2 & 0x20) != 0)) ||
											(((uVar2 & 0x300) == 0 || (((uVar2 & 0x40) == 0 || ((uVar2 & 0x10) != 0)))))) ||
											(((uVar2 & 0x80) == 0 || (((uVar26 == 0 || (local_290 != '\0')) || (local_28f == '\0')))))) ||
											(((local_28e == '\0' || (local_26c == '\0')) ||
												((local_26b != '\0' || ((local_244 == '\0' || ((pDrawData->field_0x18d & 0xc) != 0)))))))) {
											if ((((uVar2 & 1) == 0) ||
												(((((uVar2 & 0x20) == 0 || ((short)(uVar2 & 0x400) >> 10 != 0)) ||
													((short)(uVar2 & 0x800) >> 0xb != 0)) || (((uVar2 & 8) != 0 || ((uVar2 & 0x300) != 0))))))
												|| ((((uVar2 & 2) != 0 || (((uVar2 & 4) != 0 || ((uVar2 & 0x40) != 0)))) ||
													((uVar26 != 0 ||
														((((local_290 != '\0' || (local_28f == '\0')) || (local_28e == '\0')) ||
															(((local_26c == '\0' || (local_26b != '\0')) ||
																((local_244 == '\0' || ((pDrawData->field_0x18d & 0xc) != 0)))))))))))) {
												if (((((uVar2 & 1) == 0) || ((uVar2 & 0x20) != 0)) ||
													(((uVar2 & 0x300) == 0 ||
														((((uVar2 & 2) != 0 || ((uVar2 & 4) != 0)) || ((uVar2 & 0x40) != 0)))))) ||
													(((((uVar26 != 0 || (local_290 != '\0')) ||
														((local_28f == '\0' || ((local_28e == '\0' || (local_26c == '\0')))))) ||
														(local_26b != '\0')) || ((local_244 == '\0' || ((pDrawData->field_0x18d & 0xc) != 0)))))) {
													if (((((((uVar2 & 1) == 0) ||
														(((((uVar2 & 0x20) == 0 || ((short)(uVar2 & 0x400) >> 10 != 0)) ||
															((short)(uVar2 & 0x800) >> 0xb != 0)) ||
															(((uVar2 & 8) != 0 || ((uVar2 & 0x300) != 0)))))) || ((uVar2 & 2) != 0)) ||
														((((uVar2 & 4) != 0 || ((uVar2 & 0x40) == 0)) ||
															(((uVar2 & 0x10) == 0 ||
																((((uVar2 & 0x80) == 0 || (uVar26 != 0)) || (local_290 != '\0')))))))) ||
														((local_28f == '\0' || (local_28e == '\0')))) ||
														((local_26c == '\0' ||
															(((local_26b != '\0' || (local_244 == '\0')) || ((pDrawData->field_0x18d & 0xc) != 0))))))
													{
														if (((((((uVar2 & 1) == 0) || ((uVar2 & 0x20) != 0)) || ((uVar2 & 0x300) != 0)) ||
															(((uVar2 & 2) == 0 || ((uVar2 & 4) == 0)))) ||
															((((uVar2 & 0x40) == 0 || (((uVar2 & 0x10) != 0 || ((uVar2 & 0x80) == 0)))) ||
																(uVar26 != 0)))) ||
															((((((local_290 != '\0' || (local_28f == '\0')) || (local_28e == '\0')) ||
																((local_26c == '\0' || (local_26b != '\0')))) || (local_244 == '\0')) ||
																((pDrawData->field_0x18d & 0xc) != 0)))) {
															if (((((uVar2 & 1) == 0) || ((uVar2 & 0x20) == 0)) ||
																(((short)(uVar2 & 0x400) >> 10 != 0 ||
																	((((short)(uVar2 & 0x800) >> 0xb != 1 || ((uVar2 & 0x300) != 0)) ||
																		((uVar2 & 2) != 0)))))) ||
																((((((uVar2 & 4) != 0 || ((uVar2 & 0x40) != 0)) || (uVar26 != 0)) ||
																	((local_290 != '\0' || (local_28f == '\0')))) ||
																	(((local_28e == '\0' ||
																		(((local_26c == '\0' || (local_26b != '\0')) || (local_244 == '\0')))) ||
																		((pDrawData->field_0x18d & 0xc) != 0)))))) {
																if (((((uVar2 & 1) == 0) || ((uVar2 & 0x20) == 0)) ||
																	(((short)(uVar2 & 0x400) >> 10 != 0 ||
																		(((short)(uVar2 & 0x800) >> 0xb != 0 || ((uVar2 & 8) == 0)))))) ||
																	(((((uVar2 & 0x300) != 0 ||
																		((((((uVar2 & 2) != 0 || ((uVar2 & 4) != 0)) || ((uVar2 & 0x40) != 0)) ||
																			((uVar26 != 0 || (local_290 != '\0')))) ||
																			((local_28f == '\0' || ((local_28e == '\0' || (local_26c == '\0')))))))) ||
																		(local_26b != '\0')) ||
																		((local_244 == '\0' || ((pDrawData->field_0x18d & 0xc) != 0)))))) {
																	if (((((uVar2 & 1) == 0) ||
																		((((uVar2 & 0x20) == 0 || ((short)(uVar2 & 0x400) >> 10 != 1)) ||
																			((short)(uVar2 & 0x800) >> 0xb != 1)))) ||
																		((((uVar2 & 0x300) != 0 || ((uVar2 & 2) == 0)) || ((uVar2 & 4) == 0)))) ||
																		(((((uVar2 & 0x40) != 0 || (uVar26 != 0)) ||
																			(((local_290 != '\0' ||
																				(((local_28f == '\0' || (local_28e == '\0')) || (local_26c == '\0')))) ||
																				((local_26b != '\0' || (local_244 == '\0')))))) ||
																			((pDrawData->field_0x18d & 0xc) != 0)))) {
																		if ((((((uVar2 & 1) == 0) || ((uVar2 & 0x20) == 0)) ||
																			(((short)(uVar2 & 0x400) >> 10 != 1 ||
																				(((((short)(uVar2 & 0x800) >> 0xb != 0 || ((uVar2 & 8) != 0)) ||
																					((uVar2 & 0x300) != 0)) || (((uVar2 & 2) != 0 || ((uVar2 & 4) != 0))))))))
																			|| (((uVar2 & 0x40) != 0 || ((uVar26 != 0 || (local_290 != '\0')))))) ||
																			(((local_28f == '\0' ||
																				(((local_28e == '\0' || (local_26c == '\0')) || (local_26b != '\0')))) ||
																				((local_244 == '\0' || ((pDrawData->field_0x18d & 0xc) != 0)))))) {
																			if (((uVar2 & 1) == 0) ||
																				(((((((uVar2 & 0x20) != 0 || ((uVar2 & 0x300) != 0)) ||
																					(((uVar2 & 2) == 0 ||
																						((((uVar2 & 4) != 0 || ((uVar2 & 0x40) != 0)) || (uVar26 != 0)))))) ||
																					((local_290 != '\0' || (local_28f == '\0')))) || (local_28e == '\0')) ||
																					(((local_26c == '\0' || (local_26b != '\0')) ||
																						((local_244 == '\0' || ((pDrawData->field_0x18d & 0xc) != 0)))))))) {
																				if (((((((uVar2 & 1) == 0) || ((uVar2 & 0x20) != 0)) || ((uVar2 & 0x300) != 0))
																					|| (((uVar2 & 2) != 0 || ((uVar2 & 4) != 0)))) || ((uVar2 & 0x40) != 0)) ||
																					((((uVar26 == 0 || (local_290 != '\0')) ||
																						((local_28f == '\0' ||
																							(((local_28e == '\0' || (local_26c == '\0')) || (local_26b != '\0'))))))
																						|| ((local_244 == '\0' || ((pDrawData->field_0x18d & 0xc) != 0)))))) {
																					if ((((uVar2 & 1) == 0) || (((uVar2 & 0x20) != 0 || ((uVar2 & 0x300) != 0))))
																						|| (((uVar2 & 2) != 0 ||
																							((((((uVar2 & 4) != 0 || ((uVar2 & 0x40) == 0)) ||
																								((uVar2 & 0x10) == 0)) ||
																								((((uVar2 & 0x80) == 0 || (uVar26 != 0)) ||
																									((local_290 != '\0' || ((local_28f == '\0' || (local_28e == '\0'))))
																										)))) || ((local_26c == '\0' ||
																											(((local_26b != '\0' || (local_244 == '\0')) ||
																												((pDrawData->field_0x18d & 0xc) != 0)))))))))) {
																						if (((((uVar2 & 1) == 0) || ((uVar2 & 0x20) == 0)) ||
																							(((((short)(uVar2 & 0x400) >> 10 != 0 ||
																								(((short)(uVar2 & 0x800) >> 0xb != 1 || ((uVar2 & 0x300) != 0)))) ||
																								((uVar2 & 2) != 0)) ||
																								(((((uVar2 & 4) != 0 || ((uVar2 & 0x40) != 0)) || (uVar26 == 0)) ||
																									((local_290 != '\0' || (local_28f == '\0')))))))) ||
																							(((local_28e == '\0' || ((local_26c == '\0' || (local_26b != '\0')))) ||
																								((local_244 == '\0' || ((pDrawData->field_0x18d & 0xc) != 0)))))) {
																							if ((((((((uVar2 & 1) == 0) || ((uVar2 & 0x20) != 0)) ||
																								((uVar2 & 0x300) != 0)) ||
																								((((uVar2 & 2) == 0 || ((uVar2 & 4) == 0)) ||
																									(((uVar2 & 0x40) == 0 ||
																										(((uVar2 & 0x10) == 0 || ((uVar2 & 0x80) == 0)))))))) ||
																								(uVar26 != 0)) ||
																								((((local_290 != '\0' || (local_28f == '\0')) || (local_28e == '\0'))
																									|| ((local_26c == '\0' || (local_26b != '\0')))))) ||
																								((local_244 == '\0' || ((pDrawData->field_0x18d & 0xc) != 0)))) {
																								if ((((((uVar2 & 1) == 0) || ((uVar2 & 0x20) == 0)) ||
																									((short)(uVar2 & 0x400) >> 10 != 1)) ||
																									((((((short)(uVar2 & 0x800) >> 0xb != 0 || ((uVar2 & 8) == 0)) ||
																										((uVar2 & 0x300) != 0)) ||
																										(((uVar2 & 2) != 0 || ((uVar2 & 4) == 0)))) ||
																										(((uVar2 & 0x40) != 0 || ((uVar26 != 0 || (local_290 != '\0')))))))
																									) || ((local_28f == '\0' ||
																										((((local_28e == '\0' || (local_26c == '\0')) ||
																											(local_26b != '\0')) ||
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
																												(((local_26b != '\0' || (local_244 == '\0')) ||
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
																											((local_26b != '\0' ||
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
																														(((local_26c == '\0' || (local_26b != '\0')) ||
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
																														(((local_26c == '\0' || (local_26b != '\0')) ||
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
																															((local_26b != '\0' || (local_244 == '\0')))) ||
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
																																	((local_26c == '\0' || (local_26b != '\0')))) ||
																																	(local_244 == '\0')) ||
																																	((pDrawData->field_0x18d & 0xc) != 0)))))) {
																															if (((local_290 == '\0') && (local_28f != '\0')) &&
																																((local_28e != '\0' &&
																																	((((local_26c != '\0' && (local_26b == '\0')) &&
																																		(local_244 != '\0')) &&
																																		((pDrawData->field_0x18d & 0xc) == 0)))))) {
																																p_Var25 = pParticle + param_7;
																																peVar24 = param_6 + param_7;
																																for (; param_7 < param_8; param_7 = param_7 + 1) {
																																	if (p_Var25->field_0xd != 0) {
																																		if ((pDrawData->field_0x18d & 0xc) != 0) {
																																			iVar15 = table_sorted_indices$1918[param_7];
																																			p_Var25 = pParticle + iVar15;
																																			peVar24 = param_6 + iVar15;
																																			peVar9 = &pVector->field_0x0 + iVar15 * 2;
																																		}
																																		uVar12 = *(uint*)&p_Var25->field_0x3c;
																																		if ((uVar2 & 1) != 0) {
																																			fVar27 = p_Var25->field_0x1c / p_Var25->field_0x20
																																				;
																																			if ((uVar2 & 0x20) == 0) {
																																				local_70c = 0.0;
																																				local_710 = 1.0;
																																				local_704 = 1.0;
																																				local_708 = 0.0;
																																				local_700 = 0.0;
																																				fStack1788 = 0.0;
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
																																					local_708 = (peVar9[1].x * local_230 +
																																						peVar9[1].y * local_22c +
																																						peVar9[1].z * local_228) *
																																						p_Var25->field_0x30;
																																					local_710 = (peVar9[1].x * local_220 +
																																						peVar9[1].y * local_21c +
																																						peVar9[1].z * local_218) *
																																						p_Var25->field_0x30;
																																					fVar30 = SQRT(local_708 * local_708 +
																																						local_710 * local_710);
																																					fVar29 = g_TinyFloat_00448548;
																																					if (fVar30 != 0.0) {
																																						fVar29 = fVar30;
																																					}
																																					local_708 = local_708 * (1.0 / fVar29);
																																					local_710 = local_710 * (1.0 / fVar29);
																																					local_70c = -local_708;
																																					local_700 = (local_710 + local_708) * -0.5 +
																																						0.5;
																																					fStack1788 = (local_710 - local_708) * -0.5 +
																																						0.5;
																																					if ((fVar28 != 0.0) ||
																																						(local_704 = local_710,
																																							pDrawData->field_0x94 != 0.0)) {
																																						fVar29 = pDrawData->field_0x94 * fVar27;
																																						if (0.0 < local_708) {
																																							fVar29 = -fVar29;
																																						}
																																						fVar30 = (fVar28 + fVar29) *
																																							g_ScalingFactor_00448518;
																																						puVar16 = (undefined*)0x3f000000;
																																						if (fVar30 < 0.0) {
																																							puVar16 = &DAT_bf000000;
																																						}
																																						fVar28 = (fVar28 + fVar29) *
																																							g_ScalingFactor_00448518;
																																						fVar29 = edFCosinus
																																							[(int)(fVar30 + (float)puVar16) &
																																							0x1fff];
																																						if (fVar28 < 0.0) {
																																							puVar16 = &DAT_bf000000;
																																						}
																																						else {
																																							puVar16 = (undefined*)0x3f000000;
																																						}
																																						fVar32 = edFSinus[(int)(fVar28 + (float)
																																							puVar16) & 0x1fff];
																																						fVar30 = -(fVar29 * 0.5) + fVar32 * 0.5 + 0.5;
																																						fVar31 = (-(fVar32 * 0.5) - fVar29 * 0.5) + 0.5;
																																						fVar33 = -fVar32 * local_710;
																																						fVar28 = fVar32 * local_710;
																																						local_704 = fVar29 * local_710 + -fVar32 * local_70c;
																																						local_700 = local_700 +
																																							fVar31 * local_708 + fVar30 * local_710;
																																						fStack1788 = fStack1788 +
																																							fVar31 * local_710 + fVar30 * local_70c;
																																						local_710 = fVar32 * local_708 + fVar29 * local_710;
																																						local_70c = fVar28 + fVar29 * local_70c;
																																						local_708 = fVar29 * local_708 + fVar33;
																																					}
																																				}
																																				else {
																																					fVar29 = pDrawData->field_0x94 * fVar27;
																																					if (((uVar2 & 8) != 0) && ((uVar12 & 0x20) != 0)) {
																																						fVar29 = -fVar29;
																																					}
																																					fVar30 = (fVar28 + fVar29) * g_ScalingFactor_00448518;
																																					puVar16 = (undefined*)0x3f000000;
																																					if (fVar30 < 0.0) {
																																						puVar16 = &DAT_bf000000;
																																					}
																																					fVar28 = (fVar28 + fVar29) * g_ScalingFactor_00448518;
																																					local_710 = edFCosinus
																																						[(int)(fVar30 + (float)puVar16) & 0x1fff];
																																					if (fVar28 < 0.0) {
																																						puVar16 = &DAT_bf000000;
																																					}
																																					else {
																																						puVar16 = (undefined*)0x3f000000;
																																					}
																																					local_70c = edFSinus[(int)(fVar28 + (float)puVar16) &
																																						0x1fff];
																																					local_700 = -(local_710 * 0.5) + local_70c * 0.5 + 0.5;
																																					fStack1788 = (-(local_70c * 0.5) - local_710 * 0.5) + 0.5;
																																					local_708 = -local_70c;
																																					local_704 = local_710;
																																				}
																																			}
																																			if ((uVar2 & 0x300) != 0) {
																																				DAT_0041eb50 = pDrawData->field_0x88;
																																				DAT_0041eb5c = pDrawData->field_0x8c;
																																				DAT_0041eb60 = DAT_0041eb50 * -0.5 + 0.5;
																																				DAT_0041eb64 = DAT_0041eb5c * -0.5 + 0.5;
																																				local_710 = local_70c * DAT_0041eb58 +
																																					local_710 * DAT_0041eb50;
																																				local_708 = local_704 * DAT_0041eb58 +
																																					local_708 * DAT_0041eb50;
																																				local_70c = local_70c * DAT_0041eb5c +
																																					local_710 * DAT_0041eb54;
																																				local_704 = local_704 * DAT_0041eb5c +
																																					local_708 * DAT_0041eb54;
																																				local_700 = DAT_0041eb60 +
																																					fStack1788 * DAT_0041eb58 +
																																					local_700 * DAT_0041eb50;
																																				fStack1788 = DAT_0041eb64 +
																																					fStack1788 * DAT_0041eb5c +
																																					local_700 * DAT_0041eb54;
																																			}
																																			if (((uVar2 & 2) != 0) && ((uVar12 & 0x40) != 0)) {
																																				local_710 = local_70c * DAT_0041eb78 +
																																					local_710 * DAT_0041eb70;
																																				local_708 = local_704 * DAT_0041eb78 +
																																					local_708 * DAT_0041eb70;
																																				local_70c = local_70c * DAT_0041eb7c +
																																					local_710 * DAT_0041eb74;
																																				local_704 = local_704 * DAT_0041eb7c +
																																					local_708 * DAT_0041eb74;
																																				local_700 = DAT_0041eb80 +
																																					fStack1788 * DAT_0041eb78 +
																																					local_700 * DAT_0041eb70;
																																				fStack1788 = DAT_0041eb84 +
																																					fStack1788 * DAT_0041eb7c +
																																					local_700 * DAT_0041eb74;
																																			}
																																			if (((uVar2 & 4) != 0) && ((uVar12 & 0x80) != 0)) {
																																				local_710 = local_70c * DAT_0041eb98 +
																																					local_710 * DAT_0041eb90;
																																				local_708 = local_704 * DAT_0041eb98 +
																																					local_708 * DAT_0041eb90;
																																				local_70c = local_70c * DAT_0041eb9c +
																																					local_710 * DAT_0041eb94;
																																				local_704 = local_704 * DAT_0041eb9c +
																																					local_708 * DAT_0041eb94;
																																				local_700 = DAT_0041eba0 +
																																					fStack1788 * DAT_0041eb98 +
																																					local_700 * DAT_0041eb90;
																																				fStack1788 = DAT_0041eba4 +
																																					fStack1788 * DAT_0041eb9c +
																																					local_700 * DAT_0041eb94;
																																			}
																																			if ((uVar2 & 0x40) != 0) {
																																				if ((uVar2 & 0x10) == 0) {
																																					iVar15 = 0;
																																				}
																																				else {
																																					iVar15 = (int)(((int)uVar12 >> 8 & 0xffffU) *
																																						(int)pDrawData->field_0x82) >> 0x10;
																																				}
																																				if ((uVar2 & 0x80) == 0) {
																																					iVar15 = iVar15 + (int)(fVar27 * (float)(int)pDrawData->
																																						field_0x82)
																																						;
																																					lVar14 = (long)iVar15;
																																					if (lVar14 < 0) {
																																						lVar14 = (long)-iVar15;
																																					}
																																					if (pDrawData->field_0x82 <= lVar14) {
																																						lVar14 = (long)(pDrawData->field_0x82 + -1);
																																					}
																																				}
																																				else {
																																					iVar15 = (iVar15 + (int)(p_Var25->field_0x1c *
																																						(float)*(int*)&pDrawData->
																																						field_0x98)) %
																																						(int)pDrawData->field_0x82;
																																					lVar14 = (long)iVar15;
																																					if (pDrawData->field_0x82 == 0) {
																																						trap(7);
																																					}
																																					if (lVar14 < 0) {
																																						lVar14 = (long)-iVar15;
																																					}
																																				}
																																				sVar3 = pDrawData->field_0x84;
																																				DAT_0041ebb0 = 1.0 / (float)(int)sVar3;
																																				DAT_0041ebbc = 1.0 / (float)(int)pDrawData->field_0x86;
																																				if (sVar3 == 0) {
																																					trap(7);
																																				}
																																				DAT_0041ebc0 = DAT_0041ebb0 *
																																					(float)((int)lVar14 % (int)sVar3);
																																				if (pDrawData->field_0x84 == 0) {
																																					trap(7);
																																				}
																																				DAT_0041ebc4 = DAT_0041ebbc *
																																					(float)((int)lVar14 /
																																						(int)pDrawData->field_0x84);
																																				local_710 = local_70c * DAT_0041ebb8 +
																																					local_710 * DAT_0041ebb0;
																																				local_708 = local_704 * DAT_0041ebb8 +
																																					local_708 * DAT_0041ebb0;
																																				local_70c = local_70c * DAT_0041ebbc +
																																					local_710 * DAT_0041ebb4;
																																				local_704 = local_704 * DAT_0041ebbc +
																																					local_708 * DAT_0041ebb4;
																																				local_700 = DAT_0041ebc0 +
																																					fStack1788 * DAT_0041ebb8 +
																																					local_700 * DAT_0041ebb0;
																																				fStack1788 = DAT_0041ebc4 +
																																					fStack1788 * DAT_0041ebbc +
																																					local_700 * DAT_0041ebb4;
																																			}
																																			local_28c = pfVar23[1] * local_708 + *pfVar23 * local_710 +
																																				local_700;
																																			local_288 = pfVar23[1] * local_704 + *pfVar23 * local_70c +
																																				fStack1788;
																																			local_284 = pfVar23[3] * local_708 + pfVar23[2] * local_710
																																				+ local_700;
																																			local_280 = pfVar23[3] * local_704 + pfVar23[2] * local_70c
																																				+ fStack1788;
																																			local_27c = pfVar23[5] * local_708 + pfVar23[4] * local_710
																																				+ local_700;
																																			local_278 = pfVar23[5] * local_704 + pfVar23[4] * local_70c
																																				+ fStack1788;
																																			local_274 = pfVar23[7] * local_708 + pfVar23[6] * local_710
																																				+ local_700;
																																			local_270 = pfVar23[7] * local_704 + pfVar23[6] * local_70c
																																				+ fStack1788;
																																		}
																																		if (uVar26 == 0) {
																																			uVar12 = p_Var25->field_0x8;
																																			local_264 = (((uVar12 >> 0x18) *
																																				(uint) * (byte*)&p_Var25->field_0xc >> 8) *
																																				local_268 >> 8) << 0x18;
																																			local_264 = local_264 |
																																				CONCAT12((char)((uVar12 >> 0x10 & 0xff) *
																																					(uint) * (byte*)&p_Var25->
																																					field_0x12 >> 8),
																																					CONCAT11((char)((uVar12 >> 8 & 0xff) *
																																						(uint)p_Var25->field_0xf >> 8),
																																						(char)((uVar12 & 0xff) *
																																							(uint) * (byte*)&p_Var25->field_0xe >> 8)));
																																		}
																																		else {
																																			uVar11 = (int)uVar12 >> 3 & 0xff;
																																			local_50 = (&pDrawData->field_0x9c)[uVar12 & 7];
																																			local_54 = (&pDrawData->field_0xa0)[uVar12 & 7];
																																			uVar22 = (p_Var25->field_0x8 & 0xff00ff) * (0xff - uVar26)
																																				+ ((local_50 & 0xff00ff) * (0xff - uVar11) +
																																					(local_54 & 0xff00ff) * uVar11 >> 8 & 0xff00ff
																																					) * uVar26;
																																			uVar12 = (p_Var25->field_0x8 >> 8 & 0xff00ff) *
																																				(0xff - uVar26) +
																																				(((local_50 >> 8 & 0xff00ff) * (0xff - uVar11) +
																																					(local_54 >> 8 & 0xff00ff) * uVar11 &
																																					0xff00ff00) >> 8) * uVar26;
																																			local_4c = CONCAT13((char)(((uVar12 >> 0x18) *
																																				(uint) * (byte*)&p_Var25->
																																				field_0xc >> 8) * local_268 >> 8),
																																				CONCAT12((char)((uVar22 >> 0x18) *
																																					(uint) * (byte*)&p_Var25->field_0x12 >> 8),
																																					CONCAT11((char)(((uVar12 & 0xff00ff00) >> 8 & 0xff)
																																						* (uint)p_Var25->field_0xf >> 8),
																																						(char)((uVar22 >> 8 & 0xff) *
																																							(uint) * (byte*)&p_Var25->field_0xe
																																							>> 8))));
																																			local_264 = local_4c;
																																		}
																																		uVar12 = local_264;
																																		puVar17 = &local_58;
																																		if (local_26b != '\0') {
																																			puVar13 = (uint*)0x4;
																																			puVar8 = puVar17;
																																			while (puVar8 != (uint*)0x0) {
																																				*(undefined*)puVar17 = 0;
																																				puVar17 = (uint*)((int)puVar17 + 1);
																																				puVar13 = (uint*)((int)puVar13 + -1);
																																				puVar8 = puVar13;
																																			}
																																			local_58 = local_264;
																																			uVar22 = local_264 & 0xff00ff;
																																			uVar11 = local_264 >> 8 & 0xff00ff;
																																			local_264 = local_264 & 0xff000000 |
																																				uVar22 * local_254 >> 8 & 0xff00ff |
																																				uVar11 * local_254 & 0xff00;
																																			local_260 = uVar12 & 0xff000000 |
																																				uVar22 * local_250 >> 8 & 0xff00ff |
																																				uVar11 * local_250 & 0xff00;
																																			local_25c = uVar12 & 0xff000000 |
																																				uVar22 * local_24c >> 8 & 0xff00ff |
																																				uVar11 * local_24c & 0xff00;
																																			local_258 = uVar12 & 0xff000000 |
																																				uVar22 * local_248 >> 8 & 0xff00ff |
																																				uVar11 * local_248 & 0xff00;
																																		}
																																		local_238 = DAT_00448598 *
																																			p_Var25->field_0x24 *
																																			(float)(uint)p_Var25->field_0x4 * 0.5;
																																		local_23c = local_238 * local_240;
																																		FUN_002d1520(local_23c, local_238, &local_28c, &local_284,
																																			&local_27c, &local_274, &local_264,
																																			(undefined8*)peVar24);
																																	}
																																	p_Var25 = p_Var25 + 1;
																																	peVar24 = peVar24 + 1;
																																	peVar9 = peVar9 + 2;
																																}
																															}
																															else {
																																if ((local_290 == '\0') &&
																																	((pDrawData->field_0x18d & 0xc) == 0)) {
																																	p_Var25 = pParticle + param_7;
																																	peVar24 = param_6 + param_7;
																																	for (; param_7 < param_8; param_7 = param_7 + 1) {
																																		if (p_Var25->field_0xd != 0) {
																																			if ((pDrawData->field_0x18d & 0xc) != 0) {
																																				iVar15 = table_sorted_indices$1918[param_7];
																																				p_Var25 = pParticle + iVar15;
																																				peVar24 = param_6 + iVar15;
																																				peVar9 = &pVector->field_0x0 + iVar15 * 2;
																																			}
																																			uVar12 = *(uint*)&p_Var25->field_0x3c;
																																			if ((uVar2 & 1) != 0) {
																																				fVar27 = p_Var25->field_0x1c / p_Var25->field_0x20
																																					;
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
																																						local_768 = (peVar9[1].x * local_230 +
																																							peVar9[1].y * local_22c +
																																							peVar9[1].z * local_228) *
																																							p_Var25->field_0x30;
																																						local_770 = (peVar9[1].x * local_220 +
																																							peVar9[1].y * local_21c +
																																							peVar9[1].z * local_218) *
																																							p_Var25->field_0x30;
																																						fVar30 = SQRT(local_768 * local_768 +
																																							local_770 * local_770);
																																						fVar29 = g_TinyFloat_00448548;
																																						if (fVar30 != 0.0) {
																																							fVar29 = fVar30;
																																						}
																																						local_768 = local_768 * (1.0 / fVar29);
																																						local_770 = local_770 * (1.0 / fVar29);
																																						local_76c = -local_768;
																																						local_760 = (local_770 + local_768) * -0.5 +
																																							0.5;
																																						fStack1884 = (local_770 - local_768) * -0.5 +
																																							0.5;
																																						if ((fVar28 != 0.0) ||
																																							(local_764 = local_770,
																																								pDrawData->field_0x94 != 0.0)) {
																																							fVar29 = pDrawData->field_0x94 * fVar27;
																																							if (0.0 < local_768) {
																																								fVar29 = -fVar29;
																																							}
																																							fVar30 = (fVar28 + fVar29) *
																																								g_ScalingFactor_00448518;
																																							puVar16 = (undefined*)0x3f000000;
																																							if (fVar30 < 0.0) {
																																								puVar16 = &DAT_bf000000;
																																							}
																																							fVar28 = (fVar28 + fVar29) *
																																								g_ScalingFactor_00448518;
																																							fVar29 = edFCosinus
																																								[(int)(fVar30 + (float)puVar16) &
																																								0x1fff];
																																							if (fVar28 < 0.0) {
																																								puVar16 = &DAT_bf000000;
																																							}
																																							else {
																																								puVar16 = (undefined*)0x3f000000;
																																							}
																																							fVar32 = edFSinus[(int)(fVar28 + (float)
																																								puVar16) & 0x1fff];
																																							fVar30 = -(fVar29 * 0.5) + fVar32 * 0.5 + 0.5;
																																							fVar31 = (-(fVar32 * 0.5) - fVar29 * 0.5) + 0.5;
																																							fVar33 = -fVar32 * local_770;
																																							fVar28 = fVar32 * local_770;
																																							local_764 = fVar29 * local_770 + -fVar32 * local_76c;
																																							local_760 = local_760 +
																																								fVar31 * local_768 + fVar30 * local_770;
																																							fStack1884 = fStack1884 +
																																								fVar31 * local_770 + fVar30 * local_76c;
																																							local_770 = fVar32 * local_768 + fVar29 * local_770;
																																							local_76c = fVar28 + fVar29 * local_76c;
																																							local_768 = fVar29 * local_768 + fVar33;
																																						}
																																					}
																																					else {
																																						fVar29 = pDrawData->field_0x94 * fVar27;
																																						if (((uVar2 & 8) != 0) && ((uVar12 & 0x20) != 0)) {
																																							fVar29 = -fVar29;
																																						}
																																						fVar30 = (fVar28 + fVar29) * g_ScalingFactor_00448518;
																																						puVar16 = (undefined*)0x3f000000;
																																						if (fVar30 < 0.0) {
																																							puVar16 = &DAT_bf000000;
																																						}
																																						fVar28 = (fVar28 + fVar29) * g_ScalingFactor_00448518;
																																						local_770 = edFCosinus
																																							[(int)(fVar30 + (float)puVar16) & 0x1fff];
																																						if (fVar28 < 0.0) {
																																							puVar16 = &DAT_bf000000;
																																						}
																																						else {
																																							puVar16 = (undefined*)0x3f000000;
																																						}
																																						local_76c = edFSinus[(int)(fVar28 + (float)puVar16) &
																																							0x1fff];
																																						local_760 = -(local_770 * 0.5) + local_76c * 0.5 + 0.5;
																																						fStack1884 = (-(local_76c * 0.5) - local_770 * 0.5) + 0.5;
																																						local_768 = -local_76c;
																																						local_764 = local_770;
																																					}
																																				}
																																				if ((uVar2 & 0x300) != 0) {
																																					DAT_0041ebd0 = pDrawData->field_0x88;
																																					DAT_0041ebdc = pDrawData->field_0x8c;
																																					DAT_0041ebe0 = DAT_0041ebd0 * -0.5 + 0.5;
																																					DAT_0041ebe4 = DAT_0041ebdc * -0.5 + 0.5;
																																					local_770 = local_76c * DAT_0041ebd8 +
																																						local_770 * DAT_0041ebd0;
																																					local_768 = local_764 * DAT_0041ebd8 +
																																						local_768 * DAT_0041ebd0;
																																					local_76c = local_76c * DAT_0041ebdc +
																																						local_770 * DAT_0041ebd4;
																																					local_764 = local_764 * DAT_0041ebdc +
																																						local_768 * DAT_0041ebd4;
																																					local_760 = DAT_0041ebe0 +
																																						fStack1884 * DAT_0041ebd8 +
																																						local_760 * DAT_0041ebd0;
																																					fStack1884 = DAT_0041ebe4 +
																																						fStack1884 * DAT_0041ebdc +
																																						local_760 * DAT_0041ebd4;
																																				}
																																				if (((uVar2 & 2) != 0) && ((uVar12 & 0x40) != 0)) {
																																					local_770 = local_76c * DAT_0041ebf8 +
																																						local_770 * DAT_0041ebf0;
																																					local_768 = local_764 * DAT_0041ebf8 +
																																						local_768 * DAT_0041ebf0;
																																					local_76c = local_76c * DAT_0041ebfc +
																																						local_770 * DAT_0041ebf4;
																																					local_764 = local_764 * DAT_0041ebfc +
																																						local_768 * DAT_0041ebf4;
																																					local_760 = DAT_0041ec00 +
																																						fStack1884 * DAT_0041ebf8 +
																																						local_760 * DAT_0041ebf0;
																																					fStack1884 = DAT_0041ec04 +
																																						fStack1884 * DAT_0041ebfc +
																																						local_760 * DAT_0041ebf4;
																																				}
																																				if (((uVar2 & 4) != 0) && ((uVar12 & 0x80) != 0)) {
																																					local_770 = local_76c * DAT_0041ec18 +
																																						local_770 * DAT_0041ec10;
																																					local_768 = local_764 * DAT_0041ec18 +
																																						local_768 * DAT_0041ec10;
																																					local_76c = local_76c * DAT_0041ec1c +
																																						local_770 * DAT_0041ec14;
																																					local_764 = local_764 * DAT_0041ec1c +
																																						local_768 * DAT_0041ec14;
																																					local_760 = DAT_0041ec20 +
																																						fStack1884 * DAT_0041ec18 +
																																						local_760 * DAT_0041ec10;
																																					fStack1884 = DAT_0041ec24 +
																																						fStack1884 * DAT_0041ec1c +
																																						local_760 * DAT_0041ec14;
																																				}
																																				if ((uVar2 & 0x40) != 0) {
																																					if ((uVar2 & 0x10) == 0) {
																																						iVar15 = 0;
																																					}
																																					else {
																																						iVar15 = (int)(((int)uVar12 >> 8 & 0xffffU) *
																																							(int)pDrawData->field_0x82) >> 0x10;
																																					}
																																					if ((uVar2 & 0x80) == 0) {
																																						iVar15 = iVar15 + (int)(fVar27 * (float)(int)pDrawData->
																																							field_0x82)
																																							;
																																						lVar14 = (long)iVar15;
																																						if (lVar14 < 0) {
																																							lVar14 = (long)-iVar15;
																																						}
																																						if (pDrawData->field_0x82 <= lVar14) {
																																							lVar14 = (long)(pDrawData->field_0x82 + -1);
																																						}
																																					}
																																					else {
																																						iVar15 = (iVar15 + (int)(p_Var25->field_0x1c *
																																							(float)*(int*)&pDrawData->
																																							field_0x98)) %
																																							(int)pDrawData->field_0x82;
																																						lVar14 = (long)iVar15;
																																						if (pDrawData->field_0x82 == 0) {
																																							trap(7);
																																						}
																																						if (lVar14 < 0) {
																																							lVar14 = (long)-iVar15;
																																						}
																																					}
																																					sVar3 = pDrawData->field_0x84;
																																					DAT_0041ec30 = 1.0 / (float)(int)sVar3;
																																					DAT_0041ec3c = 1.0 / (float)(int)pDrawData->field_0x86;
																																					if (sVar3 == 0) {
																																						trap(7);
																																					}
																																					DAT_0041ec40 = DAT_0041ec30 *
																																						(float)((int)lVar14 % (int)sVar3);
																																					if (pDrawData->field_0x84 == 0) {
																																						trap(7);
																																					}
																																					DAT_0041ec44 = DAT_0041ec3c *
																																						(float)((int)lVar14 /
																																							(int)pDrawData->field_0x84);
																																					local_770 = local_76c * DAT_0041ec38 +
																																						local_770 * DAT_0041ec30;
																																					local_768 = local_764 * DAT_0041ec38 +
																																						local_768 * DAT_0041ec30;
																																					local_76c = local_76c * DAT_0041ec3c +
																																						local_770 * DAT_0041ec34;
																																					local_764 = local_764 * DAT_0041ec3c +
																																						local_768 * DAT_0041ec34;
																																					local_760 = DAT_0041ec40 +
																																						fStack1884 * DAT_0041ec38 +
																																						local_760 * DAT_0041ec30;
																																					fStack1884 = DAT_0041ec44 +
																																						fStack1884 * DAT_0041ec3c +
																																						local_760 * DAT_0041ec34;
																																				}
																																				local_28c = pfVar23[1] * local_768 + *pfVar23 * local_770 +
																																					local_760;
																																				local_288 = pfVar23[1] * local_764 + *pfVar23 * local_76c +
																																					fStack1884;
																																				local_284 = pfVar23[3] * local_768 + pfVar23[2] * local_770
																																					+ local_760;
																																				local_280 = pfVar23[3] * local_764 + pfVar23[2] * local_76c
																																					+ fStack1884;
																																				local_27c = pfVar23[5] * local_768 + pfVar23[4] * local_770
																																					+ local_760;
																																				local_278 = pfVar23[5] * local_764 + pfVar23[4] * local_76c
																																					+ fStack1884;
																																				local_274 = pfVar23[7] * local_768 + pfVar23[6] * local_770
																																					+ local_760;
																																				local_270 = pfVar23[7] * local_764 + pfVar23[6] * local_76c
																																					+ fStack1884;
																																			}
																																			if (uVar26 == 0) {
																																				uVar12 = p_Var25->field_0x8;
																																				local_264 = ((uVar12 >> 0x18) *
																																					(uint) * (byte*)&p_Var25->field_0xc >> 8) <<
																																					0x18;
																																				local_264 = local_264 |
																																					CONCAT12((char)((uVar12 >> 0x10 & 0xff) *
																																						(uint) * (byte*)&p_Var25->
																																						field_0x12 >> 8),
																																						CONCAT11((char)((uVar12 >> 8 & 0xff) *
																																							(uint)p_Var25->field_0xf >> 8),
																																							(char)((uVar12 & 0xff) *
																																								(uint) * (byte*)&p_Var25->field_0xe >> 8)));
																																			}
																																			else {
																																				uVar11 = (int)uVar12 >> 3 & 0xff;
																																				local_60 = (&pDrawData->field_0x9c)[uVar12 & 7];
																																				local_64 = (&pDrawData->field_0xa0)[uVar12 & 7];
																																				uVar22 = (p_Var25->field_0x8 & 0xff00ff) * (0xff - uVar26)
																																					+ ((local_60 & 0xff00ff) * (0xff - uVar11) +
																																						(local_64 & 0xff00ff) * uVar11 >> 8 & 0xff00ff
																																						) * uVar26;
																																				uVar12 = (p_Var25->field_0x8 >> 8 & 0xff00ff) *
																																					(0xff - uVar26) +
																																					(((local_60 >> 8 & 0xff00ff) * (0xff - uVar11) +
																																						(local_64 >> 8 & 0xff00ff) * uVar11 &
																																						0xff00ff00) >> 8) * uVar26;
																																				local_5c = CONCAT13((char)((uVar12 >> 0x18) *
																																					(uint) * (byte*)&p_Var25->
																																					field_0xc >> 8),
																																					CONCAT12((char)((uVar22 >> 0x18) *
																																						(uint) * (byte*)&p_Var25->field_0x12 >> 8),
																																						CONCAT11((char)(((uVar12 & 0xff00ff00) >> 8 & 0xff)
																																							* (uint)p_Var25->field_0xf >> 8),
																																							(char)((uVar22 >> 8 & 0xff) *
																																								(uint) * (byte*)&p_Var25->field_0xe
																																								>> 8))));
																																				local_264 = local_5c;
																																			}
																																			uVar12 = local_264;
																																			puVar17 = &local_68;
																																			if (local_26b != '\0') {
																																				puVar13 = (uint*)0x4;
																																				puVar8 = puVar17;
																																				while (puVar8 != (uint*)0x0) {
																																					*(undefined*)puVar17 = 0;
																																					puVar17 = (uint*)((int)puVar17 + 1);
																																					puVar13 = (uint*)((int)puVar13 + -1);
																																					puVar8 = puVar13;
																																				}
																																				local_68 = local_264;
																																				uVar22 = local_264 & 0xff00ff;
																																				uVar11 = local_264 >> 8 & 0xff00ff;
																																				local_264 = local_264 & 0xff000000 |
																																					uVar22 * local_254 >> 8 & 0xff00ff |
																																					uVar11 * local_254 & 0xff00;
																																				local_260 = uVar12 & 0xff000000 |
																																					uVar22 * local_250 >> 8 & 0xff00ff |
																																					uVar11 * local_250 & 0xff00;
																																				local_25c = uVar12 & 0xff000000 |
																																					uVar22 * local_24c >> 8 & 0xff00ff |
																																					uVar11 * local_24c & 0xff00;
																																				local_258 = uVar12 & 0xff000000 |
																																					uVar22 * local_248 >> 8 & 0xff00ff |
																																					uVar11 * local_248 & 0xff00;
																																			}
																																			local_238 = DAT_00448598 *
																																				p_Var25->field_0x24 *
																																				(float)(uint)p_Var25->field_0x4 * 0.5;
																																			local_23c = local_238 * local_240;
																																			if (local_28f != '\0') {
																																				if (local_28e == '\0') {
																																					edDListTexCoo2f(0.0, 0.0);
																																					edDListTexCoo2f(1.0, 1.0);
																																				}
																																				else {
																																					edDListTexCoo2f(local_28c, local_288);
																																					edDListTexCoo2f(local_284, local_280);
																																					edDListTexCoo2f(local_27c, local_278);
																																					edDListTexCoo2f(local_274, local_270);
																																				}
																																			}
																																			if (local_26c != '\0') {
																																				edDListColor4u8((byte)local_264, local_264._1_1_,
																																					local_264._2_1_, local_264._3_1_);
																																				if (local_26b != '\0') {
																																					edDListColor4u8((byte)local_260, local_260._1_1_,
																																						local_260._2_1_, local_260._3_1_);
																																					edDListColor4u8((byte)local_25c, local_25c._1_1_,
																																						local_25c._2_1_, local_25c._3_1_);
																																					edDListColor4u8((byte)local_258, local_258._1_1_,
																																						local_258._2_1_, local_258._3_1_);
																																				}
																																			}
																																			if (local_244 != '\0') {
																																				edDListWidthHeight2f(local_23c, local_238);
																																			}
																																			edDListVertex4f(peVar24->x, peVar24->y, peVar24->z, 0.0);
																																		}
																																		p_Var25 = p_Var25 + 1;
																																		peVar24 = peVar24 + 1;
																																		peVar9 = peVar9 + 2;
																																	}
																																}
																																else {
																																	if ((pDrawData->field_0x18d & 0xc) == 0) {
																																		p_Var25 = pParticle + param_7;
																																		peVar24 = param_6 + param_7;
																																		local_8d0 = param_8;
																																	}
																																	else {
																																		param_7 = 0;
																																		local_8d0 = local_900;
																																	}
																																	for (; param_7 < local_8d0; param_7 = param_7 + 1) {
																																		if (p_Var25->field_0xd != 0) {
																																			if ((pDrawData->field_0x18d & 0xc) != 0) {
																																				iVar15 = table_sorted_indices$1918[param_7];
																																				p_Var25 = pParticle + iVar15;
																																				peVar24 = param_6 + iVar15;
																																				peVar9 = &pVector->field_0x0 + iVar15 * 2;
																																			}
																																			uVar12 = *(uint*)&p_Var25->field_0x3c;
																																			if ((uVar2 & 1) != 0) {
																																				fVar27 = p_Var25->field_0x1c / p_Var25->field_0x20;
																																				if ((uVar2 & 0x20) == 0) {
																																					local_7cc = 0.0;
																																					local_7d0 = 1.0;
																																					local_7c4 = 1.0;
																																					local_7c8 = 0.0;
																																					local_7c0 = 0.0;
																																					fStack1980 = 0.0;
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
																																						local_7c8 = (peVar9[1].x * local_230 +
																																							peVar9[1].y * local_22c +
																																							peVar9[1].z * local_228) *
																																							p_Var25->field_0x30;
																																						local_7d0 = (peVar9[1].x * local_220 +
																																							peVar9[1].y * local_21c +
																																							peVar9[1].z * local_218) *
																																							p_Var25->field_0x30;
																																						fVar30 = SQRT(local_7c8 * local_7c8 +
																																							local_7d0 * local_7d0);
																																						fVar29 = g_TinyFloat_00448548;
																																						if (fVar30 != 0.0) {
																																							fVar29 = fVar30;
																																						}
																																						local_7c8 = local_7c8 * (1.0 / fVar29);
																																						local_7d0 = local_7d0 * (1.0 / fVar29);
																																						local_7cc = -local_7c8;
																																						local_7c0 = (local_7d0 + local_7c8) * -0.5 + 0.5
																																							;
																																						fStack1980 = (local_7d0 - local_7c8) * -0.5 +
																																							0.5;
																																						if ((fVar28 != 0.0) ||
																																							(local_7c4 = local_7d0,
																																								pDrawData->field_0x94 != 0.0)) {
																																							fVar29 = pDrawData->field_0x94 * fVar27;
																																							if (0.0 < local_7c8) {
																																								fVar29 = -fVar29;
																																							}
																																							fVar30 = (fVar28 + fVar29) *
																																								g_ScalingFactor_00448518;
																																							puVar16 = (undefined*)0x3f000000;
																																							if (fVar30 < 0.0) {
																																								puVar16 = &DAT_bf000000;
																																							}
																																							fVar28 = (fVar28 + fVar29) *
																																								g_ScalingFactor_00448518;
																																							fVar29 = edFCosinus
																																								[(int)(fVar30 + (float)puVar16) &
																																								0x1fff];
																																							if (fVar28 < 0.0) {
																																								puVar16 = &DAT_bf000000;
																																							}
																																							else {
																																								puVar16 = (undefined*)0x3f000000;
																																							}
																																							fVar32 = edFSinus[(int)(fVar28 + (float)
																																								puVar16) & 0x1fff];
																																							fVar30 = -(fVar29 * 0.5) + fVar32 * 0.5 + 0.5;
																																							fVar31 = (-(fVar32 * 0.5) - fVar29 * 0.5) + 0.5;
																																							fVar33 = -fVar32 * local_7d0;
																																							fVar28 = fVar32 * local_7d0;
																																							local_7c4 = fVar29 * local_7d0 + -fVar32 * local_7cc;
																																							local_7c0 = local_7c0 +
																																								fVar31 * local_7c8 + fVar30 * local_7d0;
																																							fStack1980 = fStack1980 +
																																								fVar31 * local_7d0 + fVar30 * local_7cc;
																																							local_7d0 = fVar32 * local_7c8 + fVar29 * local_7d0;
																																							local_7cc = fVar28 + fVar29 * local_7cc;
																																							local_7c8 = fVar29 * local_7c8 + fVar33;
																																						}
																																					}
																																					else {
																																						fVar29 = pDrawData->field_0x94 * fVar27;
																																						if (((uVar2 & 8) != 0) && ((uVar12 & 0x20) != 0)) {
																																							fVar29 = -fVar29;
																																						}
																																						fVar30 = (fVar28 + fVar29) * g_ScalingFactor_00448518;
																																						puVar16 = (undefined*)0x3f000000;
																																						if (fVar30 < 0.0) {
																																							puVar16 = &DAT_bf000000;
																																						}
																																						fVar28 = (fVar28 + fVar29) * g_ScalingFactor_00448518;
																																						local_7d0 = edFCosinus
																																							[(int)(fVar30 + (float)puVar16) & 0x1fff];
																																						if (fVar28 < 0.0) {
																																							puVar16 = &DAT_bf000000;
																																						}
																																						else {
																																							puVar16 = (undefined*)0x3f000000;
																																						}
																																						local_7cc = edFSinus[(int)(fVar28 + (float)puVar16) &
																																							0x1fff];
																																						local_7c0 = -(local_7d0 * 0.5) + local_7cc * 0.5 + 0.5;
																																						fStack1980 = (-(local_7cc * 0.5) - local_7d0 * 0.5) + 0.5;
																																						local_7c8 = -local_7cc;
																																						local_7c4 = local_7d0;
																																					}
																																				}
																																				if ((uVar2 & 0x300) != 0) {
																																					DAT_0041ec50 = pDrawData->field_0x88;
																																					DAT_0041ec5c = pDrawData->field_0x8c;
																																					DAT_0041ec60 = DAT_0041ec50 * -0.5 + 0.5;
																																					DAT_0041ec64 = DAT_0041ec5c * -0.5 + 0.5;
																																					local_7d0 = local_7cc * DAT_0041ec58 +
																																						local_7d0 * DAT_0041ec50;
																																					local_7c8 = local_7c4 * DAT_0041ec58 +
																																						local_7c8 * DAT_0041ec50;
																																					local_7cc = local_7cc * DAT_0041ec5c +
																																						local_7d0 * DAT_0041ec54;
																																					local_7c4 = local_7c4 * DAT_0041ec5c +
																																						local_7c8 * DAT_0041ec54;
																																					local_7c0 = DAT_0041ec60 +
																																						fStack1980 * DAT_0041ec58 +
																																						local_7c0 * DAT_0041ec50;
																																					fStack1980 = DAT_0041ec64 +
																																						fStack1980 * DAT_0041ec5c +
																																						local_7c0 * DAT_0041ec54;
																																				}
																																				if (((uVar2 & 2) != 0) && ((uVar12 & 0x40) != 0)) {
																																					local_7d0 = local_7cc * DAT_0041ec78 +
																																						local_7d0 * DAT_0041ec70;
																																					local_7c8 = local_7c4 * DAT_0041ec78 +
																																						local_7c8 * DAT_0041ec70;
																																					local_7cc = local_7cc * DAT_0041ec7c +
																																						local_7d0 * DAT_0041ec74;
																																					local_7c4 = local_7c4 * DAT_0041ec7c +
																																						local_7c8 * DAT_0041ec74;
																																					local_7c0 = DAT_0041ec80 +
																																						fStack1980 * DAT_0041ec78 +
																																						local_7c0 * DAT_0041ec70;
																																					fStack1980 = DAT_0041ec84 +
																																						fStack1980 * DAT_0041ec7c +
																																						local_7c0 * DAT_0041ec74;
																																				}
																																				if (((uVar2 & 4) != 0) && ((uVar12 & 0x80) != 0)) {
																																					local_7d0 = local_7cc * DAT_0041ec98 +
																																						local_7d0 * DAT_0041ec90;
																																					local_7c8 = local_7c4 * DAT_0041ec98 +
																																						local_7c8 * DAT_0041ec90;
																																					local_7cc = local_7cc * DAT_0041ec9c +
																																						local_7d0 * DAT_0041ec94;
																																					local_7c4 = local_7c4 * DAT_0041ec9c +
																																						local_7c8 * DAT_0041ec94;
																																					local_7c0 = DAT_0041eca0 +
																																						fStack1980 * DAT_0041ec98 +
																																						local_7c0 * DAT_0041ec90;
																																					fStack1980 = DAT_0041eca4 +
																																						fStack1980 * DAT_0041ec9c +
																																						local_7c0 * DAT_0041ec94;
																																				}
																																				if ((uVar2 & 0x40) != 0) {
																																					if ((uVar2 & 0x10) == 0) {
																																						iVar15 = 0;
																																					}
																																					else {
																																						iVar15 = (int)(((int)uVar12 >> 8 & 0xffffU) *
																																							(int)pDrawData->field_0x82) >> 0x10;
																																					}
																																					if ((uVar2 & 0x80) == 0) {
																																						iVar15 = iVar15 + (int)(fVar27 * (float)(int)pDrawData->
																																							field_0x82)
																																							;
																																						lVar14 = (long)iVar15;
																																						if (lVar14 < 0) {
																																							lVar14 = (long)-iVar15;
																																						}
																																						if (pDrawData->field_0x82 <= lVar14) {
																																							lVar14 = (long)(pDrawData->field_0x82 + -1);
																																						}
																																					}
																																					else {
																																						iVar15 = (iVar15 + (int)(p_Var25->field_0x1c *
																																							(float)*(int*)&pDrawData->
																																							field_0x98)) %
																																							(int)pDrawData->field_0x82;
																																						lVar14 = (long)iVar15;
																																						if (pDrawData->field_0x82 == 0) {
																																							trap(7);
																																						}
																																						if (lVar14 < 0) {
																																							lVar14 = (long)-iVar15;
																																						}
																																					}
																																					sVar3 = pDrawData->field_0x84;
																																					DAT_0041ecb0 = 1.0 / (float)(int)sVar3;
																																					DAT_0041ecbc = 1.0 / (float)(int)pDrawData->field_0x86;
																																					if (sVar3 == 0) {
																																						trap(7);
																																					}
																																					DAT_0041ecc0 = DAT_0041ecb0 *
																																						(float)((int)lVar14 % (int)sVar3);
																																					if (pDrawData->field_0x84 == 0) {
																																						trap(7);
																																					}
																																					DAT_0041ecc4 = DAT_0041ecbc *
																																						(float)((int)lVar14 /
																																							(int)pDrawData->field_0x84);
																																					local_7d0 = local_7cc * DAT_0041ecb8 +
																																						local_7d0 * DAT_0041ecb0;
																																					local_7c8 = local_7c4 * DAT_0041ecb8 +
																																						local_7c8 * DAT_0041ecb0;
																																					local_7cc = local_7cc * DAT_0041ecbc +
																																						local_7d0 * DAT_0041ecb4;
																																					local_7c4 = local_7c4 * DAT_0041ecbc +
																																						local_7c8 * DAT_0041ecb4;
																																					local_7c0 = DAT_0041ecc0 +
																																						fStack1980 * DAT_0041ecb8 +
																																						local_7c0 * DAT_0041ecb0;
																																					fStack1980 = DAT_0041ecc4 +
																																						fStack1980 * DAT_0041ecbc +
																																						local_7c0 * DAT_0041ecb4;
																																				}
																																				local_28c = pfVar23[1] * local_7c8 + *pfVar23 * local_7d0 +
																																					local_7c0;
																																				local_288 = pfVar23[1] * local_7c4 + *pfVar23 * local_7cc +
																																					fStack1980;
																																				local_284 = pfVar23[3] * local_7c8 + pfVar23[2] * local_7d0
																																					+ local_7c0;
																																				local_280 = pfVar23[3] * local_7c4 + pfVar23[2] * local_7cc
																																					+ fStack1980;
																																				local_27c = pfVar23[5] * local_7c8 + pfVar23[4] * local_7d0
																																					+ local_7c0;
																																				local_278 = pfVar23[5] * local_7c4 + pfVar23[4] * local_7cc
																																					+ fStack1980;
																																				local_274 = pfVar23[7] * local_7c8 + pfVar23[6] * local_7d0
																																					+ local_7c0;
																																				local_270 = pfVar23[7] * local_7c4 + pfVar23[6] * local_7cc
																																					+ fStack1980;
																																			}
																																			if (uVar26 == 0) {
																																				uVar12 = p_Var25->field_0x8;
																																				local_264 = ((uVar12 >> 0x18) *
																																					(uint) * (byte*)&p_Var25->field_0xc >> 8) <<
																																					0x18;
																																				local_264 = local_264 |
																																					CONCAT12((char)((uVar12 >> 0x10 & 0xff) *
																																						(uint) * (byte*)&p_Var25->
																																						field_0x12 >> 8),
																																						CONCAT11((char)((uVar12 >> 8 & 0xff) *
																																							(uint)p_Var25->field_0xf >> 8),
																																							(char)((uVar12 & 0xff) *
																																								(uint) * (byte*)&p_Var25->field_0xe >> 8)));
																																			}
																																			else {
																																				uVar11 = (int)uVar12 >> 3 & 0xff;
																																				local_70 = (&pDrawData->field_0x9c)[uVar12 & 7];
																																				local_74 = (&pDrawData->field_0xa0)[uVar12 & 7];
																																				uVar22 = (p_Var25->field_0x8 & 0xff00ff) * (0xff - uVar26)
																																					+ ((local_70 & 0xff00ff) * (0xff - uVar11) +
																																						(local_74 & 0xff00ff) * uVar11 >> 8 & 0xff00ff
																																						) * uVar26;
																																				uVar12 = (p_Var25->field_0x8 >> 8 & 0xff00ff) *
																																					(0xff - uVar26) +
																																					(((local_70 >> 8 & 0xff00ff) * (0xff - uVar11) +
																																						(local_74 >> 8 & 0xff00ff) * uVar11 &
																																						0xff00ff00) >> 8) * uVar26;
																																				local_6c = CONCAT13((char)((uVar12 >> 0x18) *
																																					(uint) * (byte*)&p_Var25->
																																					field_0xc >> 8),
																																					CONCAT12((char)((uVar22 >> 0x18) *
																																						(uint) * (byte*)&p_Var25->field_0x12 >> 8),
																																						CONCAT11((char)(((uVar12 & 0xff00ff00) >> 8 & 0xff)
																																							* (uint)p_Var25->field_0xf >> 8),
																																							(char)((uVar22 >> 8 & 0xff) *
																																								(uint) * (byte*)&p_Var25->field_0xe
																																								>> 8))));
																																				local_264 = local_6c;
																																			}
																																			uVar12 = local_264;
																																			puVar17 = &local_78;
																																			if (local_26b != '\0') {
																																				puVar13 = (uint*)0x4;
																																				puVar8 = puVar17;
																																				while (puVar8 != (uint*)0x0) {
																																					*(undefined*)puVar17 = 0;
																																					puVar17 = (uint*)((int)puVar17 + 1);
																																					puVar13 = (uint*)((int)puVar13 + -1);
																																					puVar8 = puVar13;
																																				}
																																				local_78 = local_264;
																																				uVar22 = local_264 & 0xff00ff;
																																				uVar11 = local_264 >> 8 & 0xff00ff;
																																				local_264 = local_264 & 0xff000000 |
																																					uVar22 * local_254 >> 8 & 0xff00ff |
																																					uVar11 * local_254 & 0xff00;
																																				local_260 = uVar12 & 0xff000000 |
																																					uVar22 * local_250 >> 8 & 0xff00ff |
																																					uVar11 * local_250 & 0xff00;
																																				local_25c = uVar12 & 0xff000000 |
																																					uVar22 * local_24c >> 8 & 0xff00ff |
																																					uVar11 * local_24c & 0xff00;
																																				local_258 = uVar12 & 0xff000000 |
																																					uVar22 * local_248 >> 8 & 0xff00ff |
																																					uVar11 * local_248 & 0xff00;
																																			}
																																			fVar28 = DAT_00448598 *
																																				p_Var25->field_0x24 *
																																				(float)(uint)p_Var25->field_0x4 * 0.5;
																																			fVar27 = fVar28 * local_240;
																																			local_23c = fVar27;
																																			local_238 = fVar28;
																																			if (local_290 == '\0') {
																																				if (local_28f != '\0') {
																																					if (local_28e == '\0') {
																																						edDListTexCoo2f(0.0, 0.0);
																																						edDListTexCoo2f(1.0, 1.0);
																																					}
																																					else {
																																						edDListTexCoo2f(local_28c, local_288);
																																						edDListTexCoo2f(local_284, local_280);
																																						edDListTexCoo2f(local_27c, local_278);
																																						edDListTexCoo2f(local_274, local_270);
																																					}
																																				}
																																				if (local_26c != '\0') {
																																					edDListColor4u8((byte)local_264, local_264._1_1_,
																																						local_264._2_1_, local_264._3_1_);
																																					if (local_26b != '\0') {
																																						edDListColor4u8((byte)local_260, local_260._1_1_,
																																							local_260._2_1_, local_260._3_1_);
																																						edDListColor4u8((byte)local_25c, local_25c._1_1_,
																																							local_25c._2_1_, local_25c._3_1_);
																																						edDListColor4u8((byte)local_258, local_258._1_1_,
																																							local_258._2_1_, local_258._3_1_);
																																					}
																																				}
																																				if (local_244 != '\0') {
																																					edDListWidthHeight2f(local_23c, local_238);
																																				}
																																				edDListVertex4f(peVar24->x, peVar24->y, peVar24->z, 0.0);
																																			}
																																			else {
																																				edDListTexCoo2f(local_28c, local_288);
																																				edDListColor4u8((byte)local_264, local_264._1_1_,
																																					local_264._2_1_, local_264._3_1_);
																																				edDListVertex4f(peVar24->x - fVar27, peVar24->y,
																																					peVar24->z + fVar28, 0.0);
																																				edDListTexCoo2f(local_284, local_280);
																																				if (local_26b != '\0') {
																																					edDListColor4u8((byte)local_260, local_260._1_1_,
																																						local_260._2_1_, local_260._3_1_);
																																				}
																																				edDListVertex4f(peVar24->x + fVar27, peVar24->y,
																																					peVar24->z + fVar28, 0.0);
																																				edDListTexCoo2f(local_27c, local_278);
																																				if (local_26b != '\0') {
																																					edDListColor4u8((byte)local_25c, local_25c._1_1_,
																																						local_25c._2_1_, local_25c._3_1_);
																																				}
																																				edDListVertex4f(peVar24->x - fVar27, peVar24->y,
																																					peVar24->z - fVar28, 0.0);
																																				edDListTexCoo2f(local_274, local_270);
																																				if (local_26b != '\0') {
																																					edDListColor4u8((byte)local_258, local_258._1_1_,
																																						local_258._2_1_, local_258._3_1_);
																																				}
																																				edDListVertex4f(peVar24->x + fVar27, peVar24->y,
																																					peVar24->z - fVar28, 0.0);
																																			}
																																		}
																																		p_Var25 = p_Var25 + 1;
																																		peVar24 = peVar24 + 1;
																																		peVar9 = peVar9 + 2;
																																	}
																																}
																															}
																														}
																														else {
																															p_Var25 = pParticle + param_7;
																															peVar24 = param_6 + param_7;
																															for (; param_7 < param_8; param_7 = param_7 + 1) {
																																if (p_Var25->field_0xd != 0) {
																																	uVar12 = *(uint*)&p_Var25->field_0x3c;
																																	fVar27 = pDrawData->field_0x94 *
																																		(p_Var25->field_0x1c / p_Var25->field_0x20);
																																	if ((uVar12 & 0x20) != 0) {
																																		fVar27 = -fVar27;
																																	}
																																	fVar27 = pDrawData->field_0x90 + fVar27;
																																	fVar28 = fVar27 * g_ScalingFactor_00448518;
																																	puVar16 = (undefined*)0x3f000000;
																																	if (fVar28 < 0.0) {
																																		puVar16 = &DAT_bf000000;
																																	}
																																	fVar27 = fVar27 * g_ScalingFactor_00448518;
																																	fVar28 = edFCosinus
																																		[(int)(fVar28 + (float)puVar16) & 0x1fff];
																																	if (fVar27 < 0.0) {
																																		puVar16 = &DAT_bf000000;
																																	}
																																	else {
																																		puVar16 = (undefined*)0x3f000000;
																																	}
																																	fVar31 = edFSinus[(int)(fVar27 + (float)puVar16) &
																																		0x1fff];
																																	fVar29 = -fVar31;
																																	fVar27 = -(fVar28 * 0.5) + fVar31 * 0.5 + 0.5;
																																	fVar30 = (-(fVar31 * 0.5) - fVar28 * 0.5) + 0.5;
																																	local_28c = pfVar23[1] * fVar29 +
																																		*pfVar23 * fVar28 + fVar27 + 0.0;
																																	local_288 = pfVar23[1] * fVar28 +
																																		*pfVar23 * fVar31 + fVar30 + 0.0;
																																	local_284 = pfVar23[3] * fVar29 +
																																		pfVar23[2] * fVar28 + fVar27 + 0.0;
																																	local_280 = pfVar23[3] * fVar28 +
																																		pfVar23[2] * fVar31 + fVar30 + 0.0;
																																	local_27c = pfVar23[5] * fVar29 +
																																		pfVar23[4] * fVar28 + fVar27 + 0.0;
																																	local_278 = pfVar23[5] * fVar28 +
																																		pfVar23[4] * fVar31 + fVar30 + 0.0;
																																	local_274 = pfVar23[7] * fVar29 +
																																		pfVar23[6] * fVar28 + fVar27 + 0.0;
																																	local_270 = pfVar23[7] * fVar28 +
																																		pfVar23[6] * fVar31 + fVar30 + 0.0;
																																	uVar11 = (int)uVar12 >> 3;
																																	local_44 = (&pDrawData->field_0x9c)[uVar12 & 7];
																																	iVar15 = 0xff - (uVar11 & 0xff);
																																	local_48 = (&pDrawData->field_0xa0)[uVar12 & 7];
																																	uVar22 = (p_Var25->field_0x8 & 0xff00ff) *
																																		(0xff - uVar26) +
																																		((local_44 & 0xff00ff) * iVar15 +
																																			(local_48 & 0xff00ff) * (uVar11 & 0xff) >> 8
																																			& 0xff00ff) * uVar26;
																																	uVar12 = (p_Var25->field_0x8 >> 8 & 0xff00ff) *
																																		(0xff - uVar26) +
																																		(((local_44 >> 8 & 0xff00ff) * iVar15 +
																																			(local_48 >> 8 & 0xff00ff) *
																																			(uVar11 & 0xff) & 0xff00ff00) >> 8) *
																																		uVar26;
																																	local_40 = CONCAT13((char)(((uVar12 >> 0x18) *
																																		(uint) * (byte*)&p_Var25->
																																		field_0xc >> 8) * local_268 >> 8),
																																		CONCAT12((char)((uVar22 >> 0x18) *
																																			(uint) * (byte*)&p_Var25->field_0x12 >> 8),
																																			CONCAT11((char)(((uVar12 & 0xff00ff00) >> 8 & 0xff)
																																				* (uint)p_Var25->field_0xf >> 8),
																																				(char)((uVar22 >> 8 & 0xff) *
																																					(uint) * (byte*)&p_Var25->field_0xe
																																					>> 8))));
																																	local_264 = local_40;
																																	local_238 = DAT_00448598 *
																																		p_Var25->field_0x24 *
																																		(float)(uint)p_Var25->field_0x4 * 0.5;
																																	local_23c = local_238 * local_240;
																																	FUN_002d1520(local_23c, local_238, &local_28c, &local_284,
																																		&local_27c, &local_274, &local_264,
																																		(undefined8*)peVar24);
																																}
																																p_Var25 = p_Var25 + 1;
																																peVar24 = peVar24 + 1;
																															}
																														}
																													}
																													else {
																														p_Var25 = pParticle + param_7;
																														peVar24 = param_6 + param_7;
																														for (; param_7 < param_8; param_7 = param_7 + 1) {
																															if (p_Var25->field_0xd != 0) {
																																local_688 = (peVar9[1].x * local_230 +
																																	peVar9[1].y * local_22c +
																																	peVar9[1].z * local_228) *
																																	p_Var25->field_0x30;
																																local_690 = (peVar9[1].x * local_220 +
																																	peVar9[1].y * local_21c +
																																	peVar9[1].z * local_218) *
																																	p_Var25->field_0x30;
																																fVar28 = SQRT(local_688 * local_688 +
																																	local_690 * local_690);
																																fVar27 = g_TinyFloat_00448548;
																																if (fVar28 != 0.0) {
																																	fVar27 = fVar28;
																																}
																																local_688 = local_688 * (1.0 / fVar27);
																																local_690 = local_690 * (1.0 / fVar27);
																																local_68c = -local_688;
																																local_680 = (local_690 + local_688) * -0.5 + 0.5;
																																fStack1660 = (local_690 - local_688) * -0.5 + 0.5;
																																if ((pDrawData->field_0x90 != 0.0) ||
																																	(local_684 = local_690, pDrawData->field_0x94 != 0.0)
																																	) {
																																	fVar27 = pDrawData->field_0x94 *
																																		(p_Var25->field_0x1c / p_Var25->field_0x20);
																																	if (0.0 < local_688) {
																																		fVar27 = -fVar27;
																																	}
																																	fVar27 = pDrawData->field_0x90 + fVar27;
																																	fVar28 = fVar27 * g_ScalingFactor_00448518;
																																	puVar16 = (undefined*)0x3f000000;
																																	if (fVar28 < 0.0) {
																																		puVar16 = &DAT_bf000000;
																																	}
																																	fVar27 = fVar27 * g_ScalingFactor_00448518;
																																	fVar28 = edFCosinus
																																		[(int)(fVar28 + (float)puVar16) & 0x1fff];
																																	if (fVar27 < 0.0) {
																																		puVar16 = &DAT_bf000000;
																																	}
																																	else {
																																		puVar16 = (undefined*)0x3f000000;
																																	}
																																	fVar31 = edFSinus[(int)(fVar27 + (float)puVar16) &
																																		0x1fff];
																																	fVar29 = -(fVar28 * 0.5) + fVar31 * 0.5 + 0.5;
																																	fVar30 = (-(fVar31 * 0.5) - fVar28 * 0.5) + 0.5;
																																	fVar32 = -fVar31 * local_690;
																																	fVar27 = fVar31 * local_690;
																																	local_684 = fVar28 * local_690 + -fVar31 * local_68c;
																																	local_680 = fVar30 * local_688 +
																																		fVar29 * local_690 + local_680 + 0.0;
																																	fStack1660 = fVar30 * local_690 +
																																		fVar29 * local_68c + fStack1660 + 0.0;
																																	local_690 = fVar31 * local_688 + fVar28 * local_690;
																																	local_68c = fVar27 + fVar28 * local_68c;
																																	local_688 = fVar28 * local_688 + fVar32;
																																}
																																local_28c = pfVar23[1] * local_688 +
																																	*pfVar23 * local_690 + local_680 + 0.0;
																																local_288 = pfVar23[1] * local_684 +
																																	*pfVar23 * local_68c + fStack1660 + 0.0;
																																local_284 = pfVar23[3] * local_688 +
																																	pfVar23[2] * local_690 + local_680 + 0.0;
																																local_280 = pfVar23[3] * local_684 +
																																	pfVar23[2] * local_68c + fStack1660 + 0.0;
																																local_27c = pfVar23[5] * local_688 +
																																	pfVar23[4] * local_690 + local_680 + 0.0;
																																local_278 = pfVar23[5] * local_684 +
																																	pfVar23[4] * local_68c + fStack1660 + 0.0;
																																local_274 = pfVar23[7] * local_688 +
																																	pfVar23[6] * local_690 + local_680 + 0.0;
																																local_270 = pfVar23[7] * local_684 +
																																	pfVar23[6] * local_68c + fStack1660 + 0.0;
																																uVar26 = p_Var25->field_0x8;
																																local_264 = (((uVar26 >> 0x18) *
																																	(uint) * (byte*)&p_Var25->field_0xc >> 8) *
																																	local_268 >> 8) << 0x18;
																																local_264 = local_264 |
																																	CONCAT12((char)((uVar26 >> 0x10 & 0xff) *
																																		(uint) * (byte*)&p_Var25->
																																		field_0x12 >> 8),
																																		CONCAT11((char)((uVar26 >> 8 & 0xff) *
																																			(uint)p_Var25->field_0xf >> 8),
																																			(char)((uVar26 & 0xff) *
																																				(uint) * (byte*)&p_Var25->field_0xe >> 8)));
																																local_238 = DAT_00448598 *
																																	p_Var25->field_0x24 *
																																	(float)(uint)p_Var25->field_0x4 * 0.5;
																																local_23c = local_238 * local_240;
																																FUN_002d1520(local_23c, local_238, &local_28c, &local_284,
																																	&local_27c, &local_274, &local_264,
																																	(undefined8*)peVar24);
																															}
																															p_Var25 = p_Var25 + 1;
																															peVar24 = peVar24 + 1;
																															peVar9 = peVar9 + 2;
																														}
																													}
																												}
																												else {
																													p_Var25 = pParticle + param_7;
																													peVar24 = param_6 + param_7;
																													for (; param_7 < param_8; param_7 = param_7 + 1) {
																														if (p_Var25->field_0xd != 0) {
																															uVar26 = *(uint*)&p_Var25->field_0x3c;
																															fVar27 = pDrawData->field_0x94 *
																																(p_Var25->field_0x1c / p_Var25->field_0x20);
																															if ((uVar26 & 0x20) != 0) {
																																fVar27 = -fVar27;
																															}
																															fVar27 = pDrawData->field_0x90 + fVar27;
																															fVar28 = fVar27 * g_ScalingFactor_00448518;
																															puVar16 = (undefined*)0x3f000000;
																															if (fVar28 < 0.0) {
																																puVar16 = &DAT_bf000000;
																															}
																															fVar27 = fVar27 * g_ScalingFactor_00448518;
																															local_664 = edFCosinus
																																[(int)(fVar28 + (float)puVar16) & 0x1fff];
																															if (fVar27 < 0.0) {
																																puVar16 = &DAT_bf000000;
																															}
																															else {
																																puVar16 = (undefined*)0x3f000000;
																															}
																															local_66c = edFSinus[(int)(fVar27 + (float)puVar16) &
																																0x1fff];
																															fVar28 = -(local_664 * 0.5) + local_66c * 0.5 + 0.5;
																															local_65c = (-(local_66c * 0.5) - local_664 * 0.5) + 0.5;
																															fVar27 = -local_66c;
																															local_670 = local_664;
																															local_668 = fVar27;
																															local_660 = fVar28;
																															if ((uVar26 & 0x40) != 0) {
																																local_670 = local_66c * DAT_0041eaf8 +
																																	local_664 * DAT_0041eaf0;
																																local_668 = local_664 * DAT_0041eaf8 +
																																	fVar27 * DAT_0041eaf0;
																																local_66c = local_66c * DAT_0041eafc +
																																	local_664 * DAT_0041eaf4;
																																local_664 = local_664 * DAT_0041eafc +
																																	fVar27 * DAT_0041eaf4;
																																local_660 = local_65c * DAT_0041eaf8 +
																																	fVar28 * DAT_0041eaf0 + DAT_0041eb00 + 0.0;
																																local_65c = local_65c * DAT_0041eafc +
																																	fVar28 * DAT_0041eaf4 + DAT_0041eb04 + 0.0;
																															}
																															if ((uVar26 & 0x80) != 0) {
																																fVar27 = local_66c * DAT_0041eb18;
																																fVar28 = local_664 * DAT_0041eb18;
																																local_66c = local_66c * DAT_0041eb1c +
																																	local_670 * DAT_0041eb14;
																																local_664 = local_664 * DAT_0041eb1c +
																																	local_668 * DAT_0041eb14;
																																fVar29 = local_65c * DAT_0041eb18;
																																local_65c = local_65c * DAT_0041eb1c +
																																	local_660 * DAT_0041eb14 +
																																	DAT_0041eb24 + 0.0;
																																local_670 = fVar27 + local_670 * DAT_0041eb10;
																																local_668 = fVar28 + local_668 * DAT_0041eb10;
																																local_660 = fVar29 + local_660 * DAT_0041eb10 +
																																	DAT_0041eb20 + 0.0;
																															}
																															sVar3 = pDrawData->field_0x82;
																															iVar15 = (((int)(((int)uVar26 >> 8 & 0xffffU) * (int)sVar3
																																) >> 0x10) +
																																(int)(p_Var25->field_0x1c *
																																	(float)*(int*)&pDrawData->field_0x98)) %
																																(int)sVar3;
																															if (sVar3 == 0) {
																																trap(7);
																															}
																															if (iVar15 < 0) {
																																iVar15 = -iVar15;
																															}
																															sVar3 = pDrawData->field_0x84;
																															DAT_0041eb30 = 1.0 / (float)(int)sVar3;
																															DAT_0041eb3c = 1.0 / (float)(int)pDrawData->field_0x86;
																															if (sVar3 == 0) {
																																trap(7);
																															}
																															DAT_0041eb40 = DAT_0041eb30 * (float)(iVar15 % (int)sVar3)
																																;
																															if (pDrawData->field_0x84 == 0) {
																																trap(7);
																															}
																															DAT_0041eb44 = DAT_0041eb3c *
																																(float)(iVar15 / (int)pDrawData->field_0x84
																																	);
																															fVar27 = local_66c * DAT_0041eb38 +
																																local_670 * DAT_0041eb30;
																															fVar28 = local_664 * DAT_0041eb38 +
																																local_668 * DAT_0041eb30;
																															fVar29 = local_66c * DAT_0041eb3c +
																																local_670 * DAT_0041eb34;
																															fVar31 = local_664 * DAT_0041eb3c +
																																local_668 * DAT_0041eb34;
																															fVar30 = local_65c * DAT_0041eb38 +
																																local_660 * DAT_0041eb30 + DAT_0041eb40 + 0.0;
																															fVar32 = local_65c * DAT_0041eb3c +
																																local_660 * DAT_0041eb34 + DAT_0041eb44 + 0.0;
																															local_28c = pfVar23[1] * fVar28 +
																																*pfVar23 * fVar27 + fVar30 + 0.0;
																															local_288 = pfVar23[1] * fVar31 +
																																*pfVar23 * fVar29 + fVar32 + 0.0;
																															local_284 = pfVar23[3] * fVar28 +
																																pfVar23[2] * fVar27 + fVar30 + 0.0;
																															local_280 = pfVar23[3] * fVar31 +
																																pfVar23[2] * fVar29 + fVar32 + 0.0;
																															local_27c = pfVar23[5] * fVar28 +
																																pfVar23[4] * fVar27 + fVar30 + 0.0;
																															local_278 = pfVar23[5] * fVar31 +
																																pfVar23[4] * fVar29 + fVar32 + 0.0;
																															local_274 = pfVar23[7] * fVar28 +
																																pfVar23[6] * fVar27 + fVar30 + 0.0;
																															local_270 = pfVar23[7] * fVar31 +
																																pfVar23[6] * fVar29 + fVar32 + 0.0;
																															uVar26 = p_Var25->field_0x8;
																															local_264 = (((uVar26 >> 0x18) *
																																(uint) * (byte*)&p_Var25->field_0xc >> 8) *
																																local_268 >> 8) << 0x18;
																															local_264 = local_264 |
																																CONCAT12((char)((uVar26 >> 0x10 & 0xff) *
																																	(uint) * (byte*)&p_Var25->
																																	field_0x12 >> 8),
																																	CONCAT11((char)((uVar26 >> 8 & 0xff) *
																																		(uint)p_Var25->field_0xf >> 8),
																																		(char)((uVar26 & 0xff) *
																																			(uint) * (byte*)&p_Var25->field_0xe >> 8)));
																															local_238 = DAT_00448598 *
																																p_Var25->field_0x24 *
																																(float)(uint)p_Var25->field_0x4 * 0.5;
																															local_23c = local_238 * local_240;
																															FUN_002d1520(local_23c, local_238, &local_28c, &local_284,
																																&local_27c, &local_274, &local_264,
																																(undefined8*)peVar24);
																														}
																														p_Var25 = p_Var25 + 1;
																														peVar24 = peVar24 + 1;
																													}
																												}
																											}
																											else {
																												p_Var25 = pParticle + param_7;
																												peVar24 = param_6 + param_7;
																												for (; param_7 < param_8; param_7 = param_7 + 1) {
																													if (p_Var25->field_0xd != 0) {
																														uVar12 = *(uint*)&p_Var25->field_0x3c;
																														fVar27 = pDrawData->field_0x94 *
																															(p_Var25->field_0x1c / p_Var25->field_0x20);
																														if ((uVar12 & 0x20) != 0) {
																															fVar27 = -fVar27;
																														}
																														fVar27 = pDrawData->field_0x90 + fVar27;
																														fVar28 = fVar27 * g_ScalingFactor_00448518;
																														puVar16 = (undefined*)0x3f000000;
																														if (fVar28 < 0.0) {
																															puVar16 = &DAT_bf000000;
																														}
																														fVar27 = fVar27 * g_ScalingFactor_00448518;
																														fVar28 = edFCosinus[(int)(fVar28 + (float)puVar16) & 0x1fff]
																															;
																														if (fVar27 < 0.0) {
																															puVar16 = &DAT_bf000000;
																														}
																														else {
																															puVar16 = (undefined*)0x3f000000;
																														}
																														fVar31 = edFSinus[(int)(fVar27 + (float)puVar16) & 0x1fff];
																														fVar29 = -fVar31;
																														fVar27 = -(fVar28 * 0.5) + fVar31 * 0.5 + 0.5;
																														fVar30 = (-(fVar31 * 0.5) - fVar28 * 0.5) + 0.5;
																														local_28c = pfVar23[1] * fVar29 +
																															*pfVar23 * fVar28 + fVar27 + 0.0;
																														local_288 = pfVar23[1] * fVar28 +
																															*pfVar23 * fVar31 + fVar30 + 0.0;
																														local_284 = pfVar23[3] * fVar29 +
																															pfVar23[2] * fVar28 + fVar27 + 0.0;
																														local_280 = pfVar23[3] * fVar28 +
																															pfVar23[2] * fVar31 + fVar30 + 0.0;
																														local_27c = pfVar23[5] * fVar29 +
																															pfVar23[4] * fVar28 + fVar27 + 0.0;
																														local_278 = pfVar23[5] * fVar28 +
																															pfVar23[4] * fVar31 + fVar30 + 0.0;
																														local_274 = pfVar23[7] * fVar29 +
																															pfVar23[6] * fVar28 + fVar27 + 0.0;
																														local_270 = pfVar23[7] * fVar28 +
																															pfVar23[6] * fVar31 + fVar30 + 0.0;
																														uVar11 = (int)uVar12 >> 3;
																														local_38 = (&pDrawData->field_0x9c)[uVar12 & 7];
																														iVar15 = 0xff - (uVar11 & 0xff);
																														local_3c = (&pDrawData->field_0xa0)[uVar12 & 7];
																														uVar22 = (p_Var25->field_0x8 & 0xff00ff) * (0xff - uVar26) +
																															((local_38 & 0xff00ff) * iVar15 +
																																(local_3c & 0xff00ff) * (uVar11 & 0xff) >> 8 &
																																0xff00ff) * uVar26;
																														uVar12 = (p_Var25->field_0x8 >> 8 & 0xff00ff) *
																															(0xff - uVar26) +
																															(((local_38 >> 8 & 0xff00ff) * iVar15 +
																																(local_3c >> 8 & 0xff00ff) * (uVar11 & 0xff) &
																																0xff00ff00) >> 8) * uVar26;
																														local_34 = CONCAT13((char)(((uVar12 >> 0x18) *
																															(uint) * (byte*)&p_Var25->
																															field_0xc >> 8) * local_268 >> 8),
																															CONCAT12((char)((uVar22 >> 0x18) *
																																(uint) * (byte*)&p_Var25->field_0x12 >> 8),
																																CONCAT11((char)(((uVar12 & 0xff00ff00) >> 8 & 0xff)
																																	* (uint)p_Var25->field_0xf >> 8),
																																	(char)((uVar22 >> 8 & 0xff) *
																																		(uint) * (byte*)&p_Var25->field_0xe
																																		>> 8))));
																														local_264 = local_34;
																														local_238 = DAT_00448598 *
																															p_Var25->field_0x24 *
																															(float)(uint)p_Var25->field_0x4 * 0.5;
																														local_23c = local_238 * local_240;
																														FUN_002d1520(local_23c, local_238, &local_28c, &local_284,
																															&local_27c, &local_274, &local_264,
																															(undefined8*)peVar24);
																													}
																													p_Var25 = p_Var25 + 1;
																													peVar24 = peVar24 + 1;
																												}
																											}
																										}
																										else {
																											p_Var25 = pParticle + param_7;
																											peVar24 = param_6 + param_7;
																											for (; param_7 < param_8; param_7 = param_7 + 1) {
																												if (p_Var25->field_0xd != 0) {
																													fVar28 = pDrawData->field_0x90 +
																														pDrawData->field_0x94 *
																														(p_Var25->field_0x1c / p_Var25->field_0x20);
																													fVar27 = fVar28 * g_ScalingFactor_00448518;
																													if (fVar27 < 0.0) {
																														puVar16 = &DAT_bf000000;
																													}
																													else {
																														puVar16 = (undefined*)0x3f000000;
																													}
																													fVar28 = fVar28 * g_ScalingFactor_00448518;
																													local_624 = edFCosinus
																														[(int)(fVar27 + (float)puVar16) & 0x1fff];
																													if (fVar28 < 0.0) {
																														puVar16 = &DAT_bf000000;
																													}
																													else {
																														puVar16 = (undefined*)0x3f000000;
																													}
																													local_62c = edFSinus[(int)(fVar28 + (float)puVar16) & 0x1fff];
																													fVar28 = -(local_624 * 0.5) + local_62c * 0.5 + 0.5;
																													local_61c = (-(local_62c * 0.5) - local_624 * 0.5) + 0.5;
																													fVar27 = -local_62c;
																													local_630 = local_624;
																													local_628 = fVar27;
																													local_620 = fVar28;
																													if ((*(uint*)&p_Var25->field_0x3c & 0x40) != 0) {
																														local_630 = local_62c * DAT_0041ead8 +
																															local_624 * DAT_0041ead0;
																														local_628 = local_624 * DAT_0041ead8 + fVar27 * DAT_0041ead0
																															;
																														local_62c = local_62c * DAT_0041eadc +
																															local_624 * DAT_0041ead4;
																														local_624 = local_624 * DAT_0041eadc + fVar27 * DAT_0041ead4
																															;
																														local_620 = local_61c * DAT_0041ead8 +
																															fVar28 * DAT_0041ead0 + DAT_0041eae0 + 0.0;
																														local_61c = local_61c * DAT_0041eadc +
																															fVar28 * DAT_0041ead4 + DAT_0041eae4 + 0.0;
																													}
																													local_28c = pfVar23[1] * local_628 +
																														*pfVar23 * local_630 + local_620 + 0.0;
																													local_288 = pfVar23[1] * local_624 +
																														*pfVar23 * local_62c + local_61c + 0.0;
																													local_284 = pfVar23[3] * local_628 +
																														pfVar23[2] * local_630 + local_620 + 0.0;
																													local_280 = pfVar23[3] * local_624 +
																														pfVar23[2] * local_62c + local_61c + 0.0;
																													local_27c = pfVar23[5] * local_628 +
																														pfVar23[4] * local_630 + local_620 + 0.0;
																													local_278 = pfVar23[5] * local_624 +
																														pfVar23[4] * local_62c + local_61c + 0.0;
																													local_274 = pfVar23[7] * local_628 +
																														pfVar23[6] * local_630 + local_620 + 0.0;
																													local_270 = pfVar23[7] * local_624 +
																														pfVar23[6] * local_62c + local_61c + 0.0;
																													uVar26 = p_Var25->field_0x8;
																													local_264 = (((uVar26 >> 0x18) *
																														(uint) * (byte*)&p_Var25->field_0xc >> 8) *
																														local_268 >> 8) << 0x18;
																													local_264 = local_264 |
																														CONCAT12((char)((uVar26 >> 0x10 & 0xff) *
																															(uint) * (byte*)&p_Var25->
																															field_0x12 >> 8),
																															CONCAT11((char)((uVar26 >> 8 & 0xff) *
																																(uint)p_Var25->field_0xf >> 8),
																																(char)((uVar26 & 0xff) *
																																	(uint) * (byte*)&p_Var25->field_0xe >> 8)));
																													local_238 = DAT_00448598 *
																														p_Var25->field_0x24 *
																														(float)(uint)p_Var25->field_0x4 * 0.5;
																													local_23c = local_238 * local_240;
																													FUN_002d1520(local_23c, local_238, &local_28c, &local_284,
																														&local_27c, &local_274, &local_264,
																														(undefined8*)peVar24);
																												}
																												p_Var25 = p_Var25 + 1;
																												peVar24 = peVar24 + 1;
																											}
																										}
																									}
																									else {
																										p_Var25 = pParticle + param_7;
																										peVar24 = param_6 + param_7;
																										for (; param_7 < param_8; param_7 = param_7 + 1) {
																											if (p_Var25->field_0xd != 0) {
																												local_610 = 1.0;
																												local_604 = 1.0;
																												local_60c = 0.0;
																												local_608 = 0.0;
																												local_600 = 0.0;
																												local_5fc = 0.0;
																												if ((*(uint*)&p_Var25->field_0x3c & 0x40) != 0) {
																													local_610 = DAT_0041ea78 * 0.0 + DAT_0041ea70 * 1.0;
																													local_608 = DAT_0041ea78 * 1.0 + DAT_0041ea70 * 0.0;
																													local_60c = DAT_0041ea7c * 0.0 + DAT_0041ea74 * 1.0;
																													local_604 = DAT_0041ea7c * 1.0 + DAT_0041ea74 * 0.0;
																													local_600 = DAT_0041ea78 * 0.0 +
																														DAT_0041ea70 * 0.0 + DAT_0041ea80 + 0.0;
																													local_5fc = DAT_0041ea7c * 0.0 +
																														DAT_0041ea74 * 0.0 + DAT_0041ea84 + 0.0;
																												}
																												if ((*(uint*)&p_Var25->field_0x3c & 0x80) != 0) {
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
																												iVar15 = (int)((p_Var25->field_0x1c / p_Var25->field_0x20) *
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
																												local_28c = pfVar23[1] * fVar28 +
																													*pfVar23 * fVar27 + fVar30 + 0.0;
																												local_288 = pfVar23[1] * fVar31 +
																													*pfVar23 * fVar29 + fVar32 + 0.0;
																												local_284 = pfVar23[3] * fVar28 +
																													pfVar23[2] * fVar27 + fVar30 + 0.0;
																												local_280 = pfVar23[3] * fVar31 +
																													pfVar23[2] * fVar29 + fVar32 + 0.0;
																												local_27c = pfVar23[5] * fVar28 +
																													pfVar23[4] * fVar27 + fVar30 + 0.0;
																												local_278 = pfVar23[5] * fVar31 +
																													pfVar23[4] * fVar29 + fVar32 + 0.0;
																												local_274 = pfVar23[7] * fVar28 +
																													pfVar23[6] * fVar27 + fVar30 + 0.0;
																												local_270 = pfVar23[7] * fVar31 +
																													pfVar23[6] * fVar29 + fVar32 + 0.0;
																												uVar26 = p_Var25->field_0x8;
																												local_264 = (((uVar26 >> 0x18) *
																													(uint) * (byte*)&p_Var25->field_0xc >> 8) *
																													local_268 >> 8) << 0x18;
																												local_264 = local_264 |
																													CONCAT12((char)((uVar26 >> 0x10 & 0xff) *
																														(uint) * (byte*)&p_Var25->field_0x12
																														>> 8),
																														CONCAT11((char)((uVar26 >> 8 & 0xff) *
																															(uint)p_Var25->field_0xf >>
																															8), (char)((uVar26 & 0xff) *
																																(uint) * (byte*)&
																																p_Var25->field_0xe >> 8)));
																												local_238 = DAT_00448598 *
																													p_Var25->field_0x24 *
																													(float)(uint)p_Var25->field_0x4 * 0.5;
																												local_23c = local_238 * local_240;
																												FUN_002d1520(local_23c, local_238, &local_28c, &local_284,
																													&local_27c, &local_274, &local_264,
																													(undefined8*)peVar24);
																											}
																											p_Var25 = p_Var25 + 1;
																											peVar24 = peVar24 + 1;
																										}
																									}
																								}
																								else {
																									p_Var25 = pParticle + param_7;
																									peVar24 = param_6 + param_7;
																									for (; param_7 < param_8; param_7 = param_7 + 1) {
																										if (p_Var25->field_0xd != 0) {
																											uVar26 = *(uint*)&p_Var25->field_0x3c;
																											fVar27 = pDrawData->field_0x94 *
																												(p_Var25->field_0x1c / p_Var25->field_0x20);
																											if ((uVar26 & 0x20) != 0) {
																												fVar27 = -fVar27;
																											}
																											fVar27 = ((float)(uVar26 & 0xffff) * pDrawData->field_0x90) /
																												65535.0 + fVar27;
																											fVar28 = fVar27 * g_ScalingFactor_00448518;
																											puVar16 = (undefined*)0x3f000000;
																											if (fVar28 < 0.0) {
																												puVar16 = &DAT_bf000000;
																											}
																											fVar27 = fVar27 * g_ScalingFactor_00448518;
																											local_5e4 = edFCosinus[(int)(fVar28 + (float)puVar16) & 0x1fff];
																											if (fVar27 < 0.0) {
																												puVar16 = &DAT_bf000000;
																											}
																											else {
																												puVar16 = (undefined*)0x3f000000;
																											}
																											local_5ec = edFSinus[(int)(fVar27 + (float)puVar16) & 0x1fff];
																											fVar28 = -(local_5e4 * 0.5) + local_5ec * 0.5 + 0.5;
																											local_5dc = (-(local_5ec * 0.5) - local_5e4 * 0.5) + 0.5;
																											fVar27 = -local_5ec;
																											local_5f0 = local_5e4;
																											local_5e8 = fVar27;
																											local_5e0 = fVar28;
																											if ((uVar26 & 0x80) != 0) {
																												local_5f0 = local_5ec * DAT_0041ea58 + local_5e4 * DAT_0041ea50;
																												local_5e8 = local_5e4 * DAT_0041ea58 + fVar27 * DAT_0041ea50;
																												local_5ec = local_5ec * DAT_0041ea5c + local_5e4 * DAT_0041ea54;
																												local_5e4 = local_5e4 * DAT_0041ea5c + fVar27 * DAT_0041ea54;
																												local_5e0 = local_5dc * DAT_0041ea58 +
																													fVar28 * DAT_0041ea50 + DAT_0041ea60 + 0.0;
																												local_5dc = local_5dc * DAT_0041ea5c +
																													fVar28 * DAT_0041ea54 + DAT_0041ea64 + 0.0;
																											}
																											local_28c = pfVar23[1] * local_5e8 +
																												*pfVar23 * local_5f0 + local_5e0 + 0.0;
																											local_288 = pfVar23[1] * local_5e4 +
																												*pfVar23 * local_5ec + local_5dc + 0.0;
																											local_284 = pfVar23[3] * local_5e8 +
																												pfVar23[2] * local_5f0 + local_5e0 + 0.0;
																											local_280 = pfVar23[3] * local_5e4 +
																												pfVar23[2] * local_5ec + local_5dc + 0.0;
																											local_27c = pfVar23[5] * local_5e8 +
																												pfVar23[4] * local_5f0 + local_5e0 + 0.0;
																											local_278 = pfVar23[5] * local_5e4 +
																												pfVar23[4] * local_5ec + local_5dc + 0.0;
																											local_274 = pfVar23[7] * local_5e8 +
																												pfVar23[6] * local_5f0 + local_5e0 + 0.0;
																											local_270 = pfVar23[7] * local_5e4 +
																												pfVar23[6] * local_5ec + local_5dc + 0.0;
																											uVar26 = p_Var25->field_0x8;
																											local_264 = (((uVar26 >> 0x18) *
																												(uint) * (byte*)&p_Var25->field_0xc >> 8) * local_268
																												>> 8) << 0x18;
																											local_264 = local_264 |
																												CONCAT12((char)((uVar26 >> 0x10 & 0xff) *
																													(uint) * (byte*)&p_Var25->field_0x12 >>
																													8), CONCAT11((char)((uVar26 >> 8 & 0xff)
																														* (uint)p_Var25->
																														field_0xf >> 8),
																														(char)((uVar26 & 0xff) * (uint) * (byte*)&p_Var25->field_0xe
																															>> 8)));
																											local_238 = DAT_00448598 *
																												p_Var25->field_0x24 * (float)(uint)p_Var25->field_0x4
																												* 0.5;
																											local_23c = local_238 * local_240;
																											FUN_002d1520(local_23c, local_238, &local_28c, &local_284, &local_27c,
																												&local_274, &local_264, (undefined8*)peVar24);
																										}
																										p_Var25 = p_Var25 + 1;
																										peVar24 = peVar24 + 1;
																									}
																								}
																							}
																							else {
																								p_Var25 = pParticle + param_7;
																								peVar24 = param_6 + param_7;
																								for (; param_7 < param_8; param_7 = param_7 + 1) {
																									if (p_Var25->field_0xd != 0) {
																										uVar26 = *(uint*)&p_Var25->field_0x3c;
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
																											+ (int)(p_Var25->field_0x1c *
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
																										local_28c = pfVar23[1] * fVar28 + *pfVar23 * fVar27 + fVar30 + 0.0;
																										local_288 = pfVar23[1] * fVar31 + *pfVar23 * fVar29 + fVar32 + 0.0;
																										local_284 = pfVar23[3] * fVar28 + pfVar23[2] * fVar27 + fVar30 + 0.0
																											;
																										local_280 = pfVar23[3] * fVar31 + pfVar23[2] * fVar29 + fVar32 + 0.0
																											;
																										local_27c = pfVar23[5] * fVar28 + pfVar23[4] * fVar27 + fVar30 + 0.0
																											;
																										local_278 = pfVar23[5] * fVar31 + pfVar23[4] * fVar29 + fVar32 + 0.0
																											;
																										local_274 = pfVar23[7] * fVar28 + pfVar23[6] * fVar27 + fVar30 + 0.0
																											;
																										local_270 = pfVar23[7] * fVar31 + pfVar23[6] * fVar29 + fVar32 + 0.0
																											;
																										uVar26 = p_Var25->field_0x8;
																										local_264 = (((uVar26 >> 0x18) * (uint) * (byte*)&p_Var25->field_0xc
																											>> 8) * local_268 >> 8) << 0x18;
																										local_264 = local_264 |
																											CONCAT12((char)((uVar26 >> 0x10 & 0xff) *
																												(uint) * (byte*)&p_Var25->field_0x12 >> 8
																												), CONCAT11((char)((uVar26 >> 8 & 0xff) *
																													(uint)p_Var25->
																													field_0xf >> 8),
																													(char)((uVar26 & 0xff) * (uint) * (byte*)&p_Var25->field_0xe
																														>> 8)));
																										local_238 = DAT_00448598 *
																											p_Var25->field_0x24 * (float)(uint)p_Var25->field_0x4 *
																											0.5;
																										local_23c = local_238 * local_240;
																										FUN_002d1520(local_23c, local_238, &local_28c, &local_284, &local_27c,
																											&local_274, &local_264, (undefined8*)peVar24);
																									}
																									p_Var25 = p_Var25 + 1;
																									peVar24 = peVar24 + 1;
																								}
																							}
																						}
																						else {
																							p_Var25 = pParticle + param_7;
																							peVar24 = param_6 + param_7;
																							for (; param_7 < param_8; param_7 = param_7 + 1) {
																								if (p_Var25->field_0xd != 0) {
																									local_568 = (peVar9[1].x * local_230 + peVar9[1].y * local_22c +
																										peVar9[1].z * local_228) * p_Var25->field_0x30;
																									local_570 = (peVar9[1].x * local_220 + peVar9[1].y * local_21c +
																										peVar9[1].z * local_218) * p_Var25->field_0x30;
																									fVar28 = SQRT(local_568 * local_568 + local_570 * local_570);
																									fVar27 = g_TinyFloat_00448548;
																									if (fVar28 != 0.0) {
																										fVar27 = fVar28;
																									}
																									local_568 = local_568 * (1.0 / fVar27);
																									local_570 = local_570 * (1.0 / fVar27);
																									local_56c = -local_568;
																									local_560 = (local_570 + local_568) * -0.5 + 0.5;
																									fStack1372 = (local_570 - local_568) * -0.5 + 0.5;
																									if ((pDrawData->field_0x90 != 0.0) ||
																										(local_564 = local_570, pDrawData->field_0x94 != 0.0)) {
																										fVar27 = pDrawData->field_0x94 *
																											(p_Var25->field_0x1c / p_Var25->field_0x20);
																										if (0.0 < local_568) {
																											fVar27 = -fVar27;
																										}
																										fVar27 = pDrawData->field_0x90 + fVar27;
																										fVar28 = fVar27 * g_ScalingFactor_00448518;
																										puVar16 = (undefined*)0x3f000000;
																										if (fVar28 < 0.0) {
																											puVar16 = &DAT_bf000000;
																										}
																										fVar27 = fVar27 * g_ScalingFactor_00448518;
																										fVar28 = edFCosinus[(int)(fVar28 + (float)puVar16) & 0x1fff];
																										if (fVar27 < 0.0) {
																											puVar16 = &DAT_bf000000;
																										}
																										else {
																											puVar16 = (undefined*)0x3f000000;
																										}
																										fVar31 = edFSinus[(int)(fVar27 + (float)puVar16) & 0x1fff];
																										fVar29 = -(fVar28 * 0.5) + fVar31 * 0.5 + 0.5;
																										fVar30 = (-(fVar31 * 0.5) - fVar28 * 0.5) + 0.5;
																										fVar32 = -fVar31 * local_570;
																										fVar27 = fVar31 * local_570;
																										local_564 = fVar28 * local_570 + -fVar31 * local_56c;
																										local_560 = fVar30 * local_568 +
																											fVar29 * local_570 + local_560 + 0.0;
																										fStack1372 = fVar30 * local_570 +
																											fVar29 * local_56c + fStack1372 + 0.0;
																										local_570 = fVar31 * local_568 + fVar28 * local_570;
																										local_56c = fVar27 + fVar28 * local_56c;
																										local_568 = fVar28 * local_568 + fVar32;
																									}
																									local_28c = pfVar23[1] * local_568 +
																										*pfVar23 * local_570 + local_560 + 0.0;
																									local_288 = pfVar23[1] * local_564 +
																										*pfVar23 * local_56c + fStack1372 + 0.0;
																									local_284 = pfVar23[3] * local_568 +
																										pfVar23[2] * local_570 + local_560 + 0.0;
																									local_280 = pfVar23[3] * local_564 +
																										pfVar23[2] * local_56c + fStack1372 + 0.0;
																									local_27c = pfVar23[5] * local_568 +
																										pfVar23[4] * local_570 + local_560 + 0.0;
																									local_278 = pfVar23[5] * local_564 +
																										pfVar23[4] * local_56c + fStack1372 + 0.0;
																									local_274 = pfVar23[7] * local_568 +
																										pfVar23[6] * local_570 + local_560 + 0.0;
																									local_270 = pfVar23[7] * local_564 +
																										pfVar23[6] * local_56c + fStack1372 + 0.0;
																									uVar12 = (int)*(uint*)&p_Var25->field_0x3c >> 3;
																									uVar11 = *(uint*)&p_Var25->field_0x3c & 7;
																									local_2c = (&pDrawData->field_0x9c)[uVar11];
																									iVar15 = 0xff - (uVar12 & 0xff);
																									local_30 = (&pDrawData->field_0xa0)[uVar11];
																									uVar11 = (p_Var25->field_0x8 & 0xff00ff) * (0xff - uVar26) +
																										((local_2c & 0xff00ff) * iVar15 +
																											(local_30 & 0xff00ff) * (uVar12 & 0xff) >> 8 & 0xff00ff) *
																										uVar26;
																									uVar12 = (p_Var25->field_0x8 >> 8 & 0xff00ff) * (0xff - uVar26) +
																										(((local_2c >> 8 & 0xff00ff) * iVar15 +
																											(local_30 >> 8 & 0xff00ff) * (uVar12 & 0xff) & 0xff00ff00)
																											>> 8) * uVar26;
																									local_28 = CONCAT13((char)(((uVar12 >> 0x18) *
																										(uint) * (byte*)&p_Var25->field_0xc >> 8) *
																										local_268 >> 8),
																										CONCAT12((char)((uVar11 >> 0x18) *
																											(uint) * (byte*)&p_Var25->
																											field_0x12 >> 8),
																											CONCAT11((char)(((uVar12 & 0xff00ff00) >> 8 & 0xff) *
																												(uint)p_Var25->field_0xf >> 8),
																												(char)((uVar11 >> 8 & 0xff) *
																													(uint) * (byte*)&p_Var25->field_0xe >> 8))));
																									local_264 = local_28;
																									local_238 = DAT_00448598 *
																										p_Var25->field_0x24 * (float)(uint)p_Var25->field_0x4 *
																										0.5;
																									local_23c = local_238 * local_240;
																									FUN_002d1520(local_23c, local_238, &local_28c, &local_284, &local_27c,
																										&local_274, &local_264, (undefined8*)peVar24);
																								}
																								p_Var25 = p_Var25 + 1;
																								peVar24 = peVar24 + 1;
																								peVar9 = peVar9 + 2;
																							}
																						}
																					}
																					else {
																						p_Var25 = pParticle + param_7;
																						peVar24 = param_6 + param_7;
																						for (; param_7 < param_8; param_7 = param_7 + 1) {
																							if (p_Var25->field_0xd != 0) {
																								sVar3 = pDrawData->field_0x82;
																								iVar15 = (((int)((*(int*)&p_Var25->field_0x3c >> 8 & 0xffffU) *
																									(int)sVar3) >> 0x10) +
																									(int)(p_Var25->field_0x1c *
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
																								local_28c = pfVar23[1] * fVar28 + *pfVar23 * fVar27 + fVar30 + 0.0;
																								local_288 = pfVar23[1] * fVar31 + *pfVar23 * fVar29 + fVar32 + 0.0;
																								local_284 = pfVar23[3] * fVar28 + pfVar23[2] * fVar27 + fVar30 + 0.0;
																								local_280 = pfVar23[3] * fVar31 + pfVar23[2] * fVar29 + fVar32 + 0.0;
																								local_27c = pfVar23[5] * fVar28 + pfVar23[4] * fVar27 + fVar30 + 0.0;
																								local_278 = pfVar23[5] * fVar31 + pfVar23[4] * fVar29 + fVar32 + 0.0;
																								local_274 = pfVar23[7] * fVar28 + pfVar23[6] * fVar27 + fVar30 + 0.0;
																								local_270 = pfVar23[7] * fVar31 + pfVar23[6] * fVar29 + fVar32 + 0.0;
																								uVar26 = p_Var25->field_0x8;
																								local_264 = (((uVar26 >> 0x18) * (uint) * (byte*)&p_Var25->field_0xc >> 8
																									) * local_268 >> 8) << 0x18;
																								local_264 = local_264 |
																									CONCAT12((char)((uVar26 >> 0x10 & 0xff) *
																										(uint) * (byte*)&p_Var25->field_0x12 >> 8),
																										CONCAT11((char)((uVar26 >> 8 & 0xff) *
																											(uint)p_Var25->field_0xf >> 8),
																											(char)((uVar26 & 0xff) *
																												(uint) * (byte*)&p_Var25->field_0xe
																												>> 8)));
																								local_238 = DAT_00448598 *
																									p_Var25->field_0x24 * (float)(uint)p_Var25->field_0x4 * 0.5;
																								local_23c = local_238 * local_240;
																								FUN_002d1520(local_23c, local_238, &local_28c, &local_284, &local_27c,
																									&local_274, &local_264, (undefined8*)peVar24);
																							}
																							p_Var25 = p_Var25 + 1;
																							peVar24 = peVar24 + 1;
																						}
																					}
																				}
																				else {
																					p_Var25 = pParticle + param_7;
																					peVar24 = param_6 + param_7;
																					for (; param_7 < param_8; param_7 = param_7 + 1) {
																						if (p_Var25->field_0xd != 0) {
																							local_28c = pfVar23[1] * 0.0 + *pfVar23 * 1.0 + 0.0;
																							local_288 = pfVar23[1] * 1.0 + *pfVar23 * 0.0 + 0.0;
																							local_284 = pfVar23[3] * 0.0 + pfVar23[2] * 1.0 + 0.0;
																							local_280 = pfVar23[3] * 1.0 + pfVar23[2] * 0.0 + 0.0;
																							local_27c = pfVar23[5] * 0.0 + pfVar23[4] * 1.0 + 0.0;
																							local_278 = pfVar23[5] * 1.0 + pfVar23[4] * 0.0 + 0.0;
																							local_274 = pfVar23[7] * 0.0 + pfVar23[6] * 1.0 + 0.0;
																							local_270 = pfVar23[7] * 1.0 + pfVar23[6] * 0.0 + 0.0;
																							uVar12 = (int)*(uint*)&p_Var25->field_0x3c >> 3;
																							uVar11 = *(uint*)&p_Var25->field_0x3c & 7;
																							local_20 = (&pDrawData->field_0x9c)[uVar11];
																							iVar15 = 0xff - (uVar12 & 0xff);
																							local_24 = (&pDrawData->field_0xa0)[uVar11];
																							uVar11 = (p_Var25->field_0x8 & 0xff00ff) * (0xff - uVar26) +
																								((local_20 & 0xff00ff) * iVar15 +
																									(local_24 & 0xff00ff) * (uVar12 & 0xff) >> 8 & 0xff00ff) *
																								uVar26;
																							uVar12 = (p_Var25->field_0x8 >> 8 & 0xff00ff) * (0xff - uVar26) +
																								(((local_20 >> 8 & 0xff00ff) * iVar15 +
																									(local_24 >> 8 & 0xff00ff) * (uVar12 & 0xff) & 0xff00ff00) >> 8
																									) * uVar26;
																							local_1c = CONCAT13((char)(((uVar12 >> 0x18) *
																								(uint) * (byte*)&p_Var25->field_0xc >> 8) *
																								local_268 >> 8),
																								CONCAT12((char)((uVar11 >> 0x18) *
																									(uint) * (byte*)&p_Var25->field_0x12 >>
																									8), CONCAT11((char)(((uVar12 &
																										0xff00ff00) >> 8 & 0xff) * (uint)p_Var25->field_0xf >> 8),
																										(char)((uVar11 >> 8 & 0xff) *
																											(uint) * (byte*)&p_Var25->field_0xe >> 8))));
																							local_264 = local_1c;
																							local_238 = DAT_00448598 *
																								p_Var25->field_0x24 * (float)(uint)p_Var25->field_0x4 * 0.5;
																							local_23c = local_238 * local_240;
																							FUN_002d1520(local_23c, local_238, &local_28c, &local_284, &local_27c,
																								&local_274, &local_264, (undefined8*)peVar24);
																						}
																						p_Var25 = p_Var25 + 1;
																						peVar24 = peVar24 + 1;
																					}
																				}
																			}
																			else {
																				p_Var25 = pParticle + param_7;
																				peVar24 = param_6 + param_7;
																				for (; param_7 < param_8; param_7 = param_7 + 1) {
																					if (p_Var25->field_0xd != 0) {
																						if ((*(uint*)&p_Var25->field_0x3c & 0x80) != 0) {
																							fVar27 = local_50c * DAT_0041e9b8;
																							fVar28 = local_504 * DAT_0041e9b8;
																							local_50c = local_50c * DAT_0041e9bc + local_510 * DAT_0041e9b4;
																							local_504 = local_504 * DAT_0041e9bc + local_508 * DAT_0041e9b4;
																							fVar29 = local_4fc * DAT_0041e9b8;
																							local_4fc = local_4fc * DAT_0041e9bc +
																								local_500 * DAT_0041e9b4 + DAT_0041e9c4 + 0.0;
																							local_510 = fVar27 + local_510 * DAT_0041e9b0;
																							local_508 = fVar28 + local_508 * DAT_0041e9b0;
																							local_500 = fVar29 + local_500 * DAT_0041e9b0 + DAT_0041e9c0 + 0.0;
																						}
																						local_28c = pfVar23[1] * local_508 + *pfVar23 * local_510 + local_500 + 0.0;
																						local_288 = pfVar23[1] * local_504 + *pfVar23 * local_50c + local_4fc + 0.0;
																						local_284 = pfVar23[3] * local_508 +
																							pfVar23[2] * local_510 + local_500 + 0.0;
																						local_280 = pfVar23[3] * local_504 +
																							pfVar23[2] * local_50c + local_4fc + 0.0;
																						local_27c = pfVar23[5] * local_508 +
																							pfVar23[4] * local_510 + local_500 + 0.0;
																						local_278 = pfVar23[5] * local_504 +
																							pfVar23[4] * local_50c + local_4fc + 0.0;
																						local_274 = pfVar23[7] * local_508 +
																							pfVar23[6] * local_510 + local_500 + 0.0;
																						local_270 = pfVar23[7] * local_504 +
																							pfVar23[6] * local_50c + local_4fc + 0.0;
																						uVar26 = p_Var25->field_0x8;
																						local_264 = (((uVar26 >> 0x18) * (uint) * (byte*)&p_Var25->field_0xc >> 8) *
																							local_268 >> 8) << 0x18;
																						local_264 = local_264 |
																							CONCAT12((char)((uVar26 >> 0x10 & 0xff) *
																								(uint) * (byte*)&p_Var25->field_0x12 >> 8),
																								CONCAT11((char)((uVar26 >> 8 & 0xff) *
																									(uint)p_Var25->field_0xf >> 8),
																									(char)((uVar26 & 0xff) *
																										(uint) * (byte*)&p_Var25->field_0xe >> 8
																										)));
																						local_238 = DAT_00448598 *
																							p_Var25->field_0x24 * (float)(uint)p_Var25->field_0x4 * 0.5;
																						local_23c = local_238 * local_240;
																						FUN_002d1520(local_23c, local_238, &local_28c, &local_284, &local_27c, &local_274
																							, &local_264, (undefined8*)peVar24);
																					}
																					p_Var25 = p_Var25 + 1;
																					peVar24 = peVar24 + 1;
																				}
																			}
																		}
																		else {
																			p_Var25 = pParticle + param_7;
																			peVar24 = param_6 + param_7;
																			for (; param_7 < param_8; param_7 = param_7 + 1) {
																				if (p_Var25->field_0xd != 0) {
																					fVar28 = ((float)(uint)p_Var25->field_0x3c * pDrawData->field_0x90) / 65535.0
																						+ pDrawData->field_0x94 * (p_Var25->field_0x1c / p_Var25->field_0x20)
																						;
																					fVar27 = fVar28 * g_ScalingFactor_00448518;
																					puVar16 = (undefined*)0x3f000000;
																					if (fVar27 < 0.0) {
																						puVar16 = &DAT_bf000000;
																					}
																					fVar28 = fVar28 * g_ScalingFactor_00448518;
																					fVar27 = edFCosinus[(int)(fVar27 + (float)puVar16) & 0x1fff];
																					if (fVar28 < 0.0) {
																						puVar16 = &DAT_bf000000;
																					}
																					else {
																						puVar16 = (undefined*)0x3f000000;
																					}
																					fVar31 = edFSinus[(int)(fVar28 + (float)puVar16) & 0x1fff];
																					fVar29 = -fVar31;
																					fVar28 = -(fVar27 * 0.5) + fVar31 * 0.5 + 0.5;
																					fVar30 = (-(fVar31 * 0.5) - fVar27 * 0.5) + 0.5;
																					local_28c = pfVar23[1] * fVar29 + *pfVar23 * fVar27 + fVar28 + 0.0;
																					local_288 = pfVar23[1] * fVar27 + *pfVar23 * fVar31 + fVar30 + 0.0;
																					local_284 = pfVar23[3] * fVar29 + pfVar23[2] * fVar27 + fVar28 + 0.0;
																					local_280 = pfVar23[3] * fVar27 + pfVar23[2] * fVar31 + fVar30 + 0.0;
																					local_27c = pfVar23[5] * fVar29 + pfVar23[4] * fVar27 + fVar28 + 0.0;
																					local_278 = pfVar23[5] * fVar27 + pfVar23[4] * fVar31 + fVar30 + 0.0;
																					local_274 = pfVar23[7] * fVar29 + pfVar23[6] * fVar27 + fVar28 + 0.0;
																					local_270 = pfVar23[7] * fVar27 + pfVar23[6] * fVar31 + fVar30 + 0.0;
																					uVar26 = p_Var25->field_0x8;
																					local_264 = (((uVar26 >> 0x18) * (uint) * (byte*)&p_Var25->field_0xc >> 8) *
																						local_268 >> 8) << 0x18;
																					local_264 = local_264 |
																						CONCAT12((char)((uVar26 >> 0x10 & 0xff) *
																							(uint) * (byte*)&p_Var25->field_0x12 >> 8),
																							CONCAT11((char)((uVar26 >> 8 & 0xff) *
																								(uint)p_Var25->field_0xf >> 8),
																								(char)((uVar26 & 0xff) *
																									(uint) * (byte*)&p_Var25->field_0xe >> 8))
																						);
																					local_238 = DAT_00448598 *
																						p_Var25->field_0x24 * (float)(uint)p_Var25->field_0x4 * 0.5;
																					local_23c = local_238 * local_240;
																					FUN_002d1520(local_23c, local_238, &local_28c, &local_284, &local_27c, &local_274,
																						&local_264, (undefined8*)peVar24);
																				}
																				p_Var25 = p_Var25 + 1;
																				peVar24 = peVar24 + 1;
																			}
																		}
																	}
																	else {
																		p_Var25 = pParticle + param_7;
																		peVar24 = param_6 + param_7;
																		for (; param_7 < param_8; param_7 = param_7 + 1) {
																			if (p_Var25->field_0xd != 0) {
																				uVar26 = *(uint*)&p_Var25->field_0x3c;
																				fVar28 = ((float)(uVar26 & 0xffff) * pDrawData->field_0x90) / 65535.0;
																				local_488 = (peVar9[1].x * local_230 + peVar9[1].y * local_22c +
																					peVar9[1].z * local_228) * p_Var25->field_0x30;
																				local_490 = (peVar9[1].x * local_220 + peVar9[1].y * local_21c +
																					peVar9[1].z * local_218) * p_Var25->field_0x30;
																				fVar29 = SQRT(local_488 * local_488 + local_490 * local_490);
																				fVar27 = g_TinyFloat_00448548;
																				if (fVar29 != 0.0) {
																					fVar27 = fVar29;
																				}
																				local_488 = local_488 * (1.0 / fVar27);
																				local_490 = local_490 * (1.0 / fVar27);
																				local_48c = -local_488;
																				local_480 = (local_490 + local_488) * -0.5 + 0.5;
																				fStack1148 = (local_490 - local_488) * -0.5 + 0.5;
																				if ((fVar28 != 0.0) || (local_484 = local_490, pDrawData->field_0x94 != 0.0)) {
																					fVar27 = pDrawData->field_0x94 * (p_Var25->field_0x1c / p_Var25->field_0x20);
																					if (0.0 < local_488) {
																						fVar27 = -fVar27;
																					}
																					fVar28 = fVar28 + fVar27;
																					fVar27 = fVar28 * g_ScalingFactor_00448518;
																					puVar16 = (undefined*)0x3f000000;
																					if (fVar27 < 0.0) {
																						puVar16 = &DAT_bf000000;
																					}
																					fVar28 = fVar28 * g_ScalingFactor_00448518;
																					fVar27 = edFCosinus[(int)(fVar27 + (float)puVar16) & 0x1fff];
																					if (fVar28 < 0.0) {
																						puVar16 = &DAT_bf000000;
																					}
																					else {
																						puVar16 = (undefined*)0x3f000000;
																					}
																					fVar31 = edFSinus[(int)(fVar28 + (float)puVar16) & 0x1fff];
																					fVar29 = -(fVar27 * 0.5) + fVar31 * 0.5 + 0.5;
																					fVar30 = (-(fVar31 * 0.5) - fVar27 * 0.5) + 0.5;
																					fVar32 = -fVar31 * local_490;
																					fVar28 = fVar31 * local_490;
																					local_484 = fVar27 * local_490 + -fVar31 * local_48c;
																					local_480 = fVar30 * local_488 + fVar29 * local_490 + local_480 + 0.0;
																					fStack1148 = fVar30 * local_490 + fVar29 * local_48c + fStack1148 + 0.0;
																					local_490 = fVar31 * local_488 + fVar27 * local_490;
																					local_48c = fVar28 + fVar27 * local_48c;
																					local_488 = fVar27 * local_488 + fVar32;
																				}
																				if ((uVar26 & 0x40) != 0) {
																					fVar27 = local_48c * DAT_0041e978;
																					fVar28 = local_484 * DAT_0041e978;
																					local_48c = local_48c * DAT_0041e97c + local_490 * DAT_0041e974;
																					local_484 = local_484 * DAT_0041e97c + local_488 * DAT_0041e974;
																					fVar29 = fStack1148 * DAT_0041e978;
																					fStack1148 = fStack1148 * DAT_0041e97c +
																						local_480 * DAT_0041e974 + DAT_0041e984 + 0.0;
																					local_490 = fVar27 + local_490 * DAT_0041e970;
																					local_488 = fVar28 + local_488 * DAT_0041e970;
																					local_480 = fVar29 + local_480 * DAT_0041e970 + DAT_0041e980 + 0.0;
																				}
																				if ((uVar26 & 0x80) != 0) {
																					fVar27 = local_48c * DAT_0041e998;
																					fVar28 = local_484 * DAT_0041e998;
																					local_48c = local_48c * DAT_0041e99c + local_490 * DAT_0041e994;
																					local_484 = local_484 * DAT_0041e99c + local_488 * DAT_0041e994;
																					fVar29 = fStack1148 * DAT_0041e998;
																					fStack1148 = fStack1148 * DAT_0041e99c +
																						local_480 * DAT_0041e994 + DAT_0041e9a4 + 0.0;
																					local_490 = fVar27 + local_490 * DAT_0041e990;
																					local_488 = fVar28 + local_488 * DAT_0041e990;
																					local_480 = fVar29 + local_480 * DAT_0041e990 + DAT_0041e9a0 + 0.0;
																				}
																				local_28c = pfVar23[1] * local_488 + *pfVar23 * local_490 + local_480 + 0.0;
																				local_288 = pfVar23[1] * local_484 + *pfVar23 * local_48c + fStack1148 + 0.0;
																				local_284 = pfVar23[3] * local_488 + pfVar23[2] * local_490 + local_480 + 0.0;
																				local_280 = pfVar23[3] * local_484 + pfVar23[2] * local_48c + fStack1148 + 0.0;
																				local_27c = pfVar23[5] * local_488 + pfVar23[4] * local_490 + local_480 + 0.0;
																				local_278 = pfVar23[5] * local_484 + pfVar23[4] * local_48c + fStack1148 + 0.0;
																				local_274 = pfVar23[7] * local_488 + pfVar23[6] * local_490 + local_480 + 0.0;
																				local_270 = pfVar23[7] * local_484 + pfVar23[6] * local_48c + fStack1148 + 0.0;
																				uVar26 = p_Var25->field_0x8;
																				local_264 = (((uVar26 >> 0x18) * (uint) * (byte*)&p_Var25->field_0xc >> 8) *
																					local_268 >> 8) << 0x18;
																				local_264 = local_264 |
																					CONCAT12((char)((uVar26 >> 0x10 & 0xff) *
																						(uint) * (byte*)&p_Var25->field_0x12 >> 8),
																						CONCAT11((char)((uVar26 >> 8 & 0xff) *
																							(uint)p_Var25->field_0xf >> 8),
																							(char)((uVar26 & 0xff) *
																								(uint) * (byte*)&p_Var25->field_0xe >> 8)));
																				local_238 = DAT_00448598 * p_Var25->field_0x24 * (float)(uint)p_Var25->field_0x4
																					* 0.5;
																				local_23c = local_238 * local_240;
																				FUN_002d1520(local_23c, local_238, &local_28c, &local_284, &local_27c, &local_274,
																					&local_264, (undefined8*)peVar24);
																			}
																			p_Var25 = p_Var25 + 1;
																			peVar24 = peVar24 + 1;
																			peVar9 = peVar9 + 2;
																		}
																	}
																}
																else {
																	p_Var25 = pParticle + param_7;
																	peVar24 = param_6 + param_7;
																	for (; param_7 < param_8; param_7 = param_7 + 1) {
																		if (p_Var25->field_0xd != 0) {
																			fVar27 = pDrawData->field_0x94 * (p_Var25->field_0x1c / p_Var25->field_0x20);
																			if ((*(uint*)&p_Var25->field_0x3c & 0x20) != 0) {
																				fVar27 = -fVar27;
																			}
																			fVar27 = pDrawData->field_0x90 + fVar27;
																			fVar28 = fVar27 * g_ScalingFactor_00448518;
																			puVar16 = (undefined*)0x3f000000;
																			if (fVar28 < 0.0) {
																				puVar16 = &DAT_bf000000;
																			}
																			fVar27 = fVar27 * g_ScalingFactor_00448518;
																			fVar28 = edFCosinus[(int)(fVar28 + (float)puVar16) & 0x1fff];
																			if (fVar27 < 0.0) {
																				puVar16 = &DAT_bf000000;
																			}
																			else {
																				puVar16 = (undefined*)0x3f000000;
																			}
																			fVar31 = edFSinus[(int)(fVar27 + (float)puVar16) & 0x1fff];
																			fVar29 = -fVar31;
																			fVar27 = -(fVar28 * 0.5) + fVar31 * 0.5 + 0.5;
																			fVar30 = (-(fVar31 * 0.5) - fVar28 * 0.5) + 0.5;
																			local_28c = pfVar23[1] * fVar29 + *pfVar23 * fVar28 + fVar27 + 0.0;
																			local_288 = pfVar23[1] * fVar28 + *pfVar23 * fVar31 + fVar30 + 0.0;
																			local_284 = pfVar23[3] * fVar29 + pfVar23[2] * fVar28 + fVar27 + 0.0;
																			local_280 = pfVar23[3] * fVar28 + pfVar23[2] * fVar31 + fVar30 + 0.0;
																			local_27c = pfVar23[5] * fVar29 + pfVar23[4] * fVar28 + fVar27 + 0.0;
																			local_278 = pfVar23[5] * fVar28 + pfVar23[4] * fVar31 + fVar30 + 0.0;
																			local_274 = pfVar23[7] * fVar29 + pfVar23[6] * fVar28 + fVar27 + 0.0;
																			local_270 = pfVar23[7] * fVar28 + pfVar23[6] * fVar31 + fVar30 + 0.0;
																			uVar26 = p_Var25->field_0x8;
																			local_264 = (((uVar26 >> 0x18) * (uint) * (byte*)&p_Var25->field_0xc >> 8) *
																				local_268 >> 8) << 0x18;
																			local_264 = local_264 |
																				CONCAT12((char)((uVar26 >> 0x10 & 0xff) *
																					(uint) * (byte*)&p_Var25->field_0x12 >> 8),
																					CONCAT11((char)((uVar26 >> 8 & 0xff) *
																						(uint)p_Var25->field_0xf >> 8),
																						(char)((uVar26 & 0xff) *
																							(uint) * (byte*)&p_Var25->field_0xe >> 8)));
																			local_238 = DAT_00448598 * p_Var25->field_0x24 * (float)(uint)p_Var25->field_0x4 *
																				0.5;
																			local_23c = local_238 * local_240;
																			FUN_002d1520(local_23c, local_238, &local_28c, &local_284, &local_27c, &local_274,
																				&local_264, (undefined8*)peVar24);
																		}
																		p_Var25 = p_Var25 + 1;
																		peVar24 = peVar24 + 1;
																	}
																}
															}
															else {
																p_Var25 = pParticle + param_7;
																peVar24 = param_6 + param_7;
																for (; param_7 < param_8; param_7 = param_7 + 1) {
																	if (p_Var25->field_0xd != 0) {
																		local_408 = (peVar9[1].x * local_230 + peVar9[1].y * local_22c +
																			peVar9[1].z * local_228) * p_Var25->field_0x30;
																		local_410 = (peVar9[1].x * local_220 + peVar9[1].y * local_21c +
																			peVar9[1].z * local_218) * p_Var25->field_0x30;
																		fVar28 = SQRT(local_408 * local_408 + local_410 * local_410);
																		fVar27 = g_TinyFloat_00448548;
																		if (fVar28 != 0.0) {
																			fVar27 = fVar28;
																		}
																		local_408 = local_408 * (1.0 / fVar27);
																		local_410 = local_410 * (1.0 / fVar27);
																		local_40c = -local_408;
																		local_400 = (local_410 + local_408) * -0.5 + 0.5;
																		fStack1020 = (local_410 - local_408) * -0.5 + 0.5;
																		if ((pDrawData->field_0x90 != 0.0) ||
																			(local_404 = local_410, pDrawData->field_0x94 != 0.0)) {
																			fVar27 = pDrawData->field_0x94 * (p_Var25->field_0x1c / p_Var25->field_0x20);
																			if (0.0 < local_408) {
																				fVar27 = -fVar27;
																			}
																			fVar27 = pDrawData->field_0x90 + fVar27;
																			fVar28 = fVar27 * g_ScalingFactor_00448518;
																			puVar16 = (undefined*)0x3f000000;
																			if (fVar28 < 0.0) {
																				puVar16 = &DAT_bf000000;
																			}
																			fVar27 = fVar27 * g_ScalingFactor_00448518;
																			fVar28 = edFCosinus[(int)(fVar28 + (float)puVar16) & 0x1fff];
																			if (fVar27 < 0.0) {
																				puVar16 = &DAT_bf000000;
																			}
																			else {
																				puVar16 = (undefined*)0x3f000000;
																			}
																			fVar31 = edFSinus[(int)(fVar27 + (float)puVar16) & 0x1fff];
																			fVar29 = -(fVar28 * 0.5) + fVar31 * 0.5 + 0.5;
																			fVar30 = (-(fVar31 * 0.5) - fVar28 * 0.5) + 0.5;
																			fVar32 = -fVar31 * local_410;
																			fVar27 = fVar31 * local_410;
																			local_404 = fVar28 * local_410 + -fVar31 * local_40c;
																			local_400 = fVar30 * local_408 + fVar29 * local_410 + local_400 + 0.0;
																			fStack1020 = fVar30 * local_410 + fVar29 * local_40c + fStack1020 + 0.0;
																			local_410 = fVar31 * local_408 + fVar28 * local_410;
																			local_40c = fVar27 + fVar28 * local_40c;
																			local_408 = fVar28 * local_408 + fVar32;
																		}
																		local_28c = pfVar23[1] * local_408 + *pfVar23 * local_410 + local_400 + 0.0;
																		local_288 = pfVar23[1] * local_404 + *pfVar23 * local_40c + fStack1020 + 0.0;
																		local_284 = pfVar23[3] * local_408 + pfVar23[2] * local_410 + local_400 + 0.0;
																		local_280 = pfVar23[3] * local_404 + pfVar23[2] * local_40c + fStack1020 + 0.0;
																		local_27c = pfVar23[5] * local_408 + pfVar23[4] * local_410 + local_400 + 0.0;
																		local_278 = pfVar23[5] * local_404 + pfVar23[4] * local_40c + fStack1020 + 0.0;
																		local_274 = pfVar23[7] * local_408 + pfVar23[6] * local_410 + local_400 + 0.0;
																		local_270 = pfVar23[7] * local_404 + pfVar23[6] * local_40c + fStack1020 + 0.0;
																		uVar26 = p_Var25->field_0x8;
																		local_264 = (((uVar26 >> 0x18) * (uint) * (byte*)&p_Var25->field_0xc >> 8) *
																			local_268 >> 8) << 0x18;
																		local_264 = local_264 |
																			CONCAT12((char)((uVar26 >> 0x10 & 0xff) *
																				(uint) * (byte*)&p_Var25->field_0x12 >> 8),
																				CONCAT11((char)((uVar26 >> 8 & 0xff) * (uint)p_Var25->field_0xf
																					>> 8),
																					(char)((uVar26 & 0xff) *
																						(uint) * (byte*)&p_Var25->field_0xe >> 8)));
																		local_238 = DAT_00448598 * p_Var25->field_0x24 * (float)(uint)p_Var25->field_0x4 *
																			0.5;
																		local_23c = local_238 * local_240;
																		FUN_002d1520(local_23c, local_238, &local_28c, &local_284, &local_27c, &local_274,
																			&local_264, (undefined8*)peVar24);
																	}
																	p_Var25 = p_Var25 + 1;
																	peVar24 = peVar24 + 1;
																	peVar9 = peVar9 + 2;
																}
															}
														}
														else {
															p_Var25 = pParticle + param_7;
															peVar24 = param_6 + param_7;
															for (; param_7 < param_8; param_7 = param_7 + 1) {
																if (p_Var25->field_0xd != 0) {
																	local_3f0 = 1.0;
																	local_3e4 = 1.0;
																	local_3ec = 0.0;
																	local_3e8 = 0.0;
																	local_3e0 = 0.0;
																	local_3dc = 0.0;
																	if ((*(uint*)&p_Var25->field_0x3c & 0x40) != 0) {
																		local_3f0 = DAT_0041e918 * 0.0 + DAT_0041e910 * 1.0;
																		local_3e8 = DAT_0041e918 * 1.0 + DAT_0041e910 * 0.0;
																		local_3ec = DAT_0041e91c * 0.0 + DAT_0041e914 * 1.0;
																		local_3e4 = DAT_0041e91c * 1.0 + DAT_0041e914 * 0.0;
																		local_3e0 = DAT_0041e918 * 0.0 + DAT_0041e910 * 0.0 + DAT_0041e920 + 0.0;
																		local_3dc = DAT_0041e91c * 0.0 + DAT_0041e914 * 0.0 + DAT_0041e924 + 0.0;
																	}
																	if ((*(uint*)&p_Var25->field_0x3c & 0x80) != 0) {
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
																	iVar15 = (int)(p_Var25->field_0x1c * (float)*(int*)&pDrawData->field_0x98) %
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
																	local_28c = pfVar23[1] * fVar28 + *pfVar23 * fVar27 + fVar30 + 0.0;
																	local_288 = pfVar23[1] * fVar31 + *pfVar23 * fVar29 + fVar32 + 0.0;
																	local_284 = pfVar23[3] * fVar28 + pfVar23[2] * fVar27 + fVar30 + 0.0;
																	local_280 = pfVar23[3] * fVar31 + pfVar23[2] * fVar29 + fVar32 + 0.0;
																	local_27c = pfVar23[5] * fVar28 + pfVar23[4] * fVar27 + fVar30 + 0.0;
																	local_278 = pfVar23[5] * fVar31 + pfVar23[4] * fVar29 + fVar32 + 0.0;
																	local_274 = pfVar23[7] * fVar28 + pfVar23[6] * fVar27 + fVar30 + 0.0;
																	local_270 = pfVar23[7] * fVar31 + pfVar23[6] * fVar29 + fVar32 + 0.0;
																	uVar26 = p_Var25->field_0x8;
																	local_264 = (((uVar26 >> 0x18) * (uint) * (byte*)&p_Var25->field_0xc >> 8) * local_268
																		>> 8) << 0x18;
																	local_264 = local_264 |
																		CONCAT12((char)((uVar26 >> 0x10 & 0xff) *
																			(uint) * (byte*)&p_Var25->field_0x12 >> 8),
																			CONCAT11((char)((uVar26 >> 8 & 0xff) * (uint)p_Var25->field_0xf
																				>> 8),
																				(char)((uVar26 & 0xff) *
																					(uint) * (byte*)&p_Var25->field_0xe >> 8)));
																	local_238 = DAT_00448598 * p_Var25->field_0x24 * (float)(uint)p_Var25->field_0x4 * 0.5
																		;
																	local_23c = local_238 * local_240;
																	FUN_002d1520(local_23c, local_238, &local_28c, &local_284, &local_27c, &local_274,
																		&local_264, (undefined8*)peVar24);
																}
																p_Var25 = p_Var25 + 1;
																peVar24 = peVar24 + 1;
															}
														}
													}
													else {
														p_Var25 = pParticle + param_7;
														peVar24 = param_6 + param_7;
														for (; param_7 < param_8; param_7 = param_7 + 1) {
															if (p_Var25->field_0xd != 0) {
																fVar28 = pDrawData->field_0x90 +
																	pDrawData->field_0x94 * (p_Var25->field_0x1c / p_Var25->field_0x20);
																fVar27 = fVar28 * g_ScalingFactor_00448518;
																if (fVar27 < 0.0) {
																	puVar16 = &DAT_bf000000;
																}
																else {
																	puVar16 = (undefined*)0x3f000000;
																}
																fVar28 = fVar28 * g_ScalingFactor_00448518;
																fVar27 = edFCosinus[(int)(fVar27 + (float)puVar16) & 0x1fff];
																if (fVar28 < 0.0) {
																	puVar16 = &DAT_bf000000;
																}
																else {
																	puVar16 = (undefined*)0x3f000000;
																}
																fVar28 = edFSinus[(int)(fVar28 + (float)puVar16) & 0x1fff];
																fVar29 = -(fVar27 * 0.5) + fVar28 * 0.5 + 0.5;
																fVar30 = (-(fVar28 * 0.5) - fVar27 * 0.5) + 0.5;
																sVar3 = pDrawData->field_0x82;
																iVar15 = (((int)((*(int*)&p_Var25->field_0x3c >> 8 & 0xffffU) * (int)sVar3) >> 0x10) +
																	(int)(p_Var25->field_0x1c * (float)*(int*)&pDrawData->field_0x98)) %
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
																local_28c = pfVar23[1] * fVar32 + *pfVar23 * fVar31 + fVar27 + 0.0;
																local_288 = pfVar23[1] * fVar28 + *pfVar23 * fVar33 + fVar29 + 0.0;
																local_284 = pfVar23[3] * fVar32 + pfVar23[2] * fVar31 + fVar27 + 0.0;
																local_280 = pfVar23[3] * fVar28 + pfVar23[2] * fVar33 + fVar29 + 0.0;
																local_27c = pfVar23[5] * fVar32 + pfVar23[4] * fVar31 + fVar27 + 0.0;
																local_278 = pfVar23[5] * fVar28 + pfVar23[4] * fVar33 + fVar29 + 0.0;
																local_274 = pfVar23[7] * fVar32 + pfVar23[6] * fVar31 + fVar27 + 0.0;
																local_270 = pfVar23[7] * fVar28 + pfVar23[6] * fVar33 + fVar29 + 0.0;
																uVar26 = p_Var25->field_0x8;
																local_264 = (((uVar26 >> 0x18) * (uint) * (byte*)&p_Var25->field_0xc >> 8) * local_268 >>
																	8) << 0x18;
																local_264 = local_264 |
																	CONCAT12((char)((uVar26 >> 0x10 & 0xff) *
																		(uint) * (byte*)&p_Var25->field_0x12 >> 8),
																		CONCAT11((char)((uVar26 >> 8 & 0xff) * (uint)p_Var25->field_0xf >>
																			8), (char)((uVar26 & 0xff) *
																				(uint) * (byte*)&p_Var25->field_0xe >> 8)))
																	;
																local_238 = DAT_00448598 * p_Var25->field_0x24 * (float)(uint)p_Var25->field_0x4 * 0.5;
																local_23c = local_238 * local_240;
																FUN_002d1520(local_23c, local_238, &local_28c, &local_284, &local_27c, &local_274, &local_264,
																	(undefined8*)peVar24);
															}
															p_Var25 = p_Var25 + 1;
															peVar24 = peVar24 + 1;
														}
													}
												}
												else {
													p_Var25 = pParticle + param_7;
													peVar24 = param_6 + param_7;
													for (; param_7 < param_8; param_7 = param_7 + 1) {
														if (p_Var25->field_0xd != 0) {
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
															local_28c = pfVar23[1] * fVar28 + *pfVar23 * fVar27 + fVar30 + 0.0;
															local_288 = pfVar23[1] * fVar31 + *pfVar23 * fVar29 + fVar32 + 0.0;
															local_284 = pfVar23[3] * fVar28 + pfVar23[2] * fVar27 + fVar30 + 0.0;
															local_280 = pfVar23[3] * fVar31 + pfVar23[2] * fVar29 + fVar32 + 0.0;
															local_27c = pfVar23[5] * fVar28 + pfVar23[4] * fVar27 + fVar30 + 0.0;
															local_278 = pfVar23[5] * fVar31 + pfVar23[4] * fVar29 + fVar32 + 0.0;
															local_274 = pfVar23[7] * fVar28 + pfVar23[6] * fVar27 + fVar30 + 0.0;
															local_270 = pfVar23[7] * fVar31 + pfVar23[6] * fVar29 + fVar32 + 0.0;
															uVar26 = p_Var25->field_0x8;
															local_264 = (((uVar26 >> 0x18) * (uint) * (byte*)&p_Var25->field_0xc >> 8) * local_268 >> 8
																) << 0x18;
															local_264 = local_264 |
																CONCAT12((char)((uVar26 >> 0x10 & 0xff) * (uint) * (byte*)&p_Var25->field_0x12
																	>> 8),
																	CONCAT11((char)((uVar26 >> 8 & 0xff) * (uint)p_Var25->field_0xf >> 8)
																		, (char)((uVar26 & 0xff) * (uint) * (byte*)&p_Var25->field_0xe
																			>> 8)));
															local_238 = DAT_00448598 * p_Var25->field_0x24 * (float)(uint)p_Var25->field_0x4 * 0.5;
															local_23c = local_238 * local_240;
															FUN_002d1520(local_23c, local_238, &local_28c, &local_284, &local_27c, &local_274, &local_264,
																(undefined8*)peVar24);
														}
														p_Var25 = p_Var25 + 1;
														peVar24 = peVar24 + 1;
													}
												}
											}
											else {
												p_Var25 = pParticle + param_7;
												peVar24 = param_6 + param_7;
												for (; param_7 < param_8; param_7 = param_7 + 1) {
													if (p_Var25->field_0xd != 0) {
														fVar28 = pDrawData->field_0x90 +
															pDrawData->field_0x94 * (p_Var25->field_0x1c / p_Var25->field_0x20);
														fVar27 = fVar28 * g_ScalingFactor_00448518;
														puVar16 = (undefined*)0x3f000000;
														if (fVar27 < 0.0) {
															puVar16 = &DAT_bf000000;
														}
														fVar28 = fVar28 * g_ScalingFactor_00448518;
														fVar27 = edFCosinus[(int)(fVar27 + (float)puVar16) & 0x1fff];
														if (fVar28 < 0.0) {
															puVar16 = &DAT_bf000000;
														}
														else {
															puVar16 = (undefined*)0x3f000000;
														}
														fVar31 = edFSinus[(int)(fVar28 + (float)puVar16) & 0x1fff];
														fVar29 = -fVar31;
														fVar28 = -(fVar27 * 0.5) + fVar31 * 0.5 + 0.5;
														fVar30 = (-(fVar31 * 0.5) - fVar27 * 0.5) + 0.5;
														local_28c = pfVar23[1] * fVar29 + *pfVar23 * fVar27 + fVar28 + 0.0;
														local_288 = pfVar23[1] * fVar27 + *pfVar23 * fVar31 + fVar30 + 0.0;
														local_284 = pfVar23[3] * fVar29 + pfVar23[2] * fVar27 + fVar28 + 0.0;
														local_280 = pfVar23[3] * fVar27 + pfVar23[2] * fVar31 + fVar30 + 0.0;
														local_27c = pfVar23[5] * fVar29 + pfVar23[4] * fVar27 + fVar28 + 0.0;
														local_278 = pfVar23[5] * fVar27 + pfVar23[4] * fVar31 + fVar30 + 0.0;
														local_274 = pfVar23[7] * fVar29 + pfVar23[6] * fVar27 + fVar28 + 0.0;
														local_270 = pfVar23[7] * fVar27 + pfVar23[6] * fVar31 + fVar30 + 0.0;
														uVar26 = p_Var25->field_0x8;
														local_264 = (((uVar26 >> 0x18) * (uint) * (byte*)&p_Var25->field_0xc >> 8) * local_268 >> 8)
															<< 0x18;
														local_264 = local_264 |
															CONCAT12((char)((uVar26 >> 0x10 & 0xff) * (uint) * (byte*)&p_Var25->field_0x12 >>
																8), CONCAT11((char)((uVar26 >> 8 & 0xff) *
																	(uint)p_Var25->field_0xf >> 8),
																	(char)((uVar26 & 0xff) *
																		(uint) * (byte*)&p_Var25->field_0xe >> 8)));
														local_238 = DAT_00448598 * p_Var25->field_0x24 * (float)(uint)p_Var25->field_0x4 * 0.5;
														local_23c = local_238 * local_240;
														FUN_002d1520(local_23c, local_238, &local_28c, &local_284, &local_27c, &local_274, &local_264,
															(undefined8*)peVar24);
													}
													p_Var25 = p_Var25 + 1;
													peVar24 = peVar24 + 1;
												}
											}
										}
										else {
											p_Var25 = pParticle + param_7;
											peVar24 = param_6 + param_7;
											for (; param_7 < param_8; param_7 = param_7 + 1) {
												if (p_Var25->field_0xd != 0) {
													FLOAT_ARRAY_0041e890[0] = pDrawData->field_0x88;
													FLOAT_ARRAY_0041e890[3] = pDrawData->field_0x8c;
													FLOAT_ARRAY_0041e890[4] = FLOAT_ARRAY_0041e890[0] * -0.5 + 0.5;
													FLOAT_ARRAY_0041e890[5] = FLOAT_ARRAY_0041e890[3] * -0.5 + 0.5;
													fVar27 = FLOAT_ARRAY_0041e890[2] * 0.0 + FLOAT_ARRAY_0041e890[0] * 1.0;
													fVar28 = FLOAT_ARRAY_0041e890[2] * 1.0 + FLOAT_ARRAY_0041e890[0] * 0.0;
													fVar29 = FLOAT_ARRAY_0041e890[3] * 0.0 + FLOAT_ARRAY_0041e890[1] * 1.0;
													fVar31 = FLOAT_ARRAY_0041e890[3] * 1.0 + FLOAT_ARRAY_0041e890[1] * 0.0;
													fVar30 = FLOAT_ARRAY_0041e890[2] * 0.0 +
														FLOAT_ARRAY_0041e890[0] * 0.0 + FLOAT_ARRAY_0041e890[4] + 0.0;
													fVar32 = FLOAT_ARRAY_0041e890[3] * 0.0 +
														FLOAT_ARRAY_0041e890[1] * 0.0 + FLOAT_ARRAY_0041e890[5] + 0.0;
													lVar14 = (long)pDrawData->field_0x82 << 0x30;
													iVar15 = (int)(p_Var25->field_0x1c * (float)*(int*)&pDrawData->field_0x98) %
														(int)(lVar14 >> 0x30);
													if (lVar14 >> 0x30 == 0) {
														trap(7);
													}
													if (iVar15 < 0) {
														iVar15 = -iVar15;
													}
													sVar3 = pDrawData->field_0x84;
													FLOAT_ARRAY_0041e8b0[0] = 1.0 / (float)(int)sVar3;
													FLOAT_ARRAY_0041e8b0[3] = 1.0 / (float)(int)pDrawData->field_0x86;
													if (sVar3 == 0) {
														trap(7);
													}
													FLOAT_ARRAY_0041e8b0[4] = FLOAT_ARRAY_0041e8b0[0] * (float)(iVar15 % (int)sVar3);
													if (pDrawData->field_0x84 == 0) {
														trap(7);
													}
													FLOAT_ARRAY_0041e8b0[5] =
														FLOAT_ARRAY_0041e8b0[3] * (float)(iVar15 / (int)pDrawData->field_0x84);
													fVar33 = fVar29 * FLOAT_ARRAY_0041e8b0[2] + fVar27 * FLOAT_ARRAY_0041e8b0[0];
													fVar34 = fVar31 * FLOAT_ARRAY_0041e8b0[2] + fVar28 * FLOAT_ARRAY_0041e8b0[0];
													fVar27 = fVar29 * FLOAT_ARRAY_0041e8b0[3] + fVar27 * FLOAT_ARRAY_0041e8b0[1];
													fVar29 = fVar31 * FLOAT_ARRAY_0041e8b0[3] + fVar28 * FLOAT_ARRAY_0041e8b0[1];
													fVar28 = fVar32 * FLOAT_ARRAY_0041e8b0[2] +
														fVar30 * FLOAT_ARRAY_0041e8b0[0] + FLOAT_ARRAY_0041e8b0[4] + 0.0;
													fVar30 = fVar32 * FLOAT_ARRAY_0041e8b0[3] +
														fVar30 * FLOAT_ARRAY_0041e8b0[1] + FLOAT_ARRAY_0041e8b0[5] + 0.0;
													local_28c = pfVar23[1] * fVar34 + *pfVar23 * fVar33 + fVar28 + 0.0;
													local_288 = pfVar23[1] * fVar29 + *pfVar23 * fVar27 + fVar30 + 0.0;
													local_284 = pfVar23[3] * fVar34 + pfVar23[2] * fVar33 + fVar28 + 0.0;
													local_280 = pfVar23[3] * fVar29 + pfVar23[2] * fVar27 + fVar30 + 0.0;
													local_27c = pfVar23[5] * fVar34 + pfVar23[4] * fVar33 + fVar28 + 0.0;
													local_278 = pfVar23[5] * fVar29 + pfVar23[4] * fVar27 + fVar30 + 0.0;
													local_274 = pfVar23[7] * fVar34 + pfVar23[6] * fVar33 + fVar28 + 0.0;
													local_270 = pfVar23[7] * fVar29 + pfVar23[6] * fVar27 + fVar30 + 0.0;
													uVar12 = (int)*(uint*)&p_Var25->field_0x3c >> 3;
													uVar11 = *(uint*)&p_Var25->field_0x3c & 7;
													local_14 = (&pDrawData->field_0x9c)[uVar11];
													iVar15 = 0xff - (uVar12 & 0xff);
													local_18 = (&pDrawData->field_0xa0)[uVar11];
													uVar11 = (p_Var25->field_0x8 & 0xff00ff) * (0xff - uVar26) +
														((local_14 & 0xff00ff) * iVar15 + (local_18 & 0xff00ff) * (uVar12 & 0xff) >> 8 &
															0xff00ff) * uVar26;
													uVar12 = (p_Var25->field_0x8 >> 8 & 0xff00ff) * (0xff - uVar26) +
														(((local_14 >> 8 & 0xff00ff) * iVar15 + (local_18 >> 8 & 0xff00ff) * (uVar12 & 0xff)
															& 0xff00ff00) >> 8) * uVar26;
													local_10 = CONCAT13((char)(((uVar12 >> 0x18) * (uint) * (byte*)&p_Var25->field_0xc >> 8) *
														local_268 >> 8),
														CONCAT12((char)((uVar11 >> 0x18) * (uint) * (byte*)&p_Var25->field_0x12 >>
															8), CONCAT11((char)(((uVar12 & 0xff00ff00) >> 8 & 0xff) *
																(uint)p_Var25->field_0xf >> 8),
																(char)((uVar11 >> 8 & 0xff) *
																	(uint) * (byte*)&p_Var25->field_0xe >> 8)
															)));
													local_264 = local_10;
													local_238 = DAT_00448598 * p_Var25->field_0x24 * (float)(uint)p_Var25->field_0x4 * 0.5;
													local_23c = local_238 * local_240;
													FUN_002d1520(local_23c, local_238, &local_28c, &local_284, &local_27c, &local_274, &local_264,
														(undefined8*)peVar24);
												}
												p_Var25 = p_Var25 + 1;
												peVar24 = peVar24 + 1;
											}
										}
									}
									else {
										p_Var25 = pParticle + param_7;
										peVar24 = param_6 + param_7;
										for (; param_7 < param_8; param_7 = param_7 + 1) {
											if (p_Var25->field_0xd != 0) {
												uVar12 = *(uint*)&p_Var25->field_0x3c;
												fVar27 = pDrawData->field_0x94 * (p_Var25->field_0x1c / p_Var25->field_0x20);
												if ((uVar12 & 0x20) != 0) {
													fVar27 = -fVar27;
												}
												fVar27 = ((float)(uVar12 & 0xffff) * pDrawData->field_0x90) / 65535.0 + fVar27;
												fVar28 = fVar27 * g_ScalingFactor_00448518;
												puVar16 = (undefined*)0x3f000000;
												if (fVar28 < 0.0) {
													puVar16 = &DAT_bf000000;
												}
												fVar27 = fVar27 * g_ScalingFactor_00448518;
												local_344 = edFCosinus[(int)(fVar28 + (float)puVar16) & 0x1fff];
												if (fVar27 < 0.0) {
													puVar16 = &DAT_bf000000;
												}
												else {
													puVar16 = (undefined*)0x3f000000;
												}
												local_34c = edFSinus[(int)(fVar27 + (float)puVar16) & 0x1fff];
												fVar28 = -(local_344 * 0.5) + local_34c * 0.5 + 0.5;
												local_33c = (-(local_34c * 0.5) - local_344 * 0.5) + 0.5;
												fVar27 = -local_34c;
												local_350 = local_344;
												local_348 = fVar27;
												local_340 = fVar28;
												if ((uVar12 & 0x40) != 0) {
													local_350 = local_34c * FLOAT_ARRAY_0041e850[2] + local_344 * FLOAT_ARRAY_0041e850[0];
													local_348 = local_344 * FLOAT_ARRAY_0041e850[2] + fVar27 * FLOAT_ARRAY_0041e850[0];
													local_34c = local_34c * FLOAT_ARRAY_0041e850[3] + local_344 * FLOAT_ARRAY_0041e850[1];
													local_344 = local_344 * FLOAT_ARRAY_0041e850[3] + fVar27 * FLOAT_ARRAY_0041e850[1];
													local_340 = local_33c * FLOAT_ARRAY_0041e850[2] +
														fVar28 * FLOAT_ARRAY_0041e850[0] + FLOAT_ARRAY_0041e850[4] + 0.0;
													local_33c = local_33c * FLOAT_ARRAY_0041e850[3] +
														fVar28 * FLOAT_ARRAY_0041e850[1] + FLOAT_ARRAY_0041e850[5] + 0.0;
												}
												if ((uVar12 & 0x80) != 0) {
													fVar27 = local_34c * FLOAT_ARRAY_0041e870[2];
													fVar28 = local_344 * FLOAT_ARRAY_0041e870[2];
													local_34c = local_34c * FLOAT_ARRAY_0041e870[3] + local_350 * FLOAT_ARRAY_0041e870[1];
													local_344 = local_344 * FLOAT_ARRAY_0041e870[3] + local_348 * FLOAT_ARRAY_0041e870[1];
													fVar29 = local_33c * FLOAT_ARRAY_0041e870[2];
													local_33c = local_33c * FLOAT_ARRAY_0041e870[3] +
														local_340 * FLOAT_ARRAY_0041e870[1] + FLOAT_ARRAY_0041e870[5] + 0.0;
													local_350 = fVar27 + local_350 * FLOAT_ARRAY_0041e870[0];
													local_348 = fVar28 + local_348 * FLOAT_ARRAY_0041e870[0];
													local_340 = fVar29 + local_340 * FLOAT_ARRAY_0041e870[0] + FLOAT_ARRAY_0041e870[4] + 0.0;
												}
												local_28c = pfVar23[1] * local_348 + *pfVar23 * local_350 + local_340 + 0.0;
												local_288 = pfVar23[1] * local_344 + *pfVar23 * local_34c + local_33c + 0.0;
												local_284 = pfVar23[3] * local_348 + pfVar23[2] * local_350 + local_340 + 0.0;
												local_280 = pfVar23[3] * local_344 + pfVar23[2] * local_34c + local_33c + 0.0;
												local_27c = pfVar23[5] * local_348 + pfVar23[4] * local_350 + local_340 + 0.0;
												local_278 = pfVar23[5] * local_344 + pfVar23[4] * local_34c + local_33c + 0.0;
												local_274 = pfVar23[7] * local_348 + pfVar23[6] * local_350 + local_340 + 0.0;
												local_270 = pfVar23[7] * local_344 + pfVar23[6] * local_34c + local_33c + 0.0;
												uVar11 = (int)uVar12 >> 3;
												local_8 = (&pDrawData->field_0x9c)[uVar12 & 7];
												iVar15 = 0xff - (uVar11 & 0xff);
												local_c = (&pDrawData->field_0xa0)[uVar12 & 7];
												uVar22 = (p_Var25->field_0x8 & 0xff00ff) * (0xff - uVar26) +
													((local_8 & 0xff00ff) * iVar15 + (local_c & 0xff00ff) * (uVar11 & 0xff) >> 8 & 0xff00ff
														) * uVar26;
												uVar12 = (p_Var25->field_0x8 >> 8 & 0xff00ff) * (0xff - uVar26) +
													(((local_8 >> 8 & 0xff00ff) * iVar15 + (local_c >> 8 & 0xff00ff) * (uVar11 & 0xff) &
														0xff00ff00) >> 8) * uVar26;
												local_4 = CONCAT13((char)(((uVar12 >> 0x18) * (uint) * (byte*)&p_Var25->field_0xc >> 8) *
													local_268 >> 8),
													CONCAT12((char)((uVar22 >> 0x18) * (uint) * (byte*)&p_Var25->field_0x12 >> 8),
														CONCAT11((char)(((uVar12 & 0xff00ff00) >> 8 & 0xff) *
															(uint)p_Var25->field_0xf >> 8),
															(char)((uVar22 >> 8 & 0xff) *
																(uint) * (byte*)&p_Var25->field_0xe >> 8))));
												local_264 = local_4;
												local_238 = DAT_00448598 * p_Var25->field_0x24 * (float)(uint)p_Var25->field_0x4 * 0.5;
												local_23c = local_238 * local_240;
												FUN_002d1520(local_23c, local_238, &local_28c, &local_284, &local_27c, &local_274, &local_264,
													(undefined8*)peVar24);
											}
											p_Var25 = p_Var25 + 1;
											peVar24 = peVar24 + 1;
										}
									}
								}
								else {
									p_Var25 = pParticle + param_7;
									peVar24 = param_6 + param_7;
									for (; param_7 < param_8; param_7 = param_7 + 1) {
										if (p_Var25->field_0xd != 0) {
											fVar27 = pDrawData->field_0x94 * (p_Var25->field_0x1c / p_Var25->field_0x20);
											if ((*(uint*)&p_Var25->field_0x3c & 0x20) != 0) {
												fVar27 = -fVar27;
											}
											fVar27 = ((float)(*(uint*)&p_Var25->field_0x3c & 0xffff) * pDrawData->field_0x90) / 65535.0 +
												fVar27;
											fVar28 = fVar27 * g_ScalingFactor_00448518;
											puVar16 = (undefined*)0x3f000000;
											if (fVar28 < 0.0) {
												puVar16 = &DAT_bf000000;
											}
											fVar27 = fVar27 * g_ScalingFactor_00448518;
											fVar28 = edFCosinus[(int)(fVar28 + (float)puVar16) & 0x1fff];
											if (fVar27 < 0.0) {
												puVar16 = &DAT_bf000000;
											}
											else {
												puVar16 = (undefined*)0x3f000000;
											}
											fVar31 = edFSinus[(int)(fVar27 + (float)puVar16) & 0x1fff];
											fVar29 = -fVar31;
											fVar27 = -(fVar28 * 0.5) + fVar31 * 0.5 + 0.5;
											fVar30 = (-(fVar31 * 0.5) - fVar28 * 0.5) + 0.5;
											local_28c = pfVar23[1] * fVar29 + *pfVar23 * fVar28 + fVar27 + 0.0;
											local_288 = pfVar23[1] * fVar28 + *pfVar23 * fVar31 + fVar30 + 0.0;
											local_284 = pfVar23[3] * fVar29 + pfVar23[2] * fVar28 + fVar27 + 0.0;
											local_280 = pfVar23[3] * fVar28 + pfVar23[2] * fVar31 + fVar30 + 0.0;
											local_27c = pfVar23[5] * fVar29 + pfVar23[4] * fVar28 + fVar27 + 0.0;
											local_278 = pfVar23[5] * fVar28 + pfVar23[4] * fVar31 + fVar30 + 0.0;
											local_274 = pfVar23[7] * fVar29 + pfVar23[6] * fVar28 + fVar27 + 0.0;
											local_270 = pfVar23[7] * fVar28 + pfVar23[6] * fVar31 + fVar30 + 0.0;
											uVar26 = p_Var25->field_0x8;
											local_264 = (((uVar26 >> 0x18) * (uint) * (byte*)&p_Var25->field_0xc >> 8) * local_268 >> 8) <<
												0x18;
											local_264 = local_264 |
												CONCAT12((char)((uVar26 >> 0x10 & 0xff) * (uint) * (byte*)&p_Var25->field_0x12 >> 8),
													CONCAT11((char)((uVar26 >> 8 & 0xff) * (uint)p_Var25->field_0xf >> 8),
														(char)((uVar26 & 0xff) * (uint) * (byte*)&p_Var25->field_0xe >> 8)));
											local_238 = DAT_00448598 * p_Var25->field_0x24 * (float)(uint)p_Var25->field_0x4 * 0.5;
											local_23c = local_238 * local_240;
											FUN_002d1520(local_23c, local_238, &local_28c, &local_284, &local_27c, &local_274, &local_264,
												(undefined8*)peVar24);
										}
										p_Var25 = p_Var25 + 1;
										peVar24 = peVar24 + 1;
									}
								}
							}
							else {
								p_Var25 = pParticle + param_7;
								peVar24 = param_6 + param_7;
								for (; param_7 < param_8; param_7 = param_7 + 1) {
									if (p_Var25->field_0xd != 0) {
										uVar26 = *(uint*)&p_Var25->field_0x3c;
										fVar27 = pDrawData->field_0x94 * (p_Var25->field_0x1c / p_Var25->field_0x20);
										if ((uVar26 & 0x20) != 0) {
											fVar27 = -fVar27;
										}
										fVar27 = pDrawData->field_0x90 + fVar27;
										fVar28 = fVar27 * g_ScalingFactor_00448518;
										puVar16 = (undefined*)0x3f000000;
										if (fVar28 < 0.0) {
											puVar16 = &DAT_bf000000;
										}
										fVar27 = fVar27 * g_ScalingFactor_00448518;
										local_304 = edFCosinus[(int)(fVar28 + (float)puVar16) & 0x1fff];
										if (fVar27 < 0.0) {
											puVar16 = &DAT_bf000000;
										}
										else {
											puVar16 = (undefined*)0x3f000000;
										}
										local_30c = edFSinus[(int)(fVar27 + (float)puVar16) & 0x1fff];
										fVar28 = -(local_304 * 0.5) + local_30c * 0.5 + 0.5;
										local_2fc = (-(local_30c * 0.5) - local_304 * 0.5) + 0.5;
										fVar27 = -local_30c;
										local_310 = local_304;
										local_308 = fVar27;
										local_300 = fVar28;
										if ((uVar26 & 0x40) != 0) {
											local_310 = local_30c * FLOAT_ARRAY_0041e810[2] + local_304 * FLOAT_ARRAY_0041e810[0];
											local_308 = local_304 * FLOAT_ARRAY_0041e810[2] + fVar27 * FLOAT_ARRAY_0041e810[0];
											local_30c = local_30c * FLOAT_ARRAY_0041e810[3] + local_304 * FLOAT_ARRAY_0041e810[1];
											local_304 = local_304 * FLOAT_ARRAY_0041e810[3] + fVar27 * FLOAT_ARRAY_0041e810[1];
											local_300 = local_2fc * FLOAT_ARRAY_0041e810[2] +
												fVar28 * FLOAT_ARRAY_0041e810[0] + FLOAT_ARRAY_0041e810[4] + 0.0;
											local_2fc = local_2fc * FLOAT_ARRAY_0041e810[3] +
												fVar28 * FLOAT_ARRAY_0041e810[1] + FLOAT_ARRAY_0041e810[5] + 0.0;
										}
										if ((uVar26 & 0x80) != 0) {
											fVar27 = local_30c * FLOAT_ARRAY_0041e830[2];
											fVar28 = local_304 * FLOAT_ARRAY_0041e830[2];
											local_30c = local_30c * FLOAT_ARRAY_0041e830[3] + local_310 * FLOAT_ARRAY_0041e830[1];
											local_304 = local_304 * FLOAT_ARRAY_0041e830[3] + local_308 * FLOAT_ARRAY_0041e830[1];
											fVar29 = local_2fc * FLOAT_ARRAY_0041e830[2];
											local_2fc = local_2fc * FLOAT_ARRAY_0041e830[3] +
												local_300 * FLOAT_ARRAY_0041e830[1] + FLOAT_ARRAY_0041e830[5] + 0.0;
											local_310 = fVar27 + local_310 * FLOAT_ARRAY_0041e830[0];
											local_308 = fVar28 + local_308 * FLOAT_ARRAY_0041e830[0];
											local_300 = fVar29 + local_300 * FLOAT_ARRAY_0041e830[0] + FLOAT_ARRAY_0041e830[4] + 0.0;
										}
										local_28c = pfVar23[1] * local_308 + *pfVar23 * local_310 + local_300 + 0.0;
										local_288 = pfVar23[1] * local_304 + *pfVar23 * local_30c + local_2fc + 0.0;
										local_284 = pfVar23[3] * local_308 + pfVar23[2] * local_310 + local_300 + 0.0;
										local_280 = pfVar23[3] * local_304 + pfVar23[2] * local_30c + local_2fc + 0.0;
										local_27c = pfVar23[5] * local_308 + pfVar23[4] * local_310 + local_300 + 0.0;
										local_278 = pfVar23[5] * local_304 + pfVar23[4] * local_30c + local_2fc + 0.0;
										local_274 = pfVar23[7] * local_308 + pfVar23[6] * local_310 + local_300 + 0.0;
										local_270 = pfVar23[7] * local_304 + pfVar23[6] * local_30c + local_2fc + 0.0;
										uVar26 = p_Var25->field_0x8;
										local_264 = (((uVar26 >> 0x18) * (uint) * (byte*)&p_Var25->field_0xc >> 8) * local_268 >> 8) << 0x18;
										local_264 = local_264 |
											CONCAT12((char)((uVar26 >> 0x10 & 0xff) * (uint) * (byte*)&p_Var25->field_0x12 >> 8),
												CONCAT11((char)((uVar26 >> 8 & 0xff) * (uint)p_Var25->field_0xf >> 8),
													(char)((uVar26 & 0xff) * (uint) * (byte*)&p_Var25->field_0xe >> 8)));
										local_238 = DAT_00448598 * p_Var25->field_0x24 * (float)(uint)p_Var25->field_0x4 * 0.5;
										local_23c = local_238 * local_240;
										FUN_002d1520(local_23c, local_238, &local_28c, &local_284, &local_27c, &local_274, &local_264,
											(undefined8*)peVar24);
									}
									p_Var25 = p_Var25 + 1;
									peVar24 = peVar24 + 1;
								}
							}
						}
						else {
							p_Var25 = pParticle + param_7;
							peVar24 = param_6 + param_7;
							for (; param_7 < param_8; param_7 = param_7 + 1) {
								if (p_Var25->field_0xd != 0) {
									uVar26 = *(uint*)&p_Var25->field_0x3c;
									fVar27 = pDrawData->field_0x94 * (p_Var25->field_0x1c / p_Var25->field_0x20);
									if ((uVar26 & 0x20) != 0) {
										fVar27 = -fVar27;
									}
									fVar27 = ((float)(uVar26 & 0xffff) * pDrawData->field_0x90) / 65535.0 + fVar27;
									fVar28 = fVar27 * g_ScalingFactor_00448518;
									puVar16 = (undefined*)0x3f000000;
									if (fVar28 < 0.0) {
										puVar16 = &DAT_bf000000;
									}
									fVar27 = fVar27 * g_ScalingFactor_00448518;
									local_2e4 = edFCosinus[(int)(fVar28 + (float)puVar16) & 0x1fff];
									if (fVar27 < 0.0) {
										puVar16 = &DAT_bf000000;
									}
									else {
										puVar16 = (undefined*)0x3f000000;
									}
									local_2ec = edFSinus[(int)(fVar27 + (float)puVar16) & 0x1fff];
									fVar28 = -(local_2e4 * 0.5) + local_2ec * 0.5 + 0.5;
									local_2dc = (-(local_2ec * 0.5) - local_2e4 * 0.5) + 0.5;
									fVar27 = -local_2ec;
									local_2f0 = local_2e4;
									local_2e8 = fVar27;
									local_2e0 = fVar28;
									if ((uVar26 & 0x40) != 0) {
										local_2f0 = local_2ec * FLOAT_ARRAY_0041e7d0[2] + local_2e4 * FLOAT_ARRAY_0041e7d0[0];
										local_2e8 = local_2e4 * FLOAT_ARRAY_0041e7d0[2] + fVar27 * FLOAT_ARRAY_0041e7d0[0];
										local_2ec = local_2ec * FLOAT_ARRAY_0041e7d0[3] + local_2e4 * FLOAT_ARRAY_0041e7d0[1];
										local_2e4 = local_2e4 * FLOAT_ARRAY_0041e7d0[3] + fVar27 * FLOAT_ARRAY_0041e7d0[1];
										local_2e0 = local_2dc * FLOAT_ARRAY_0041e7d0[2] +
											fVar28 * FLOAT_ARRAY_0041e7d0[0] + FLOAT_ARRAY_0041e7d0[4] + 0.0;
										local_2dc = local_2dc * FLOAT_ARRAY_0041e7d0[3] +
											fVar28 * FLOAT_ARRAY_0041e7d0[1] + FLOAT_ARRAY_0041e7d0[5] + 0.0;
									}
									if ((uVar26 & 0x80) != 0) {
										fVar27 = local_2ec * FLOAT_ARRAY_0041e7f0[2];
										fVar28 = local_2e4 * FLOAT_ARRAY_0041e7f0[2];
										local_2ec = local_2ec * FLOAT_ARRAY_0041e7f0[3] + local_2f0 * FLOAT_ARRAY_0041e7f0[1];
										local_2e4 = local_2e4 * FLOAT_ARRAY_0041e7f0[3] + local_2e8 * FLOAT_ARRAY_0041e7f0[1];
										fVar29 = local_2dc * FLOAT_ARRAY_0041e7f0[2];
										local_2dc = local_2dc * FLOAT_ARRAY_0041e7f0[3] +
											local_2e0 * FLOAT_ARRAY_0041e7f0[1] + FLOAT_ARRAY_0041e7f0[5] + 0.0;
										local_2f0 = fVar27 + local_2f0 * FLOAT_ARRAY_0041e7f0[0];
										local_2e8 = fVar28 + local_2e8 * FLOAT_ARRAY_0041e7f0[0];
										local_2e0 = fVar29 + local_2e0 * FLOAT_ARRAY_0041e7f0[0] + FLOAT_ARRAY_0041e7f0[4] + 0.0;
									}
									local_28c = pfVar23[1] * local_2e8 + *pfVar23 * local_2f0 + local_2e0 + 0.0;
									local_288 = pfVar23[1] * local_2e4 + *pfVar23 * local_2ec + local_2dc + 0.0;
									local_284 = pfVar23[3] * local_2e8 + pfVar23[2] * local_2f0 + local_2e0 + 0.0;
									local_280 = pfVar23[3] * local_2e4 + pfVar23[2] * local_2ec + local_2dc + 0.0;
									local_27c = pfVar23[5] * local_2e8 + pfVar23[4] * local_2f0 + local_2e0 + 0.0;
									local_278 = pfVar23[5] * local_2e4 + pfVar23[4] * local_2ec + local_2dc + 0.0;
									local_274 = pfVar23[7] * local_2e8 + pfVar23[6] * local_2f0 + local_2e0 + 0.0;
									local_270 = pfVar23[7] * local_2e4 + pfVar23[6] * local_2ec + local_2dc + 0.0;
									uVar26 = p_Var25->field_0x8;
									local_264 = (((uVar26 >> 0x18) * (uint) * (byte*)&p_Var25->field_0xc >> 8) * local_268 >> 8) << 0x18;
									local_264 = local_264 |
										CONCAT12((char)((uVar26 >> 0x10 & 0xff) * (uint) * (byte*)&p_Var25->field_0x12 >> 8),
											CONCAT11((char)((uVar26 >> 8 & 0xff) * (uint)p_Var25->field_0xf >> 8),
												(char)((uVar26 & 0xff) * (uint) * (byte*)&p_Var25->field_0xe >> 8)));
									local_238 = DAT_00448598 * p_Var25->field_0x24 * (float)(uint)p_Var25->field_0x4 * 0.5;
									local_23c = local_238 * local_240;
									FUN_002d1520(local_23c, local_238, &local_28c, &local_284, &local_27c, &local_274, &local_264,
										(undefined8*)peVar24);
								}
								p_Var25 = p_Var25 + 1;
								peVar24 = peVar24 + 1;
							}
						}
						edDListEnd();
						if (pDrawData->field_0x124[0] != (DisplayList*)0x0) {
							edDlistAddtoView(pDrawData->field_0x124[0]);
						})
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
								p_Var25 = pParticle + param_7;
								peVar24 = param_6 + param_7;
								for (; param_7 < (int)uVar26; param_7 = param_7 + 1) {
									if (p_Var25->field_0xd == 0) {
										local_94 = 0;
										local_98 = 0;
										iVar15 = param_7;
										if (0x48 < (uint)(param_7 << 2)) {
											iVar15 = param_7 + ((param_7 << 2) / 0x48) * 2;
										}
										puVar20 = (undefined2*)(local_88 + iVar15 * 4);
										*puVar20 = 0;
										puVar20[1] = 0;
									}
									else {
										if (pDrawData->field_0x6f != 0) {
											local_90 = p_Var25->field_0x24;
											iVar15 = param_7;
											if (0x48 < (uint)(param_7 << 2)) {
												iVar15 = param_7 + ((param_7 << 2) / 0x48) * 2;
											}
											puVar20 = (undefined2*)(local_88 + iVar15 * 4);
											*puVar20 = (short)(int)(local_90 * 4096.0);
											puVar20[1] = (short)(int)(local_90 * 4096.0);
											local_8c = local_90;
										}
										if (pDrawData->field_0x6e != 0) {
											puVar17 = (uint*)(local_80 + param_7 * 0x10);
											*puVar17 = p_Var25->field_0x8;
											puVar17[1] = p_Var25->field_0x8;
											puVar17[2] = p_Var25->field_0x8;
											puVar17[3] = p_Var25->field_0x8;
										}
										if (pDrawData->field_0x6c != 0) {
											fVar28 = peVar24->y;
											fVar27 = peVar24->z;
											pfVar23 = (float*)(local_7c + param_7 * 0x40);
											*pfVar23 = peVar24->x;
											pfVar23[1] = fVar28;
											pfVar23[2] = fVar27;
										}
									}
									p_Var25 = p_Var25 + 1;
									peVar24 = peVar24 + 1;
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
									edDListBegin(0.0, 0.0, 0.0, 0xb, pGroup->field_0x10);
									for (iVar10 = 0; iVar10 < pGroup->field_0x10; iVar10 = iVar10 + 1) {
										edDListColor4u8((pDrawData->field_0x50).r, (pDrawData->field_0x50).g, (pDrawData->field_0x50).b,
											(pDrawData->field_0x50).a);
										edDListTexCoo2f(pDrawData->field_0x54, pDrawData->field_0x58);
										edDListTexCoo2f(pDrawData->field_0x5c, pDrawData->field_0x60);
										edDListWidthHeight2f(pDrawData->field_0x64, pDrawData->field_0x68);
										edDListVertex4f(0.0, 0.0, 0.0, 0.0);
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
	Particle_0x1a0* pDrawData;
	uint uVar2;
	uint uVar4;
	int iVar5;
	OffsetPointer<Particle_0x1a0*>* ppPVar6;
	int iVar7;
	_ed_particle_group* pGroup;

	if (g_particle_system_config.bProfile != 0) {
		edProfileBegin(gPartProfIdShapers);
	}

	pGroup = p_current_particle_manager->aGroups.pData;
	for (iVar7 = 0; iVar7 < p_current_particle_manager->nbGroups; iVar7 = iVar7 + 1) {
		if (pGroup->field_0x0 != 0) {
			int iVar2 = pGroup->field_0x10;
			short sVar1 = pGroup->pParticle.pData->field_0x6;
			pGroup->field_0x70 = (Particle_0x1a0*)0x0;
			ppPVar6 = pGroup->field_0x48.pData;
			for (iVar5 = 0; iVar5 < pGroup->field_0x40; iVar5 = iVar5 + 1) {
				pDrawData = ppPVar6->pData;
				if ((pDrawData != (Particle_0x1a0*)0x0) && (pDrawData->field_0x2 != 0)) {
					pGroup->field_0x70 = pDrawData;
					edPartDrawShaper(time, pGroup, pDrawData, p_current_particle_manager->aParticles.pData,
						p_current_particle_manager->aParticleVectors.pData, p_current_particle_manager->field_0x2c.pData,
						(int)sVar1, sVar1 + iVar2);
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
