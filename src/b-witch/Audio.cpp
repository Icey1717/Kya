#include "Audio.h"
#include "WayPoint.h"
#include "MathOps.h"
#include "MemoryStream.h"
#include "edStr.h"
#include "TimeController.h"
#include "edMusic/edMusic.h"
#include "edBank/edBankFile.h"
#include "edBank/edBankBuffer.h"
#include "edFile/edFileNoWaitStack.h"
#include "edFile/edFile.h"
#include "edFile/edFileFiler.h"
#include "EdFileBase.h"
#include "ActorManager.h"
#include "Camera.h"
#include "CameraViewManager.h"
#include "LevelScheduler.h"

int NoAudio = 0;

CAudioManager* g_GlobalSoundPtr_00448e34;
CAudioManager* g_GlobalSoundPtr_00448e38;

CDoubleLinkedNode<PendingSoundPlay> gPendingSoundPlayNodes_00449c50[16];
CDoubleLinkedList<PendingSoundPlay> gFreePendingSoundPlays;

CDoubleLinkedList<SoundSampleEntry> gLoadedSoundSamples;
CDoubleLinkedList<SoundSampleEntry> gQueuedSoundSamples;
edFILEH* gCurrentSoundSampleFile;
CDoubleLinkedNode<SoundSampleEntry>* gCurrentLoadingSoundSample;
void* gSoundSampleFileData_00448e68;
CDoubleLinkedList<SoundSampleEntry> gFreeSoundSamples;

// The Windows edMusic port has no streamed-music backend yet. Keep the
// scheduler's stream state coherent so adding that backend only requires
// replacing these small adapters with its commands.
static void edMusicStreamPlay(int streamIndex, int mode)
{
	if (NoAudio != 0 || pedMusicStreams == nullptr || streamIndex < 0 || streamIndex >= static_cast<int>(edMusicStatus.nbStreams)) {
		return;
	}
	pedMusicStreams[streamIndex].field_0xc = mode == 2 ? 1 : 0;
}

static void edMusicStreamSetSong(int streamIndex, int songIndex)
{
	if (NoAudio == 0 && pedMusicStreams != nullptr && _pedMusicSongs != nullptr && streamIndex >= 0 && streamIndex < static_cast<int>(edMusicStatus.nbStreams)) {
		pedMusicStreams[streamIndex].pSong = _pedMusicSongs + songIndex;
	}
}

static void edMusicStreamSetBank(int streamIndex, int bankIndex)
{
	// Bank selection is submitted to the PS2 IOP in the original engine. The
	// current Windows backend does not own a corresponding stream-bank field.
	(void)streamIndex;
	(void)bankIndex;
}

static void edMusicStreamSetAutoLooping(int streamIndex, bool bAutoLoop)
{
	(void)streamIndex;
	(void)bAutoLoop;
}

static void edMusicStreamSetVolume(int streamIndex, int volume)
{
	if (NoAudio != 0 || pedMusicStreams == nullptr || streamIndex < 0 || streamIndex >= static_cast<int>(edMusicStatus.nbStreams)) {
		return;
	}
	pedMusicStreams[streamIndex].volume = volume;
}

CMusicManager::CMusicManager()
{
	memset(this, 0, sizeof(*this));
}

bool CMusicManager::IsMusic(int handle, CMusic* pMusic)
{
	CMusic* pCVar1;
	bool bVar2;

	bVar2 = false;
	if (handle != -1) {
		pCVar1 = this->aMusicCells[handle].pMusic;
		if ((pCVar1 == (CMusic*)0x0) || (pCVar1 != pMusic)) {
			bVar2 = false;
		}
		else {
			bVar2 = true;
		}
	}

	return bVar2;
}

void CMusicManager::CancelStop(float fadeInTime, int handle)
{
	s_music_cell* pCell = &aMusicCells[handle];

	if (pCell->pMusic != (CMusic*)0x0) {
		uint stateFlags = pCell->stateFlags;
		if ((stateFlags & 1) == 0) {
			if ((stateFlags & 4) == 0) {
				if ((stateFlags & 8) == 0) {
					pCell->transitionDuration = fadeInTime;
					pCell->transitionRemaining = fadeInTime;
					pCell->targetVolume = pCell->initialVolume;
				}
			}
		}
	}

	return;
}

void CMusicManager::Stop(float fadeOutTime, float param_2, int handle)
{
	s_music_cell* pCell = &this->aMusicCells[handle];

	if (pCell->pMusic != (CMusic*)0x0) {
		if ((pCell->stateFlags & 1U) != 0) {
			edDebugPrintf("(warning) : Stop()ing a music currently Start()ing -> cancelling Start()\n");
			pCell->stateFlags = pCell->stateFlags & 0xfffffffe;
		}

		if ((pCell->stateFlags & 4U) != 0) {
			edDebugPrintf("(warning) : Stop()ing a music currently Pause()ing -> cancelling Pause()\n");
			pCell->stateFlags = pCell->stateFlags & 0xfffffffb;
		}

		if ((pCell->stateFlags & 8U) != 0) {
			edDebugPrintf("(warning) : Stop()ing a music currently Resume()ing -> cancelling Resume()\n");
			pCell->stateFlags = pCell->stateFlags & 0xfffffff7;
		}
	}

	pCell->stateFlags |= 2;
	if (fadeOutTime < pCell->transitionRemaining || pCell->transitionRemaining < 0.0f) {
		pCell->transitionRemaining = fadeOutTime;
	}

	pCell->transitionDuration = pCell->transitionRemaining;
	pCell->field_0x3c = param_2;

	return;
}

int CMusicManager::Start(float priority, float initialVolume, float fadeInTime, float fadeOutTime, CMusic* pMusic, uint flags)
{
	if ((flags & 0x80) != 0) {
		for (int handle = 9; handle >= 0; --handle) {
			s_music_cell& cell = aMusicCells[handle];
			if (cell.pMusic != nullptr && cell.priority == priority && (cell.flags & 0x80) != 0) {
				if ((cell.stateFlags & 0x10) != 0 && NoAudio == 0) {
					edMusicStreamPlay(cell.streamIndex, 2);
				}
				cell.pMusic = 0;
				aStreamUsed[cell.streamIndex] = 0;
				break;
			}
		}
	}

	int handle = -1;
	for (int i = 9; i >= 0; --i) {
		if (aMusicCells[i].pMusic == nullptr) {
			handle = i;
			break;
		}
	}
	if (handle == -1) {
		return -1;
	}

	int streamIndex = -1;
	for (int i = 9; i >= 0; --i) {
		if (aStreamUsed[i] == 0) {
			streamIndex = i;
			break;
		}
	}
	if (streamIndex == -1) {
		return -1;
	}

	s_music_cell& cell = aMusicCells[handle];
	memset(&cell, 0, sizeof(cell));
	cell.pMusic = pMusic;
	cell.priority = priority;
	cell.flags = flags;
	cell.volume = initialVolume;
	cell.targetVolume = initialVolume;
	cell.initialVolume = initialVolume;
	cell.gain = 1.0f;
	cell.targetGain = 1.0f;
	cell.initialGain = 1.0f;
	cell.stopDuration = fadeOutTime;
	cell.stopRemaining = fadeOutTime;
	cell.streamIndex = streamIndex;
	aStreamUsed[streamIndex] = 1;

	if (NoAudio == 0) {
		edMusicStreamSetSong(streamIndex, pMusic->songIndex);
		edMusicStreamSetBank(streamIndex, pMusic->bankIndex);
		edMusicStreamSetAutoLooping(streamIndex, (pMusic->flags & 1) != 0);
	}

	// The original Start(handle) marks the cell as starting and uses fadeInTime
	// as its transition timer.
	cell.stateFlags = 1;
	cell.transitionDuration = fadeInTime;
	cell.transitionRemaining = fadeInTime;
	return handle;
}

uint CMusicManager::_ManageCell(float param_1, float param_2, CAudioManager* pAudioManager, CMusicManager::s_music_cell* pMusicCell, int* pStart, int* pStop, int* pClearCell, int* pResume,
	int* pForceStop, float* pGain, float* pGainSource)
{
	CMusic* pCVar1;
	int streamIndex;
	bool bVar2;
	uint uVar3;
	uint uVar4;
	float fVar5;
	float fVar6;
	float fVar7;

	uVar4 = 0;
	fVar7 = *pGain;
	if (((*pStop != 0) && ((pMusicCell->stateFlags & 0x10) != 0)) && (pMusicCell->priority < param_1)) {
		if ((pMusicCell->stateFlags & 0x20) == 0) {
			pMusicCell->volume = pMusicCell->volume * fVar7;
			fVar5 = *pGainSource;
			pMusicCell->transitionDuration = fVar5;
			pMusicCell->transitionRemaining = fVar5;
			edMusicStreamPlay(pMusicCell->streamIndex, 1);
		}
		else {
			if (NoAudio == 0) {
				edMusicStreamPlay(pMusicCell->streamIndex, 0);
			}

			pMusicCell->stateFlags = pMusicCell->stateFlags & 0xffffffdf;
		}
	}

	if ((*pStart != 0) && (pMusicCell->priority < param_1)) {
		edMusicStreamPlay(pMusicCell->streamIndex, 2);
	}

	if ((*pResume == 0) || (param_1 <= pMusicCell->priority)) {
		uVar3 = pMusicCell->stateFlags;
		if ((uVar3 & 1) == 0) {
			if ((uVar3 & 2) == 0) {
				if ((uVar3 & 4) == 0) {
					if ((uVar3 & 8) != 0) {
						pMusicCell->stateFlags = uVar3 | 0x10;
						if (pMusicCell->priority == param_1) {
							edMusicStreamPlay(pMusicCell->streamIndex, 1);
						}
						pMusicCell->stateFlags = pMusicCell->stateFlags & 0xfffffff7;
					}
				}
				else {
					if ((pMusicCell->transitionRemaining <= 0.0f) || (pMusicCell->transitionDuration == 0.0f)) {
						pMusicCell->stateFlags = pMusicCell->stateFlags & 0xfffffffb;
						pMusicCell->stateFlags = pMusicCell->stateFlags & 0xffffffef;
						if (pMusicCell->priority == param_1) {
							edMusicStreamPlay(pMusicCell->streamIndex, 2);
						}
					}
				}
			}
			else {
				uVar3 = pMusicCell->flags & 0x10;
				if (((uVar3 == 0) || (pMusicCell->transitionRemaining != pMusicCell->transitionDuration)) || (pMusicCell->transitionDuration <= 0.0f)) {
					if ((pMusicCell->transitionRemaining <= 0.0f) || (((pMusicCell->flags & 0x40) == 0 && (uVar3 == 0)))) {
						pMusicCell->transitionRemaining = pMusicCell->transitionDuration;
						if (pMusicCell->pQueuedMusic == (CMusic*)0x0) {
							pMusicCell->stateFlags = pMusicCell->stateFlags & 0xffffffef;
							if (NoAudio == 0) {
								edMusicStreamPlay(pMusicCell->streamIndex, 2);
							}

							uVar4 = (uint)(pMusicCell->pCallback != (void*)0x0);
							if (((pMusicCell->flags & 0x40) != 0) || (pMusicCell->transitionDuration == 0.0f)) {
								*pStop = 1;
								*pGainSource = pMusicCell->transitionDuration;
							}

							*pClearCell = 1;
						}
						else {
							pMusicCell->stateFlags = pMusicCell->stateFlags & 0xfffffffd;
							if (NoAudio == 0) {
								edMusicStreamPlay(pMusicCell->streamIndex, 2);
							}

							pMusicCell->pMusic = pMusicCell->pQueuedMusic;
							pMusicCell->pQueuedMusic = (CMusic*)0x0;
							pCVar1 = pMusicCell->pMusic;
							streamIndex = pMusicCell->streamIndex;
							if (NoAudio == 0) {
								edMusicStreamSetSong(streamIndex, pCVar1->songIndex);
								edMusicStreamSetBank(streamIndex, pCVar1->bankIndex);
								edMusicStreamSetAutoLooping(streamIndex, (uint)((pCVar1->flags & 1) != 0));
							}

							if (pMusicCell->priority == param_1) {
								if (NoAudio == 0) {
									edMusicStreamPlay(pMusicCell->streamIndex, 0);
								}
							}
							else {
								pMusicCell->stateFlags = pMusicCell->stateFlags | 0x20;
							}

							if (0.0f < pMusicCell->field_0x3c) {
								pMusicCell->transitionDuration = pMusicCell->field_0x3c;
								pMusicCell->volume = 0.0f;
								pMusicCell->targetVolume = pMusicCell->initialVolume;
							}
						}
					}
				}
				else {
					pMusicCell->targetVolume = 0.0f;
					if ((pMusicCell->flags & 0x40) == 0) {
						*pStop = 1;
						*pGainSource = pMusicCell->transitionDuration;
					}
				}
			}

			uVar3 = pMusicCell->flags;
			if ((uVar3 & 1) == 0) {
				if ((uVar3 & 2) != 0) {
					fVar7 = *pGain * 0.5f;
				}
			}
			else {
				if ((((pMusicCell->stateFlags & 2) == 0) || ((uVar3 & 0x40) != 0)) || (pMusicCell->transitionDuration == pMusicCell->transitionRemaining)) {
					fVar7 = 0.0f;
				}
			}
		}
		else {
			uVar3 = pMusicCell->flags;
			bVar2 = false;
			if (((uVar3 & 0x20) == 0) && (pMusicCell->stopRemaining == pMusicCell->stopDuration)) {
				bVar2 = true;
			}

			if (pMusicCell->stopRemaining <= 0.0f) {
				pMusicCell->stopRemaining = 0.0f;
				pMusicCell->stateFlags = pMusicCell->stateFlags & 0xfffffffe;
				if ((pMusicCell->flags & 1) == 0) {
					if ((pMusicCell->flags & 4) != 0) {
						*pResume = 1;
					}
				}
				else {
					*pStart = 1;
				}
				uVar3 = pMusicCell->flags;
				if ((uVar3 & 0x20) != 0) {
					bVar2 = true;
				}
			}

			fVar5 = 0.0f;
			if (pMusicCell->stopDuration != 0.0f) {
				fVar5 = pMusicCell->stopRemaining / pMusicCell->stopDuration;
			}

			if ((uVar3 & 5) == 0) {
				if ((uVar3 & 2) != 0) {
					fVar7 = *pGain * (fVar5 * 0.5f + 0.5f);
				}
			}
			else {
				fVar7 = *pGain * fVar5;
			}

			if (0.0f < pMusicCell->stopRemaining) {
				pMusicCell->stopRemaining = pMusicCell->stopRemaining - param_2;
			}

			if (pMusicCell->stopRemaining < 0.0f) {
				pMusicCell->stopRemaining = 0.0f;
			}

			if (bVar2) {
				pMusicCell->stateFlags = pMusicCell->stateFlags | 0x10;
				if (pMusicCell->priority == param_1) {
					if (NoAudio == 0) {
						edMusicStreamPlay(pMusicCell->streamIndex, 0);
					}
				}
				else {
					pMusicCell->stateFlags = pMusicCell->stateFlags | 0x20;
				}
				pMusicCell->transitionRemaining = pMusicCell->transitionDuration;
				if ((pMusicCell->flags & 8) == 0) {
					pMusicCell->volume = pMusicCell->targetVolume;
				}
				else {
					pMusicCell->volume = 0.0;
				}
			}
		}

		if (pMusicCell->transitionRemaining < 0.0f) {
			pMusicCell->transitionRemaining = 0.0f;
		}

		fVar5 = pMusicCell->targetGain;
		fVar6 = pMusicCell->gain;
		if (fVar6 != fVar5) {
			if (param_2 < pMusicCell->transitionRemaining) {
				pMusicCell->gain = fVar6 + (fVar5 - fVar6) * (param_2 / pMusicCell->transitionRemaining);
			}
			else {
				pMusicCell->gain = fVar5;
			}
		}

		fVar5 = pMusicCell->targetVolume;
		fVar6 = pMusicCell->volume;
		if (fVar6 != fVar5) {
			if (param_2 < pMusicCell->transitionRemaining) {
				pMusicCell->volume = fVar6 + (fVar5 - fVar6) * (param_2 / pMusicCell->transitionRemaining);
			}
			else {
				pMusicCell->volume = fVar5;
			}
		}

		fVar5 = pMusicCell->volume;
		if (pMusicCell->priority < param_1) {
			fVar5 = fVar5 * *pGain;
		}

		if (NoAudio == 0) {
			fVar6 = 65535.0f;
			fVar5 = fVar5 * pAudioManager->musicVolume * pMusicCell->pMusic->field_0xc * 65535.0f;
			if (fVar5 <= 65535.0f) {
				fVar6 = fVar5;
			}

			edMusicStreamSetVolume(pMusicCell->streamIndex, (int)fVar6);
		}
		*pGain = fVar7;
	}
	else {
		if (NoAudio == 0) {
			edMusicStreamPlay(pMusicCell->streamIndex, 2);
		}

		uVar4 = (uint)(pMusicCell->pCallback != (void*)0x0);
		*pClearCell = 1;
	}

	return uVar4;
}

