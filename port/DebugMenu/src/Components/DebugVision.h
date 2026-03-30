#pragma once

class CVision;

namespace Debug::Components::Vision
{
	void ShowVisionDetails(CVision* pVision);
	void DrawVisionShapesFilled(CVision* pVision);
	void DrawVisionShapesWireframe(CVision* pVision);
	void DrawVisionShapes(CVision* pVision); // filled + wireframe
}
