#pragma once

#ifdef _WIN32
struct IXAudio2;
struct IXAudio2MasteringVoice;
namespace Audio
{
IXAudio2* GetAudioEngine();
IXAudio2MasteringVoice* GetAudioMasteringVoice();
// Call only after both sample and stream voices have been destroyed.
void ShutdownAudioDevice();
}
#endif
