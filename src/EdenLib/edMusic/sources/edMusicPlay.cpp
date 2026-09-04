#include "edMusic/edMusic.h"

edCMusicStatus edMusicStatus;

void edMusicSetMasterVolume(uint newVolume)
{
	uint local_4;

	edMusicStatus.masterVolume = newVolume;

	IMPLEMENTATION_GUARD_PS2(
		local_4 = newVolume;
	_edMusicQueueCommand(4, 0, &local_4, 4);)

		return;
}

void edMusicSetMasterTempo(uint newTempo)
{
	uint local_4;

	edMusicStatus.masterTempo = newTempo;
	IMPLEMENTATION_GUARD_PS2(
	local_4 = newTempo;
	_edMusicQueueCommand(5, 0, &local_4, 4);)

	return;
}

void edMusicSetEndOfSongCallback(edMusicEndOfSongCallback pCallback)
{
	edMusicStatus.endOfSongCallback = pCallback;
	return;
}

#define ED_MUSIC_FADE_VOLUME_TEMPO	0x1
#define ED_MUSIC_FADE_CHANNELS		0x2
#define ED_MUSIC_FADE_TICK			0.02f
#define ED_MUSIC_CHANNEL_COUNT		16

void _edMusicStreamSetChannelVolume(ed_music_stream* pMusicStream, int channel, int volume)
{
#ifdef PLATFORM_PS2
	int local_8;
	int local_4;

	local_8 = channel;
	local_4 = volume;
	_edMusicQueueCommand(0xf, pMusicStream->index, &local_8, 8);
#else

#endif
	return;
}

void _edMusicStreamPause(ed_music_stream* pMusicStream)
{
#ifdef PLATFORM_PS2
	undefined4 local_4;

	local_4 = 2;
	_edMusicQueueCommand(10, pMusicStream->index, &local_4, 4);
#else

#endif
	return;
}


void _edMusicStreamSetTempo(ed_music_stream* pMusicStream, int tempo)
{
#ifdef PLATFORM_PS2
	int local_4;

	local_4 = tempo;
	_edMusicQueueCommand(0x10, pMusicStream->index, &local_4, 4);
#else

#endif
	return;
}

void _edMusicStreamSetVolume(ed_music_stream* pMusicStream, int volume)
{
#ifdef PLATFORM_PS2
	int local_4;

	local_4 = volume;
	_edMusicQueueCommand(0xe, pMusicStream->index, &local_4, 4);
#else

#endif
	return;
}

void _edMusicWaitHardwareUpdate(void)
{
#ifdef PLATFORM_PS2
	_edSysWaitRPCCompletion((int*)(_pedMusicRPCClient + 8), *(int*)(_pedMusicRPCClient + 4), (sceSifClientData*)(_pedMusicRPCClient + 0xc));
#endif
	return;
}

uint _edMusicFlush(void)
{
	uint* puVar1;
	int iVar2;
	uint result;
	ulong uVar4;
#ifdef PLATFORM_PS2
	_edMusicCommandQueueFilter();
	puVar1 = edMusicCallbacksExecute(&_edMusicReturnQueue);
	uVar3 = 0;
	if (edMusicStatus.nbStreams != 0) {
		iVar2 = 0;
		do {
			uVar3 = uVar3 + 1;
			*(uint*)((int)pedMusicStreams->aTargetChannelVolumes + iVar2 + 0x54) = *puVar1;
			puVar1 = puVar1 + 1;
			iVar2 = iVar2 + 0xc4;
		} while (uVar3 < (uint)edMusicStatus.nbStreams);
	}
	uVar4 = SEXT48((int)_edMusicCommandQueue);
	if ((uVar4 & 0xf) != 0) {
		uVar4 = SEXT48((int)((_edMusicCommandQueue & 0xfffffff0) + 0x10));
	}
	uVar3 = _edSysCallRPC(2, 0, 1, 0x4822b0, uVar4, 0x4826b0, 0x400, 0, (uint*)0x0, *(int*)(_pedMusicRPCClient + 4), (sceSifClientData*)(_pedMusicRPCClient + 0xc), (int*)(_pedMusicRPCClient + 8));
	_edMusicCommandQueuePtr = &_edMusicUnfilteredCommandQueue;
#else
	// TODO(Windows): _edMusicFlush only submits PS2 music commands through IOP RPC.
	// The PC port currently has no streamed-music backend, so command submission
	// and IOP return-queue processing are intentionally omitted. When a backend is
	// added, submit pending stream state here and poll it for completion/end-of-song
	// events, invoking edMusicStatus.endOfSongCallback on the main thread.
	result = 0;
#endif
	return result;
}

