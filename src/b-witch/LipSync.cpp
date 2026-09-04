#include "Lipsync.h"
#include "MemoryStream.h"

CLipTrackManager::CLipTrackManager()
{
	this->nbLipTracks = 0;

	for (int i = 0; i < 0x80; ++i) {
		this->aLipTracks[i] = nullptr;
	}

	return;
}

void CLipTrackManager::Level_Init()
{
	return;
}

void CLipTrackManager::Level_Term()
{
	return;
}

void CLipTrackManager::Level_ClearAll()
{
	CKFrameTrackReader* pTrack;
	CKFrameTrackReader** ppTrack;
	int iVar3;

	iVar3 = 0;
	ppTrack = this->aLipTracks;
	do {
		pTrack = *ppTrack;
		if (pTrack != (CKFrameTrackReader*)0x0) {
			delete pTrack;
			*ppTrack = (CKFrameTrackReader*)0x0;
		}

		iVar3 = iVar3 + 1;
		ppTrack = ppTrack + 1;
	} while (iVar3 < 0x80);

	this->nbLipTracks = 0;

	return;
}

void CLipTrackManager::Level_Manage()
{
	return;
}

void CLipTrackManager::Level_ManagePaused()
{
	return;
}

int CLipTrackManager::InstallFromMem(char* pFileData)
{
	CKFrameTrackReader* pCVar1;
	CKFrameTrackReader** pCVar2;
	int iVar3;

	ByteCode BStack16;
	BStack16.Init(pFileData);

	iVar3 = 0;
	pCVar2 = this->aLipTracks;
	do {
		if (*pCVar2 == (CKFrameTrackReader*)0x0) {
			pCVar1 = new CKFrameTrackReader;

			this->aLipTracks[iVar3] = pCVar1;
			this->aLipTracks[iVar3]->Create(&BStack16);
			this->nbLipTracks = this->nbLipTracks + 1;
			break;
		}

		iVar3 = iVar3 + 1;
		pCVar2 = pCVar2 + 1;
	} while (iVar3 < 0x80);

	BStack16.Term();

	return iVar3;
}

void CKFrameTrackReader::Create(ByteCode* pByteCode)
{
	int* piVar1;
	int iVar2;
	CLipTrack** ppCVar3;
	int iVar4;
	int iVar5;
	undefined4 uVar6;
	long lVar7;
	CKFrameTrackReader* pCVar8;
	int iVar9;
	CLipTrack* pNewTrack;
	float fVar10;

	pByteCode->GetS32();
	pByteCode->GetS32();

	this->nbTracks = pByteCode->GetS32();
	this->field_0xc = pByteCode->GetF32();
	this->aTracks = new CLipTrack*[this->nbTracks];

	iVar2 = 0;
	if (0 < this->nbTracks) {
		iVar9 = 0;
		do {
			iVar4 = pByteCode->GetS32();
			switch (iVar4) {
			case 0:
				pNewTrack = new CTrackMultiChannel;
				break;
			case 1:
				pNewTrack = new CTrackMonoChannel;
				break;
			case 2:
				pNewTrack = new CTrackVector;
				break;
			case 3:
				pNewTrack = new CTrackRndTickMonoChannel;
				break;
			case 4:
				pNewTrack = new CTrackRndSwitchMonoChannel;
				break;
			case 5:
				pNewTrack = new CTrackRndImpulseMonoChannel;
				break;
			case 6:
				pNewTrack = new CTrackRndCos;
				break;
			case 7:
				pNewTrack = new CTrackRndVarMonoChannel;
			}

			pNewTrack->Create(pByteCode);

			this->aTracks[iVar2] = pNewTrack;
			iVar2 = iVar2 + 1;
			iVar9 = iVar9 + 4;
		} while (iVar2 < this->nbTracks);
	}

	for (int i = 0; i < 18; ++i) {
		this->field_0x14[i] = (float*)0x0;
	}

	iVar2 = 0;
	if (0 < this->nbTracks) {
		do {
			iVar4 = 0;
			lVar7 = this->aTracks[iVar2]->GetOutputWidth();
			if (0 < lVar7) {
				do {
					iVar5 = this->aTracks[iVar2]->GetOutputType(iVar4);
					assert(iVar5 < 18);
					this->field_0x14[iVar5] = this->aTracks[iVar2]->GetOutputPtr(iVar4);
					iVar4 = iVar4 + 1;
					iVar5 = this->aTracks[iVar2]->GetOutputWidth();
				} while (iVar4 < iVar5);
			}

			iVar2 = iVar2 + 1;
		} while (iVar2 < this->nbTracks);
	}

	return;
}

void CKFrameTrackReader::Init()
{
	this->field_0x4 = 0.0f;

	return;
}

void CKFrameTrackReader::Manage(float param_1)
{
	int iVar2;
	float fVar3;

	iVar2 = 0;
	fVar3 = this->field_0x4;
	if (0 < this->nbTracks) {
		do {
			this->aTracks[iVar2]->UpdateKFrame(param_1 - fVar3);
			iVar2 = iVar2 + 1;
		} while (iVar2 < this->nbTracks);
	}

	return;
}

float CKFrameTrackReader::GetValue(int index)
{
	float fVar1;

	if (this->field_0x14[index] == (float*)0x0) {
		fVar1 = 0.0f;
	}
	else {
		fVar1 = *this->field_0x14[index];
	}

	return fVar1;
}

void CLipDataTrack::Create(ByteCode* pByteCode)
{
	this->field_0x8 = pByteCode->GetS32();

	return;
}

