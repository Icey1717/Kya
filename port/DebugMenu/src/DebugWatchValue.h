#pragma once

#include <algorithm>
#include <optional>
#include <string>
#include <utility>

namespace Debug::Watch {
	struct ValueState {
		std::optional<std::string> value;
		double changedAt = -10.0;
		bool sampled = false;

		void Update(std::optional<std::string> next, double now)
		{
			if (sampled && next != value) changedAt = now;
			value = std::move(next);
			sampled = true;
		}

		float Highlight(double now) const
		{
			return value ? float(std::clamp(1.0 - (now - changedAt) / 0.8, 0.0, 1.0)) : 0.0f;
		}
	};
}
