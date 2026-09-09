#include "edSysTransferService.h"
#include "edSoundSampleService.h"
#include "edSoundStreamService.h"
#include "edSoundDevice.h"
#include "log.h"
#include <cstring>
#include <deque>
#include <unordered_map>
#include <vector>

namespace Audio
{
namespace
{

constexpr std::size_t MaxReturnBytes = 256;
struct Loaded
{
	std::vector<std::uint8_t> bytes;
	std::uint32_t alignment;
	TransferFlags flags;
};

struct Pending
{
	TransferInfo info;
	std::vector<std::uint8_t> bytes;
	int setup, transfer, end;
	void* params;
	std::uint32_t returnBytes, userData;
	TransferCallback callback;
	std::vector<std::uint8_t> result;
	std::uint32_t handle = 0;
};

std::deque<Pending> queue;
std::unordered_map<std::uint32_t, Loaded> loaded;
std::unordered_map<std::uint32_t, TransferInfo> transfers;
std::uint32_t nextIndex = 1, nextHandle = 1, completed = 0;

void Error(const char* message, int id)
{
	Log::GetInstance().AddLog(LogLevel::Error, "Audio", "%s (callback id %d)", message, id);
}

std::uint32_t NewHandle()
{
	for (;;)
	{
		auto h = nextHandle++;
		if (h && !loaded.contains(h))
			return h;
	}
}

bool Dispatch(Pending& t, int id)
{
	if (!id || (id >= 4 && id <= 6))
		return true;
	switch (id)
	{
	case 1:
		t.handle = NewHandle();
		loaded.emplace(t.handle, Loaded{{}, t.info.alignment, t.info.flags});
		return true;
	case 2:
	{
		auto i = loaded.find(t.handle);
		if (i == loaded.end())
		{
			Error("sound transfer has no setup handle", id);
			return false;
		}
		i->second.bytes = t.bytes;
		return true;
	}
	case 3:
		if (!t.handle || t.returnBytes < sizeof(t.handle))
		{
			Error("sound end callback cannot return a handle", id);
			return false;
		}
		std::memcpy(t.result.data(), &t.handle, sizeof(t.handle));
		return true;
	default:
		Error("unknown transfer callback", id);
		return false;
	}
}

void Complete(Pending& t)
{
	bool ok = Dispatch(t, t.setup);
	if (ok)
		ok = Dispatch(t, t.transfer);
	if (ok)
		ok = Dispatch(t, t.end);
	if (ok && t.params && t.returnBytes)
		std::memcpy(t.params, t.result.data(), t.returnBytes);
	if (!ok && t.handle)
		loaded.erase(t.handle);
	if (t.callback)
		t.callback(t.params);
}
} // namespace

void Initialize()
{
	Reset();
}

void Shutdown()
{
	Reset();
	ResetStreams();
#ifdef _WIN32
	ShutdownAudioDevice();
#endif
}

std::uint32_t Submit(const void* source, std::uint32_t size, std::uint32_t alignment, TransferFlags flags, int setup, int transfer, int end,
					 void* params, std::uint32_t returnBytes, std::uint32_t userData, TransferCallback callback)
{
	if (!source || !size || returnBytes > MaxReturnBytes)
		return 0;
	auto index = nextIndex++;
	Pending t{{index, size, alignment, flags},		 {}, setup, transfer, end, params, returnBytes, userData, callback,
			  std::vector<std::uint8_t>(returnBytes)};
	t.bytes.assign(static_cast<const std::uint8_t*>(source), static_cast<const std::uint8_t*>(source) + size);
	transfers.emplace(index, t.info);
	queue.emplace_back(std::move(t));
	return index;
}

std::uint32_t PumpThrough(std::uint32_t index)
{
	while (!queue.empty() && completed < index)
	{
		auto t = std::move(queue.front());
		queue.pop_front();
		Complete(t);
		completed = t.info.index;
	}
	return completed;
}

std::uint32_t PumpAll()
{
	return queue.empty() ? completed : PumpThrough(queue.back().info.index);
}

bool GetTransferInfo(std::uint32_t i, TransferInfo& out)
{
	auto it = transfers.find(i);
	if (it == transfers.end())
		return false;
	out = it->second;
	return true;
}

bool LookupLoadedData(std::uint32_t h, LoadedDataInfo& out)
{
	auto it = loaded.find(h);
	if (it == loaded.end())
		return false;
	out = {it->second.bytes.data(), it->second.bytes.size(), it->second.alignment, it->second.flags};
	return true;
}

bool ReleaseLoadedData(std::uint32_t h)
{
	InvalidateSample(h);
	return loaded.erase(h) != 0;
}

void Reset()
{
	ResetSamples();
	queue.clear();
	loaded.clear();
	transfers.clear();
	nextIndex = nextHandle = 1;
	completed = 0;
}

} // namespace Audio
