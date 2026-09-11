#pragma once
#include "edMusicData.h"
#include "edMusicControls.h"
#include <memory>

namespace Audio
{
// Deterministic renderer; its owner serializes access. It never touches Eden data.
class MusicSynth
{
public:
	MusicSynth(std::shared_ptr<const MusicSong> song, std::shared_ptr<const MusicBank> bank, unsigned voiceLimit);
	~MusicSynth();
	void Restart();
	void SetControls(const MusicControls& controls);
	void Render(std::span<float> stereo);
	bool Finished() const;
	std::uint64_t Frames() const;
private:
	struct State;
	std::unique_ptr<State> state;
};
}