uint edMusicFlush(void)
{
	for (int streamIndex = edMusicStatus.nbStreams - 1;
		streamIndex >= 0;
		--streamIndex)
	{
		ed_music_stream* stream = &pedMusicStreams[streamIndex];
		ed_music_stream* hardwareStream = &pedMusicStreams[stream->index];

		if (stream->fadeFlags & ED_MUSIC_FADE_VOLUME_TEMPO) {
			if (stream->volumeTempoFadeTime > ED_MUSIC_FADE_TICK) {
				const float progress = ED_MUSIC_FADE_TICK / stream->volumeTempoFadeTime;

				stream->volume = (uint32_t)(
					(float)stream->volume +
					progress * ((float)stream->targetVolume - (float)stream->volume));
				_edMusicStreamSetVolume(hardwareStream, stream->volume);

				stream->tempo = (uint32_t)(
					(float)stream->tempo +
					progress * ((float)stream->targetTempo - (float)stream->tempo));
				_edMusicStreamSetTempo(hardwareStream, stream->tempo);

				stream->volumeTempoFadeTime -= ED_MUSIC_FADE_TICK;
			}
			else {
				stream->fadeFlags &= ~ED_MUSIC_FADE_VOLUME_TEMPO;
				stream->volumeTempoFadeTime = 0.0f;

				_edMusicStreamSetVolume(hardwareStream, stream->volume);
				_edMusicStreamSetTempo(hardwareStream, stream->tempo);

				if ((stream->targetVolume == 0 || stream->targetTempo == 0) &&
					hardwareStream->field_0xc != 1)
				{
					_edMusicStreamPause(hardwareStream);
					hardwareStream->field_0xc = 1;
				}
			}
		}

		if (stream->fadeFlags & ED_MUSIC_FADE_CHANNELS) {
			if (stream->channelFadeTime > ED_MUSIC_FADE_TICK) {
				const float progress = ED_MUSIC_FADE_TICK / stream->channelFadeTime;

				for (int channel = ED_MUSIC_CHANNEL_COUNT - 1; channel >= 0; --channel) {
					const int volume = (int)(
						(float)stream->aChannelVolumes[channel] +
						progress * (
							(float)stream->aTargetChannelVolumes[channel] -
							(float)stream->aChannelVolumes[channel]));

					hardwareStream->aChannelVolumes[channel] = volume;
					_edMusicStreamSetChannelVolume(hardwareStream, channel, volume);
				}

				stream->channelFadeTime -= ED_MUSIC_FADE_TICK;
			}
			else {
				stream->fadeFlags &= ~ED_MUSIC_FADE_CHANNELS;
				stream->channelFadeTime = 0.0f;

				for (int channel = ED_MUSIC_CHANNEL_COUNT - 1; channel >= 0; --channel) {
					const int volume = stream->aTargetChannelVolumes[channel];

					hardwareStream->aChannelVolumes[channel] = volume;
					_edMusicStreamSetChannelVolume(hardwareStream, channel, volume);
				}
			}
		}
	}

	_edMusicWaitHardwareUpdate();
	return _edMusicFlush();
}

void edMusicSetReverb(float param_1, float param_2, float param_3, float param_4, ED_SOUND_REVERB_TYPE param_5)
{
	IMPLEMENTATION_GUARD_PS2(
	uint uVar1;
	int iVar2;

	uVar1 = edMusicStatus.field_0x18 - 1;
	if (edMusicStatus.field_0x18 - 1 == 0xffffffff) {
		uVar1 = edSoundParam.field_0x4;
	}
	iVar2 = (int)edComBuffer + (int)*edComBuffer;)

	edMusicStatus.reverbType = param_5;
	edMusicStatus.field_0x30 = param_1;
	edMusicStatus.field_0x34 = param_2;
	edMusicStatus.field_0x38 = param_3;
	edMusicStatus.field_0x3c = param_4;

	IMPLEMENTATION_GUARD_PS2(
	*(undefined4*)(iVar2 + 4) = 5;
	*(ED_SOUND_REVERB_TYPE*)(iVar2 + 8) = param_5;
	*(float*)(iVar2 + 0xc) = param_1;
	*(float*)(iVar2 + 0x10) = param_2;
	*(float*)(iVar2 + 0x14) = param_3;
	*(float*)(iVar2 + 0x18) = param_4;
	*(undefined4*)(iVar2 + 0x1c) = 0;
	gSoundFlushCurrent_0044915c = (SoundFlush_0x8*)(iVar2 + 0x20);
	gSoundFlushCurrent_0044915c->pToDelete = uVar1;
	*edComBuffer = (SoundFlush_0x8*)((int)gSoundFlushCurrent_0044915c - (int)edComBuffer);)

	return;
}