void CMusicManager::Manage(CAudioManager* pAudioManager)
{
	struct s_sorted_cell { s_sorted_cell* pNext; s_music_cell* pMusicCell; } sortedCells[10]{};
	s_sorted_cell* pSortedHead = nullptr;
	s_sorted_cell* pSortedNode = sortedCells;

	for (int handle = 9; handle >= 0; --handle) {
		s_music_cell* pMusicCell = &aMusicCells[handle];
		if (pMusicCell->pMusic == nullptr) continue;
		pSortedNode->pMusicCell = pMusicCell;
		s_sorted_cell* pPrevious = nullptr;
		s_sorted_cell* pCurrent = pSortedHead;
		while (pCurrent != nullptr && pMusicCell->priority < pCurrent->pMusicCell->priority) {
			pPrevious = pCurrent;
			pCurrent = pCurrent->pNext;
		}
		pSortedNode->pNext = pCurrent;
		if (pPrevious == nullptr) pSortedHead = pSortedNode;
		else pPrevious->pNext = pSortedNode;
		++pSortedNode;
	}
	if (pSortedHead == nullptr) return;

	const float deltaTime = Timer::GetTimer()->lastFrameTime;
	const float highestPriority = pSortedHead->pMusicCell->priority;
	int callbackHandles[10];
	int callbackCount = 0;
	int bStart = 0, bStop = 0, bClearCell = 0, bResume = 0, bForceStop = 0;
	float gain = 1.0f;

	float gainSource = 0.0f;
	for (s_sorted_cell* pCurrent = pSortedHead; pCurrent != nullptr; pCurrent = pCurrent->pNext) {
		s_music_cell* pMusicCell = pCurrent->pMusicCell;
		if (_ManageCell(highestPriority, deltaTime, pAudioManager, pMusicCell, &bStart, &bStop, &bClearCell, &bResume, &bForceStop, &gain, &gainSource) != 0) {
			callbackHandles[callbackCount++] = static_cast<int>(pMusicCell - aMusicCells);
		}

		pMusicCell->transitionRemaining -= deltaTime;
		if (bClearCell != 0) {
			bClearCell = 0;
			pMusicCell->pMusic = nullptr;
			aStreamUsed[pMusicCell->streamIndex] = 0;
		}
	}

	while (callbackCount != 0) {
		const int handle = callbackHandles[--callbackCount];
		const auto callback = aMusicCells[handle].pCallback;
		aMusicCells[handle].pCallback = nullptr;
		callback();
	}
}

CAudioManager::CAudioManager()
{
	bool bVar1;
	CMusicManager* pCVar2;
	CMusicManager* pCVar3;
	CDoubleLinkedList<s_sound_3d_data>* pCVar4;
	CMusicManager* otherAllocatedMemory;
	edsound_listener* peVar5;
	undefined* puVar6;
	CMusicManager* pCVar7;
	int iVar8;
	CDoubleLinkedNode<PendingSoundPlay>* pCVar9;

	this->field_0xd0 = 0;
	this->field_0xd1 = 0;
	this->bActive_0xd2 = false;
	this->field_0xd3 = 0;
	this->field_0xd4 = 0;
	this->field_0xd5 = 0;


	this->field_0xac = new CDoubleLinkedList<s_sound_3d_data>;
	this->field_0xb0 = new CDoubleLinkedList<s_sound_3d_data>;
	this->field_0x38 = new CMusicManager;

	this->pSoundListener = new edsound_listener;
	this->musicVolume = 1.0f;
	this->sfxVolume = 1.0f;

	ClearInternalData();

	g_GlobalSoundPtr_00448e34 = this;
	g_GlobalSoundPtr_00448e38 = this;

	this->field_0xb4 = 0;
	this->musicIndex = -1;
	pCVar9 = gPendingSoundPlayNodes_00449c50;
	this->field_0x2d8 = -1;
	iVar8 = 0xf;
	do {
		gFreePendingSoundPlays.InsertAfterQueue(pCVar9);
		pCVar9 = pCVar9 + 1;
		bVar1 = iVar8 != 0;
		iVar8 = iVar8 + -1;
	} while (bVar1);

	return;
}

void CAudioManager::SetMusicVolume(float newVolume)
{
	float fVar1;

	if (newVolume < 0.0f) {
		newVolume = 0.0f;
	}

	fVar1 = 1.0f;
	if (newVolume <= 1.0f) {
		fVar1 = newVolume;
	}

	this->musicVolume = fVar1;
	fVar1 = this->field_0xc4 * this->musicVolume * this->field_0x68 * 65535.0f;
	if (this->field_0xd1 != 0) {
		fVar1 = 0.0f;
	}

	if (NoAudio == 0) {
		edMusicSetMasterVolume(static_cast<int>(fVar1));
	}

	return;
}

void CAudioManager::SetSfxVolume(float newVolume)
{
	float fVar1;

	if (newVolume < 0.0f) {
		newVolume = 0.0f;
	}

	fVar1 = 1.0f;
	if (newVolume <= 1.0f) {
		fVar1 = newVolume;
	}

	this->sfxVolume = fVar1;
	fVar1 = this->field_0xc8 * this->sfxVolume;
	if (this->field_0xd0 != 0) {
		fVar1 = 0.0f;
	}

	if (NoAudio == 0) {
		edSoundSetMasterVolume(fVar1);
	}

	return;
}

void CAudioManager::SetOutputMode(AUDIO_MODE newMode)
{
	AUDIO_MODE unaff_s0_lo;
	AUDIO_MODE newMode_00;

	edSoundOutputModeSet(newMode);

	newMode_00 = SURROUND;
	if (((newMode != SURROUND) && (newMode_00 = STEREO, newMode != STEREO)) && (newMode_00 = unaff_s0_lo, newMode == MONO)) {
		newMode_00 = MONO;
	}

	edMusicSetOutputMode(newMode_00);

	return;
}

void CAudioManager::PlayCombatMusic()
{
	IMPLEMENTATION_GUARD_AUDIO();
}

void CAudioManager::StopCombatMusic()
{
	IMPLEMENTATION_GUARD_AUDIO();
}


bool CAudioManager::EnsureSoundMemoryAvailable(uint param_2, uint size)
{
#ifdef PLATFORM_PS2
	IMPLEMENTATION_GUARD_AUDIO(
	bool bVar1;
	CDoubleLinkedNode<SoundSampleEntry>* pCVar2;
	CDoubleLinkedNode<SoundSampleEntry>* pCVar3;
	uint lagestIOP;
	undefined4 uStack4;

	bVar1 = gLoadedSoundSamples.pHead != (CDoubleLinkedNode<SoundSampleEntry> *)0x0;
	pCVar3 = gLoadedSoundSamples.pHead;
	while (bVar1) {
		pCVar2 = pCVar3->pPrev;
		lagestIOP = GetLargestIOPMemory_00292c70();
		bVar1 = param_2 <= lagestIOP;
		thunk_FUN_00289b80(&uStack4, &lagestIOP);
		if (size <= lagestIOP && bVar1) break;
		UnloadSoundSample(pCVar3);
		bVar1 = pCVar2 != (CDoubleLinkedNode<SoundSampleEntry> *)0x0;
		pCVar3 = pCVar2;
	}
	lagestIOP = GetLargestIOPMemory_00292c70();
	bVar1 = param_2 <= lagestIOP;
	thunk_FUN_00289b80(&uStack4, &lagestIOP);
	return size <= lagestIOP && bVar1;)
#else
	// Ignore IOP boundaries.
	return true;
#endif
}

void CAudioManager::WillLoadFileFromBank(edCBankBufferEntry* pBankBuffer)
{
	bool bVar1;
	uint uVar2;
	int iVar3;
	undefined* puVar4;
	char* pcVar5;
	int iVar6;
	int iVar7;
	uint uVar8;
	edBANK_ENTRY_INFO local_220;
	char acStack512[512];

	uVar2 = pBankBuffer->get_element_count();
	iVar6 = 0;
	iVar7 = 0;
	uVar8 = 0;
	if (uVar2 != 0) {
		do {
			bVar1 = pBankBuffer->get_info(uVar8, &local_220, acStack512);
			if (bVar1 == false) break;
			if ((local_220.type << 0x10 | local_220.stype) == 0x30001) {
				iVar3 = edStrLength(acStack512);
				iVar7 = iVar7 + 1;
				iVar6 = iVar6 + iVar3 + 1;
			}
			uVar8 = uVar8 + 1;
		} while (uVar8 < uVar2);
	}

	if (iVar7 == 0) {
		this->aLoadedSamplePaths = (char**)0x0;
	}
	else {
		this->aLoadedSamplePaths = (char**)edMemAlloc(TO_HEAP(H_MAIN), iVar7 * sizeof(char*));
	}

	if (iVar6 == 0) {
		this->pString_0x1c = (char*)0x0;
	}
	else {
		this->pString_0x1c = (char*)edMemAlloc(TO_HEAP(H_MAIN), iVar6);
	}

	pcVar5 = this->pString_0x1c;
	if ((pcVar5 != (char*)0x0) && (uVar8 = 0, uVar2 != 0)) {
		iVar6 = 0;
		do {
			bVar1 = pBankBuffer->get_info(uVar8, &local_220, acStack512);
			if (bVar1 == false) {
				return;
			}
			if ((local_220.type << 0x10 | local_220.stype) == 0x30001) {
				edStrCopy(pcVar5, acStack512);
				this->aLoadedSamplePaths[uVar8] = pcVar5;
				iVar7 = edStrLength(pcVar5);
				pcVar5 = pcVar5 + iVar7 + 1;
			}

			uVar8 = uVar8 + 1;
		} while (uVar8 < uVar2);
	}

	return;
}

