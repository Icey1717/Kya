#pragma once

namespace Debug {
	bool GetShowWorldPanel();
	void SetShowWorldPanel(bool bShow);

	bool GetShowInspectorPanel();
	void SetShowInspectorPanel(bool bShow);

	void DrawWorldPanel();
	void DrawInspectorPanel();
}
