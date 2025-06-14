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
#include "DebugHelpers.h"
#include "Debug.h"
#include "TimeController.h"
#include "CameraGame.h"
#include "DebugSetting.h"
#include "CameraFixePerso.h"
#include "ActorHero.h"
#include "Actor/DebugActor.h"

GLFWwindow* GetGLFWWindow();

namespace Debug::Camera {
	static float cameraSpeed = 10.f;
	static float cameraSensitivity = 0.001f;
	static float horizontalAngle = 3.14f;
	static float verticalAngle = 0.0f;
	static bool bActive = false;

	static std::unordered_map<ECameraType, std::string> gCameraTypeNames = {
		{ CT_AroundSpecial, "Around Special" },
		{ CT_Cinematic, "Cinematic" },
		{ CT_Death, "Death" },
		{ CT_Frontend, "Frontend" },
		{ CT_IntView, "Int View" },
		{ CT_Main, "Main" },
		{ CT_Fight, "Fight" },
		{ CT_KyaJamgut, "Kya Jamgut" },
		{ CT_KyaWindWall, "Kya Wind Wall" },
		{ CT_MainCamera, "Main Camera" },
		{ CT_MouseAround, "Mouse Around" },
		{ CT_MouseQuake, "Mouse Quake" },
		{ CT_ShadowSun, "Shadow Sun" },
		{ CT_SilverBoomy, "Silver Boomy" },
		{ CT_Camera_6, "Camera 6" },
		{ CT_RailSimple, "Rail Simple" },
		{ CT_Rail, "Rail" },
		{ CT_DCA, "DCA" }
	};

	Debug::Setting<bool> gShowStackWindow = { "Show Camera Stack", false };

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

	float gMouseDeltaX = 0.0f;
	float gMouseDeltaY = 0.0f;

	constexpr float gDeadzone = 5.f;

	void CameraHackCallback(GLFWwindow* window, double xpos, double ypos) {
		if (bActive) {
			{
				static double lastX = xpos;
				const float delta = static_cast<float>(xpos - lastX);
				gMouseDeltaX = fabs(delta) > gDeadzone ? delta : 0.0f;
				lastX = xpos;
			}

			{
				static double lastY = ypos;
				const float delta = static_cast<float>(lastY - ypos);
				gMouseDeltaY = fabs(delta) > gDeadzone ? delta : 0.0f;
				lastY = ypos;
			}
		}
	}

	void MouseCallback(GLFWwindow* window, double xpos, double ypos) {
		auto* pCameraManager = CCameraManager::_gThis->pActiveCamera;
		edF32VECTOR4& cameraLookAt = pCameraManager->lookAt;
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

		edF32MATRIX4 cameraDirection2 = pCameraManager->transformationMatrix;
		cameraDirection = rotationX * rotationY * cameraDirection;
		cameraDirection.rowT = cameraDirection2.rowT;
	}

	void SetActive(const bool bNewActive, GLFWcursorposfun callback)
	{
		bActive = bNewActive;
		GLFWwindow* window = GetGLFWWindow();
		glfwSetInputMode(window, GLFW_CURSOR, bNewActive ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);

		static GLFWcursorposfun og = NULL;
		og = glfwSetCursorPosCallback(window, bNewActive ? callback : og);
	}