void CMusicManager_EndOfSongCallback(uint index)
{
	CMusicManager* pCVar1;
	CMusicManager* pCVar2;
	CMusicManager* pCVar3;
	ulong uVar4;
	CMusicManager* pCVar5;

	pCVar1 = (CScene::ptable.g_AudioManager_00451698)->field_0x38;
	IMPLEMENTATION_GUARD(
	if (pCVar1->aStreamUsed[index] != 0) {
		uVar4 = edMusicStreamGetAutoLooping(index);
		if (uVar4 == 0) {
			for (int handle = 0; handle < 10; ++handle) {
				if (pCVar1->aMusicCells[handle].pMusic != 0 && pCVar1->aMusicCells[handle].streamIndex == static_cast<int>(index)) {
					pCVar1->MusicStop(0.0f, 0, handle);
					break;
				}
			}
		}
	})

	return;
}

void CAudioManager_SoundFinishedInstancesCallback(ed_sound_instance_finished* pFinishedInstances, uint nbCount)

{
	bool bVar1;
	undefined* puVar2;
	int iVar3;

	if ((nbCount != 0) && (iVar3 = nbCount + -1, nbCount != 0)) {
		do {
			IMPLEMENTATION_GUARD(
			puVar2 = pFinishedInstances->field_0x4;
			if ((puVar2 != (undefined*)0x0) && (*static_cast<code**>(puVar2 + 0x18) != (code*)0x0)) {
				(**static_cast<code**>(puVar2 + 0x18))(puVar2, *static_cast<undefined4*>(puVar2 + 0x1c));
			})
			pFinishedInstances = pFinishedInstances + 1;
			bVar1 = iVar3 != 0;
			iVar3 = iVar3 + -1;
		} while (bVar1);
	}

	return;
}

void CAudioManager::Level_Init()
{
	CDoubleLinkedNode<CMusic*>* pCVar1;
	CMusicAmbiance* pStartMusicAmbiance;
	bool bVar3;
	SoundEntry* pSVar4;
	CAudioManager* pCVar5;
	SoundEntry* pSVar6;
	char** ppcVar7;
	CDoubleLinkedNode<SoundSampleEntry>* pCVar8;
	int* pBase;
	CDoubleLinkedNode<s_sound_3d_data>* pCVar9;
	CDoubleLinkedNode<s_sound_3d_data>* iVar10;
	CMusicAmbiance* pCVar10;
	CMusic* pCurMusic;
	SoundEntry* pSVar12;
	int iVar13;
	CSoundSample* pCVar14;
	uint nbMaxMusicAbiance;
	CSoundWind* pCurSoundWind;
	CMusicAmbiance* pMusicAmbiance;
	int iVar16;
	CSoundAmbiance* pSoundAmbiance;
	uint uVar17;

	if (NoAudio == 0) {
		edMusicFlush();
	}

	for (int sampleBankIndex = 0; sampleBankIndex < this->field_0x4; ++sampleBankIndex) {
		CSoundSample* pSoundSample = &this->aSoundSamplesA[sampleBankIndex];

		for (int entryIndex = pSoundSample->setupIntFieldA - 1; entryIndex >= 0; --entryIndex) {
			SoundEntry* pEntry = &pSoundSample->aSoundEntries[entryIndex];

			const int sampleIndex = (int)pEntry->pNode;
			pEntry->pNode = g_GlobalSoundPtr_00448e38->aSamples + sampleIndex;
		}
	}

	for (int soundBankIndex = 0; soundBankIndex < this->field_0x20; ++soundBankIndex) {
		CSoundSample* pSoundBank = &this->aSoundSamplesB[soundBankIndex];

		for (int entryIndex = pSoundBank->setupIntFieldA - 1; entryIndex >= 0; --entryIndex) {
			SoundEntry* pEntry = &pSoundBank->aSoundEntries[entryIndex];

			const int streamIndex = reinterpret_cast<intptr_t>(pEntry->pNode);

			if (streamIndex == -1) {
				pEntry->pNode = nullptr;
			}
			else {
				pEntry->pNode = reinterpret_cast<CDoubleLinkedNode<SoundSampleEntry>*>(
					reinterpret_cast<byte*>(&g_GlobalSoundPtr_00448e38->aEdSoundStreams->streamFileId) +
					g_GlobalSoundPtr_00448e38->field_0x2c * streamIndex);
			}
		}
	}

	iVar13 = this->nbMusic;
	pCurMusic = this->aMusic;
	pCVar5 = CScene::ptable.g_AudioManager_00451698;
	while (CScene::ptable.g_AudioManager_00451698 = pCVar5, iVar13 != 0) {
		iVar13 = iVar13 + -1;
		if (pCurMusic->songIndex == -1) {
			iVar16 = -1;
		}
		else {
			iVar16 = pCVar5->aSongIndexes[pCurMusic->songIndex];
		}

		pCurMusic->songIndex = iVar16;
		if (pCurMusic->bankIndex == -1) {
			iVar16 = -1;
		}
		else {
			iVar16 = pCVar5->aBankIndexes[pCurMusic->bankIndex];
		}

		pCurMusic->bankIndex = iVar16;
		pCurMusic = pCurMusic + 1;
		pCVar5 = CScene::ptable.g_AudioManager_00451698;
	}

	edMusicSetEndOfSongCallback(CMusicManager_EndOfSongCallback);

	int musicOffset = reinterpret_cast<int>(this->field_0x290);
	if ((musicOffset == -1) || (bVar3 = CScene::ptable.g_AudioManager_00451698->nbMusic <= musicOffset, bVar3)) {
		pCurMusic = (CMusic*)0x0;
	}
	else {
		if (bVar3) {
			pCurMusic = (CMusic*)0x0;
		}
		pCurMusic = CScene::ptable.g_AudioManager_00451698->aMusic + musicOffset;
	}
	this->field_0x290 = pCurMusic;

	pCurSoundWind = this->aSoundWind;
	for (iVar13 = this->nbSoundWind; iVar13 != 0; iVar13 = iVar13 + -1) {
		pCurSoundWind->Init();
		pCurSoundWind = pCurSoundWind + 1;
	}

	nbMaxMusicAbiance = this->nbMusicAmbiance;
	pMusicAmbiance = this->aMusicAmbiance;
	while (nbMaxMusicAbiance != 0) {
		nbMaxMusicAbiance = nbMaxMusicAbiance - 1;
		pMusicAmbiance->field_0x10 = 0;
		pMusicAmbiance->field_0x14 = 0;
		pMusicAmbiance->field_0x18 = 0;
		pMusicAmbiance->field_0x1c = 0;
		pSoundAmbiance = pMusicAmbiance->aSoundAmbiance;
		for (uVar17 = pMusicAmbiance->nbSoundAmbiance; uVar17 != 0; uVar17 = uVar17 - 1) {
			pSoundAmbiance->Init();
			pSoundAmbiance = pSoundAmbiance + 1;
		}
		pMusicAmbiance = pMusicAmbiance + 1;
	}

	this->nbSoundDataNodes = 0x20;
	nbMaxMusicAbiance = this->nbSoundDataNodes;
	this->aSoundDataNodes = new CDoubleLinkedNode<s_sound_3d_data>[nbMaxMusicAbiance];
	iVar13 = this->nbSoundDataNodes + -1;
	if (this->nbSoundDataNodes != 0) {
		do {
			CDoubleLinkedNode<s_sound_3d_data>* pSoundDataNode = this->aSoundDataNodes + iVar13;

			pSoundDataNode->node.edSoundData.field_0x1c = 1.0f;
			pSoundDataNode->node.edSoundData.field_0x18 = 1.0f;

			this->field_0xac->InsertBeforeHead(pSoundDataNode);

			bVar3 = iVar13 != 0;
			iVar13 = iVar13 + -1;
		} while (bVar3);
	}

	edSoundSetFinishedInstancesCallback(CAudioManager_SoundFinishedInstancesCallback);

	pCVar1 = (this->musicFreeList).pHead;
	pCurMusic = (CMusic*)0x0;
	if (pCVar1 != (CDoubleLinkedNode<CMusic*> *)0x0) {
		pCurMusic = pCVar1->node;
	}

	if (pCurMusic == (CMusic*)0x0) {
		pCurMusic = this->field_0x114;
	}

	iVar13 = -1;
	if (pCurMusic != (CMusic*)0x0) {
		iVar13 = static_cast<int>((uintptr_t)pCurMusic - (uintptr_t)this->aMusic) / sizeof(CMusic);
	}
	this->musicIndex = iVar13;

	// Set ambiance index.
	pStartMusicAmbiance = this->aMusicAmbiance;
	nbMaxMusicAbiance = this->nbMusicAmbiance;
	for (pCVar10 = pStartMusicAmbiance; (nbMaxMusicAbiance != 0 && (nbMaxMusicAbiance = nbMaxMusicAbiance - 1, pCVar10->field_0x14 == 0)); pCVar10 = pCVar10 + 1) {
	}

	iVar13 = -1;
	if (pCVar10 < pStartMusicAmbiance + this->nbMusicAmbiance) {
		int ambianceOffset = ((uintptr_t)pCVar10 - (uintptr_t)pStartMusicAmbiance);
		iVar13 = ambianceOffset / sizeof(CMusicAmbiance);
		if (ambianceOffset < 0) {
			this->field_0x2d8 = (ambianceOffset + 0x1f) / sizeof(CMusicAmbiance);
			goto LAB_00185b58;
		}
	}
	this->field_0x2d8 = iVar13;

LAB_00185b58:
	Level_Reset();

	return;
}

void CAudioManager::Level_Term()
{
	delete[] this->aSoundDataNodes;
	this->aSoundDataNodes = (CDoubleLinkedNode<s_sound_3d_data> *)0x0;
	edSoundSetFinishedInstancesCallback(0);
	this->musicIndex = -1;
	this->field_0x2d8 = -1;

	return;
}

void CAudioManager::Level_AddAll(ByteCode* pByteCode)
{
	uint uVar1;
	CSoundSample* pCVar4;
	CSoundWind* pCVar6;
	int iVar7;

	pByteCode->GetChunk();
	this->nbMaxSamples = pByteCode->GetU32();
	uVar1 = this->nbMaxSamples;
	if (uVar1 != 0) {
		this->aSamples = new ed_sound_sample[uVar1];
	}

	this->field_0x4 = pByteCode->GetU32();
	uVar1 = this->field_0x4;
	if (uVar1 != 0) {
		this->aSoundSamplesA = new CSoundSample[uVar1];
	}

	pCVar4 = this->aSoundSamplesA;
	for (iVar7 = this->field_0x4; iVar7 != 0; iVar7 = iVar7 + -1) {
		pCVar4->Create(pByteCode);
		pCVar4 = pCVar4 + 1;
	}

	AddMusics(pByteCode);

	pByteCode->GetChunk();
	pByteCode->GetU32();
	pByteCode->GetChunk();

	this->nbMusicAmbiance = pByteCode->GetU32();
	uVar1 = this->nbMusicAmbiance;
	if (uVar1 != 0) {
		this->aMusicAmbiance = new CMusicAmbiance[uVar1];
		uVar1 = 0;
		if (this->nbMusicAmbiance != 0) {
			do {
				this->aMusicAmbiance[uVar1].Add(pByteCode);
				uVar1 = uVar1 + 1;
			} while (uVar1 < this->nbMusicAmbiance);
		}
	}

	pByteCode->GetChunk();
	this->nbSoundWind = pByteCode->GetU32();
	uVar1 = this->nbSoundWind;
	if (uVar1 != 0) {
		this->aSoundWind = new CSoundWind[uVar1];
		pCVar6 = this->aSoundWind;
		for (iVar7 = this->nbSoundWind; iVar7 != 0; iVar7 = iVar7 + -1) {
			pCVar6->Add(pByteCode);
			pCVar6 = pCVar6 + 1;
		}
	}

	AddSceneData(pByteCode);
	LoadGlobalSoundFunc_00184a70(pByteCode);
	AddSoundStreams(pByteCode);

	return;
}