void CTrackMultiChannel::Create(ByteCode* pByteCode)
{
	int iVar1;
	int* piVar2;
	float** ppfVar3;
	int iVar4;
	float* pfVar5;
	CTrackMultiSubObj* pCVar6;
	int iVar7;
	int iVar8;
	float fVar9;
	float fVar10;

	this->outputWidth = pByteCode->GetS32();
	this->aOutputTypes = new int[this->outputWidth];
	this->aOutputPtrs = new float[this->outputWidth];

	iVar1 = 0;
	if (0 < this->outputWidth) {
		do {
			this->aOutputTypes[iVar1] = pByteCode->GetS32();
			iVar1 = iVar1 + 1;
		} while (iVar1 < this->outputWidth);
	}

	this->field_0x8 = pByteCode->GetS32();
	this->field_0x4 = new float[this->field_0x8];
	this->aSubObj = new CTrackMultiSubObj[this->field_0x8];

	iVar1 = 0;
	if (0 < this->field_0x8) {
		do {
			iVar7 = pByteCode->GetS32();
			fVar9 = pByteCode->GetF32();
			fVar10 = pByteCode->GetF32();
			this->field_0x4[iVar1] = fVar10;
			this->aSubObj[iVar1].field_0x0 = iVar7;
			this->aSubObj[iVar1].field_0x4 = fVar9;
			iVar1 = iVar1 + 1;
		} while (iVar1 < this->field_0x8);
	}

	iVar1 = 0;
	if (0 < this->outputWidth) {
		do {
			this->aOutputPtrs[iVar1] = 0.0f;
			iVar1 = iVar1 + 1;
		} while (iVar1 < this->outputWidth);
	}

	return;
}

int CTrackMultiChannel::UpdateKFrame(float param_1)
{
	int iVar1;
	int iVar2;
	int* piVar3;
	int iVar4;
	float* pfVar5;
	CTrackMultiSubObj* pCVar6;
	int iVar7;
	int iVar8;
	float fVar9;
	float pfVar9;
	float fVar10;
	float fVar11;

	iVar8 = this->field_0x8;
	iVar1 = 0;
	if (0 < iVar8 + -1) {
		do {
			iVar2 = iVar1 + iVar8;
			if (iVar2 < 0) {
				iVar2 = iVar2 + 1;
			}

			iVar2 = iVar2 >> 1;
			iVar4 = iVar2;
			if (this->field_0x4[iVar2] <= param_1) {
				iVar4 = iVar8;
				iVar1 = iVar2;
			}

			iVar8 = iVar4;
		} while (iVar1 < iVar8 + -1);
	}

	iVar8 = this->outputWidth;
	iVar2 = 0;
	if (0 < iVar8) {
		iVar4 = 0;
		do {
			iVar2 = iVar2 + 1;
			this->aOutputPtrs[iVar2] = 0.0f;
			iVar8 = this->outputWidth;
			iVar4 = iVar4 + 4;
		} while (iVar2 < iVar8);
	}

	pfVar5 = this->field_0x4 + iVar1;
	if (*pfVar5 <= param_1) {
		iVar2 = this->field_0x8;
		iVar4 = iVar2 + -1;
		if ((iVar2 == 1) || (iVar1 == iVar4)) {
			iVar2 = 0;

			if (0 < iVar8) {
				piVar3 = this->aOutputTypes;
				do {
					if (this->aSubObj[iVar1].field_0x0 == *piVar3) {
						this->aOutputPtrs[iVar2] = this->aSubObj[iVar1].field_0x4;
						break;
					}

					iVar2 = iVar2 + 1;
					piVar3 = piVar3 + 1;
				} while (iVar2 < iVar8);
			}

			iVar1 = -1;
		}
		else {
			if (iVar1 < iVar4) {
				pfVar9 = 100.0f;
				pCVar6 = this->aSubObj + iVar1;
				iVar2 = pCVar6->field_0x0;
				fVar11 = (pfVar5[1] - param_1) / (pfVar5[1] - *pfVar5);
				fVar10 = (1.0f - fVar11) * this->aSubObj[iVar1 + 1].field_0x4;
				fVar11 = fVar11 * pCVar6->field_0x4;
				fVar9 = fVar11 + fVar10;
				iVar4 = pCVar6[1].field_0x0;
				if (fVar9 < 100.0f) {
					pfVar9 = fVar9;
				}
				if (iVar2 == iVar4) {
					iVar4 = 0;
					if (0 < iVar8) {
						piVar3 = this->aOutputTypes;
						do {
							if (iVar2 == *piVar3) {
								this->aOutputPtrs[iVar4] = pfVar9;
								return iVar1;
							}
							iVar4 = iVar4 + 1;
							piVar3 = piVar3 + 1;
						} while (iVar4 < iVar8);
					}
				}
				else {
					iVar7 = 0;
					if (0 < iVar8) {
						piVar3 = this->aOutputTypes;
						do {
							if (iVar2 == *piVar3) {
								this->aOutputPtrs[iVar7] = fVar11;
								break;
							}
							iVar7 = iVar7 + 1;
							piVar3 = piVar3 + 1;
						} while (iVar7 < iVar8);
					}

					iVar8 = 0;
					if (0 < this->outputWidth) {
						piVar3 = this->aOutputTypes;
						do {
							if (iVar4 == *piVar3) {
								this->aOutputPtrs[iVar8] = fVar10;
								return iVar1;
							}

							iVar8 = iVar8 + 1;
							piVar3 = piVar3 + 1;
						} while (iVar8 < this->outputWidth);
					}
				}
			}
		}
	}

	return iVar1;
}

int CTrackMultiChannel::GetOutputWidth()
{
	return this->outputWidth;
}

int CTrackMultiChannel::GetOutputType(int index)
{
	return this->aOutputTypes[index];
}

float* CTrackMultiChannel::GetOutputPtr(int index)
{
	return this->aOutputPtrs + index;
}

