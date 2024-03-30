#pragma once

#include <string>
#include <optional>
#include <nlohmann/json.hpp>


namespace DebugSetting {
	using Settings = std::optional<nlohmann::json>;

	Settings LoadSettings(bool bCreateIfNotExisting = false);

	template<typename SettingType>
	class Setting {
	public:
		Setting(const char* inName, SettingType defaultValue)
			: name(inName)
			, value(defaultValue)
		{
			// Load the value so we have it at runtime
			auto settings = LoadSettings();
			if (settings) {
				if(settings->contains(name)) {
					value = (*settings)[name].get<SettingType>();
				}
			}
		}

		bool DrawImguiControl();

		virtual operator SettingType() const { return value; }

	protected:
		std::string name;
		SettingType value;
	};

	class ConditionalBoolSetting : public Setting<bool> {
	public:
		ConditionalBoolSetting(const char* inName, bool defaultValue)
			: Setting(inName, defaultValue)
		{
		}

		virtual operator bool() const override {
			if (bCondition) {
				return false;
			}

			return value; 
		}

		void MarkConditionCompleted() {
			bCondition = true;
		}

		bool bCondition = false;
	};

	class DoOnceBoolSetting : public ConditionalBoolSetting {
	public:

		DoOnceBoolSetting(const char* inName, bool defaultValue)
			: ConditionalBoolSetting(inName, defaultValue)
		{
		}

		operator bool() {
			if (bCondition) {
				return false;
			}

			MarkConditionCompleted();
			return value;
		}
	};
}