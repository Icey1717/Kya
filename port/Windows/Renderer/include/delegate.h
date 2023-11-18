#pragma once

#include <functional>
#include <vector>

template<typename... Args>
class Multidelegate {
public:
	using Delegate = std::function<void(Args...)>;

	void operator+=(const Delegate& delegate) {
		delegates.push_back(delegate);
	}

	void operator-=(const Delegate& delegate) {
		delegates.erase(std::remove(delegates.begin(), delegates.end(), delegate), delegates.end());
	}

	void operator()(Args... args) const {
		for (const auto& delegate : delegates) {
			delegate(args...);
		}
	}

	void RemoveAll() {
		delegates.clear();
	}

private:
	std::vector<Delegate> delegates;
};