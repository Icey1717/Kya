#pragma once

#include <chrono>

class ScopedTimer {
public:
	ScopedTimer(double& outDuration) : duration(outDuration), start(std::chrono::high_resolution_clock::now()) {}

	~ScopedTimer() {
		auto end = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double, std::milli> durationMilli = end - start;
		duration = durationMilli.count();
	}

private:
	double& duration;
	std::chrono::time_point<std::chrono::high_resolution_clock> start;
};