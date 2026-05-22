#include "pointer_conv.h"
#include <cassert>
#include <cstdint>
#include <unordered_map>
#include <vector>

#ifdef PLATFORM_WIN
namespace PointerConv
{
	namespace
	{
		constexpr uint32_t HANDLE_SLOT_BITS = 20;
		constexpr uint32_t HANDLE_SLOT_MASK = (1u << HANDLE_SLOT_BITS) - 1;
		constexpr uint32_t HANDLE_MAX_SLOTS = HANDLE_SLOT_MASK + 1;
		constexpr uint32_t HANDLE_GENERATION_MASK = 0x7ff;
		constexpr uint32_t HANDLE_GENERATION_SHIFT = HANDLE_SLOT_BITS;
		constexpr uint32_t HANDLE_TRANSIENT_BIT = 0x80000000;

		struct PointerSlot {
			void* pValue = nullptr;
			uint32_t refCount = 0;
			uint16_t generation = 1;
			int nextFree = -1;
		};

		struct PointerRegistry {
			std::vector<PointerSlot> slots;
			std::unordered_map<void*, int> pointerToHandle;
			int freeListHead = -1;
			size_t liveHandleCount = 0;
		};

		PointerRegistry gPersistentRegistry;
		PointerRegistry gTransientRegistry;

		inline bool IsTransientHandle(int handle)
		{
			return (static_cast<uint32_t>(handle) & HANDLE_TRANSIENT_BIT) != 0;
		}

		inline uint32_t DecodeSlotIndex(int handle)
		{
			return static_cast<uint32_t>(handle) & HANDLE_SLOT_MASK;
		}

		inline uint16_t DecodeGeneration(int handle)
		{
			return static_cast<uint16_t>((static_cast<uint32_t>(handle) >> HANDLE_GENERATION_SHIFT) & HANDLE_GENERATION_MASK);
		}

		inline int EncodeHandle(uint32_t slotIndex, uint16_t generation, bool bTransient)
		{
			uint32_t encodedHandle = slotIndex | (static_cast<uint32_t>(generation) << HANDLE_GENERATION_SHIFT);
			if (bTransient) {
				encodedHandle |= HANDLE_TRANSIENT_BIT;
			}

			return static_cast<int>(encodedHandle);
		}

		inline uint16_t NextGeneration(uint16_t generation)
		{
			generation = static_cast<uint16_t>((generation + 1) & HANDLE_GENERATION_MASK);
			return generation == 0 ? 1 : generation;
		}

		PointerRegistry& GetRegistry(bool bTransient)
		{
			return bTransient ? gTransientRegistry : gPersistentRegistry;
		}

		std::optional<void*> ResolvePointerInternal(PointerRegistry& registry, int key)
		{
			uint32_t slotIndex = DecodeSlotIndex(key);
			if (slotIndex >= registry.slots.size()) {
				return std::nullopt;
			}

			PointerSlot& slot = registry.slots[slotIndex];
			if ((slot.refCount == 0) || (slot.generation != DecodeGeneration(key))) {
				return std::nullopt;
			}

			return slot.pValue;
		}

		int AddPointerInternal(PointerRegistry& registry, void* value, bool bTransient)
		{
			if (value == nullptr) {
				return 0;
			}

			auto existingEntry = registry.pointerToHandle.find(value);
			if (existingEntry != registry.pointerToHandle.end()) {
				std::optional<void*> resolvedPointer = ResolvePointerInternal(registry, existingEntry->second);
				if (resolvedPointer.has_value()) {
					PointerSlot& slot = registry.slots[DecodeSlotIndex(existingEntry->second)];
					slot.refCount++;
					return existingEntry->second;
				}

				registry.pointerToHandle.erase(existingEntry);
			}

			uint32_t slotIndex;
			if (registry.freeListHead != -1) {
				slotIndex = static_cast<uint32_t>(registry.freeListHead);
				PointerSlot& slot = registry.slots[slotIndex];
				registry.freeListHead = slot.nextFree;
				slot.nextFree = -1;
			}
			else {
				assert(registry.slots.size() < HANDLE_MAX_SLOTS);
				slotIndex = static_cast<uint32_t>(registry.slots.size());
				registry.slots.emplace_back();
			}

			PointerSlot& slot = registry.slots[slotIndex];
			slot.pValue = value;
			slot.refCount = 1;

			int handle = EncodeHandle(slotIndex, slot.generation, bTransient);
			registry.pointerToHandle[value] = handle;
			registry.liveHandleCount++;
			return handle;
		}

		void ReleasePointerInternal(PointerRegistry& registry, int key)
		{
			if (key == 0) {
				return;
			}

			uint32_t slotIndex = DecodeSlotIndex(key);
			if (slotIndex >= registry.slots.size()) {
				assert(false);
				return;
			}

			PointerSlot& slot = registry.slots[slotIndex];
			if ((slot.refCount == 0) || (slot.generation != DecodeGeneration(key))) {
				assert(false);
				return;
			}

			slot.refCount--;
			if (slot.refCount != 0) {
				return;
			}

			registry.pointerToHandle.erase(slot.pValue);
			slot.pValue = nullptr;
			slot.generation = NextGeneration(slot.generation);
			slot.nextFree = registry.freeListHead;
			registry.freeListHead = slotIndex;
			registry.liveHandleCount--;
		}

		void ResetRegistry(PointerRegistry& registry)
		{
			registry.pointerToHandle.clear();
			registry.slots.clear();
			registry.freeListHead = -1;
			registry.liveHandleCount = 0;
		}
	}
}

int PointerConv::AddPointer(void* value)
{
	return AddPointerInternal(gPersistentRegistry, value, false);
}

int PointerConv::AddTransientPointer(void* value)
{
	return AddPointerInternal(gTransientRegistry, value, true);
}

std::optional<void*> PointerConv::ResolvePointer(int key)
{
	if (key == 0) {
		return nullptr;
	}

	PointerRegistry& registry = GetRegistry(IsTransientHandle(key));
	return ResolvePointerInternal(registry, key);
}

void* PointerConv::ResolvePointerChecked(int key)
{
	if (key == 0)
	{
		return (void*)0x0;
	}

	std::optional<void*> value = ResolvePointer(key);
	assert(value.has_value());
	return *value;
}

void PointerConv::ReleasePointer(int key)
{
	PointerRegistry& registry = GetRegistry(IsTransientHandle(key));
	ReleasePointerInternal(registry, key);
}

void PointerConv::ResetTransient()
{
	ResetRegistry(gTransientRegistry);
}

void PointerConv::ResetAll()
{
	ResetRegistry(gPersistentRegistry);
	ResetRegistry(gTransientRegistry);
}

size_t PointerConv::GetLiveHandleCount()
{
	return gPersistentRegistry.liveHandleCount + gTransientRegistry.liveHandleCount;
}

size_t PointerConv::GetCapacity()
{
	return gPersistentRegistry.slots.size() + gTransientRegistry.slots.size();
}
#endif
