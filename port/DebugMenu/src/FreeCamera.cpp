#include "FreeCamera.h"

#include "CameraViewManager.h"
#include "GLFW/glfw3.h"

#include <algorithm>
#include <chrono>
#include <cmath>

GLFWwindow* GetGLFWWindow();

// ----------------------------------------------------------------
// Static state
// ----------------------------------------------------------------

CFreeCamera* CFreeCamera::sActive = nullptr;

// ----------------------------------------------------------------
// Constructor / destructor
// ----------------------------------------------------------------

CFreeCamera::CFreeCamera()
{
	sActive = this;

	// Must be SWITCH_MODE_B so Level_Manage reads orientation from GetAngleAlpha/Beta
	// on this camera rather than routing through the activeCamManager blend path (which
	// would ignore our Manage() output entirely).
	switchMode = SWITCH_MODE_B;

	// field_0x94 is the switch mode the camera stack reads when THIS camera is the
	// one being popped (i.e. what mode to use when restoring the game camera below us).
	// If left uninitialised (garbage), the stack may select a blended restore mode,
	// which leaves pActiveCamera pointing at the (about-to-be-deleted) free camera
	// for several frames, causing a use-after-free. Force immediate (B) restoration.
	field_0x94 = SWITCH_MODE_B;
	field_0x9c = 0.f;  // blend duration for the restore — 0 = instant

	// 0x40000 = Manage() called even when game time is frozen (timeScale == 0).
	// 0x0b7c  = activatable-in-state mask (same as CCameraFixePerso). FindActivableCameraInStack
	//           checks (GetCurHeroState() & flags_0xc) != 0 to decide if the camera can
	//           become active. GetCurHeroState() returns 4 (normal), and 4 & 0xb7c = 4 != 0.
	//           Without this, our camera is never selected and Manage() is never called.
	flags_0xc |= 0x40000 | 0xb7c;

	// CCamera::CCamera() does not initialise fov; set a safe default matching CCameraMouseQuake.
	// SetPositionFromCamera will overwrite this with the source camera's fov.
	fov = 0.6f;

	lastFrameNs = std::chrono::duration_cast<std::chrono::nanoseconds>(
		std::chrono::steady_clock::now().time_since_epoch()).count();
}

CFreeCamera::~CFreeCamera()
{
	if (sActive == this) {
		sActive = nullptr;
	}
}

// ----------------------------------------------------------------
// CCamera overrides
// ----------------------------------------------------------------

ECameraType CFreeCamera::GetMode()
{
	return CT_FreeCamera;
}

void CFreeCamera::Init()
{
	CCameraExt::Init();

	// CCameraExt::Init() calls edF32Matrix4ToEulerSoft on transformationMatrix
	// (whose rotation rows are identity at this point) and stores (0,0,0) in
	// CCameraExt::angles — overwriting any angles we may have set. Restore them
	// from our yaw/pitch so the first rendered frame is correct.
	SetAngleAlpha(pitch);
	SetAngleBeta(yaw);
	SetAngleGamma(0.f);

	// Defensive: re-apply flags that CCameraExt::Init may not preserve.
	switchMode    = SWITCH_MODE_B;
	flags_0xc    |= 0x40000 | 0xb7c;

	mouseDeltaX = 0.f;
	mouseDeltaY = 0.f;

	lastFrameNs = std::chrono::duration_cast<std::chrono::nanoseconds>(
		std::chrono::steady_clock::now().time_since_epoch()).count();
}

bool CFreeCamera::Manage()
{
	// --- Wall-clock delta time (works regardless of game timeScale) ---
	const long long nowNs = std::chrono::duration_cast<std::chrono::nanoseconds>(
		std::chrono::steady_clock::now().time_since_epoch()).count();
	const float dt = std::min(static_cast<float>(nowNs - lastFrameNs) * 1e-9f, 0.1f);
	lastFrameNs = nowNs;

	// --- Mouse look ---
	yaw   -= mouseDeltaX * sensitivity;
	pitch += mouseDeltaY * sensitivity;
	pitch = std::max(-1.5f, std::min(1.5f, pitch));
	mouseDeltaX = 0.f;
	mouseDeltaY = 0.f;

	// --- Movement vectors (Y-up world, +Z forward at yaw=0) ---
	const float cy = cosf(yaw),  sy = sinf(yaw);
	const float cp = cosf(pitch), sp = sinf(pitch);

	// Camera's local axes derived from yaw/pitch
	const float fwdX = sy * cp,  fwdY = -sp, fwdZ = cy * cp;
	const float rgtX = cy,       rgtY = 0.f, rgtZ = -sy;

	// --- Speed (Shift for 5× boost) ---
	GLFWwindow* window = GetGLFWWindow();
	float currentSpeed = speed;
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT)  == GLFW_PRESS ||
		glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS) {
		currentSpeed *= 5.f;
	}

	// --- Apply movement to position ---
	edF32VECTOR4& pos = transformationMatrix.rowT;

	auto applyMove = [&](float dx, float dy, float dz, float scale) {
		pos.x += dx * scale;
		pos.y += dy * scale;
		pos.z += dz * scale;
	};

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		applyMove(fwdX, fwdY, fwdZ,  currentSpeed * dt);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		applyMove(fwdX, fwdY, fwdZ, -currentSpeed * dt);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		applyMove(rgtX, rgtY, rgtZ, -currentSpeed * dt);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		applyMove(rgtX, rgtY, rgtZ,  currentSpeed * dt);
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		pos.y += currentSpeed * dt;
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		pos.y -= currentSpeed * dt;

	// --- Feed orientation into the angle state that BuildDisplayMatrix reads.
	// "ZXY" Euler order: alpha (x) = pitch, beta (y) = yaw, gamma (z) = 0.
	SetAngleAlpha(pitch);
	SetAngleBeta(yaw);
	SetAngleGamma(0.f);

	return true;
}

// ----------------------------------------------------------------
// Helpers
// ----------------------------------------------------------------

void CFreeCamera::SetPositionFromCamera(CCamera* pFrom)
{
	transformationMatrix.rowT = pFrom->transformationMatrix.rowT;
	fov   = pFrom->fov;
	pitch = pFrom->GetAngleAlpha();
	yaw   = pFrom->GetAngleBeta();
	// Apply immediately so CCameraExt::angles is valid even before the first Manage() tick.
	SetAngleAlpha(pitch);
	SetAngleBeta(yaw);
	SetAngleGamma(0.f);
}

// ----------------------------------------------------------------
// Static callbacks / direct input feeds
// ----------------------------------------------------------------

void CFreeCamera::AccumulateRawDelta(int dx, int dy)
{
	if (!sActive)
		return;
	// Called directly from KyaWndProc for each WM_INPUT packet (can be multiple
	// per frame on high-polling-rate mice). Manage() drains these each tick.
	sActive->mouseDeltaX += static_cast<float>(dx);
	sActive->mouseDeltaY += static_cast<float>(dy);
}

void CFreeCamera::ScrollCallback(GLFWwindow* /*window*/, double /*xoffset*/, double yoffset)
{
	if (!sActive) {
		return;
	}

	// Scroll up = faster, scroll down = slower. Multiplicative steps feel natural.
	const float factor = (yoffset > 0) ? 1.25f : (1.f / 1.25f);
	sActive->speed = std::max(0.5f, std::min(sActive->speed * factor, 5000.f));
}
