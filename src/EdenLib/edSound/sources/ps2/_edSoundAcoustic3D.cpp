#include "edSound/ps2/_edSoundAcoustic3D.h"
#include "edSound/edSoundPlay.h"
#include "edSound/edSoundInstance.h"
#include "MathOps.h"
#ifdef PLATFORM_WIN
#include "log.h"
#endif

float edSoundAcousticVolumeAttenuation(edsound_3d_data* pData, edF32VECTOR3* pPosition)
{
	float z;
	float y;
	float x;

	x = (pData->position).y - pPosition->y;
	y = (pData->position).x - pPosition->x;
	z = (pData->position).z - pPosition->z;
	y = (z * z + y * y + x * x) * edSoundGlobalParams.field_0x64;
	z = 1.0f;
	if (pData->field_0x1c < y) {
		z = pData->field_0x1c / y;
	}

	return z;
}

float _edSoundAcousticDopplerShift(float param_1, float param_2)
{
	float fVar1;

	fVar1 = param_2 - param_1;
	if (fVar1 == 0.0f) {
		fVar1 = 1e-20f;
	}

	return fabsf(param_2 / fVar1);
}

float edSoundAcousticDopplerShift(edsound_3d_data* pData, edF32VECTOR3* pPosition, edF32VECTOR3* pRotation)
{
	float fVar1;
	float fVar2;
	edF32VECTOR3 rotDelta;
	edF32VECTOR3 posDelta;

	rotDelta.x = pRotation->x - (pData->rotation).x;
	rotDelta.y = pRotation->y - (pData->rotation).y;
	rotDelta.z = pRotation->z - (pData->rotation).z;
	posDelta.x = (pData->position).x - pPosition->x;
	posDelta.y = (pData->position).y - pPosition->y;
	posDelta.z = (pData->position).z - pPosition->z;
	fVar1 = edF32Vector3DotProductSoft(&rotDelta, &posDelta);
	fVar2 = edF32Vector3GetLengthSoft(&posDelta);
	if (fVar2 == 0.0f) {
		fVar1 = edF32Vector3GetLengthSoft(&rotDelta);
		fVar1 = -fVar1;
	}
	else {
		fVar1 = fVar1 / fVar2;
	}

	fVar1 = _edSoundAcousticDopplerShift(fVar1, edSoundGlobalParams.field_0x5c);

	return fVar1;
}

