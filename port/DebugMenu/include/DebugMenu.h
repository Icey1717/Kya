#pragma once
#include <stdint.h>
#include <vector>
#include <string>
#include <functional>
#include <memory>

namespace Input
{
	struct InputFunctions;
}

namespace Debug
{
	template<typename SettingType>
	class Setting;

	struct Menu
	{
		void Show();
		bool GetOpen() const;
		void SetOpen(bool bNewOpen);

		std::string name{};
		std::function<void(bool*)> ShowFunction;
		std::shared_ptr<Debug::Setting<bool>> pOpenSetting;
	};

	struct MenuRegisterer
	{
		MenuRegisterer(const std::string& name, std::function<void(bool*)> showFunction, bool bDefaultOpen = false);

		static std::vector<Menu>& GetMenus();

		static std::unique_ptr<std::vector<Menu>> gMenus;
	};

	// Registers a callback to be called once per frame from BuildImguiCommands.
	struct UpdateRegisterer
	{
		UpdateRegisterer(std::function<void()> updateFunction);

		static std::vector<std::function<void()>>& GetCallbacks();

		static std::unique_ptr<std::vector<std::function<void()>>> gCallbacks;
	};

	// Registers a callback to be called once from DebugMenu::Init.
	struct StartupRegisterer
	{
		StartupRegisterer(std::function<void()> startupFunction);

		static std::vector<std::function<void()>>& GetCallbacks();

		static std::unique_ptr<std::vector<std::function<void()>>> gCallbacks;
	};
}

namespace DebugMenu
{
	void BuildImguiCommands();
	double GetDeltaTime();
	void Init();

	void AddKeyboardMouseSupport();

	bool GetMousePressed(uint32_t routeId);
	bool GetMouseReleased(uint32_t routeId);
	float GetMouseAnalog(uint32_t routeId);

	bool GetKeyPressed(uint32_t routeId);
	bool GetKeyReleased(uint32_t routeId);
	float GetKeyAnalog(uint32_t routeId);
}