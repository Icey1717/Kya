#pragma once
#include <stdexcept>
#include <string>

#include <GLFW/glfw3.h>

#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

constexpr int MAX_FRAMES_IN_FLIGHT = 2;

inline void CheckVk(VkResult result, const char* operation)
{
	if (result != VK_SUCCESS) {
		throw std::runtime_error(std::string(operation) + " failed");
	}
}
