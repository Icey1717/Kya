#include "gizmo.h"
#include "imGuIZMOquat.h"

Gizmo::Widget::Widget(const char* inLabel)
	: label(inLabel)
{
	state = new quat(1.f, 0.f, 0.f, 0.f);
}

glm::mat4 Gizmo::Widget::Show()
{
	glm::mat4 outMatrix;
	quat* qRot = static_cast<quat*>(state);
	ImGui::gizmo3D(label.c_str(), *qRot /*, size,  mode */);
	mat4 modelMatrix = mat4_cast(*qRot);

	memcpy(&outMatrix, &modelMatrix, sizeof(modelMatrix));

	return outMatrix;
}