	void UpdateCameraPosition(float deltaTime) {
		auto* pActiveCamera = CCameraManager::_gThis->pActiveCamera;
		edF32VECTOR4& cameraLookAt = pActiveCamera->lookAt;
		edF32VECTOR4& cameraPosition = pActiveCamera->transformationMatrix.rowT;
		edF32MATRIX4& cameraDirection = pActiveCamera->transformationMatrix;

		GLFWwindow* window = GetGLFWWindow();

		edF32VECTOR4 right = { cameraDirection.rowX.x, cameraDirection.rowX.y, cameraDirection.rowX.z, 1.0f };
		edF32VECTOR4 up = { cameraDirection.rowY.x, cameraDirection.rowY.y, cameraDirection.rowY.z, 1.0f };
		edF32VECTOR4 forward = { cameraDirection.rowZ.x, cameraDirection.rowZ.y, cameraDirection.rowZ.z, 1.0f };

		float cameraSpeedDelta = cameraSpeed * deltaTime;

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

	static void ShowCameraDetails(CCamera* pCamera)
	{
		ImGui::Text("Camera Type: %s", gCameraTypeNames[pCamera->GetMode()].c_str());
		ImGui::Text("field_0x8: %d", pCamera->field_0x8);
		ImGui::Text("flags_0xc: 0x%x", pCamera->flags_0xc);

		ImGui::Text("transformationMatrix: %s", pCamera->transformationMatrix.ToString().c_str());

		ImGui::Text("field_0x50: %s", pCamera->field_0x50.ToString().c_str());
		ImGui::Text("lookAt: %s", pCamera->lookAt.ToString().c_str());

		if (pCamera->GetTarget()) {
			ImGui::TextColored(DebugHelpers::sValidColor, "Target: %s", pCamera->GetTarget()->name);
		}
		else {
			ImGui::TextColored(DebugHelpers::sInvalidColor, "Target: None");
		}

		if (pCamera->pOtherTarget) {
			ImGui::TextColored(DebugHelpers::sValidColor, "Other Target: %s", pCamera->pOtherTarget->name);
		}
		else {
			ImGui::TextColored(DebugHelpers::sInvalidColor, "Other Target: None");
		}

		ImGui::Text("fov: %f", pCamera->fov);
	}

	static CCameraFixePerso* pDebugCamera = nullptr;
	static CActor* pOverrideTarget = nullptr;

	static void ShowPushedCameraWindow()
	{
		bool bOpen = true;
		ImGui::Begin("Pushed Camera", &bOpen, ImGuiWindowFlags_AlwaysAutoResize);

		ShowCameraDetails(pDebugCamera);

		auto ShowFlagCheckbox = [](const char* name, uint flag) {
			bool bFlag = (pDebugCamera->flags_0xc & flag) != 0;
			if (ImGui::Checkbox(name, &bFlag)) {
				if (bFlag) {
					pDebugCamera->flags_0xc = pDebugCamera->flags_0xc | flag;
				}
				else {
					pDebugCamera->flags_0xc = pDebugCamera->flags_0xc & ~flag;
				}
			}
			};

		static Debug::Setting<bool> gFilterBySector("Filter by Sector", false);
		gFilterBySector.DrawImguiControl();

		// Use a list box
		const ImVec2 listboxSize = ImVec2(300, 300);
		if (ImGui::ListBoxHeader("##ActorList", listboxSize)) {
			auto pFunc = gFilterBySector ? Debug::Actor::ForEachSectorActor : Debug::Actor::ForEachActor;

			pFunc([](CActor* pActor)
				{
					std::string name = std::string(pActor->name) + " (" + Debug::Actor::GetActorTypeString(pActor->typeID) + ")";
					if (ImGui::Selectable(name.c_str())) {
						pOverrideTarget = pActor;
						pDebugCamera->SetTarget(pOverrideTarget);
						pDebugCamera->transformationMatrix.rowT = pActor->currentLocation;
						pDebugCamera->transformationMatrix.rowT.y += 10.f;
						pDebugCamera->flags_0xc = pDebugCamera->flags_0xc & ~0x800000;
					}
				});

			ImGui::EndListBox();
		}

		ShowFlagCheckbox("Flag A", 0x400);
		ShowFlagCheckbox("Flag B", 0x800);
		ShowFlagCheckbox("Flag C", 0x800000);

		ImGui::End();

		if (!bOpen) {
			CCameraManager::_gThis->PopCamera(pDebugCamera);
			delete pDebugCamera;
			pDebugCamera = nullptr;
		}
	}

	static void ShowStackWindow()
	{
		if (pDebugCamera) {
			ShowPushedCameraWindow();
		}

		bool bOpen = true;
		ImGui::Begin("Camera Stack", &bOpen, ImGuiWindowFlags_AlwaysAutoResize);

		auto& cameraStack = CCameraManager::_gThis->cameraStack;

		ImGui::Text("Camera Stack Size: %d", cameraStack.stackSize);

		if (ImGui::Button("Push Debug Camera")) {
			if (!pDebugCamera) {
				pDebugCamera = new CCameraFixePerso();
				pDebugCamera->SetTarget(CActorHero::_gThis);
			}

			CCameraManager::_gThis->PushCamera(pDebugCamera, 0);
		}

		for (int i = 0; i < cameraStack.stackSize; i++) {
			if (cameraStack.pActiveCamera == cameraStack.aCameras[i].pCamera) {
				ImGui::Text("Active Camera: %d", i);
			}
		}

		for (int i = 0; i < cameraStack.stackSize; i++) {
			std::string name = "Camera: " + std::to_string(i);

			if (ImGui::CollapsingHeader(name.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
				auto& entry = cameraStack.aCameras[i];

				ImGui::Text("field_0x0: %u", entry.field_0x0);

				ImGui::Separator();

				ShowCameraDetails(entry.pCamera);
			}
		}

		ImGui::End();

		if (!bOpen) {
			gShowStackWindow = false;
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

static bool bShouldEnableMouseLock = true;

void Debug::Camera::ShowCamera()
{
	// Get the display size
	ImGui::Begin("Camera", nullptr);

	gShowStackWindow.DrawImguiControl();

	if (ImGui::Button("Toggle Camera Hack")) {
		ToggleCameraHack();

		if (bShouldEnableMouseLock) {
			SetActive(true, CameraHackCallback);
			bShouldEnableMouseLock = false;
		}
		else {
			bShouldEnableMouseLock = true;
		}
	}

	if (glfwGetKey(GetGLFWWindow(), GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		gMouseDeltaX = 0.0f;
		gMouseDeltaY = 0.0f;
		SetActive(false, nullptr);
	}

	auto* pActiveCamera = CCameraManager::_gThis->pActiveCamera;

	if (ImGui::CollapsingHeader("Active Camera", ImGuiTreeNodeFlags_DefaultOpen)) {
		ShowCameraDetails(pActiveCamera);
	}

	if (pActiveCamera->GetMode() == CT_Main) {
		if (ImGui::Button("Disable Collision")) {
			CCameraGame* pGameCamera = static_cast<CCameraGame*>(pActiveCamera);

			if (pGameCamera) {
				pGameCamera->cameraRayFlags = 0;
			}
		}
	}

	{
		static bool bFixAtLocation = false;
		ImGui::Checkbox("Fix at location", &bFixAtLocation);

		if (bFixAtLocation && pActiveCamera) {
			GetTimer()->timeScale = 0.0f;
			//pActiveCamera->transformationMatrix = {
			//-0.0707696, 0, -0.997493, 0,
			//-0.907605, 0.414858, 0.0643923, 0,
			//0.413818, 0.909886, -0.0293593, 0,
			//214.523, 33.5794, -9.72168, 1,
			//};
		}
	}

	if (ImGui::Button("Save Matrix")) {
		// Assuming you have access to the transformation matrix (e.g., camera.GetViewMatrix())
		// Replace 'viewMatrix' below with your actual matrix variable.
		DebugHelpers::SaveTypeToFile("camera.bin", CCameraManager::_gThis->pActiveCamera->transformationMatrix);
	}

	if (ImGui::Button("Load Matrix")) {
		// Assuming you have access to the transformation matrix (e.g., camera.GetViewMatrix())
		// Replace 'viewMatrix' below with your actual matrix variable.
		DebugHelpers::LoadTypeFromFile("camera.bin", CCameraManager::_gThis->pActiveCamera->transformationMatrix);
	}

	if (ImGui::Button("Copy Matrix")) {
		// Assuming you have access to the transformation matrix (e.g., camera.GetViewMatrix())
		// Replace 'viewMatrix' below with your actual matrix variable.
		edF32MATRIX4 matrix;
		DebugHelpers::LoadTypeFromFile("camera.bin", (matrix));
		CopyMatrixCodeToClipboard(matrix);
	}

	if (ImGui::Checkbox("Debug Camera", &bActive))
	{
		SetActive(bActive, MouseCallback);
	}

	if (ImGui::CollapsingHeader("Camera List")) {
		// Show the total number of cameras
		ImGui::Text("Total Cameras: %d (Active mode: %d)", CCameraManager::_gThis->loadedCameraCount_0x9f0, CCameraManager::_gThis->pActiveCamera->GetMode());

		CCamera* pCamera = CCameraManager::_gThis->pInitialView_0x4b4;

		while (pCamera != nullptr) {
			ImGui::Text("Camera: %d", pCamera->GetMode());
			pCamera = pCamera->pNextCameraView_0xa4;
		}
	}

	if (bActive) {
		Update();
	}

	ImGui::End();

	if (gShowStackWindow) {
		ShowStackWindow();
	}
}