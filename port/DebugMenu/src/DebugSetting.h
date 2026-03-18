#pragma once

#include <array>
#include <vector>
#include <string>
#include <optional>
#include <nlohmann/json.hpp>


namespace Debug {
	using Settings = std::optional<nlohmann::json>;

	void SaveSettings(Settings settings);
	Settings LoadSettings(bool bCreateIfNotExisting = false);

	template<typename SettingType>
	class Setting {
	public:
		Setting(const char* inName, SettingType defaultValue)
			: name(inName)
			, value(defaultValue)
		{
			Init();
		}

		Setting(std::string inName, SettingType defaultValue)
			: name(inName)
			, value(defaultValue)
		{
			Init();
		}

		inline void Init()
		{
			// Load the value so we have it at runtime
			auto settings = LoadSettings();
			if (settings) {
				if (settings->contains(name)) {
					value = (*settings)[name].get<SettingType>();
				}
			}
		}

		bool UpdateValue()
		{
			Settings settings = LoadSettings(true);

			if (settings) {
				(*settings)[name.c_str()] = value;
				SaveSettings(settings);
				return true;
			}

			return false;
		}

		bool DrawImguiControl();

		virtual operator SettingType() const { return value; }
		virtual operator SettingType&() { return value; }
		const SettingType& get() const { return value; }

		inline Setting& operator=(const SettingType& newValue) {
			value = newValue;
			UpdateValue();
			return *this;
		}


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

	// A Setting<int> that renders as a labelled combo box in ImGui.
	// Values map to the indices of the labels array passed at construction.
	class ComboSetting : public Setting<int> {
	public:
		ComboSetting(const char* inName, int defaultValue, std::initializer_list<const char*> inLabels)
			: Setting<int>(inName, defaultValue)
			, labels(inLabels)
		{}

		bool DrawImguiControl();

	private:
		std::vector<const char*> labels;
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