void CancelQueuedSoundSampleLoad(CDoubleLinkedNode<SoundSampleEntry>* param_1)
{
	int iVar1;

	iVar1 = (param_1->node).mode;
	if ((iVar1 != 1) && (iVar1 == 2)) {
		if (gCurrentSoundSampleFile != (edFILEH*)0x0) {
			iVar1 = gCurrentSoundSampleFile->nbQueuedActions;

			while (iVar1 != 0) {
				edFileNoWaitStackFlush();
				iVar1 = gCurrentSoundSampleFile->nbQueuedActions;
			}
			edFileReleaseHandle(gCurrentSoundSampleFile);
		}
		gCurrentSoundSampleFile = (edFILEH*)0x0;
		gCurrentLoadingSoundSample = (CDoubleLinkedNode<SoundSampleEntry>*)0x0;
		edMemFree(gSoundSampleFileData_00448e68);
		gSoundSampleFileData_00448e68 = (void*)0x0;
	}

	gQueuedSoundSamples.RemoveNode(param_1);
	gFreeSoundSamples.InsertFront(param_1);

	(param_1->node).mode = 0;

	return;
}

void FreeSamples_00189150(void)
{
	bool bVar1;
	CDoubleLinkedNode<PendingSoundPlay>* pCVar2;
	CSoundStream* pCVar3;
	CDoubleLinkedNode<SoundSampleEntry>* pCVar4;
	CDoubleLinkedNode<SoundSampleEntry>* pCVar5;
	PendingSoundPlay* pPVar6;

	bVar1 = gLoadedSoundSamples.pHead != (CDoubleLinkedNode<SoundSampleEntry> *)0x0;
	pCVar4 = gLoadedSoundSamples.pHead;
	while (bVar1) {
		pCVar2 = (pCVar4->node).field_0x6c.pHead;
		while (pCVar2 != (CDoubleLinkedNode<PendingSoundPlay> *)0x0) {
			pCVar3 = (pCVar2->node).pSoundStream;
			pPVar6 = &pCVar2->node;
			pCVar2 = pCVar2->pPrev;
			pCVar3->Stop(*pPVar6->field_0x18);
		}
		pCVar4 = pCVar4->pPrev;
		bVar1 = pCVar4 != (CDoubleLinkedNode<SoundSampleEntry> *)0x0;
	}

	edSoundFlush();

	pCVar4 = gLoadedSoundSamples.pHead;
	while (pCVar5 = gQueuedSoundSamples.pHead, pCVar4 != (CDoubleLinkedNode<SoundSampleEntry> *)0x0) {
		pCVar5 = pCVar4->pPrev;
		edSoundSampleFree(&(pCVar4->node).edSoundSample);
		(pCVar4->node).mode = 0;
		gLoadedSoundSamples.RemoveNode(pCVar4);
		gFreeSoundSamples.InsertFront(pCVar4);
		pCVar4 = pCVar5;
	}

	while (pCVar5 != (CDoubleLinkedNode<SoundSampleEntry> *)0x0) {
		pCVar4 = pCVar5->pPrev;
		CancelQueuedSoundSampleLoad(pCVar5);
		pCVar5 = pCVar4;
	}

	while (gFreeSoundSamples.pHead != (CDoubleLinkedNode<SoundSampleEntry> *)0x0) {
		gFreeSoundSamples.RemoveNode(gFreeSoundSamples.pHead);
	}

	return;
}

void CAudioManager::Level_ClearAll()
{
	CMusicManager* this_00;
	CSoundAmbiance* pSectorArray;
	CMusicManager* pCVar1;
	CMusicManager* pCVar2;
	int* piVar3;
	uint uVar4;
	int iVar5;
	uint uVar6;
	_ed_sound_stream* pSoundStream;
	CMusicManager* pCVar7;

	if (NoAudio == 0) {
		edSoundTerminateAllInstances();
		edSoundFlush();
	}

	edSoundSetFinishedInstancesCallback(0);
	FreeSamples_00189150();

	if (this->pGlobalSoundFileData != (GlobalSound_FileData*)0x0) {
		delete(this->pGlobalSoundFileData);
		this->pGlobalSoundFileData = (GlobalSound_FileData*)0x0;
	}

	if (this->field_0x7c != 0) {
		delete[] this->aSoundStreamNodes;
		this->aSoundStreamNodes = (CDoubleLinkedNode<SoundSampleEntry> *)0x0;
	}

	if (this->field_0x78 != 0) {
		delete[] this->aSoundStreams;
		this->aSoundStreams = (CSoundStream*)0x0;
	}

	if (this->nbMaxSamples != 0) {
		if (NoAudio == 0) {
			uVar6 = 0;
			if (this->nbMaxSamples != 0) {
				do {
					ed_sound_sample* pCurSample = this->aSamples + uVar6;
					uVar4 = pCurSample->dataSize;
					if ((uVar4 & 0x3f) != 0) {
						uVar4 = (uVar4 & 0xffffffc0) + 0x40;
					}

					if (pCurSample->soundRamAddress != 0) {
						this->field_0xb4 = this->field_0xb4 - uVar4;
						edSoundSampleFree(this->aSamples + uVar6);
					}
					uVar6 = uVar6 + 1;
				} while (uVar6 < (uint)this->nbMaxSamples);
			}
			edSoundFlush();
		}

		if (this->aSamples != (ed_sound_sample*)0x0) {
			delete(this->aSamples);
		}

		this->aSamples = (ed_sound_sample*)0x0;
	}

	if (this->field_0x4 != 0) {
		delete[] this->aSoundSamplesA;
		this->aSoundSamplesA = (CSoundSample*)0x0;
	}

	if (this->field_0x20 != 0) {
		pSoundStream = this->aEdSoundStreams;
		uVar6 = 0;
		if (this->field_0x20 != 0) {
			do {
				edSoundStreamFree(pSoundStream);
				edSoundFlush();
				uVar6 = uVar6 + 1;
				pSoundStream = reinterpret_cast<_ed_sound_stream*>(reinterpret_cast<char*>(pSoundStream) + this->field_0x2c);
			} while (uVar6 < (uint)this->field_0x20);
		}

		edMemFree(this->aEdSoundStreams);

		delete[] this->aSoundSamplesB;
		this->aSoundSamplesB = (CSoundSample*)0x0;
	}

	this_00 = this->field_0x38;
	for (int handle = 0; handle < 10; ++handle) {
		if (this_00->aMusicCells[handle].pMusic != (CMusic*)0x0) {
			this_00->Stop(0.0f, 0, handle);
		}
	}

	this_00->Manage(this);

	if (NoAudio == 0) {
		edMusicFlush();
	}

	edMusicSetEndOfSongCallback(0);

	if (NoAudio == 0) {
		edMusicFlush();
	}

	if (NoAudio == 0) {
		edMusicStopAllSongs();
		edMusicFlush();
	}

	if (this->aSoundWind != (CSoundWind*)0x0) {
		delete[] this->aSoundWind;
	}

	if (this->field_0x44 != 0) {
		if ((NoAudio == 0) && (uVar6 = 0, this->field_0x44 != 0)) {
			do {
				edMusicSongRemove(this->aSongIndexes[uVar6]);
				uVar6 = uVar6 + 1;
			} while (uVar6 < (uint)this->field_0x44);
		}

		delete(this->aSongIndexes);
		delete(this->field_0x54);
	}

	if (this->nbBanks != 0) {
		if ((NoAudio == 0) && (uVar6 = 0, this->nbBanks != 0)) {
			do {
				edMusicBankRemove(uVar6);
				uVar6 = uVar6 + 1;
				this->field_0xb4 = this->field_0xb4 - this->field_0xb8;
			} while (uVar6 < this->nbBanks);
		}

		delete(this->field_0x5c);
		delete(this->field_0x58);
		delete(this->aBankIndexes);
	}

	if (NoAudio == 0) {
		edMusicFlush();
	}

	if (this->nbMusic != 0) {
		delete[] this->aMusic;
	}

	if (this->nbMusicAmbiance != 0) {
		uVar6 = 0;
		if (this->nbMusicAmbiance != 0) {
			do {
				pSectorArray = this->aMusicAmbiance->aSoundAmbiance + uVar6;
				if (pSectorArray != (CSoundAmbiance*)0x0) {
					delete[] pSectorArray;
				}

				uVar6 = uVar6 + 1;
			} while (uVar6 < this->nbMusicAmbiance);
		}
		
		delete[] this->aMusicAmbiance;
	}

	if (this->pString_0x1c != (char*)0x0) {
		edMemFree(this->pString_0x1c);
	}

	if (this->aLoadedSamplePaths != (char**)0x0) {
		edMemFree(this->aLoadedSamplePaths);
	}

	ClearInternalData();

	return;
}

int INT_00448e78 = 0;

void CAudioManager::Level_Manage()
{
	bool bVar1;
	CActorManager* pActorManager;
	int iVar2;
	edF32MATRIX4* m1;
	uint uVar4;
	long unaff_s0;
	CMusicAmbiance* pCurAmbiance;
	float fVar5;
	float fVar6;
	CActor* local_4;
	CActor* pActor;
	CCamera* pCamera;
	CDoubleLinkedNode<s_sound_3d_data>* pDataNode;
	CMusic* pMusic;
	CMusicManager* pMusicManager;

	if (NoAudio == 0) {
		ManageSoundSamples();

		GetTimer();

		iVar2 = this->cellIndex_0x270;
		pMusicManager = this->field_0x38;
		if (iVar2 == -1) {
			bVar1 = false;
		}
		else {
			pMusic = pMusicManager->aMusicCells[iVar2].pMusic;
			if ((pMusic == (CMusic*)0x0) || (bVar1 = true, pMusic != this->field_0x26c)) {
				bVar1 = false;
			}
		}

		if (((bVar1) && (unaff_s0 = 0, iVar2 != -1)) && ((pMusicManager->aMusicCells[iVar2].pMusic == (CMusic*)0x0 || (unaff_s0 = 1, (pMusicManager->aMusicCells[iVar2].flags & 2) == 0)))) {
			unaff_s0 = 0;
		}

		if (this->count_0x274 == 0) {
			if ((bVar1) && (unaff_s0 == 0)) {
				pMusicManager->Stop(this->field_0x284, 0.0f, iVar2);
			}
		}
		else {
			if (bVar1) {
				if (unaff_s0 != 0) {
					pMusicManager->CancelStop(this->field_0x280, iVar2);
				}
			}
			else {
				iVar2 = pMusicManager->Start(3.0f, this->field_0x27c, this->field_0x280, this->field_0x280, this->field_0x26c, this->field_0x28c);
				this->cellIndex_0x270 = iVar2;
			}
		}

		iVar2 = this->field_0x298;
		pMusicManager = this->field_0x38;
		if (iVar2 == -1) {
			bVar1 = false;
		}
		else {
			pMusic = pMusicManager->aMusicCells[iVar2].pMusic;
			if ((pMusic == (CMusic*)0x0) || (bVar1 = true, pMusic != this->field_0x290)) {
				bVar1 = false;
			}
		}

		if (((bVar1) && (unaff_s0 = 0, iVar2 != -1)) && ((pMusicManager->aMusicCells[iVar2].pMusic == (CMusic*)0x0 || (unaff_s0 = 1, (pMusicManager->aMusicCells[iVar2].flags & 2) == 0)))) {
			unaff_s0 = 0;
		}

		if (this->field_0x294 == 0) {
			if ((bVar1) && (unaff_s0 == 0)) {
				pMusicManager->Stop(this->field_0x2a4, 0.0f, iVar2);
			}
		}
		else {
			if (bVar1) {
				if (unaff_s0 != 0) {
					pMusicManager->CancelStop(this->field_0x2a0, iVar2);
				}
			}
			else {
				iVar2 = pMusicManager->Start(3.0f, this->field_0x29c, this->field_0x2a0, this->field_0x2a0, this->field_0x290, this->field_0x28c);
				this->field_0x298 = iVar2;
			}
		}

		pActorManager = CScene::ptable.g_ActorManager_004516a4;
		pCamera = (CScene::ptable.g_CameraManager_0045167c)->pActiveCamera;
		local_4 = pCamera->GetTarget();
		if (local_4 == (CActor*)0x0) {
			local_4 = (CActor*)0x0;
			IMPLEMENTATION_GUARD(
			pActorManager->SoundFunc_00106570(6, &local_4, 1);)
		}

		if (local_4 == (CActor*)0x0) {
			if (INT_00448e78 == 0) {
				INT_00448e78 = 1;
			}
		}
		else {
			this->pSoundListener->field_0x0 = (pCamera->transformationMatrix).rowT.xyz;
			this->pSoundListener->field_0xc = local_4->currentLocation.xyz - (pCamera->transformationMatrix).rowT.xyz;
			this->pSoundListener->field_0x18 = (pCamera->transformationMatrix).rowY.xyz;

			edSoundSetListener(this->pSoundListener);

			fVar5 = (pCamera->transformationMatrix).db;
			fVar6 = (pCamera->transformationMatrix).dc;
			this->cameraPosition = (pCamera->transformationMatrix).rowT.xyz;
		}

		for (pDataNode = this->field_0xb0->pHead; pDataNode != (CDoubleLinkedNode<s_sound_3d_data> *)0x0; pDataNode = pDataNode->pPrev) {
			pActor = (pDataNode->node).pActor;
			m1 = pActor->pAnimationController->GetCurBoneMatrix((pDataNode->node).boneId);
			static edF32MATRIX4 total_matrix;
			edF32Matrix4MulF32Matrix4Hard(&total_matrix, m1, &pActor->pMeshTransform->base.transformA);
			(pDataNode->node).edSoundData.position = total_matrix.rowT.xyz;

			(pDataNode->node).edSoundData.rotation = (pDataNode->node).edSoundData.position - (pDataNode->node).field_0x28;

			fVar5 = GetTimer()->lastFrameTime;
			if (fVar5 != 0.0f) {
				fVar5 = 1.0f / fVar5;
			}

			(pDataNode->node).edSoundData.rotation = (pDataNode->node).edSoundData.rotation * fVar5;
			(pDataNode->node).field_0x28 = (pDataNode->node).edSoundData.position;
		}

		fVar5 = 1.0f;
		if (this->field_0x2dc != 0) {
			fVar5 = this->field_0x68;
		}

		if (this->bActive_0xd2 != false) {
			fVar5 = 0.0f;
		}

		pCurAmbiance = this->aMusicAmbiance;
		for (uVar4 = this->nbMusicAmbiance; uVar4 != 0; uVar4 = uVar4 - 1) {
			pCurAmbiance->Play(fVar5);
			pCurAmbiance = pCurAmbiance + 1;
		}

		this->field_0x38->Manage(this);

		if (NoAudio == 0) {
			edMusicFlush();
		}

		FUN_00182c10();
	}

	return;
}

