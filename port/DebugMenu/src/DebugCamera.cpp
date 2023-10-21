#include "DebugCamera.h"
#include "Types.h"
#include "CameraViewManager.h"
#include "GLFW/glfw3.h"
#include "imgui.h"
#include "DebugMenu.h"
#include "glm/glm.hpp"

#define NOMINMAX
#include <Windows.h>

#include <sstream>
#include <fstream>

GLFWwindow* GetGLFWWindow();

namespace DebugCamera {
	static float cameraSpeed = 100.f;
	static float cameraSensitivity = 0.001f;
	static float horizontalAngle = 3.14f;
	static float verticalAngle = 0.0f;
	static bool bActive = false;

	std::string GenerateMatrixConstructionCode(const edF32MATRIX4& matrix) {
		std::ostringstream ss;
		ss << "edF32MATRIX4 matrix = {\n";
		ss << "    " << matrix.aa << ", " << matrix.ab << ", " << matrix.ac << ", " << matrix.ad << ",\n";
		ss << "    " << matrix.ba << ", " << matrix.bb << ", " << matrix.bc << ", " << matrix.bd << ",\n";
		ss << "    " << matrix.ca << ", " << matrix.cb << ", " << matrix.cc << ", " << matrix.cd << ",\n";
		ss << "    " << matrix.da << ", " << matrix.db << ", " << matrix.dc << ", " << matrix.dd << ",\n";
		ss << "};";

		return ss.str();
	}

	void CopyMatrixCodeToClipboard(const edF32MATRIX4& matrix) {
		std::string code = GenerateMatrixConstructionCode(matrix);

		// Open the clipboard
		if (OpenClipboard(NULL)) {
			// Allocate a global memory block to hold the code
			HGLOBAL hClipboardData = GlobalAlloc(GMEM_MOVEABLE, code.size() + 1);
			if (hClipboardData) {
				// Lock the global memory block and get a pointer to it
				char* pClipboardData = static_cast<char*>(GlobalLock(hClipboardData));
				if (pClipboardData) {
					// Copy the code into the global memory block
					strcpy_s(pClipboardData, code.size() + 1, code.c_str());

					// Unlock the global memory block
					GlobalUnlock(hClipboardData);

					// Set the clipboard data to the CF_TEXT format
					EmptyClipboard();
					SetClipboardData(CF_TEXT, hClipboardData);
				}
				else {
					// Failed to lock global memory block
					GlobalFree(hClipboardData);
				}
			}

			// Close the clipboard
			CloseClipboard();
		}
	}

	edF32MATRIX4 operator*(const edF32MATRIX4& m1, const edF32MATRIX4& m2) {
		edF32MATRIX4 result;

		for (int i = 0; i < 4; ++i) {
			for (int j = 0; j < 4; ++j) {
				result.raw[i * 4 + j] = m1.raw[i * 4 + 0] * m2.raw[0 * 4 + j] +
					m1.raw[i * 4 + 1] * m2.raw[1 * 4 + j] +
					m1.raw[i * 4 + 2] * m2.raw[2 * 4 + j] +
					m1.raw[i * 4 + 3] * m2.raw[3 * 4 + j];
			}
		}

		return result;
	}

	void MouseCallback(GLFWwindow* window, double xpos, double ypos) {
		auto pCameraManager = CCameraManager::_gThis->pActiveCamera;
		edF32VECTOR4& cameraLookAt = pCameraManager->lookAt;
		edF32VECTOR4& cameraPosition = pCameraManager->transformationMatrix.rowT;
		edF32MATRIX4& cameraDirection = pCameraManager->transformationMatrix;

		static bool firstMouse = true;
		static double lastX = xpos;
		static double lastY = ypos;

		if (firstMouse) {
			lastX = xpos;
			lastY = ypos;
			firstMouse = false;
		}

		float xOffset = static_cast<float>(lastX - xpos);
		float yOffset = static_cast<float>(lastY - ypos);

		lastX = xpos;
		lastY = ypos;

		xOffset *= cameraSensitivity;
		yOffset *= cameraSensitivity * -1.0f;


		// Update camera direction matrix based on mouse movement (yaw and pitch only)
		float yaw = xOffset;
		float pitch = yOffset;

		// Limit the pitch to avoid camera flipping
		pitch = std::max(-1.57f, std::min(1.57f, pitch));

		edF32MATRIX4 rotationX = {
			1.0f,          0.0f,            0.0f, 0.0f,
			0.0f,  cos(pitch),  sin(pitch), 0.0f,
			0.0f, -sin(pitch),  cos(pitch), 0.0f,
			0.0f,          0.0f,            0.0f, 1.0f
		};

		edF32MATRIX4 rotationY = {
			cos(yaw), 0.0f, -sin(yaw), 0.0f,
				0.0f, 1.0f,        0.0f, 0.0f,
			sin(yaw), 0.0f,  cos(yaw), 0.0f,
				0.0f, 0.0f,        0.0f, 1.0f
		};

		cameraDirection = rotationX * rotationY * cameraDirection;
	}