void _edSoundAcousticCompute(ed_sound_instance* pInstance)
{
	edsound_3d_data* pData;
	byte bVar1;
	uint uVar2;
	ed_sound_instance* peVar3;
	float fVar4;
	float fVar5;
	float fVar6;
	float puVar7;
	float puVar8;
	float puVar11;
	float puVar10;
	float puVar9;
	float puVar12;
	float local_f12_21304;
#ifdef PLATFORM_WIN
	float in_f21 = 0.0f;
	float unaff_f20 = 0.0f;
#else
	float in_f21;
	float unaff_f20;
#endif
	float fVar7;
	edF32VECTOR3 local_40;
	edF32VECTOR3 eStack48;
	edF32VECTOR3 listenerSpacePosition;
	edF32VECTOR3 listenerSpaceDirection;
	uint local_20;
	uint local_1c;
	uint local_18;
	uint local_14;
	uint local_10;
	uint local_c;
	uint local_8;
	uint local_4;

	if (edSoundGlobalParams.field_0x6c != (void*)0x0) {
		IMPLEMENTATION_GUARD(
		edSound_GetFirstSpatializedInstance();
		(*edSoundGlobalParams.field_0x6c)();)
	}

	if (edSoundGlobalParams.outputMode != MONO) {
#ifdef PLATFORM_WIN
		if (edF32Vector3GetLengthSoft(&edSoundGlobalParams.field_0x30) > 0.0f)
			edF32Vector3NormalizeSoft(&edSoundGlobalParams.field_0x30, &edSoundGlobalParams.field_0x30);
		else edSoundGlobalParams.field_0x30 = {0.0f, 0.0f, 1.0f};
		if (edF32Vector3GetLengthSoft(&edSoundGlobalParams.field_0x3c) > 0.0f)
			edF32Vector3NormalizeSoft(&edSoundGlobalParams.field_0x3c, &edSoundGlobalParams.field_0x3c);
		else edSoundGlobalParams.field_0x3c = {0.0f, 1.0f, 0.0f};
#else
		edF32Vector3NormalizeSoft(&edSoundGlobalParams.field_0x30, &edSoundGlobalParams.field_0x30);
		edF32Vector3NormalizeSoft(&edSoundGlobalParams.field_0x3c, &edSoundGlobalParams.field_0x3c);
#endif
		edF32Vector3CrossProductSoft(&eStack48, &edSoundGlobalParams.field_0x30, &edSoundGlobalParams.field_0x3c);
	}

	while (peVar3 = pInstance, peVar3 != (ed_sound_instance*)0x0) {
		pInstance = peVar3->lowerPrioritySoundInstance;
#ifdef PLATFORM_WIN
		if ((edSoundInstanceCom[peVar3->fullSoundInstanceId & 0xffff].flags & 2) != 0) {
			AUDIO_INSTANCE_LOG(LogLevel::Info,
				"acoustic-start id=0x{:08x} flags=0x{:x} next=0x{:08x} volume={} frequency={}",
				peVar3->fullSoundInstanceId, peVar3->flags, pInstance ? pInstance->fullSoundInstanceId : 0u,
				peVar3->volume, peVar3->frequency);
			if ((peVar3->flags & 0x1000) != 0 && peVar3->p3dData) {
				AUDIO_INSTANCE_LOG(LogLevel::Info,
					"spatial-start id=0x{:08x} source=({},{},{}) listener=({},{},{}) radius={} radiusSquared={} distanceScale={}",
					peVar3->fullSoundInstanceId, peVar3->p3dData->position.x, peVar3->p3dData->position.y, peVar3->p3dData->position.z,
					edSoundGlobalParams.field_0x24.x, edSoundGlobalParams.field_0x24.y, edSoundGlobalParams.field_0x24.z,
					peVar3->p3dData->field_0x18, peVar3->p3dData->field_0x1c, edSoundGlobalParams.field_0x64);
			}
		}
#endif
		if ((peVar3->flags & 0x28) == 0) {
			uVar2 = 1;
		}
		else {
			uVar2 = 2;
		}

		if ((peVar3->flags & 0x1000) == 0) {
			peVar3->field_0x4c = peVar3->volume;
			peVar3->field_0x50 = peVar3->frequency;
			if ((peVar3->flags & 0x20) == 0) {
				while (uVar2 != 0) {
					fVar4 = peVar3->field_0x68;
					fVar5 = peVar3->field_0x4c;
					// Ghidra's indexed accesses resolve to these float gain fields in the PS2 layout.
					if (uVar2 == 1) {
						peVar3->field_0x88 = fVar5 * sqrtf((1.0f - fVar4) * 0.5f);
						peVar3->field_0x8c = fVar5 * sqrtf((fVar4 + 1.0f) * 0.5f);
					}
					else {
						peVar3->field_0x94 = fVar5 * sqrtf((1.0f - fVar4) * 0.5f);
						peVar3->field_0x98 = fVar5 * sqrtf((fVar4 + 1.0f) * 0.5f);
					}
					local_20 = peVar3->fullSoundInstanceId;
					edSoundInstanceCom[local_20 & 0xffff].flags = edSoundInstanceCom[local_20 & 0xffff].flags | 0x20 << (uVar2 - 1 & 0x1f);
					edSoundInstanceCom[local_20 & 0xffff].soundInstanceId = local_20;
					uVar2 = uVar2 - 1;
				}
			}
			else {
				peVar3->field_0x88 = peVar3->field_0x4c;
				peVar3->field_0x8c = 0.0f;
				peVar3->field_0x94 = 0.0f;
				peVar3->field_0x98 = peVar3->field_0x4c;
				local_18 = peVar3->fullSoundInstanceId;
				edSoundInstanceCom[local_18 & 0xffff].flags = edSoundInstanceCom[local_18 & 0xffff].flags | 0x20;
				edSoundInstanceCom[local_18 & 0xffff].soundInstanceId = local_18;
				local_1c = peVar3->fullSoundInstanceId;
				edSoundInstanceCom[local_1c & 0xffff].flags = edSoundInstanceCom[local_1c & 0xffff].flags | 0x40;
				edSoundInstanceCom[local_1c & 0xffff].soundInstanceId = local_1c;
			}
		}
		else {
			pData = peVar3->p3dData;
			local_40.x = (pData->position).x - edSoundGlobalParams.field_0x24.x;
			local_40.y = (pData->position).y - edSoundGlobalParams.field_0x24.y;
			local_40.z = (pData->position).z - edSoundGlobalParams.field_0x24.z;
			fVar4 = edSoundAcousticVolumeAttenuation(pData, &edSoundGlobalParams.field_0x24);
			fVar4 = peVar3->volume * fVar4;
			peVar3->field_0x4c = fVar4;
			if (1.0f < fVar4) {
				peVar3->field_0x4c = 1.0f;
			}

			fVar4 = edSoundAcousticDopplerShift(pData, &edSoundGlobalParams.field_0x24, &edSoundGlobalParams.field_0x48);
			peVar3->field_0x50 = peVar3->frequency * fVar4;
			local_4 = peVar3->fullSoundInstanceId;
			edSoundInstanceCom[local_4 & 0xffff].flags = edSoundInstanceCom[local_4 & 0xffff].flags | 0x80;
			edSoundInstanceCom[local_4 & 0xffff].soundInstanceId = local_4;
			if (edSoundGlobalParams.outputMode == MONO) {
				fVar4 = peVar3->field_0x4c;
				if (1.0f < fVar4) {
					peVar3->field_0x8c = 1.0f;
					peVar3->field_0x88 = 1.0f;
				}
				else {
					peVar3->field_0x8c = fVar4;
					peVar3->field_0x88 = fVar4;
				}

				if (((uVar2 == 2) && (peVar3->field_0x94 != 0.0f)) && (peVar3->field_0x98 != 0.0f)) {
					peVar3->field_0x94 = 0.0f;
					peVar3->field_0x98 = 0.0f;
					local_10 = peVar3->fullSoundInstanceId;
					edSoundInstanceCom[local_10 & 0xffff].flags = edSoundInstanceCom[local_10 & 0xffff].flags | 0x40;
					edSoundInstanceCom[local_10 & 0xffff].soundInstanceId = local_10;
				}

				local_14 = peVar3->fullSoundInstanceId;
				edSoundInstanceCom[local_14 & 0xffff].flags = edSoundInstanceCom[local_14 & 0xffff].flags | 0x20;
				edSoundInstanceCom[local_14 & 0xffff].soundInstanceId = local_14;
			}
			else {
				listenerSpacePosition.x = edF32Vector3DotProductSoft(&local_40, &eStack48);
				listenerSpacePosition.y = edF32Vector3DotProductSoft(&local_40, &edSoundGlobalParams.field_0x30);
				listenerSpacePosition.z = edF32Vector3DotProductSoft(&local_40, &edSoundGlobalParams.field_0x3c);
				fVar4 = pData->field_0x18;
				if (fVar4 < 0.0f) {
					fVar4 = sqrtf(pData->field_0x1c);
					pData->field_0x18 = fVar4;
				}

				fVar5 = edF32Vector3GetLengthSoft(&listenerSpacePosition);
				if (fVar4 < fVar5) {
					edF32Vector3NormalizeSoft(&listenerSpacePosition, &listenerSpacePosition);
				}
				else
#ifdef PLATFORM_WIN
				if (fVar4 > 0.0f)
#endif
				{
					fVar4 = pData->field_0x18;
					if (fVar4 < 0.0f) {
						fVar4 = sqrtf(pData->field_0x1c);
						pData->field_0x18 = fVar4;
					}
					listenerSpacePosition.x = listenerSpacePosition.x / fVar4;
					fVar4 = pData->field_0x18;
					if (fVar4 < 0.0f) {
						fVar4 = sqrtf(pData->field_0x1c);
						pData->field_0x18 = fVar4;
					}

					listenerSpacePosition.y = listenerSpacePosition.y / fVar4;
					if (pData->field_0x18 < 0.0f) {
						pData->field_0x18 = sqrtf(pData->field_0x1c);
					}

					listenerSpacePosition.z = sqrtf(1.0f - (listenerSpacePosition.y * listenerSpacePosition.y + listenerSpacePosition.x * listenerSpacePosition.x));
				}

				listenerSpaceDirection.z = 0.0f;
				listenerSpaceDirection.x = listenerSpacePosition.x;
				listenerSpaceDirection.y = listenerSpacePosition.y;
#ifdef PLATFORM_WIN
				if (edF32Vector3GetLengthSoft(&listenerSpaceDirection) > 0.0f)
					edF32Vector3NormalizeSoft(&listenerSpaceDirection, &listenerSpaceDirection);
				else listenerSpaceDirection = {0.0f, 1.0f, 0.0f};
#else
				edF32Vector3NormalizeSoft(&listenerSpaceDirection, &listenerSpaceDirection);
#endif
				if (0.0f <= listenerSpaceDirection.y) {
					puVar7 = -listenerSpaceDirection.x;
					if (1.0f < puVar7) {
						puVar8 = 1.0f;
					}
					else {
						puVar8 = -1.0f;
						if (-1.0f <= puVar7) {
							puVar8 = puVar7;
						}
					}
					fVar4 = asinf(puVar8);
					fVar4 = -fVar4;
				}
				else {
					if (0.0f <= listenerSpaceDirection.x) {
						if (1.0f < listenerSpaceDirection.y) {
							puVar11 = 1.0f;
						}
						else {
							puVar11 = -1.0f;
							if (-1.0f <= listenerSpaceDirection.y) {
								puVar11 = listenerSpaceDirection.y;
							}
						}
						fVar4 = acosf(puVar11);
					}
					else {
						if (1.0f < listenerSpaceDirection.y) {
							puVar10 = 1.0f;
						}
						else {
							puVar10 = -1.0f;
							if (-1.0f <= listenerSpaceDirection.y) {
								puVar10 = listenerSpaceDirection.y;
							}
						}
						fVar4 = acosf(puVar10);
						fVar4 = -fVar4;
					}
				}

				if (fVar4 < 0.0f) {
					fVar4 = fVar4 + 6.283185f;
				}

				if (6.283185f <= fVar4) {
					fVar4 = fVar4 - 6.283185f;
				}

				bVar1 = pData->field_0x24;
				fVar5 = cosf((fVar4 + 1.570796f) * 0.5f);
				fVar7 = cosf((fVar4 - 1.570796f) * 0.5f);
				if (((4.712389f <= pData->field_0x20) || (4.712389f <= fVar4)) && ((pData->field_0x20 < 1.570796f || (fVar4 < 1.570796f)))) {
					bVar1 = bVar1 == 0;
				}

				pData->field_0x20 = fVar4;
				pData->field_0x24 = bVar1;
				if (bVar1 != 0) {
					fVar5 = -fVar5;
					fVar7 = -fVar7;
				}

				if (uVar2 == 2) {
					fVar4 = fVar4 + 3.141593f;
					if (bVar1 != 0) {
						fVar4 = fVar4 + 6.283185f;
					}
					puVar9 = sqrtf(listenerSpacePosition.y * listenerSpacePosition.y + listenerSpacePosition.x * listenerSpacePosition.x);
					in_f21 = cosf((fVar4 + 1.570796f) * 0.5f);
					fVar4 = cosf((fVar4 - 1.570796f) * 0.5f);
					if (1.0f < puVar9) {
						puVar12 = 1.0f;
					}
					else {
						puVar12 = -1.0f;
						if (-1.0f <= puVar9) {
							puVar12 = puVar9;
						}
					}
					fVar6 = acosf(puVar12);
					fVar6 = 1.0f - fVar6 / 3.141593f;
					fVar5 = fVar5 * fVar6;
					fVar7 = fVar7 * fVar6;
					in_f21 = in_f21 * (1.0f - fVar6);
					unaff_f20 = fVar4 * (1.0f - fVar6);
				}

				fVar4 = peVar3->field_0x4c;
				in_f21 = in_f21 * fVar4;
				unaff_f20 = unaff_f20 * fVar4;
				if (uVar2 == 2) {
					if (edSoundGlobalParams.outputMode == SURROUND) {
						peVar3->field_0x94 = in_f21;
						peVar3->field_0x98 = unaff_f20;
					}
					else {
						peVar3->field_0x98 = 0.0f;
						peVar3->field_0x94 = 0.0f;
					}

					local_8 = peVar3->fullSoundInstanceId;
					edSoundInstanceCom[local_8 & 0xffff].flags = edSoundInstanceCom[local_8 & 0xffff].flags | 0x40;
					edSoundInstanceCom[local_8 & 0xffff].soundInstanceId = local_8;
				}

				peVar3->field_0x88 = fVar5 * fVar4;
				peVar3->field_0x8c = fVar7 * fVar4;
				local_c = peVar3->fullSoundInstanceId;
				edSoundInstanceCom[local_c & 0xffff].flags = edSoundInstanceCom[local_c & 0xffff].flags | 0x20;
				edSoundInstanceCom[local_c & 0xffff].soundInstanceId = local_c;
			}
		}
	}

	return;
}

void edSoundAcousticCompute(ed_sound_instance* pInstance)
{
	_edSoundAcousticCompute(pInstance);
}