void CAudioManager::Level_ManagePaused()
{
	Level_Manage();

	return;
}

void FUN_00182e40(void)
{
	CAudioManager* pCVar1;

	pCVar1 = CScene::ptable.g_AudioManager_00451698;
	(CScene::ptable.g_AudioManager_00451698)->field_0xf0 = 0xffffffff;
	pCVar1->pPrevMusic = (CMusic*)0x0;
	pCVar1->field_0x114 = (CMusic*)0x0;
	return;
}

void CAudioManager::Level_Reset()
{
	bool bVar1;
	uint uVar2;
	CSoundSample* pCVar3;
	edsound_listener* peVar4;
	CCamera* pCVar5;
	undefined4 uVar8;
	CDoubleLinkedNode<CMusic*>* pCVar9;
	CMusicManager::s_music_cell* psVar10;
	int iVar11;
	CSoundAmbiance* pSoundAmbiance;
	uint* puVar13;
	uint uVar14;
	uint uVar15;
	CMusicAmbiance* pMusicAmbiance;
	float fVar17;
	float fVar18;
	CMusicManager* pMusicManager;

	if (NoAudio == 0) {
		edSoundTerminateAllInstances();
		edSoundFlush();
	}

	fVar17 = this->musicVolume;
	if (fVar17 < 0.0f) {
		fVar17 = 0.0f;
	}

	fVar18 = 1.0f;
	if (fVar17 <= 1.0f) {
		fVar18 = fVar17;
	}

	this->musicVolume = fVar18;
	fVar17 = this->field_0xc4 * this->musicVolume * this->field_0x68 * 65535.0f;
	if (this->field_0xd1 != 0) {
		fVar17 = 0.0f;
	}

	if (NoAudio == 0) {
		edMusicSetMasterVolume(static_cast<int>(fVar17));
	}

	fVar17 = this->sfxVolume;
	if (fVar17 < 0.0f) {
		fVar17 = 0.0f;
	}

	fVar18 = 1.0f;
	if (fVar17 <= 1.0f) {
		fVar18 = fVar17;
	}
	this->sfxVolume = fVar18;

	fVar17 = this->field_0xc8 * this->sfxVolume;
	if (this->field_0xd0 != 0) {
		fVar17 = 0.0f;
	}

	if (NoAudio == 0) {
		edSoundSetMasterVolume(fVar17);
	}

	this->field_0x2dc = 0;
	uVar15 = this->nbMusicAmbiance;
	pMusicAmbiance = this->aMusicAmbiance;
	while (uVar15 != 0) {
		uVar15 = uVar15 - 1;
		pMusicAmbiance->field_0x18 = 0.0f;
		pMusicAmbiance->field_0x14 = pMusicAmbiance->field_0x10;
		pMusicAmbiance->field_0x10 = 0;
		pMusicAmbiance->field_0x1c = 0;
		uVar2 = pMusicAmbiance->nbSoundAmbiance;
		uVar14 = 0;
		if (uVar2 != 0) {
			do {
				pSoundAmbiance = pMusicAmbiance->aSoundAmbiance + uVar14;
				if ((NoAudio == 0) && (pCVar3 = pSoundAmbiance->field_0x4c, pCVar3 != (CSoundSample*)0x0)) {
					uVar8 = pCVar3->Stop(pSoundAmbiance->field_0x50);
					pSoundAmbiance->field_0x50 = uVar8;
				}
				uVar14 = uVar14 + 1;
				pSoundAmbiance->field_0x68 = pSoundAmbiance->field_0x6c;
			} while (uVar14 < uVar2);
		}
		pMusicAmbiance = pMusicAmbiance + 1;
	}

	this->count_0x274 = 0;
	iVar11 = 10;
	this->cellIndex_0x270 = -1;
	this->field_0x294 = 0;
	this->field_0x298 = 0xffffffff;

	pMusicManager = this->field_0x38;
	for (int index = 9; index >= 0; --index) {
		CMusicManager::s_music_cell& musicCell = pMusicManager->aMusicCells[index];

		if (musicCell.pMusic != nullptr) {
			pMusicManager->Stop(0.0f, 0.0f, index);
		}
	}

	pMusicManager->Manage(this);

	if (NoAudio == 0) {
		edMusicFlush();
	}
	this->field_0xf0 = -1;

	if (((this->musicActiveList).pHead == (CDoubleLinkedNode<CMusic*>*)0x0) && ((this->musicFreeList).pHead == (CDoubleLinkedNode<CMusic*>*)0x0)) {
		iVar11 = 0xf;
		pCVar9 = this->aMusicNodes;
		do {
			this->musicActiveList.InsertAfterQueue(pCVar9);
			bVar1 = iVar11 != 0;
			iVar11 = iVar11 + -1;
			pCVar9 = pCVar9 + 1;
		} while (bVar1);
	}
	else {
		while (pCVar9 = this->musicFreeList.RemoveHead(), pCVar9 != (CDoubleLinkedNode<CMusic*>*)0x0) {
			this->musicActiveList.InsertAfterQueue(pCVar9);
		}
	}

	this->pPrevMusic = this->pMusic;
	this->field_0xf0 = this->field_0xdc;
	this->field_0xf4 = this->field_0xe0;
	this->field_0xf8 = this->field_0xe4;
	this->field_0xfc = this->field_0xe8;

	if (this->pPrevMusic != (CMusic*)0x0) {
		this->field_0x114 = this->pPrevMusic;
		iVar11 = this->field_0x38->Start(1.0f, this->field_0xf8, 0.0f, 0.0f, this->pPrevMusic, 0x7a);
		this->field_0xf0 = iVar11;

		if (this->field_0xf0 != -1) {
			uVar15 = this->field_0xf4;
			if (this->field_0xf0 != -1) {
				puVar13 = field_0x228 + 16;
				iVar11 = 0xf;
				do {
					puVar13 = puVar13 + -1;
					fVar17 = 0.0f;
					if ((*puVar13 & uVar15) != 0) {
						fVar17 = 1.0f;
					}

					psVar10 = this->field_0x38->aMusicCells + this->field_0xf0;
					if ((psVar10->pMusic != (CMusic*)0x0) && (NoAudio == 0)) {
						edMusicStreamChannelFade(0.0f, psVar10->streamIndex, iVar11, 0x10000, static_cast<int>(fVar17 * 65535.0f));
					}

					bVar1 = iVar11 != 0;
					iVar11 = iVar11 + -1;
				} while (bVar1);
			}

			psVar10 = this->field_0x38->aMusicCells + this->field_0xf0;
			if (psVar10->pMusic != (CMusic*)0x0) {
				psVar10->pCallback = FUN_00182e40;
			}
		}
	}

	*this->pSoundListener = edSoundListenerDefault;

	pCVar5 = (CScene::ptable.g_CameraManager_0045167c)->pActiveCamera;
	this->cameraPosition = (pCVar5->transformationMatrix).rowT.xyz;

	this->field_0x68 = 1.0f;
	this->field_0x6c = 2;
	this->field_0x64 = 0.0f;
	this->field_0x2dc = 0;
	this->field_0x2e0 = 0;

	return;
}

void CAudioManager::Level_CheckpointReset()
{
	Level_Reset();

	return;
}

void CAudioManager::Level_PauseChange(bool bPaused)
{
	this->bActive_0xd2 = bPaused != 0;

	return;
}

void CAudioManager::ClearInternalData()
{
	bool bVar1;
	CDoubleLinkedList<s_sound_3d_data>* pCVar2;
	float* pfVar3;
	float* pfVar4;
	CDoubleLinkedNode<CMusic*>* pCVar5;
	undefined4* puVar6;
	float* pfVar7;
	int iVar8;

	this->nbMaxSamples = 0;
	this->field_0x44 = 0;
	this->nbBanks = 0;
	this->aSamples = (ed_sound_sample*)0x0;
	this->aSoundStreamNodes = (CDoubleLinkedNode<SoundSampleEntry> *)0x0;
	this->field_0x58 = 0;
	this->field_0x54 = 0;
	this->aSongIndexes = (int*)0x0;
	this->aBankIndexes = (int*)0x0;
	this->field_0x5c = 0;
	pCVar2 = this->field_0xac;
	pCVar2->pHead = (CDoubleLinkedNode<s_sound_3d_data>*)0x0;
	pCVar2->pTail = (CDoubleLinkedNode<s_sound_3d_data>*)0x0;
	pCVar2 = this->field_0xb0;
	pCVar2->pHead = (CDoubleLinkedNode<s_sound_3d_data>*)0x0;
	pCVar2->pTail = (CDoubleLinkedNode<s_sound_3d_data>*)0x0;
	this->field_0x4 = 0;
	this->field_0x20 = 0;
	this->field_0x30 = 0;
	this->field_0x2c = 0;
	this->nbMusic = 0;
	this->nbMusicAmbiance = 0;
	this->aSoundSamplesA = (CSoundSample*)0x0;
	this->aSoundStreams = (CSoundStream*)0x0;
	this->aSoundSamplesB = (CSoundSample*)0x0;
	this->aEdSoundStreams = (_ed_sound_stream*)0x0;
	this->field_0x34 = (char*)0x0;
	this->aMusic = (CMusic*)0x0;
	this->aMusicAmbiance = (CMusicAmbiance*)0x0;
	this->nbLoadedSamples = 0;
	this->field_0x114 = (CMusic*)0x0;
	this->pMusic = (CMusic*)0x0;
	this->field_0xdc = 0xffffffff;
	this->field_0xe0 = 7;
	this->field_0xe4 = 1.0f;
	this->field_0xe8 = 0;
	this->field_0x100 = this->pMusic;
	this->field_0x104 = this->field_0xdc;
	this->field_0x108 = this->field_0xe0;
	this->field_0x10c = this->field_0xe4;
	this->field_0x110 = this->field_0xe8;
	this->pPrevMusic = this->field_0x100;
	this->field_0xf0 = this->field_0x104;
	this->field_0xf4 = this->field_0x108;
	this->field_0xf8 = this->field_0x10c;
	this->field_0xfc = this->field_0x110;

	if (((this->musicActiveList).pHead == (CDoubleLinkedNode<CMusic*>*)0x0) && ((this->musicFreeList).pHead == (CDoubleLinkedNode<CMusic*>*)0x0)) {
		iVar8 = 0xf;
		pCVar5 = this->aMusicNodes;
		do {
			this->musicActiveList.InsertAfterQueue(pCVar5);
			bVar1 = iVar8 != 0;
			iVar8 = iVar8 + -1;
			pCVar5 = pCVar5 + 1;
		} while (bVar1);
	}
	else {
		while (pCVar5 = this->musicFreeList.RemoveHead(), pCVar5 != (CDoubleLinkedNode<CMusic*>*)0x0) {
			this->musicActiveList.InsertAfterQueue(pCVar5);
		}
	}

	//puVar6 = (undefined4*)&this->field_0x268;
	//pfVar4 = this->field_0x1e8 + 0xf;
	//pfVar7 = (float*)this->field_0x228;
	//while (pfVar3 = pfVar4, this->field_0x1e8 < pfVar7) {
	//	*pfVar3 = 1.0f;
	//	puVar6 = puVar6 + -1;
	//	*puVar6 = 4;
	//	pfVar4 = pfVar3 + -1;
	//	pfVar7 = pfVar3;
	//}

	this->field_0x26c = 0;
	this->cellIndex_0x270 = -1;
	this->count_0x274 = 0;
	this->field_0x278 = 0;
	this->field_0x27c = 1.0f;
	this->field_0x280 = 0.0f;
	this->field_0x284 = 1.0f;
	this->field_0x288 = 0;
	this->field_0x294 = 0;
	this->field_0x298 = 0xffffffff;
	this->field_0x29c = 1.0f;
	this->field_0x2a0 = 0.0f;
	this->field_0x2a4 = 1.0f;
	this->nbSoundWind = 0;
	this->aSoundWind = (CSoundWind*)0x0;
	this->field_0xc4 = 1.0f;
	this->field_0xc8 = 1.0f;
	this->field_0xcc = 2.0f;
	this->field_0x68 = 1.0f;
	this->field_0x6c = 2;
	this->field_0x64 = 0.0f;
	this->field_0x2dc = 0;
	this->field_0x2e0 = 0;

	return;
}

