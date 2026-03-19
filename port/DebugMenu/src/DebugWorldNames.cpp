#include "DebugWorldNames.h"

#include <fstream>
#include <string>
#include <nlohmann/json.hpp>

namespace Debug {
namespace WorldNames {

	/*
	* Let's you name a sector or checkpoint for easier identification in the debug menu. 
	* Names are stored in a JSON file in the same directory as the executable, so they are on a 
	* per machine basis and not shared between users.
	*/

	static constexpr const char* kNamesFile = "world_names.json";

	// In-memory store: json["levelId"]["sectors"]["sectorId"] = "Name"
	//                  json["levelId"]["checkpoints"]["managerName/index"] = "Name"
	static nlohmann::json gNames;
	static bool gLoaded = false;

	static void EnsureLoaded()
	{
		if (gLoaded) {
			return;
		}
		gLoaded = true;

		std::ifstream file(kNamesFile);
		if (file) {
			gNames = nlohmann::json::parse(file, nullptr, /*allow_exceptions=*/false);
			if (gNames.is_discarded()) {
				gNames = nlohmann::json::object();
			}
		}
		else {
			gNames = nlohmann::json::object();
		}
	}

	static void Save()
	{
		std::ofstream file(kNamesFile);
		file << gNames.dump(2);
	}

	static std::string LevelKey(int levelId)
	{
		return std::to_string(levelId);
	}

	static std::string CheckpointKey(const char* managerName, int checkpointIndex)
	{
		return std::string(managerName ? managerName : "") + "/" + std::to_string(checkpointIndex);
	}

	std::string GetSectorName(int levelId, int sectorId)
	{
		EnsureLoaded();
		const std::string levelKey = LevelKey(levelId);
		const std::string sectorKey = std::to_string(sectorId);
		if (gNames.contains(levelKey) &&
			gNames[levelKey].contains("sectors") &&
			gNames[levelKey]["sectors"].contains(sectorKey))
		{
			return gNames[levelKey]["sectors"][sectorKey].get<std::string>();
		}
		return {};
	}

	void SetSectorName(int levelId, int sectorId, const std::string& name)
	{
		EnsureLoaded();
		const std::string levelKey = LevelKey(levelId);
		const std::string sectorKey = std::to_string(sectorId);
		if (name.empty()) {
			if (gNames.contains(levelKey) && gNames[levelKey].contains("sectors")) {
				gNames[levelKey]["sectors"].erase(sectorKey);
			}
		}
		else {
			gNames[levelKey]["sectors"][sectorKey] = name;
		}
		Save();
	}

	std::string GetCheckpointName(int levelId, const char* managerName, int checkpointIndex)
	{
		EnsureLoaded();
		const std::string levelKey = LevelKey(levelId);
		const std::string cpKey = CheckpointKey(managerName, checkpointIndex);
		if (gNames.contains(levelKey) &&
			gNames[levelKey].contains("checkpoints") &&
			gNames[levelKey]["checkpoints"].contains(cpKey))
		{
			return gNames[levelKey]["checkpoints"][cpKey].get<std::string>();
		}
		return {};
	}

	void SetCheckpointName(int levelId, const char* managerName, int checkpointIndex, const std::string& name)
	{
		EnsureLoaded();
		const std::string levelKey = LevelKey(levelId);
		const std::string cpKey = CheckpointKey(managerName, checkpointIndex);
		if (name.empty()) {
			if (gNames.contains(levelKey) && gNames[levelKey].contains("checkpoints")) {
				gNames[levelKey]["checkpoints"].erase(cpKey);
			}
		}
		else {
			gNames[levelKey]["checkpoints"][cpKey] = name;
		}
		Save();
	}

} // namespace WorldNames
} // namespace Debug