void _edMusicStreamMute(ed_music_stream* pMusicStream)
{
	IMPLEMENTATION_GUARD_PS2(
	_edMusicQueueCommand(0x11, pMusicStream->index, (void*)0x0, 0);)

	return;
}

void edMusicStopAllSongs(void)
{
	uint uVar2;
	ed_music_stream* pMusicStream;

	uVar2 = 0;
	if (edMusicStatus.nbStreams != 0) {
		do {
			pMusicStream = pedMusicStreams + uVar2;
			if (pMusicStream->field_0xc != 1) {
				_edMusicStreamPause(pMusicStream);
				pMusicStream->field_0xc = 1;
			}

			_edMusicStreamMute(pedMusicStreams + uVar2);
			uVar2 = uVar2 + 1;
		} while (uVar2 < (uint)edMusicStatus.nbStreams);
	}

	return;
}

uint edMusicBankInstallNoWait(void* pAdpcm, void* pFileData, uint dataSize, uint size)
{
	_ed_music_bank* pMusicMank;
	uint nextFreeIndex;

	nextFreeIndex = 0;
	pMusicMank = _pedMusicBanks;
	if (edMusicStatus.nbBanks != 0) {
		do {
			if ((pMusicMank->flags & 1) != 0) break;

			nextFreeIndex = nextFreeIndex + 1;
			pMusicMank = pMusicMank + 1;
		} while (nextFreeIndex < edMusicStatus.nbBanks);
	}

	if (nextFreeIndex == edMusicStatus.nbBanks) {
		do {
			// CRASH?
		} while (true);
	}

	// Mark in use.
	pMusicMank->flags = pMusicMank->flags & 0xfffffffe;

	_edMusicBankInstallNoWait(_pedMusicBanks + nextFreeIndex, pFileData, pAdpcm, size, dataSize);

	return nextFreeIndex;
}

void edMusicSongRemove(int songIndex)
{
	ed_music_song* pSong;

	pSong = _pedMusicSongs + songIndex;
	_edMusicSongRemove(pSong);
	pSong->flags = pSong->flags | 1;

	return;
}


void edMusicBankRemove(int bankIndex)
{
	_ed_music_bank* pMusicBank;

	pMusicBank = _pedMusicBanks + bankIndex;
	_edMusicBankRemove(pMusicBank);
	pMusicBank->flags = pMusicBank->flags | 1;

	return;
}

bool edMusicAreAllMusicDataLoaded()
{
	return _edMusicAreAllMusicDataLoaded();
}

void edMusicStreamChannelFade(float fadeTime, int musicStreamIndex, int channel, uint otherVolume, int volume)
{
	ed_music_stream* pMusicStream;

	pMusicStream = pedMusicStreams + musicStreamIndex;
	pMusicStream->fadeFlags = pMusicStream->fadeFlags | 2;
	if (otherVolume != 0x10000) {
		pMusicStream->aTargetChannelVolumes[channel] = otherVolume;
	}

	pMusicStream->aTargetChannelVolumes[channel] = volume;
	pMusicStream->channelFadeTime = fadeTime;

	return;
}

uint edMusicSongInstallNoWait(void* pFileData, uint size)
{
	ed_music_song* peVar1;
	uint uVar2;

	uVar2 = 0;
	peVar1 = _pedMusicSongs;
	if (edMusicStatus.nbSongs != 0) {
		do {
			if ((peVar1->flags & 1) != 0) break;
			uVar2 = uVar2 + 1;
			peVar1 = peVar1 + 1;
		} while (uVar2 < (uint)edMusicStatus.nbSongs);
	}
	if (uVar2 == edMusicStatus.nbSongs) {
		do {
			/* WARNING: Do nothing block with infinite loop */
		} while (true);
	}
	peVar1->flags = peVar1->flags & 0xfffffffe;
	_edMusicSongInstallNoWait(_pedMusicSongs + uVar2, pFileData, size);
	return uVar2;
}