void CAudioManager::AddMusics(ByteCode* pByteCode)
{
	uint uVar1;
	void* pvVar2;
	int* piVar3;
	void** ppvVar4;
	CMusic* pCVar5;
	uint uVar6;
	int iVar7;
	CMusic* pCurMusic;
	float fVar8;

	pByteCode->GetChunk();
	this->field_0x44 = pByteCode->GetU32();
	if (this->field_0x44 != 0) {
		this->field_0x54 = new int[this->field_0x44];
		this->aSongIndexes = new int[this->field_0x44];

		uVar1 = 0;
		if (this->field_0x44 != 0) {
			do {
				this->aSongIndexes[uVar1] = -1;
				uVar1 = uVar1 + 1;
			} while (uVar1 < (uint)this->field_0x44);
		}
	}

	this->nbBanks = pByteCode->GetU32();
	if (this->nbBanks != 0) {
		this->aBankIndexes = new int[this->nbBanks];
		this->field_0x58 = new int[this->nbBanks];
		this->field_0x5c = new int[this->nbBanks];
		this->field_0x60 = (MusicSomething_0x8*)edMemAlloc(TO_HEAP(H_MAIN), this->nbBanks * sizeof(MusicSomething_0x8));

		uVar1 = 0;
		if (this->nbBanks != 0) {
			do {
				this->aSongIndexes[uVar1] = -1;
				uVar1 = uVar1 + 1;
			} while (uVar1 < this->nbBanks);
		}
	}

	this->nbMusic = pByteCode->GetU32();
	uVar1 = this->nbMusic;
	if (uVar1 != 0) {
		this->aMusic = new CMusic[uVar1];
		uVar1 = 0;
		if (this->nbMusic != 0) {
			do {
				pCurMusic = this->aMusic + uVar1;
				pCurMusic->songIndex = pByteCode->GetU32();
				pCurMusic->bankIndex = pByteCode->GetU32();
				pCurMusic->flags = pByteCode->GetU32();
				pCurMusic->field_0xc = pByteCode->GetF32();
				uVar1 = uVar1 + 1;
			} while (uVar1 < (uint)this->nbMusic);
		}
	}

	return;
}

void CAudioManager::AddSceneData(ByteCode* pByteCode)
{
	ED_SOUND_REVERB_TYPE EVar1;
	bool bVar2;
	uint uVar3;
	CMusic* pCVar4;
	uint* puVar5;
	float fVar8;
	float fVar9;
	float fVar10;
	float fVar11;

	pByteCode->GetChunk();
	uVar3 = pByteCode->GetU32();
	if (uVar3 == 0) {
		this->field_0x290 = reinterpret_cast<CMusic*>(0xffffffff);
	}
	else {
		this->field_0xc8 = pByteCode->GetF32();
		this->field_0xc4 = pByteCode->GetF32();

		fVar8 = this->field_0xc8;
		if (fVar8 < 0.0f) {
			fVar8 = 0.0f;
		}

		fVar9 = 1.0f;
		if (fVar8 <= 1.0f) {
			fVar9 = fVar8;
		}

		this->field_0xc8 = fVar9;
		fVar9 = fVar9 * this->sfxVolume;
		if (this->field_0xd0 != 0) {
			fVar9 = 0.0f;
		}

		if (NoAudio == 0) {
			edSoundSetMasterVolume(fVar9);
		}

		fVar8 = this->field_0xc4;
		if (fVar8 < 0.0f) {
			fVar8 = 0.0f;
		}

		fVar9 = 1.0f;
		if (fVar8 <= 1.0f) {
			fVar9 = fVar8;
		}
		this->field_0xc4 = fVar9;

		fVar8 = fVar9 * this->musicVolume * this->field_0x68 * 65535.0f;
		if (this->field_0xd1 != 0) {
			fVar8 = 0.0f;
		}

		if (NoAudio == 0) {
			edMusicSetMasterVolume(static_cast<int>(fVar8));
		}

		this->field_0xe4 = pByteCode->GetF32();
		uVar3 = pByteCode->GetU32();
		if ((uVar3 == 0xffffffff) || (bVar2 = (uint)this->nbMusic <= uVar3, bVar2)) {
			pCVar4 = (CMusic*)0x0;
		}
		else {
			if (bVar2) {
				uVar3 = 0;
			}

			pCVar4 = this->aMusic + uVar3;
		}
		this->pMusic = pCVar4;

		this->pPrevMusic = pCVar4;
		this->field_0xe0 = pByteCode->GetU32();
		puVar5 = this->field_0x228 + 0xf;
		if (this->field_0x228 < (this->field_0x228 + 0xf)) {
			do {
				*puVar5 = pByteCode->GetU32();
				bVar2 = this->field_0x228 < puVar5;
				puVar5 = puVar5 + -1;
			} while (bVar2);
		}

		this->field_0x27c = pByteCode->GetF32();
		this->field_0x280 = pByteCode->GetF32();
		this->field_0x284 = pByteCode->GetF32();
		this->field_0x288 = pByteCode->GetU32();

		this->field_0x28c = 0x19;
		if ((this->field_0x288 & 1) == 0) {
			this->field_0x28c = this->field_0x28c | 0x20;
		}
		if ((this->field_0x288 & 2) == 0) {
			this->field_0x28c = this->field_0x28c | 0x40;
		}

		uVar3 = pByteCode->GetU32();
		if ((uVar3 == 0xffffffff) || (bVar2 = (uint)this->nbMusic <= uVar3, bVar2)) {
			pCVar4 = (CMusic*)0x0;
		}
		else {
			if (bVar2) {
				uVar3 = 0;
			}
			pCVar4 = this->aMusic + uVar3;
		}

		this->field_0x26c = pCVar4;

		this->field_0x290 = reinterpret_cast<CMusic*>(pByteCode->GetS32());
		this->field_0x29c = pByteCode->GetF32();
		this->field_0x2a0 = pByteCode->GetF32();
		this->field_0x2a4 = pByteCode->GetF32();
		for (uint musicSlot = 0; musicSlot < 3; ++musicSlot) {
			uVar3 = pByteCode->GetU32();
			if ((uVar3 == 0xffffffff) || (bVar2 = (uint)this->nbMusic <= uVar3, bVar2)) {
				pCVar4 = (CMusic*)0x0;
			}
			else {
				pCVar4 = this->aMusic + uVar3;
			}
			this->field_0x2a8[musicSlot] = pCVar4;
		}

		this->field_0x2b4 = pByteCode->GetU32();
		this->field_0x2b8 = pByteCode->GetF32();
		this->field_0x2bc = pByteCode->GetF32();
		fVar9 = pByteCode->GetF32();
		this->field_0x2c0 = fVar9;
		fVar10 = this->field_0x2b8;
		EVar1 = this->field_0x2b4;
		fVar11 = this->field_0x2bc;
		fVar8 = fVar10;
		if (fVar10 < 0.0f) {
			fVar8 = -fVar10;
		}

		if (NoAudio == 0) {
			edSoundPrepareReverbTypes(1, &this->field_0x2b4);
			edSoundFlush();
			edMusicSetReverb(fVar10, fVar8, fVar11, fVar9, EVar1);
		}

		this->field_0xcc = pByteCode->GetF32();
	}

	return;
}

void CAudioManager::LoadGlobalSoundFunc_00184a70(ByteCode* pByteCode)
{
	char cVar1;
	uint uVar2;
	int* pBase;
	CSoundSample* pCurSample;
	char* pcVar4;
	GlobalSound_FileData* pGVar5;
	int iVar6;
	GlobalSound_FileData* pGVar7;
	int iVar8;

	pByteCode->GetChunk();
	this->field_0x20 = pByteCode->GetU32();
	uVar2 = this->field_0x20;
	if (uVar2 == 0) {
		this->aSoundSamplesB = (CSoundSample*)0x0;
	}
	else {
		this->aSoundSamplesB = new CSoundSample[uVar2];
	}
	pCurSample = this->aSoundSamplesB;

	for (iVar8 = this->field_0x20; iVar8 != 0; iVar8 = iVar8 + -1) {
		pCurSample->Create(pByteCode);
		pCurSample = pCurSample + 1;
	}

	pByteCode->GetChunk();
	this->field_0x30 = pByteCode->GetU32();
	pcVar4 = pByteCode->GetPosition();
	this->field_0x34 = pcVar4;

	if (this->field_0x30 == 0) {
		this->pGlobalSoundFileData = (GlobalSound_FileData*)0x0;
	}
	else {
		this->pGlobalSoundFileData = new GlobalSound_FileData[this->field_0x30];
		iVar8 = this->field_0x30;
		pGVar5 = this->pGlobalSoundFileData;
		while (iVar8 != 0) {
			iVar8 = iVar8 + -1;
			pcVar4 = pByteCode->GetString();
			edStrCopy(pGVar5->field_0x10, pcVar4);
			pcVar4 = pByteCode->currentSeekPos;
			iVar6 = 8;
			pByteCode->currentSeekPos = pcVar4 + 0x10;
			//pGVar7 = pGVar5;
			//do {
			//	iVar6 = iVar6 + -1;
			//	cVar1 = pcVar4[1];
			//	*(char*)&pGVar7->field_0x0 = *pcVar4;
			//	pcVar4 = pcVar4 + 2;
			//	*static_cast<char*>((int)&pGVar7->field_0x0 + 1) = cVar1;
			//	pGVar7 = static_cast<GlobalSound_FileData*>((int)&pGVar7->field_0x0 + 2);
			//} while (0 < iVar6);

			pGVar5 = pGVar5 + 1;
		}
	}

	return;
}

void CAudioManager::AddSoundStreams(ByteCode* pByteCode)
{
	bool bVar1;
	byte bVar2;
	uint uVar3;
	CSoundStream* pCVar4;
	undefined* puVar5;
	uint uVar6;
	char cVar7;
	uint uVar8;
	int* piVar9;
	CSoundStream* pCVar10;
	char* __s;
	int iVar11;
	CDoubleLinkedNode<SoundSampleEntry>* pCVar12;
	int iVar13;
	byte bVar14;
	uint uVar15;

	pByteCode->GetChunk();
	this->field_0x78 = pByteCode->GetU32();
	uVar8 = this->field_0x78;
	if (uVar8 != 0) {
		this->aSoundStreams = new CSoundStream[uVar8];
	}

	pCVar10 = this->aSoundStreams;
	for (uVar8 = this->field_0x78; uVar8 != 0; uVar8 = uVar8 - 1) {
		pCVar10->Create(pByteCode);
		pCVar10 = pCVar10 + 1;
	}

	this->field_0x7c = 0;
	if (this->field_0x78 != 0) {
		edMemSetFlags(TO_HEAP(H_MAIN), 0x100);
		__s = new char[this->field_0x78];
		edMemClearFlags(TO_HEAP(H_MAIN), 0x100);
		memset(__s, 0, this->field_0x78);
		cVar7 = '\x01';
		pCVar10 = this->aSoundStreams;
		uVar8 = this->field_0x78;
		while (uVar8 != 0) {
			uVar15 = uVar8 - 1;
			if (__s[uVar15] == '\0') {
				__s[uVar15] = cVar7;
				this->field_0x7c = this->field_0x7c + 1;
				uVar6 = uVar8 - 2;
				pCVar4 = pCVar10;
				uVar8 = uVar15;
				while (uVar3 = uVar6, uVar8 != 0) {
					if ((__s[uVar3] == '\0') && (iVar11 = edStrCmp(pCVar10->sample.aSoundEntries[0].fileName, pCVar4[1].sample.aSoundEntries[0].fileName), iVar11 == 0)) {
						__s[uVar3] = cVar7;
					}

					uVar6 = uVar3 - 1;
					pCVar4 = pCVar4 + 1;
					uVar8 = uVar3;
				}

				cVar7 = cVar7 + '\x01';
			}

			pCVar10 = pCVar10 + 1;
			uVar8 = uVar15;
		}

		uVar8 = this->field_0x7c;
		if (uVar8 != 0) {
			this->aSoundStreamNodes = new CDoubleLinkedNode<SoundSampleEntry>[uVar8];
			bVar14 = 1;
			iVar11 = this->field_0x7c;
			pCVar12 = this->aSoundStreamNodes;
			pCVar10 = this->aSoundStreams;
			iVar13 = this->field_0x78 - 1;
			while (iVar11 != 0) {
				iVar11 = iVar11 + -1;
				gFreeSoundSamples.InsertAfterQueue(pCVar12);
				pCVar12->node.loadFunc = &SoundSampleEntry::LoadStreamCh;

				edStrCopy((pCVar12->node).fileName, pCVar10->sample.aSoundEntries[0].fileName);

				bVar1 = 0 < iVar13;
				iVar13 = iVar13 + -1;
				if (bVar1) {
					bVar2 = __s[iVar13];
					while (pCVar10 = pCVar10 + 1, bVar2 <= bVar14) {
						iVar13 = iVar13 + -1;
						bVar2 = __s[iVar13];
					}
					if (-1 < iVar13) {
						bVar14 = bVar2;
					}
				}

				pCVar12 = pCVar12 + 1;
			}

			uVar8 = this->field_0x78;
			pCVar10 = this->aSoundStreams;
			while (uVar8 != 0) {
				uVar8 = uVar8 - 1;
				pCVar10->sample.aSoundEntries[0].pNode = this->aSoundStreamNodes + ((byte)__s[uVar8] - 1);
				pCVar10 = pCVar10 + 1;
			}
		}

		delete[] __s;
	}

	return;
}

