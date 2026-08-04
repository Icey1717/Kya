#pragma once
#include <cstddef>
#include <cstdint>
namespace Audio
{
using TransferCallback = void (*)(void*);
enum class TransferFlags : std::uint32_t
{
	None = 0,
	OneBlockOnly = 1,
	KeepInIop = 2,
	HighMem = 4
};
struct TransferInfo
{
	std::uint32_t index = 0, size = 0, alignment = 0;
	TransferFlags flags = TransferFlags::None;
};
struct LoadedDataInfo
{
	const std::uint8_t* data = nullptr;
	std::size_t size = 0;
	std::uint32_t alignment = 0;
	TransferFlags flags = TransferFlags::None;
};
void Initialize();
void Shutdown();
std::uint32_t Submit(const void*, std::uint32_t, std::uint32_t, TransferFlags, int, int, int, void*, std::uint32_t, std::uint32_t,
					 TransferCallback);
std::uint32_t PumpThrough(std::uint32_t);
std::uint32_t PumpAll();
bool GetTransferInfo(std::uint32_t, TransferInfo&);
bool LookupLoadedData(std::uint32_t, LoadedDataInfo&);
bool ReleaseLoadedData(std::uint32_t);
void Reset();
} // namespace Audio
