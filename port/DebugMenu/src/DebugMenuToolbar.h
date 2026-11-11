#pragma once

namespace Debug {
	bool GetShowCameraWindow();
	void SetShowCameraWindow(bool show);

	void UpdateSingleStepState();
	bool IsSimulationPaused();
	bool SetSimulationPaused(bool paused);
	bool StepSimulationFrame();
	void DrawToolbar();
	void DrawCameraWindow();
}