void CAudioManager::FUN_00182c10()
{
	bool bVar1;
	Timer* pTVar2;
	float fVar3;
	float fVar4;

	if (this->field_0x6c != 2) {
		pTVar2 = Timer::GetTimer();
		bVar1 = false;
		fVar4 = pTVar2->lastFrameTime / this->field_0x64;

		if (this->field_0x6c == 1) {
			fVar4 = this->field_0x68 - fVar4;
			this->field_0x68 = fVar4;
			if (fVar4 <= 0.0f) {
				this->field_0x68 = 0.0f;
				this->field_0x6c = 2;
				if (this->field_0x2e0 != 0) {
					bVar1 = true;
				}
			}
		}
		else {
			if ((this->field_0x6c == 0) && (fVar4 = this->field_0x68 + fVar4, this->field_0x68 = fVar4, 1.0 <= fVar4)) {
				this->field_0x68 = 1.0f;
				this->field_0x6c = 2;
				this->field_0x2dc = 0;
				this->field_0x2e0 = 0;
			}
		}

		if ((this->field_0x2e0 != 0) || (bVar1)) {
			fVar4 = this->musicVolume;
			if (fVar4 < 0.0f) {
				fVar4 = 0.0f;
			}

			fVar3 = 1.0f;
			if (fVar4 <= 1.0f) {
				fVar3 = fVar4;
			}

			this->musicVolume = fVar3;
			fVar4 = this->field_0xc4 * this->musicVolume * this->field_0x68 * 65535.0f;
			if (this->field_0xd1 != 0) {
				fVar4 = 0.0f;
			}

			if (NoAudio == 0) {
				edMusicSetMasterVolume((int)fVar4);
			}
		}
	}

	return;
}

bool CanAllocateIOP_002887e0(void* param_1, ulong param_2)
{
	uint uVar1;
	bool bVar2;
	uint uVar3;
	undefined4 uStack8;
	uint local_4;

#ifdef PLATFORM_PS2
	bVar2 = false;
	uVar3 = (uint) * (byte*)((int)param_1 + 0xf) | (uint) * (byte*)((int)param_1 + 0xe) << 8 | (uint) * (byte*)((int)param_1 + 0xc) << 0x18 | (uint) * (byte*)((int)param_1 + 0xd) << 0x10;
	if ((param_2 & 1) == 0) {
		thunk_FUN_00289b80(&uStack8, &local_4);
		if (uVar3 <= local_4) {
			bVar2 = true;
		}
	}
	else {
		uVar1 = GetLargestIOPMemory_00292c70();
		if (uVar3 <= uVar1) {
			bVar2 = true;
		}
	}
	return bVar2;
#else
	return true;
#endif
}

uint gSoundSampleMemoryUsed = 0;
uint gAlternateSoundSampleMemoryUsed = 0;

void UnloadSoundSample(CDoubleLinkedNode<SoundSampleEntry>* pNode)
{
	CSoundStream* pCVar1;
	uint* piVar2;
	bool bVar3;
	CDoubleLinkedNode<SoundSampleEntry>* pCVar4;
	int iVar5;
	CDoubleLinkedNode<PendingSoundPlay>* pCVar6;
	float fVar7;
	float fVar8;

	pCVar6 = (pNode->node).field_0x6c.pHead;
	bVar3 = false;
	if (pCVar6 != (CDoubleLinkedNode<PendingSoundPlay>*)0x0) {
		bVar3 = true;
		while (pCVar6 != (CDoubleLinkedNode<PendingSoundPlay>*)0x0) {
			pCVar1 = (pCVar6->node).pSoundStream;
			piVar2 = (pCVar6->node).field_0x18;
			pCVar6 = pCVar6->pPrev;
			iVar5 = pCVar1->Stop(*piVar2);
			*piVar2 = iVar5;
		}
	}

	if (bVar3) {
		edSoundFlush();
	}

	if ((pNode->node).field_0x74 == 0) {
		gSoundSampleMemoryUsed = gSoundSampleMemoryUsed - (pNode->node).edSoundSample.dataSize;
	}
	else {
		gAlternateSoundSampleMemoryUsed = gAlternateSoundSampleMemoryUsed - (pNode->node).edSoundSample.dataSize;
	}

	edSoundSampleFree(&(pNode->node).edSoundSample);

	gLoadedSoundSamples.RemoveNode(pNode);

	if (bVar3) {
		pCVar6 = (pNode->node).field_0x6c.pHead;
		pCVar4 = gQueuedSoundSamples.pHead;
		if (pCVar6 == (CDoubleLinkedNode<PendingSoundPlay> *)0x0) {
			fVar7 = 0.0f;
		}
		else {
			fVar7 = (pCVar6->node).priority;
		}

		for (; pCVar4 != (CDoubleLinkedNode<SoundSampleEntry> *)0x0; pCVar4 = pCVar4->pPrev) {
			pCVar6 = (pCVar4->node).field_0x6c.pHead;
			if (pCVar6 == (CDoubleLinkedNode<PendingSoundPlay> *)0x0) {
				fVar8 = 0.0f;
			}
			else {
				fVar8 = (pCVar6->node).priority;
			}

			if (fVar8 <= fVar7) break;
		}

		if (pCVar4 == (CDoubleLinkedNode<SoundSampleEntry> *)0x0) {
			gQueuedSoundSamples.InsertAfterQueue(pNode);
		}
		else {
			gQueuedSoundSamples.InsertBefore(pNode, pCVar4);
		}

		(pNode->node).mode = 1;
	}
	else {
		gFreeSoundSamples.InsertFront(pNode);
		(pNode->node).mode = 0;
	}

	return;
}

int gTransferIndex_00448e74;

void FUN_00188f10(CDoubleLinkedNode<SoundSampleEntry>* pNode, char* pData)
{
	CDoubleLinkedNode<PendingSoundPlay>* pCVar1;
	ulong flags;

	pCVar1 = (pNode->node).field_0x6c.pHead;
	if (pCVar1 == (CDoubleLinkedNode<PendingSoundPlay> *)0x0) {
		gCurrentLoadingSoundSample = (CDoubleLinkedNode<SoundSampleEntry> *)0x0;
		gCurrentSoundSampleFile = (edFILEH*)0x0;
		edMemFree(gSoundSampleFileData_00448e68);
		gSoundSampleFileData_00448e68 = (void*)0x0;
		(pNode->node).mode = 0;
		gQueuedSoundSamples.RemoveNode(pNode);
		gFreeSoundSamples.InsertFront(pNode);
	}
	else {
		(pNode->node).mode = 3;
		flags = 1;
		if ((pNode->node).field_0x74 != 0) {
			flags = 3;
		}

		gTransferIndex_00448e74 = edSoundSampleLoad(pData, &(pNode->node).edSoundSample, flags);

		(pNode->node).field_0x74 = (((pCVar1->node).pSoundStream)->sample).field_0x88;
		if ((pNode->node).field_0x74 == 0) {
			gSoundSampleMemoryUsed = gSoundSampleMemoryUsed + (pNode->node).edSoundSample.dataSize;
		}
		else {
			gAlternateSoundSampleMemoryUsed = gAlternateSoundSampleMemoryUsed + (pNode->node).edSoundSample.dataSize;
		}
	}

	return;
}

/* Per-frame sound-sample streaming and pending-playback manager.

   Advances the asynchronous sample-load state machine:
   - Waits for the active file read to finish.
   - Compares queued priorities and may cancel the current load in favor of a more important sample.
   - Ensures enough IOP memory is available, unloading the least-important loaded sample when worthwhile.
   - Finalizes completed loads and moves their entries from the queued list to the loaded list.
   - Starts the next queued sample through its load callback when no load is active.

   Then services every pending play attached to loaded samples. New requests create an edSound instance and apply 3D data, randomized volume, randomized frequency, and user data. Finished instances
   are cleared, removed from the sample's pending list when safe, returned to the free-request list, and deducted from the sample reference count.

   Called once per frame near the beginning of CAudioManager::Level_Manage. Although Ghidra currently models a CAudioManager this parameter, the body does not use it; most working state is held in
   global sample queues and the current-load globals. */

