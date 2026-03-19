#pragma once

#include <string>

// Human-readable names for sectors and checkpoints, persisted in world_names.json.
// Names are keyed by level ID so the same sector/checkpoint index in different
// levels can carry different labels.

namespace Debug {
namespace WorldNames {

	// Returns the human-readable name for a sector, or "" if not assigned.
	std::string GetSectorName(int levelId, int sectorId);

	// Assigns a human-readable name to a sector and saves the file.
	// Pass an empty string to clear the name.
	void SetSectorName(int levelId, int sectorId, const std::string& name);

	// Returns the human-readable name for a checkpoint, or "" if not assigned.
	// managerName must be the null-terminated name field of the CActorCheckpointManager.
	std::string GetCheckpointName(int levelId, const char* managerName, int checkpointIndex);

	// Assigns a human-readable name to a checkpoint and saves the file.
	// Pass an empty string to clear the name.
	void SetCheckpointName(int levelId, const char* managerName, int checkpointIndex, const std::string& name);

} // namespace WorldNames
} // namespace Debug
