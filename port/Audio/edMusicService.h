#pragma once
#include "edMusicControls.h"
#include <cstdint>
#include <memory>
#include <vector>

namespace Audio
{
enum class MusicCommandType { Song, Bank, Restart, Resume, Pause, Mute, ChannelVolume };
struct MusicCommand { MusicCommandType type; unsigned stream; unsigned value = 0; unsigned channel = 0; };
class MusicOutput
{
public:
	virtual ~MusicOutput() = default;
	virtual bool Submit(const float* samples, unsigned count) = 0;
	virtual unsigned Queued() = 0;
	virtual bool Play(bool playing) = 0;
};
using MusicOutputFactory = std::unique_ptr<MusicOutput>(*)();

void InitializeMusic(unsigned streams, unsigned voices);
void ResetMusic();
bool InstallMusicSong(unsigned index, std::uint32_t handle);
bool InstallMusicBank(unsigned index, std::uint32_t headerHandle, std::uint32_t bodyHandle);
void RemoveMusicSong(unsigned index);
void RemoveMusicBank(unsigned index);
void QueueMusicCommand(MusicCommand command);
std::vector<unsigned> FlushMusicCommands(const std::vector<MusicControls>& controls);
// Deterministic output tests can service queued audio explicitly without a worker.
void SetMusicOutputFactory(MusicOutputFactory factory);
void ServiceMusic();
}
