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
	struct Menu
	{
		void Show() {
			if (bOpen) {
				ShowFunction(&bOpen);
			}
		}

		std::string name{};
		std::function<void(bool*)> ShowFunction;
		bool bOpen{};
	};

	struct MenuRegisterer
	{
		MenuRegisterer(const std::string& name, std::function<void(bool*)> showFunction)
		{
			if (!menus) {
				menus = std::make_unique<std::vector<Menu>>();
			}

			menus->push_back({
				name,
				showFunction,
				false
				});
		}

		static std::vector<Menu>& GetMenus() {
			return *menus;
		}

		static std::unique_ptr<std::vector<Menu>> menus;
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