	void SetActive(const bool bNewActive)
	{
		bActive = bNewActive;
		GLFWwindow* window = GetGLFWWindow();
		glfwSetInputMode(window, GLFW_CURSOR, bNewActive ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);

		static GLFWcursorposfun og = NULL;
		og = glfwSetCursorPosCallback(window, bNewActive ? MouseCallback : og);
	}

	void UpdateCameraPosition(float deltaTime) {
		auto pCameraManager = CCameraManager::_gThis->pActiveCamera;
		edF32VECTOR4& cameraLookAt = pCameraManager->lookAt;
		edF32VECTOR4& cameraPosition = pCameraManager->transformationMatrix.rowT;
		edF32MATRIX4& cameraDirection = pCameraManager->transformationMatrix;

		GLFWwindow* window = GetGLFWWindow();

		edF32VECTOR4 right = { cameraDirection.rowX.x, cameraDirection.rowX.y, cameraDirection.rowX.z, 1.0f };
		edF32VECTOR4 up = { cameraDirection.rowY.x, cameraDirection.rowY.y, cameraDirection.rowY.z, 1.0f };
		edF32VECTOR4 forward = { cameraDirection.rowZ.x, cameraDirection.rowZ.y, cameraDirection.rowZ.z, 1.0f };

		float cameraSpeedDelta = cameraSpeed * deltaTime;

		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			SetActive(false);

		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			cameraPosition = { cameraPosition.x + forward.x * cameraSpeedDelta,
							   cameraPosition.y + forward.y * cameraSpeedDelta,
							   cameraPosition.z + forward.z * cameraSpeedDelta,
							   1.0f };

		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			cameraPosition = { cameraPosition.x - forward.x * cameraSpeedDelta,
							   cameraPosition.y - forward.y * cameraSpeedDelta,
							   cameraPosition.z - forward.z * cameraSpeedDelta,
							   1.0f };

		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			cameraPosition = { cameraPosition.x - right.x * cameraSpeedDelta,
							   cameraPosition.y - right.y * cameraSpeedDelta,
							   cameraPosition.z - right.z * cameraSpeedDelta,
							   1.0f };

		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			cameraPosition = { cameraPosition.x + right.x * cameraSpeedDelta,
							   cameraPosition.y + right.y * cameraSpeedDelta,
							   cameraPosition.z + right.z * cameraSpeedDelta,
							   1.0f };
	}

	void Update()
	{
		UpdateCameraPosition(DebugMenu::GetDeltaTime());
	}

	// Function to write the matrix to a binary file
	void SaveMatrixToFile(const edF32MATRIX4& matrix) {
		std::ofstream file("matrix_data.bin", std::ios::binary);
		if (file) {
			file.write(reinterpret_cast<const char*>(&matrix), sizeof(edF32MATRIX4));
			file.close();
		}
	}

	// Function to read the matrix from a binary file
	void LoadMatrixFromFile(edF32MATRIX4& matrix) {
		std::ifstream file("matrix_data.bin", std::ios::binary);
		if (file) {
			file.read(reinterpret_cast<char*>(&matrix), sizeof(edF32MATRIX4));
			file.close();
		}
	}

}

// ImGui widget to edit edF32VECTOR4
void EditEdF32Vector4(edF32VECTOR4& vector) {
	ImGui::InputFloat("X", &vector.x);
	ImGui::InputFloat("Y", &vector.y);
	ImGui::InputFloat("Z", &vector.z);
	ImGui::InputFloat("W", &vector.w);
}

void DebugCamera::ShowCamera()
{
	// Get the display size
	ImGui::Begin("Camera", nullptr);

	auto pCameraManager = CCameraManager::_gThis->pActiveCamera;
	ImGui::Text("Position");
	EditEdF32Vector4(pCameraManager->transformationMatrix.rowT);
	ImGui::Text("LookAt");
	//EditEdF32Vector4(pCameraManager->lookAt);

	if (ImGui::Button("Save Matrix")) {
		// Assuming you have access to the transformation matrix (e.g., camera.GetViewMatrix())
		// Replace 'viewMatrix' below with your actual matrix variable.
		SaveMatrixToFile(CCameraManager::_gThis->pActiveCamera->transformationMatrix);
	}

	if (ImGui::Button("Load Matrix")) {
		// Assuming you have access to the transformation matrix (e.g., camera.GetViewMatrix())
		// Replace 'viewMatrix' below with your actual matrix variable.
		LoadMatrixFromFile(CCameraManager::_gThis->pActiveCamera->transformationMatrix);
	}

	if (ImGui::Button("Copy Matrix")) {
		// Assuming you have access to the transformation matrix (e.g., camera.GetViewMatrix())
		// Replace 'viewMatrix' below with your actual matrix variable.
		edF32MATRIX4 matrix;
		LoadMatrixFromFile(matrix);
		CopyMatrixCodeToClipboard(matrix);
	}

	if (ImGui::Checkbox("Debug Camera", &bActive))
	{
		SetActive(bActive);
	}

	if (bActive) {
		Update();
	}

	ImGui::End();
}