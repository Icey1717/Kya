#pragma once

class CActor;

namespace Debug {
	CActor* GetInspectedActor();
	bool GetShowWorldPanel();
	void SetShowWorldPanel(bool bShow);

	bool GetShowInspectorPanel();
	void SetShowInspectorPanel(bool bShow);

	void DrawWorldPanel();
	void DrawInspectorPanel();
	void DrawSelectedActorMarker();
}
