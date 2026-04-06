#pragma once

#include "camera.h"

struct GLFWwindow;

#define CT_FreeCamera (ECameraType)0x20

// A fully free-fly debug camera with Unreal/Maya-style controls.
// Inherits from CCameraExt so it integrates with the camera stack
// (PushCamera/PopCamera) and feeds orientation through the standard
// GetAngleAlpha/Beta/Gamma interface that BuildDisplayMatrix expects.
class CFreeCamera : public CCameraExt {
public:
	CFreeCamera();
	~CFreeCamera();

	// CCamera
	virtual ECameraType GetMode() override;
	virtual void Init() override;
	virtual bool Manage() override;

	// Snap position and yaw/pitch from another camera (call before pushing to stack).
	void SetPositionFromCamera(CCamera* pFrom);

	// GLFW scroll callback (speed control). Mouse delta is fed via AccumulateRawDelta
	// from KyaWndProc, bypassing the GLFW/ImGui dispatch chain entirely.
	static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

	// Called directly from KyaWndProc when a WM_INPUT (raw mouse) message arrives.
	// Accumulates delta across multiple raw-input packets per frame; Manage() drains it.
	static void AccumulateRawDelta(int dx, int dy);

	float speed = 20.f;
	float sensitivity = 0.002f;

	static bool IsActive() { return sActive != nullptr; }

private:
	float yaw   = 0.f;
	float pitch = 0.f;

	float mouseDeltaX = 0.f;
	float mouseDeltaY = 0.f;

	// Wall-clock time for delta, independent of game timeScale.
	long long lastFrameNs = 0;

	static CFreeCamera* sActive;
};