void CAudioManager::ManageSoundSamples()
{
	int iVar1;
	CDoubleLinkedNode<PendingSoundPlay>* pCVar2;
	CDoubleLinkedNode<SoundSampleEntry>* pCVar3;
	edsound_3d_data* pData;
	uint* existingSoundIDPtr;
	int* piVar4;
	CSoundStream* pCVar5;
	CDoubleLinkedNode<PendingSoundPlay>* pNode;
	CDoubleLinkedNode<SoundSampleEntry>* pCVar6;
	bool bVar7;
	uint uVar8;
	long lVar9;
	float fVar10;
	float fVar11;
	float fVar12;

	pCVar3 = gCurrentLoadingSoundSample;
	if (gCurrentLoadingSoundSample != (CDoubleLinkedNode<SoundSampleEntry> *)0x0) {
		iVar1 = (gCurrentLoadingSoundSample->node).mode;
		if (iVar1 == 2) {
			if ((gCurrentSoundSampleFile != (edFILEH*)0x0) && (gCurrentSoundSampleFile->nbQueuedActions == 0)) {
				edFileReleaseHandle(gCurrentSoundSampleFile);
				gCurrentSoundSampleFile = (edFILEH*)0x0;
			}

			if (gCurrentSoundSampleFile == (edFILEH*)0x0) {
				pCVar2 = ((gQueuedSoundSamples.pHead)->node).field_0x6c.pHead;
				if (pCVar2 == (CDoubleLinkedNode<PendingSoundPlay> *)0x0) {
					fVar10 = 0.0f;
				}
				else {
					fVar10 = (pCVar2->node).priority;
				}

				pCVar2 = (gCurrentLoadingSoundSample->node).field_0x6c.pHead;
				if (pCVar2 == (CDoubleLinkedNode<PendingSoundPlay> *)0x0) {
					fVar11 = 0.0f;
				}
				else {
					fVar11 = (pCVar2->node).priority;
				}

				if (fVar11 < fVar10) {
					edMemFree(gSoundSampleFileData_00448e68);
					gSoundSampleFileData_00448e68 = (void*)0x0;
					(gCurrentLoadingSoundSample->node).mode = 1;
					gCurrentLoadingSoundSample = (CDoubleLinkedNode<SoundSampleEntry> *)0x0;
				}
				else {
					bVar7 = CanAllocateIOP_002887e0(gSoundSampleFileData_00448e68, 1);
					if (bVar7 == false) {
						if (gLoadedSoundSamples.pTail != (CDoubleLinkedNode<SoundSampleEntry> *)0x0) {
							pCVar2 = ((gLoadedSoundSamples.pTail)->node).field_0x6c.pHead;
							if (pCVar2 == (CDoubleLinkedNode<PendingSoundPlay> *)0x0) {
								fVar10 = 0.0f;
							}
							else {
								fVar10 = (pCVar2->node).priority;
							}

							pCVar2 = (gCurrentLoadingSoundSample->node).field_0x6c.pHead;
							if (pCVar2 == (CDoubleLinkedNode<PendingSoundPlay> *)0x0) {
								fVar11 = 0.0f;
							}
							else {
								fVar11 = (pCVar2->node).priority;
							}

							// If the least-important loaded sample has lower priority than the current load, unload it to free IOP memory.
							if (fVar10 < fVar11) {
								UnloadSoundSample(gLoadedSoundSamples.pTail);
							}
						}
					}
					else {
						// If the current load is complete, finalize it and start the next queued sample.
						FUN_00188f10(gCurrentLoadingSoundSample, (char*)gSoundSampleFileData_00448e68);
					}
				}
			}
		}
		else {
			if ((iVar1 == 3) && (((gCurrentLoadingSoundSample->node).edSoundSample.flags & 4) != 0)) {
				edMemFree(gSoundSampleFileData_00448e68);
				gSoundSampleFileData_00448e68 = (void*)0x0;
				gQueuedSoundSamples.RemoveNode(pCVar3);
				gLoadedSoundSamples.InsertFront(pCVar3);
				gCurrentLoadingSoundSample = (CDoubleLinkedNode<SoundSampleEntry> *)0x0;
				(pCVar3->node).mode = 4;
			}
		}
	}

	pCVar6 = gQueuedSoundSamples.pHead;
	pCVar3 = gLoadedSoundSamples.pHead;
	if ((((gCurrentLoadingSoundSample == (CDoubleLinkedNode<SoundSampleEntry> *)0x0) && (gQueuedSoundSamples.pHead != (CDoubleLinkedNode<SoundSampleEntry> *)0x0)) &&
		(((gQueuedSoundSamples.pHead)->node).mode == 1)) &&
		(lVar9 = (gQueuedSoundSamples.pHead->node.*gQueuedSoundSamples.pHead->node.loadFunc)(), pCVar3 = gLoadedSoundSamples.pHead, lVar9 != 0)) {
		gCurrentLoadingSoundSample = pCVar6;
		(pCVar6->node).mode = 2;
		pCVar3 = gLoadedSoundSamples.pHead;
	}
	do {
		pCVar6 = pCVar3;
		if (pCVar6 == (CDoubleLinkedNode<SoundSampleEntry> *)0x0) {
			return;
		}

		pCVar3 = pCVar6->pPrev;
		pCVar2 = (pCVar6->node).field_0x6c.pHead;
		while (pNode = pCVar2, pNode != (CDoubleLinkedNode<PendingSoundPlay> *)0x0) {
			IMPLEMENTATION_GUARD(
			uVar8 = *(pNode->node).field_0x18;
			pCVar2 = pNode->pPrev;
			if (uVar8 == 0) {
				uVar8 = edSoundSamplePlay((pNode->node).priority, &(((pNode->node).pNode)->node).edSoundSample);
				pData = (pNode->node).p3dData;
				existingSoundIDPtr = (pNode->node).field_0x14;
				piVar4 = (pNode->node).field_0x10;
				pCVar5 = (pNode->node).pSoundStream;
				if (pData != (edsound_3d_data*)0x0) {
					fVar10 = pCVar5->sample.field_0x78;
					pData->field_0x1c = fVar10 * fVar10;
					pData->field_0x18 = fVar10;
				}

				edSoundInstanceSet3DData(uVar8, pData, existingSoundIDPtr);
				fVar10 = pCVar5->sample.field_0x6c;
				fVar11 = static_cast<float>(pCVar5)->sample.edSoundSample.loopEndOffset;
				if (fVar10 != 0.0f) {
					fVar10 = fVar10 * fVar11;
					fVar12 = fVar11;
					do {
						fVar11 = edFRndGauss(fVar12, fVar10);
					} while (fVar11 < 0.0f);
				}

				edSoundInstanceSetVolume(fVar11, uVar8);
				fVar10 = pCVar5->sample.field_0x74;
				fVar11 = pCVar5->sample.field_0x70;
				if (fVar10 != 0.0f) {
					fVar10 = fVar10 * fVar11;
					fVar12 = fVar11;
					do {
						fVar11 = edFRndGauss(fVar12, fVar10);
					} while (fVar11 < 0.0f);
				}

				edSoundInstanceSetFrequency(fVar11, uVar8);
				edSoundInstanceSetUserData(uVar8, piVar4);
				*(pNode->node).field_0x18 = uVar8;
			LAB_00188ce0:
				bVar7 = false;
			}
			else {
				uVar8 = FUN_00283f70(uVar8);

				if (uVar8 != 0) goto LAB_00188ce0;

				bVar7 = true;
				*(pNode->node).field_0x18 = 0;
			}

			if ((bVar7) && ((gCurrentLoadingSoundSample != pCVar6 || ((pCVar6->node).field_0x6c.pHead != (pCVar6->node).field_0x6c.pTail)))) {
				(pCVar6->node).field_0x6c.RemoveNode(pNode);
				gFreePendingSoundPlays.InsertFront(pNode);
				(pCVar6->node).nbCount = (pCVar6->node).nbCount + -1;
			})
		}
	} while (true);
}

bool SoundSampleEntry::LoadStreamCh()
{
	uint someSize;
	bool bVar1;
	int iVar2;
	uint uVar3;
	edCFiler* peVar4;
	edCFileNoWaitStack* peVar5;
	char acStack768[512];
	char acStack256[256];

	bVar1 = StaticEdFileBase_004497f0.FUN_00401f30();
	if (bVar1 != false) {
		/* \\STREAMCH\\ */
		edStrCatMulti(acStack256, CLevelScheduler::gThis->levelPath, CLevelScheduler::gThis->aLevelInfo[CLevelScheduler::gThis->currentLevelID].levelName, "\\STREAMCH\\", this->fileName, NULL);
		gCurrentSoundSampleFile = edFileOpen(acStack256, 0x29);
		if (gCurrentSoundSampleFile != (edFILEH*)0x0) {
			iVar2 = edFileLoadSize(gCurrentSoundSampleFile);
			someSize = iVar2 + 0x7ffU & 0xfffff800;
			uVar3 = edMemGetMemoryAvailable(TO_HEAP(H_MAIN));
			if (someSize + 0x100 <= uVar3) {
				gSoundSampleFileData_00448e68 = edMemAllocAlign(TO_HEAP(H_MAIN), someSize, 0x40);
			}
			if (gSoundSampleFileData_00448e68 == (void*)0x0) {
				edFileClose(gCurrentSoundSampleFile);
				peVar4 = edFileGetFiler(acStack768, "", 1);
				if (peVar4 != (edCFiler*)0x0) {
					while (peVar5 = peVar4->getnowaitfilestack(), peVar5->nbInUse != 0) {
						peVar5 = peVar4->getnowaitfilestack();
						edFileNoWaitStackCallBack(peVar5);
					}
				}
				edFileReleaseHandle(gCurrentSoundSampleFile);
				gCurrentSoundSampleFile = (edFILEH*)0x0;
			}
			else {
				edFileRead(gCurrentSoundSampleFile, (char*)gSoundSampleFileData_00448e68, someSize);
				edFileClose(gCurrentSoundSampleFile);
			}
		}
	}

	return gSoundSampleFileData_00448e68 != (void*)0x0;
}

float GetRandomTime(float param_1, float param_2)
{
	float fVar1;
	float fVar2;

	fVar1 = param_1;
	if ((param_2 != 0.0) && (fVar1 = 0.0, param_1 != 0.0)) {
		fVar2 = param_2 * param_1;
		do {
			fVar1 = edFRndGauss(param_1, fVar2);
		} while (fVar1 <= 0.0);
	}
	return fVar1;
}

void CSoundAmbiance::Init()
{
	CWayPoint* pCVar1;
	CSoundSample* pCVar3;
	float fVar4;
	float fVar5;

	GetTimer();


	this->field_0x0.Init();
	this->field_0x4.Init();
	this->field_0x8.Init();

	pCVar3 = this->field_0x0.pSample;
	if (pCVar3 == (void*)0x0) {
		pCVar3 = this->field_0x4.pSample;
	}
	if (pCVar3 == (void*)0x0) {
		pCVar3 = &this->field_0x8.pStream->sample;
	}
	this->field_0x4c = pCVar3;

	this->wayPointRef.Init();
	pCVar1 = (this->wayPointRef).Get();
	if (pCVar1 != (CWayPoint*)0x0) {
		this->location = pCVar1->location;
		this->field_0x24 = gF32Vector3Zero;
		fVar4 = pCVar3->field_0x78;
		this->field_0x34 = fVar4 * fVar4;
		this->field_0x30 = fVar4;
	}

	this->field_0x68 = 0;

	if ((this->field_0xc == 0.0f) || (this->field_0x10 == 0.0f)) {
		this->field_0x6c = 0.0f;
	}
	else {
		fVar4 = GetRandomTime(this->field_0xc, this->field_0x10);
		this->field_0x6c = fVar4;
	}

	return;
}

void SOUND_SAMPLE_REF::Init()
{
	CSoundSample* pCVar2;

	if (this->index == -1) {
		pCVar2 = (CSoundSample*)0x0;
	}
	else {
		if (this->index < CScene::ptable.g_AudioManager_00451698->field_0x4) {
			pCVar2 = (CScene::ptable.g_AudioManager_00451698)->aSoundSamplesA + this->index;
		}
		else {
			pCVar2 = (CScene::ptable.g_AudioManager_00451698)->aSoundSamplesB + this->index - CScene::ptable.g_AudioManager_00451698->field_0x4;
		}
	}

	this->pSample = pCVar2;

	return;
}

void SOUND_STREAM_REF::Init()
{
	CSoundStream* pCVar3;

	if (this->index == -1) {
		pCVar3 = (CSoundStream*)0x0;
	}
	else {
		pCVar3 = (CScene::ptable.g_AudioManager_00451698)->aSoundStreams + this->index;
	}
	this->pStream = pCVar3;

	return;
}

void CSoundWind::Init()
{
	this->field_0x0.Init();
	this->field_0x4.Init();
	this->field_0x8.Init();

	return;
}

void CSoundWind::Add(ByteCode* pByteCode)
{
	this->field_0x0.index = pByteCode->GetS32();
	this->field_0x4.index = pByteCode->GetS32();
	this->field_0x8.index = pByteCode->GetS32();

	return;
}

void CSoundSample::Create(ByteCode* pByteCode)
{
	this->setupIntFieldA = pByteCode->GetU32();

	for (uint entryIndex = this->setupIntFieldA; entryIndex != 0; --entryIndex) {
		SoundEntry& entry = this->aSoundEntries[entryIndex - 1];
		entry.selector = pByteCode->GetS32();
		entry.fileName = pByteCode->GetString();
		entry.pNode = reinterpret_cast<void*>(static_cast<uintptr_t>(pByteCode->GetU32()));
	}

	this->edSoundSample.loopEndOffset = static_cast<uint>(pByteCode->GetF32());
	this->field_0x6c = pByteCode->GetF32();
	this->field_0x70 = pByteCode->GetF32();
	this->field_0x74 = pByteCode->GetF32();
	this->field_0x78 = pByteCode->GetF32();
	this->priority = pByteCode->GetF32();
	this->field_0x80 = pByteCode->GetU32();
	this->field_0x84 = pByteCode->GetU32();
	this->field_0x88 = pByteCode->GetU32();
}

CMusicAmbiance::CMusicAmbiance()
{
	this->aSoundAmbiance = (CSoundAmbiance*)0x0;

	return;
}

void CMusicAmbiance::Add(ByteCode* pByteCode)
{
	uint uVar1;
	CSoundAmbiance* pCurAmbiance;
	float fVar6;

	fVar6 = pByteCode->GetF32();
	this->field_0x8 = fVar6;
	if (fVar6 < 0.0f) {
		this->field_0x8 = 0.0f;
	}

	fVar6 = pByteCode->GetF32();
	this->field_0xc = fVar6;
	if (fVar6 < 0.0f) {
		this->field_0xc = 0.0f;
	}

	this->nbSoundAmbiance = pByteCode->GetU32();
	uVar1 = this->nbSoundAmbiance;
	if (uVar1 == 0) {
		this->aSoundAmbiance = (CSoundAmbiance*)0x0;
	}
	else {
		this->aSoundAmbiance = new CSoundAmbiance[uVar1];

		pCurAmbiance = this->aSoundAmbiance;
		for (uVar1 = this->nbSoundAmbiance; uVar1 != 0; uVar1 = uVar1 - 1) {
			pCurAmbiance->field_0x0.index = pByteCode->GetS32();
			pCurAmbiance->field_0x4.index = pByteCode->GetS32();
			pCurAmbiance->field_0x8.index = pByteCode->GetS32();
			pCurAmbiance->field_0xc = pByteCode->GetF32();
			pCurAmbiance->field_0x10 = pByteCode->GetF32();
			pCurAmbiance->wayPointRef.index = pByteCode->GetS32();
			pCurAmbiance = pCurAmbiance + 1;
		}
	}

	return;
}
