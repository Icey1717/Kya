#pragma once

#include <cmath>
#include <cstdint>
#include <cstring>
#include <string_view>

namespace Debug::SaveLoad
{
	struct SavedCheckpoint
	{
		int level = -1;
		int sector = -1;
		bool hasPosition = false;
		float position[3] = {};
		float rotation[3] = {};
	};

	namespace SaveChunks
	{
		inline uint32_t ReadWord(std::string_view bytes, size_t offset)
		{
			uint32_t value;
			std::memcpy(&value, bytes.data() + offset, sizeof(value));
			return value;
		}

		inline bool Next(std::string_view& bytes, uint32_t& hash, std::string_view& payload)
		{
			// CChunk::offset is the payload length; size is a format descriptor.
			if (bytes.size() < 16) return false;
			const auto length = ReadWord(bytes, 12);
			if (length > bytes.size() - 16) return false;
			hash = ReadWord(bytes, 4);
			payload = bytes.substr(16, length);
			bytes.remove_prefix(16 + length);
			return true;
		}

		inline bool Find(std::string_view bytes, uint32_t wanted, std::string_view& payload)
		{
			uint32_t hash;
			while (!bytes.empty()) {
				if (!Next(bytes, hash, payload)) return false;
				if (hash == wanted) return true;
			}
			return false;
		}
	}

	// Read only the serialized main block. Never load it into the live scheduler.
	inline bool ReadSavedCheckpoint(std::string_view bytes, SavedCheckpoint& result)
	{
		using namespace SaveChunks;
		result = {};
		std::string_view root, header;
		uint32_t hash;
		if (!Next(bytes, hash, root) || hash != 0x56415342 || // BSAV
			!Find(root, 0x44485342, header) || header.size() < 20) return false; // BSHD
		result.level = static_cast<int>(ReadWord(header, 0));
		result.sector = static_cast<int>(ReadWord(header, 4));
		if (result.level < 0 || result.level >= 0xe) return false;
		std::string_view level;
		while (!root.empty()) {
			if (!Next(root, hash, level)) return false;
			if (hash != 0x56454c42) continue; // BLEV
			if (!Find(level, 0x44484c42, header) || header.size() < 12) return false; // BLHD
			if (ReadWord(header, 0) != result.level) continue;
			std::string_view classes, actors;
			if (!Find(level, 0x4c434c42, classes) || !Find(level, 0x43414c42, actors)) return true;
			if (classes.size() < 4) return false;
			const auto classCount = ReadWord(classes, 0);
			if (classCount > (classes.size() - 4) / 12) return false;
			for (uint32_t i = 0; i < classCount; ++i) {
				const auto mode = ReadWord(classes, 4 + i * 12);
				const auto size = ReadWord(classes, 8 + i * 12);
				const auto count = ReadWord(classes, 12 + i * 12);
				const uint64_t stride = uint64_t(size) + 4; // actor hash precedes each payload
				if (count > actors.size() / stride) return false;
				// ActorFactory's hero class: mode 3, S_SAVE_CLASS_HERO (0x138 bytes).
				if (mode == 3 && size == 0x138 && count == 1) {
					const auto hero = actors.substr(4, size);
					std::memcpy(result.position, hero.data(), sizeof(result.position));
					std::memcpy(result.rotation, hero.data() + 12, sizeof(result.rotation));
					result.sector = static_cast<int>(ReadWord(hero, 24));
					for (int axis = 0; axis < 3; ++axis) {
						if (!std::isfinite(result.position[axis]) || !std::isfinite(result.rotation[axis])) return false;
					}
					result.hasPosition = true;
					return true;
				}
				actors.remove_prefix(static_cast<size_t>(stride * count));
			}
			return true;
		}
		return true; // Fresh saves may not yet contain actor state for the starting level.
	}
}
