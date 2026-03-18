#include "DebugMenu.h"
#include "DebugEvent.h"
#include <imgui.h>
#include "Types.h"
#include "EventManager.h"
#include "LargeObject.h"
#include "DebugSetting.h"
#include "DebugProjection.h"
#include "DebugFrameBuffer.h"
#include "DebugHelpers.h"
#include "Actor.h"
#include "ActorManager.h"
#include "Native/NativeDebugShapes.h"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

extern ed_event_chunk** pedEventChunks;
extern uint edEventNbChunks;

void EventCallbackSendMessage(edCEventMessage* pEventMessage);

// Returns a human-readable name for one of the 4 per-collider send-info slots.
// Slot index directly corresponds to the collider flag bit that enables it:
//   slot 0 (ED_COLLIDER_FLAG_INSIDE)       – fires every frame while inside  (sustained)
//   slot 1 (ED_COLLIDER_FLAG_OUTSIDE)      – fires every frame while outside (sustained)
//   slot 2 (ED_COLLIDER_FLAG_JUST_ENTERED) – fires once on the frame of entry
//   slot 3 (ED_COLLIDER_FLAG_JUST_EXITED)  – fires once on the frame of exit
// ED_MSG_FLAG_ONE_SHOT additionally disables the slot after its first fire.
static std::string GetColliderSlotLabel(const _ed_event_collider_test* pCollider, int slotId)
{
	static const char* const baseNames[] = { "While Inside", "While Outside", "On Enter", "On Exit" };
	bool isEnabled = (pCollider->messageFlags[slotId] & ED_MSG_FLAG_ENABLED) != 0;
	bool isOneShot = (pCollider->messageFlags[slotId] & ED_MSG_FLAG_ONE_SHOT) != 0;

	std::string label = baseNames[slotId];
	if (isOneShot)  label += " (one-shot)";
	if (!isEnabled) label += " (disabled)";
	label += " [" + std::to_string(slotId) + "]";
	return label;
}

// Returns the ACTOR_MESSAGE name for a known message ID, or nullptr if unknown.
static const char* GetActorMessageName(int msgId)
{
	switch (msgId) {
	case 0x02: return "KICKED";
	case 0x07: return "GET_VISUAL_DETECTION_POINT";
	case 0x0c: return "GET_RUN_SPEED";
	case 0x0d: return "TIED";
	case 0x0e: return "TOGGLE";
	case 0x0f: return "ACTIVATE";
	case 0x10: return "DEACTIVATE";
	case 0x12: return "GET_ACTION";
	case 0x14: return "TRAP_STRUGGLE";
	case 0x16: return "IN_WIND_AREA";
	case 0x17: return "ENTER_WIND";
	case 0x18: return "LEAVE_WIND";
	case 0x1b: return "FIGHT_ACTION_SUCCESS";
	case 0x1d: return "ENTER_TRAMPO";
	case 0x1e: return "IMPULSE";
	case 0x23: return "ENTER_SHOP";
	case 0x24: return "LEAVE_SHOP";
	case 0x25: return "DISABLE_INPUT";
	case 0x26: return "ENABLE_INPUT";
	case 0x2c: return "SPAWN";
	case 0x2f: return "MAGIC_DEACTIVATE";
	case 0x30: return "MAGIC_ACTIVATE";
	case 0x31: return "TRAP_CAUGHT";
	case 0x35: return "SOCCER_START";
	case 0x49: return "REQUEST_CAMERA_TARGET";
	case 0x4d: return "GET_BONE_ID";
	case 0x4e: return "NATIV_CMD";
	case 0x62: return "BOOMY_CHANGED";
	case 0x6b: return "FIGHT_RING_CHANGED";
	case 0x79: return "RECEPTACLE_CHANGED";
	case 0x7c: return "CINEMATIC_INSTALL";
	case 0x7d: return "NEW_MAP_GAINED";
	case 0x85: return "NEW_LIFE_GAUGE";
	default:   return nullptr;
	}
}

// Returns the EVENT_PRIM_* name for a known event primitive type, or nullptr if unknown.
static const char* GetEventPrimTypeName(uint eventType)
{
	switch (eventType) {
	case EVENT_PRIM_KILL:               return "KILL";
	case EVENT_PRIM_SECTOR_SWITCH:      return "SECTOR_SWITCH";
	case EVENT_PRIM_SECTOR_SWITCH_ANIM: return "SECTOR_SWITCH_ANIM";
	case EVENT_PRIM_TELEPORT:           return "TELEPORT";
	case EVENT_PRIM_ZONE_ACTIVATE:      return "ZONE_ACTIVATE";
	case EVENT_PRIM_ZONE_DEACTIVATE:    return "ZONE_DEACTIVATE";
	case EVENT_PRIM_ZONE_TOGGLE:        return "ZONE_TOGGLE";
	case EVENT_PRIM_ZONE_MESSAGE:       return "ZONE_MESSAGE";
	case EVENT_PRIM_ZONE_PROJECTILE:    return "ZONE_PROJECTILE";
	default:
		if (eventType >= EVENT_PRIM_AUDIO_FIRST && eventType <= EVENT_PRIM_AUDIO_LAST)
			return "AUDIO";
		return nullptr;
	}
}

// Returns the kill sub-type name for pEventData[1] when eventType == EVENT_PRIM_KILL.
static const char* GetKillSubtypeName(uint subtype)
{
	switch (subtype) {
	case EVENT_PRIM_KILL_FALL:  return "FALL";
	case EVENT_PRIM_KILL_DROWN: return "DROWN";
	case 2:                     return "DEATH_TYPE_2";
	case 3:                     return "DEATH_TYPE_3";
	default:                    return "COL_WALL";
	}
}

// Returns the projectile sub-type name for pEventData[1] when eventType == EVENT_PRIM_ZONE_PROJECTILE.
static const char* GetProjectileSubtypeName(uint subtype)
{
	switch (subtype) {
	case 0: return "NONE";
	case 1: return "ARROW";
	case 2: return "MAGIC_SMALL";
	case 3: return "MAGIC_LARGE";
	case 4: return "SHOCKWAVE";
	case 5: return "MAGIC_WIDE";
	default: return nullptr;
	}
}

// Displays structured, human-readable event data for the payload passed to ReceiveEvent.
// pData[0] = e_ed_event_prim3d_type; subsequent elements are type-specific parameters.
static const char* GetZonePrimShapeName(uint shapeType)
{
	switch (shapeType) {
	case 0: return "half-space";
	case 1: return "open-top box";
	case 2: return "box";
	case 3: return "cylinder";
	case 4: return "cone";
	case 5: return "sphere";
	default: return nullptr;
	}
}

static void ShowEventData(const int* pData, int count)
{
	if (count <= 0) return;

	uint eventType = static_cast<uint>(pData[0]);
	const char* typeName = GetEventPrimTypeName(eventType);

	if (typeName)
		ImGui::Text("Event Type: %s (0x%x)", typeName, eventType);
	else
		ImGui::Text("Event Type: 0x%x (unknown)", eventType);

	if (count < 2) return;
	uint param1 = static_cast<uint>(pData[1]);

	switch (eventType) {
	case EVENT_PRIM_KILL:
		ImGui::Text("  Kill sub-type: %s (%u)", GetKillSubtypeName(param1), param1);
		break;
	case EVENT_PRIM_SECTOR_SWITCH:
	case EVENT_PRIM_SECTOR_SWITCH_ANIM:
		ImGui::Text("  Sector ID: %u", param1);
		break;
	case EVENT_PRIM_TELEPORT:
		ImGui::Text("  Level ID: %u", param1);
		if (count >= 3) ImGui::Text("  Param2: 0x%x", static_cast<uint>(pData[2]));
		if (count >= 4) ImGui::Text("  Param3: 0x%x", static_cast<uint>(pData[3]));
		break;
	case EVENT_PRIM_ZONE_ACTIVATE:
	case EVENT_PRIM_ZONE_DEACTIVATE:
	case EVENT_PRIM_ZONE_TOGGLE:
		ImGui::Text("  Param: 0x%x", param1);
		break;
	case EVENT_PRIM_ZONE_MESSAGE: {
		const char* msgName = GetActorMessageName(static_cast<int>(param1));
		if (msgName)
			ImGui::Text("  Message: %s (0x%x)", msgName, param1);
		else
			ImGui::Text("  Message: 0x%x (unknown)", param1);
		if (count >= 3)
			ImGui::Text("  Msg Param: 0x%x", static_cast<uint>(pData[2]));
		break;
	}
	case EVENT_PRIM_ZONE_PROJECTILE: {
		const char* projName = GetProjectileSubtypeName(param1);
		if (projName)
			ImGui::Text("  Projectile sub-type: %s (%u)", projName, param1);
		else
			ImGui::Text("  Projectile sub-type: %u (unknown)", param1);
		if (count >= 3) ImGui::Text("  Damage: 0x%x", static_cast<uint>(pData[2]));
		if (count >= 4) ImGui::Text("  Waypoint: 0x%x", static_cast<uint>(pData[3]));
		if (count >= 5) ImGui::Text("  Force: 0x%x", static_cast<uint>(pData[4]));
		break;
	}
	default:
		if (eventType >= EVENT_PRIM_AUDIO_FIRST && eventType <= EVENT_PRIM_AUDIO_LAST) {
			ImGui::Text("  Audio Index: %u (event %u)", eventType - EVENT_PRIM_AUDIO_FIRST, param1);
		} else {
			for (int m = 1; m < count; m++)
				ImGui::Text("  param[%d]: 0x%x", m, static_cast<uint>(pData[m]));
		}
		break;
	}
}

// Builds a short display label for an event slot in the "Select Event" combo.
// Format: "[idx] ActorName | N coll | ACTIVE"
static std::string BuildEventLabel(ed_event_chunk* pChunk, int eventIndex)
{
	auto* pEvent = LOAD_POINTER_CAST(ed_event*, pChunk->aEvents[eventIndex]);

	const char* actorName = "?";
	bool bActive = false;

	if (pEvent->nbColliders > 0) {
		auto* pCollider = reinterpret_cast<_ed_event_collider_test*>(pEvent + 1);

		auto* pActorRef = LOAD_POINTER_CAST(ed_event_actor_ref*, pCollider->pActorRef);
		if (pActorRef) {
			auto* pActor = LOAD_POINTER_CAST(CActor*, pActorRef->pActor);
			if (pActor) actorName = pActor->name;
		}

		auto* pColliderScan = pCollider;
		for (uint j = 0; j < pEvent->nbColliders; j++) {
			if (pColliderScan->flags & ED_COLLIDER_FLAG_INSIDE) { // actor is currently inside zone
				bActive = true;
				break;
			}
			pColliderScan++;
		}
	}

	std::stringstream ss;
	ss << "[0x" << std::hex << eventIndex << "] " << actorName
	   << " | " << pEvent->nbColliders << " coll";
	if (bActive) ss << " | ACTIVE";
	return ss.str();
}

namespace Debug {
	namespace Event {
		static int gSelectedChunk = 0;

		static Setting<bool> gShowEventsOverlay("Show Events Overlay", true);
		static Setting<bool> gShowAllZones("Show All Zones", false);
		static Setting<bool> gShowSelectedEvent("Show Selected Event", true);
		static Setting<bool> gShowActiveEvents("Show Active Events", true);

		static Setting<bool> gShowEventShapes3D   ("Show 3D Event Shapes",          false);
		static Setting<bool> gShowZoneSpheres      ("3D: Zone Bounding Spheres",     true);
		static Setting<bool> gShowZonePrimitives   ("3D: Zone Primitives",           true);
		static Setting<bool> gShowAllZonePrimitives("3D: All Zone Primitives",       false);
		static Setting<bool> gShowZonePrimitivesShaded("3D: Zone Primitives Shaded", false);
		static Setting<bool> gShowColliderPositions("3D: Collider Positions",        true);
		static Setting<bool> gShowOnlyKillPrimitives("3D: Only Kill Primitives",      false);

		static Setting<int> gSelectedEvent("Selected Event", 0);

		static bool bTrackingEvents = false;

		struct EventKey
		{
			uint colliderId;
			int eventIndex;
			int colliderIndex;

			bool operator==(const EventKey& other) const
			{
				return colliderId == other.colliderId && eventIndex == other.eventIndex && colliderIndex == other.colliderIndex;
			}

			// Hash
			struct Hasher
			{
				std::size_t operator()(const EventKey& key) const
				{
					return std::hash<uint>()(key.colliderId) ^ std::hash<int>()(key.eventIndex) ^ std::hash<int>()(key.colliderIndex);
				}
			};
		};

		struct TriggerRecord
		{
			std::vector<CActor*> receivers;
		};

		struct EventRecord
		{
			std::vector<TriggerRecord> triggers;
		};

		static std::unordered_map<EventKey, EventRecord, EventKey::Hasher> gEventRecords;

		static void EventCallbackSendMessageIntercept(edCEventMessage* pEventMessage)
		{
			bool bVar1;
			EventSendInfo* pSendInfo;
			int iVar3;
			CActor* pActor;
			int* pCurrentActorIndex;
			int iVar6;
			int nbActorIndexes;

			EventKey eventKey;
			eventKey.colliderId = pEventMessage->colliderId;

			for (int i = 0; i < pEventMessage->pEventChunk->nbEvents; i++) {
				auto* pEvent = LOAD_POINTER_CAST(ed_event*, pEventMessage->pEventChunk->aEvents[i]);

				auto* pCollider = reinterpret_cast<_ed_event_collider_test*>(pEvent + 1);

				for (int j = 0; j < pEvent->nbColliders; j++) {
					if (pCollider == pEventMessage->pEventCollider) {
						eventKey.eventIndex = i;
						eventKey.colliderIndex = j;
						break;
					}

					pCollider++;
				}
			}

			EventRecord& record = gEventRecords[eventKey];
			TriggerRecord newRecord = record.triggers.emplace_back();

			pSendInfo = LOAD_POINTER_CAST(EventSendInfo*, pEventMessage->pEventCollider->aSendInfo[pEventMessage->colliderId]);

			nbActorIndexes = pSendInfo->nbActorIndexes;
			pCurrentActorIndex = reinterpret_cast<int*>(pSendInfo + 1);
			iVar6 = (pSendInfo->field_0x0 + -1) - nbActorIndexes;

			int* pReceiveData = pCurrentActorIndex + nbActorIndexes;

			if (nbActorIndexes == 0) {
				ed_event_actor_ref* pRef = LOAD_POINTER_CAST(ed_event_actor_ref*, pEventMessage->pEventCollider->pActorRef);
				CActor* pActorRef = LOAD_POINTER_CAST(CActor*, pRef->pActor);
				newRecord.receivers.push_back(pActorRef);
			}
			else {
				while (bVar1 = nbActorIndexes != 0, nbActorIndexes = nbActorIndexes + -1, bVar1) {
					iVar3 = *pCurrentActorIndex;

					if (iVar3 == -1) {
						ed_event_actor_ref* pRef = LOAD_POINTER_CAST(ed_event_actor_ref*, pEventMessage->pEventCollider->pActorRef);
						CActor* pActorRef = LOAD_POINTER_CAST(CActor*, pRef->pActor);

						pActor = pActorRef;
					}
					else {
						if (iVar3 == -1) {
							pActor = (CActor*)0x0;
						}
						else {
							pActor = (CScene::ptable.g_ActorManager_004516a4)->aActors[iVar3];
						}
					}

					newRecord.receivers.push_back(pActor);
					pCurrentActorIndex = pCurrentActorIndex + 1;
				}
			}

			EventCallbackSendMessage(pEventMessage);

			return;
		}

		static void EventCallbackSendAllMessagesIntercept(edCEventMessage* pQueue, int count)
		{
			bool bProcessedAllEvents;
			int curIndex = count + -1;
			if (count != 0) {
				do {
					EventCallbackSendMessageIntercept(pQueue);
					pQueue = pQueue + 1;
					bProcessedAllEvents = curIndex != 0;
					curIndex = curIndex + -1;
				} while (bProcessedAllEvents);
			}

			return;
		}
	}
}

void Debug::Event::ShowMenu(bool* bOpen)
{
	ImGui::Begin("Event", bOpen, ImGuiWindowFlags_AlwaysAutoResize);

	auto* pEventManager = CScene::ptable.g_EventManager_006f5080;

	ImGui::Text("Event Chunks: %d", edEventNbChunks);

	ImGui::Separator();

	ImGui::Text("Active Chunk ID: 0x%x", pEventManager->activeChunkId);
	ImGui::Text("Event count: 0x%x", pEventManager->nbEvents);

	ImGui::Separator();

	if (ImGui::Button("Track Events")) {
		CEventManager::callbackFunctions.pSendAllMessages = EventCallbackSendAllMessagesIntercept;
		bTrackingEvents = true;
	}

	auto BuildChunkLabel = [](int i) -> std::string {
		auto* pC = pedEventChunks[i];
		if (!pC) return "Chunk " + std::to_string(i) + " (Invalid)";
		return "Chunk " + std::to_string(i) + " (" + std::to_string(pC->nbEvents) + " events)";
	};

	if (ImGui::BeginCombo("Select Chunk", BuildChunkLabel(gSelectedChunk).c_str())) {
		for (int i = 0; i < (int)edEventNbChunks; i++) {
			bool isSelected = gSelectedChunk == i;
			if (ImGui::Selectable(BuildChunkLabel(i).c_str(), isSelected)) {
				gSelectedChunk = i;
			}
			if (isSelected) ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	ImGui::Spacing();

	// show chunk details
	if (gSelectedChunk < edEventNbChunks) {
		auto* pChunk = pedEventChunks[gSelectedChunk];

		if (pChunk && ImGui::CollapsingHeader("Chunk Details")) {
			DebugHelpers::TextValidValue("pFileData %p", pChunk->pFileData);

			ImGui::Separator();

			ImGui::Text("nbMatrices: %d", pChunk->nbMatrices);
			DebugHelpers::TextValidValue("field_0x8 %p", pChunk->field_0x8);
			DebugHelpers::TextValidValue("aMatrices %p", pChunk->aMatrices);

			ImGui::Separator();

			ImGui::Text("field_0x10: %d", pChunk->field_0x10);
			DebugHelpers::TextValidValue("field_0x14 %p", pChunk->field_0x14);

			ImGui::Separator();

			ImGui::Text("count_0x18: %d", pChunk->nbSendInfo);
			DebugHelpers::TextValidValue("field_0x1c %p", pChunk->aSendInfo);

			ImGui::Separator();

			ImGui::Text("nbActorRefs: %d", pChunk->nbActorRefs);
			DebugHelpers::TextValidValue("aActorRefs %p", pChunk->aActorRefs);

			ImGui::Separator();

			ImGui::Text("nbZones: %d", pChunk->nbZones);
			DebugHelpers::TextValidValue("aZones %p", pChunk->aZones);

			ImGui::Separator();

			ImGui::Text("nbEvents: %d", pChunk->nbEvents);
			DebugHelpers::TextValidValue("aEvents %p", pChunk->aEvents);

			ImGui::Separator();

			ImGui::Text("count_0x38: %d", pChunk->count_0x38);
			DebugHelpers::TextValidValue("field_0x3c %p", pChunk->field_0x3c);

			ImGui::Spacing();

			static ImGuiTextFilter gEventFilter;
			gEventFilter.Draw("Filter Events", -1);

			std::string eventPreview = (gSelectedEvent < (int)pChunk->nbEvents)
				? BuildEventLabel(pChunk, gSelectedEvent)
				: "(none)";

			if (ImGui::BeginCombo("Select Event", eventPreview.c_str())) {
				for (int i = 0; i < (int)pChunk->nbEvents; i++) {
					std::string label = BuildEventLabel(pChunk, i);
					if (!gEventFilter.PassFilter(label.c_str())) continue;

					bool isSelected = gSelectedEvent == i;
					bool bActive = label.find("ACTIVE") != std::string::npos;

					if (bActive)
						ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 1.0f, 0.2f, 1.0f));

					if (ImGui::Selectable(label.c_str(), isSelected))
						gSelectedEvent = i;

					if (bActive)
						ImGui::PopStyleColor();

					if (isSelected) ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			if (gSelectedEvent < pChunk->nbEvents && ImGui::CollapsingHeader("Event Details")) {
				auto* pEvent = LOAD_POINTER_CAST(ed_event*, pChunk->aEvents[gSelectedEvent]);

				ImGui::Text("flags: 0x%x", pEvent->flags);

				DebugHelpers::TextValidValue("pZone %p", pEvent->pZone);

				auto* pZone = LOAD_POINTER_CAST(ed_zone_3d*, pEvent->pZone);

				if (pZone) {
					ImGui::Separator();
					ImGui::Text("Zone Sphere: %s", pZone->boundSphere.ToString().c_str());
					ImGui::Text("Zone Flags: 0x%x", pZone->flags);
					DebugHelpers::TextValidValue("Zone Matrix %p", pZone->pMatrix);
					ImGui::Separator();
				}

				ImGui::Text("field_0x8: 0x%x", pEvent->field_0x8);
				ImGui::Text("nbColliders: %d", pEvent->nbColliders);

				ImGui::Separator();

				auto* pCollider = reinterpret_cast<_ed_event_collider_test*>(pEvent + 1);

				for (int i = 0; i < (int)pEvent->nbColliders; i++) {
					bool bColliderActive = (pCollider->flags & ED_COLLIDER_FLAG_INSIDE) != 0;

					if (bColliderActive)
						ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 1.0f, 0.2f, 1.0f));
					ImGui::Text("Collider %d%s", i, bColliderActive ? " [ACTIVE]" : "");
					if (bColliderActive)
						ImGui::PopStyleColor();

					ImGui::Text("worldLocation: %s", pCollider->worldLocation.ToString().c_str());

					ImGui::Spacing();

					// messageFlags per slot: bit 7 (ED_MSG_FLAG_ENABLED) = active, bit 1 (ED_MSG_FLAG_ONE_SHOT) = fires once
					for (int s = 0; s < 4; s++) {
						byte mf = pCollider->messageFlags[s];
						ImGui::Text("  messageFlags[%d]: 0x%02x  (%s%s)", s, mf,
							(mf & ED_MSG_FLAG_ENABLED)  ? "enabled" : "disabled",
							(mf & ED_MSG_FLAG_ONE_SHOT) ? ", one-shot" : "");
					}

					ImGui::Spacing();

					ImGui::Text("flags: 0x%x", pCollider->flags);

					auto* pActorRef = LOAD_POINTER_CAST(ed_event_actor_ref*, pCollider->pActorRef);

					if (pActorRef) {
						auto* pActor = LOAD_POINTER_CAST(CActor*, pActorRef->pActor);
						ImGui::Text("pActor (%s)", pActor ? pActor->name : "null");

						auto* pLocation = LOAD_POINTER_CAST(edF32VECTOR4*, pActorRef->pLocation);

						ImGui::Text("pLocation %s", pLocation ? pLocation->ToString().c_str() : "null");
					}

					DebugHelpers::TextValidValue("field_0x1c %p", pCollider->field_0x1c);

					ImGui::Spacing();

					for (int j = 0; j < 4; j++) {
						auto* pSendInfo = LOAD_POINTER_CAST(EventSendInfo*, pCollider->aSendInfo[j]);

						// Unique ID required because CollapsingHeader uses the label as ID
						std::string slotLabel = GetColliderSlotLabel(pCollider, j)
							+ "##col" + std::to_string(i) + "_slot" + std::to_string(j);

						if (pSendInfo) {
							if (ImGui::CollapsingHeader(slotLabel.c_str())) {
								if (pSendInfo->nbActorIndexes == 0) {
									if (pActorRef) {
										auto* pActor = LOAD_POINTER_CAST(CActor*, pActorRef->pActor);
										ImGui::Text("Target Actor (self): %s", pActor->name);
									}
								}
								else {
									ImGui::Text("nbActorIndexes: %d", pSendInfo->nbActorIndexes);

									int* pIndex = reinterpret_cast<int*>(pSendInfo + 1);

									for (int k = 0; k < pSendInfo->nbActorIndexes; k++) {
										if (*pIndex == -1) {
											if (pActorRef) {
												auto* pActor = LOAD_POINTER_CAST(CActor*, pActorRef->pActor);
												ImGui::Text("Target Actor (self): %s", pActor->name);
											}
										}
										else {
											CActor* pActor = CScene::ptable.g_ActorManager_004516a4->aActors[*pIndex];
											ImGui::Text("[%d] %s", k, pActor->name);
										}

										pIndex++;
									}
								}

								// Show structured event data for the payload passed to ReceiveEvent
								int* pIndex = reinterpret_cast<int*>(pSendInfo + 1);
								int* pReceiveData = pIndex + pSendInfo->nbActorIndexes;
								int nbEventData = (pSendInfo->field_0x0 - 1) - pSendInfo->nbActorIndexes;
								ShowEventData(pReceiveData, nbEventData);

								if (ImGui::Button(("Activate##" + std::to_string(i) + "_" + std::to_string(j)).c_str())) {
									edCEventMessage msg;
									msg.colliderId = j;
									msg.pEventChunk = pChunk;
									msg.pEventCollider = pCollider;

									EventCallbackSendMessage(&msg);
								}
							}
						}
						else {
							ImGui::TextDisabled("%s (no targets)", GetColliderSlotLabel(pCollider, j).c_str());
						}
					}

					ImGui::Spacing();
					ImGui::Separator();

					pCollider++;
				}
			}
		}
	}

	ImGui::Separator();

	gShowEventsOverlay.DrawImguiControl();

	if (gShowEventsOverlay) {
		gShowAllZones.DrawImguiControl();
		gShowSelectedEvent.DrawImguiControl();
		gShowActiveEvents.DrawImguiControl();
	}

	ImGui::Spacing();

	gShowEventShapes3D.DrawImguiControl();

	if (gShowEventShapes3D) {
		ImGui::Indent();
		gShowZoneSpheres.DrawImguiControl();
		gShowZonePrimitives.DrawImguiControl();
		if (gShowZonePrimitives) {
			ImGui::Indent();
			gShowAllZonePrimitives.DrawImguiControl();
			gShowZonePrimitivesShaded.DrawImguiControl();
			ImGui::Unindent();
		}
		gShowColliderPositions.DrawImguiControl();
		gShowOnlyKillPrimitives.DrawImguiControl();
		ImGui::Unindent();

		// Show computed world-space positions for the selected event so the user can cross-check
		// them against what the 3D shapes draw.
		const uint selChunkU = static_cast<uint>(gSelectedChunk);
		if (selChunkU < edEventNbChunks) {
			auto* pChunkDbg = pedEventChunks[selChunkU];
			const int selEvt = gSelectedEvent.get();
			if (pChunkDbg && selEvt >= 0 && selEvt < static_cast<int>(pChunkDbg->nbEvents)) {
				auto* pEvtDbg  = LOAD_POINTER_CAST(ed_event*, pChunkDbg->aEvents[selEvt]);
				auto* pZoneDbg = LOAD_POINTER_CAST(ed_zone_3d*, pEvtDbg->pZone);
				if (pZoneDbg && ImGui::CollapsingHeader("3D Shape Debug Info")) {
					auto* pZMat = LOAD_POINTER_CAST(edF32MATRIX4*, pZoneDbg->pMatrix);
					ImGui::TextDisabled("Zone pMatrix: %s", pZMat ? "YES (zone-local space)" : "null (world space)");

					// Bounding sphere
					{
						float cx = pZoneDbg->boundSphere.x;
						float cy = pZoneDbg->boundSphere.y;
						float cz = pZoneDbg->boundSphere.z;
						ImGui::Text("BoundSphere raw  (%.1f, %.1f, %.1f)  r=%.1f", cx, cy, cz, pZoneDbg->boundSphere.w);
						if (pZMat) {
							const glm::vec3 wc(glm::inverse(glm::make_mat4(pZMat->raw)) * glm::vec4(cx, cy, cz, 1.0f));
							ImGui::Text("BoundSphere world(%.1f, %.1f, %.1f)", wc.x, wc.y, wc.z);
						}
					}

					// Primitives
					const int nbPDbg = edEventGetChunkZoneNbInclusivePrimitives(static_cast<int>(selChunkU), pZoneDbg);
					ImGui::Text("Primitives: %d", nbPDbg);
					for (int p = 0; p < nbPDbg; p++) {
						e_ed_event_prim3d_type ptDbg = 0;
						edF32MATRIX4* pPMat = edEventGetChunkZonePrimitive(selChunkU, pZoneDbg, static_cast<uint>(p), &ptDbg);
						if (pPMat) {
							glm::mat4 w2l = glm::make_mat4(pPMat->raw);
							if (pZMat) w2l = w2l * glm::make_mat4(pZMat->raw);
							const glm::vec3 wc(glm::inverse(w2l)[3]);
							const char* shapeName = GetZonePrimShapeName(static_cast<uint>(ptDbg));
						if (shapeName)
							ImGui::Text("  [%d] shape=%s (%d)  world center (%.1f, %.1f, %.1f)", p, shapeName, static_cast<int>(ptDbg), wc.x, wc.y, wc.z);
						else
							ImGui::Text("  [%d] shape=%d (unknown)  world center (%.1f, %.1f, %.1f)", p, static_cast<int>(ptDbg), wc.x, wc.y, wc.z);
						}
					}

					// Colliders
					auto* pCDbg = reinterpret_cast<_ed_event_collider_test*>(pEvtDbg + 1);
					for (int c = 0; c < static_cast<int>(pEvtDbg->nbColliders); c++, pCDbg++) {
						auto* pARef = LOAD_POINTER_CAST(ed_event_actor_ref*, pCDbg->pActorRef);
						const char* aName = "?";
						if (pARef) {
							auto* pAc = LOAD_POINTER_CAST(CActor*, pARef->pActor);
							if (pAc) aName = pAc->name;
						}
						ImGui::Text("  Collider[%d] (%s)", c, aName);
						if (pARef) {
							auto* pLoc = LOAD_POINTER_CAST(edF32VECTOR4*, pARef->pLocation);
							if (pLoc) ImGui::Text("    world pos:  (%.1f, %.1f, %.1f)", pLoc->x, pLoc->y, pLoc->z);
						}
						// worldLocation is actually the primitive-local position, not world space
						ImGui::TextDisabled("    prim-local:  (%.1f, %.1f, %.1f) [worldLocation]",
						                    pCDbg->worldLocation.x, pCDbg->worldLocation.y, pCDbg->worldLocation.z);
					}
				}
			}
		}
	}

	ImGui::End();

	if (bTrackingEvents) {
		ImGui::Begin("Event Tracking", &bTrackingEvents, ImGuiWindowFlags_AlwaysAutoResize);

		if (ImGui::Button("Clear")) gEventRecords.clear();

		ImGui::Separator();

		// List the events that have been triggered
		for (auto& record : gEventRecords) {
			// Build a readable header: slot name + event index + collider index + actor name (if resolvable)
			const char* slotName = "?";
			switch (record.first.colliderId) {
			case 0: slotName = "On Enter";       break;
			case 1: slotName = "On Exit";         break;
			case 2: slotName = "While Inside";    break;
			case 3: slotName = "While Outside";   break;
			}

			// Try to resolve actor name from the active chunk
			const char* actorName = "?";
			auto* pEventChunk = pedEventChunks[gSelectedChunk];
			if (pEventChunk && record.first.eventIndex < (int)pEventChunk->nbEvents) {
				auto* pEvent = LOAD_POINTER_CAST(ed_event*, pEventChunk->aEvents[record.first.eventIndex]);
				if (record.first.colliderIndex < (int)pEvent->nbColliders) {
					auto* pCol = reinterpret_cast<_ed_event_collider_test*>(pEvent + 1) + record.first.colliderIndex;
					auto* pActorRef = LOAD_POINTER_CAST(ed_event_actor_ref*, pCol->pActorRef);
					if (pActorRef) {
						auto* pActor = LOAD_POINTER_CAST(CActor*, pActorRef->pActor);
						if (pActor) actorName = pActor->name;
					}
				}
			}

			std::stringstream ss;
			ss << slotName << " | evt " << record.first.eventIndex
			   << " coll " << record.first.colliderIndex
			   << " | " << actorName
			   << " (" << record.second.triggers.size() << "x)";

			if (ImGui::CollapsingHeader(ss.str().c_str())) {
				ImGui::Text("Triggered %d time(s)", (int)record.second.triggers.size());

				static int selectedTrigger = 0;

				// Add selector for the trigger with a combo box
				if (ImGui::BeginCombo("Select Trigger", std::to_string(selectedTrigger).c_str())) {
					for (int i = 0; i < (int)record.second.triggers.size(); i++) {
						if (ImGui::Selectable(std::to_string(i).c_str())) {
							selectedTrigger = i;
						}
					}

					ImGui::EndCombo();
				}

				selectedTrigger = std::clamp<int>(selectedTrigger, 0, (int)record.second.triggers.size() - 1);

				if (selectedTrigger < (int)record.second.triggers.size()) {
					auto& trigger = record.second.triggers[selectedTrigger];
					for (auto* pActor : trigger.receivers) {
						ImGui::Text("Receiver: %s", pActor->name);
					}
				}

				// Jump to this event in the inspector
				if (pEventChunk && record.first.eventIndex < (int)pEventChunk->nbEvents) {
					if (ImGui::Button("Select in Inspector")) {
						gSelectedEvent = record.first.eventIndex;
					}
				}
			}

			ImGui::Separator();
		}

		ImGui::End();
	}

	auto* pEventChunk = pedEventChunks[gSelectedChunk];

	if (gShowEventsOverlay && pEventChunk) {
		// Remove the title bar and window border and make the window transparent
		ImGui::Begin("Event Overlay", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar);

		// Overlay this window on top of the game window
		ImGui::SetWindowPos(FrameBuffer::GetGameWindowPosition(), ImGuiCond_Always);
		ImGui::SetWindowSize(FrameBuffer::GetGameWindowSize(), ImGuiCond_Always);

		if (gShowAllZones) {
			for (int i = 0; i < edEventGetChunkNbEvents(gSelectedChunk); i++) {
				const auto* pZone = edEventGetChunkZone(gSelectedChunk, i);

				ImVec2 screenPos;
				if (Debug::Projection::WorldToScreen(pZone->boundSphere, screenPos)) {
					ImGui::SetCursorPos(screenPos);
					ImGui::Text("Event ID: 0x%x", i);
				}
			}
		}

		if (gShowSelectedEvent) {
			if (gSelectedEvent < pEventChunk->nbEvents) {
				auto* pEvent = LOAD_POINTER_CAST(ed_event*, pEventChunk->aEvents[gSelectedEvent]);

				auto* pZone = LOAD_POINTER_CAST(ed_zone_3d*, pEvent->pZone);

				{
					ImVec2 screenPos;
					if (Debug::Projection::WorldToScreen(pZone->boundSphere, screenPos)) {
						ImGui::SetCursorPos(screenPos);
						ImGui::Text("Zone");
					}
				}

				auto* pCollider = reinterpret_cast<_ed_event_collider_test*>(pEvent + 1);

				for (int i = 0; i < pEvent->nbColliders; i++) {
					auto* pActorRef = LOAD_POINTER_CAST(ed_event_actor_ref*, pCollider->pActorRef);

					if (pActorRef) {
						auto* pActor = LOAD_POINTER_CAST(CActor*, pActorRef->pActor);
						auto* pLocation = LOAD_POINTER_CAST(edF32VECTOR4*, pActorRef->pLocation);

						ImVec2 screenPos;
						if (Debug::Projection::WorldToScreen(*pLocation, screenPos)) {
							ImGui::SetCursorPos(screenPos);
							std::string buff;
							buff += "(" + std::to_string(i) + ") " + pActor->name + "\n";

							// append the hex result of the zone test
							buff += "Result: 0x" + std::to_string(edEventComputeZoneAgainstVertex(pEventChunk, pZone, pLocation, 0));

							ImGui::Text("%s", buff.c_str());
						}
					}
				}
			}
		}

		if (gShowActiveEvents) {
			for (int i = 0; i < pEventChunk->nbEvents; i++) {
				bool bActive = false;

				auto* pEvent = LOAD_POINTER_CAST(ed_event*, pEventChunk->aEvents[i]);

				auto* pZone = LOAD_POINTER_CAST(ed_zone_3d*, pEvent->pZone);

				auto* pCollider = reinterpret_cast<_ed_event_collider_test*>(pEvent + 1);

				for (int j = 0; j < pEvent->nbColliders; j++) {
					if (pCollider->flags & ED_COLLIDER_FLAG_INSIDE) {
						bActive = true;

						auto* pActorRef = LOAD_POINTER_CAST(ed_event_actor_ref*, pCollider->pActorRef);
						auto* pActor = LOAD_POINTER_CAST(CActor*, pActorRef->pActor);
						auto* pLocation = LOAD_POINTER_CAST(edF32VECTOR4*, pActorRef->pLocation);

						bool bTriggerSelf = false;

						for (int k = 0; k < 4; k++) {
							auto* pSendInfo = LOAD_POINTER_CAST(EventSendInfo*, pCollider->aSendInfo[k]);

							if (pSendInfo) {
								if (pSendInfo->nbActorIndexes == 0) {
									bTriggerSelf = true;
								}
								else {
									int* pIndex = reinterpret_cast<int*>(pSendInfo + 1);

									for (int l = 0; l < pSendInfo->nbActorIndexes; l++) {
										if (*pIndex == -1) {
											bTriggerSelf = true;
										}
										else {
											CActor* pActor = CScene::ptable.g_ActorManager_004516a4->aActors[*pIndex];
											
											ImVec2 screenPos;
											if (Debug::Projection::WorldToScreen(pActor->currentLocation, screenPos)) {
												ImGui::SetCursorPos(screenPos);
												ImGui::Text("\nActor Index: %d (%s)", l, pActor->name);
											}
										}

										pIndex++;
									}
								}
							}
						}

						ImVec2 screenPos;
						if (Debug::Projection::WorldToScreen(*pLocation, screenPos)) {
							ImGui::SetCursorPos(screenPos);
							ImGui::Text("Collider ID: 0x%x (%s) %s", j, pActor->name, bTriggerSelf ? "(TS)" : "");
						}
					}
				}

				if (bActive) {
					ImVec2 screenPos;
					if (Debug::Projection::WorldToScreen(pZone->boundSphere, screenPos)) {
						ImGui::SetCursorPos(screenPos);
						ImGui::Text("Event ID: 0x%x %s", i, pZone->boundSphere.ToString().c_str());
					}
				}
			}
		}

		ImGui::End();
	}
}

// Draws a zone primitive in world space.
// pPrimMatrix  : matrix returned by edEventGetChunkZonePrimitive — transforms zone-local positions
//                into unit-primitive local space.
// pZoneMatrix  : optional pZone->pMatrix — transforms world positions into zone-local space first.
//                When non-null the full world-to-primitive-local = pPrimMatrix * pZoneMatrix.
//
// GLM key: glm::make_mat4(raw) produces the same linear operator as edF32Matrix4MulF32Vector4Hard,
// so glm::inverse(glm::make_mat4(raw)) gives the local-to-world transform ready for glm::value_ptr.
// DecomposeLocalToWorld reads the bytes in column-major order (columns = world-space axes + center).
static void DrawZonePrimitive(const edF32MATRIX4* pPrimMatrix, const edF32MATRIX4* pZoneMatrix,
                               e_ed_event_prim3d_type primType, float r, float g, float b, float a,
                               bool bShaded = false)
{
	glm::mat4 worldToLocal = glm::make_mat4(pPrimMatrix->raw);
	if (pZoneMatrix) {
		// pPrimMatrix maps zone-local → prim-local; compose with pZoneMatrix (world → zone-local)
		worldToLocal = worldToLocal * glm::make_mat4(pZoneMatrix->raw);
	}
	const glm::mat4 localToWorld = glm::inverse(worldToLocal);
	const float* mat = glm::value_ptr(localToWorld);

	switch (primType) {
	case 0: // Half-space: y ≥ 0
		Renderer::Native::DebugShapes::AddHalfSpace(mat, r, g, b, a);
		break;
	case 1: // Plane+box: x,z ∈ [-0.5, 0.5], y ≥ 0
		Renderer::Native::DebugShapes::AddOpenTopBox(mat, r, g, b, a);
		break;
	case 2: // Box: x,y,z ∈ [-0.5, 0.5]
		if (bShaded)
			Renderer::Native::DebugShapes::AddFilledOBB(mat, 0.5f, 0.5f, 0.5f, r, g, b, a);
		else
			Renderer::Native::DebugShapes::AddOBB(mat, 0.5f, 0.5f, 0.5f, r, g, b, a);
		break;
	case 3: // Cylinder: y ∈ [-0.5, 0.5], xz radius = 1
		if (bShaded)
			Renderer::Native::DebugShapes::AddFilledCylinder(mat, r, g, b, a);
		else
			Renderer::Native::DebugShapes::AddCylinder(mat, r, g, b, a);
		break;
	case 4: // Cone: apex at y=0.5, base at y=-0.5 with xz radius = 1
		if (bShaded)
			Renderer::Native::DebugShapes::AddFilledCone(mat, r, g, b, a);
		else
			Renderer::Native::DebugShapes::AddCone(mat, r, g, b, a);
		break;
	case 5: { // Sphere: x²+y²+z² ≤ 1 (unit sphere)
		// Column 3 of localToWorld = world-space origin; column 0 length = scale (radius for unit sphere)
		const glm::vec3 worldCenter(localToWorld[3]);
		const float radius = glm::length(glm::vec3(localToWorld[0]));
		if (bShaded)
			Renderer::Native::DebugShapes::AddFilledSphere(worldCenter.x, worldCenter.y, worldCenter.z, radius, r, g, b, a);
		else
			Renderer::Native::DebugShapes::AddSphere(worldCenter.x, worldCenter.y, worldCenter.z, radius, r, g, b, a);
		break;
	}
	default:
		break;
	}
}

// Returns true if any send-info slot in any collider of pEvent carries EVENT_PRIM_KILL as its event type.
static bool EventHasKillPrim(ed_event* pEvent)
{
	auto* pCollider = reinterpret_cast<_ed_event_collider_test*>(pEvent + 1);
	for (uint c = 0; c < pEvent->nbColliders; c++, pCollider++) {
		for (int k = 0; k < 4; k++) {
			auto* pSendInfo = LOAD_POINTER_CAST(EventSendInfo*, pCollider->aSendInfo[k]);
			if (!pSendInfo) continue;
			int* pIndex      = reinterpret_cast<int*>(pSendInfo + 1);
			int* pReceiveData = pIndex + pSendInfo->nbActorIndexes;
			int  nbEventData  = (pSendInfo->field_0x0 - 1) - pSendInfo->nbActorIndexes;
			if (nbEventData >= 1 && static_cast<uint>(pReceiveData[0]) == EVENT_PRIM_KILL)
				return true;
		}
	}
	return false;
}

void Debug::Event::DrawEventShapes()
{
	if (!gShowEventShapes3D) return;
	if (!pedEventChunks || edEventNbChunks == 0) return;

	const uint selectedChunk = static_cast<uint>(gSelectedChunk);
	if (selectedChunk >= edEventNbChunks) return;

	auto* pChunk = pedEventChunks[selectedChunk];
	if (!pChunk) return;

	const int nbEvents = static_cast<int>(pChunk->nbEvents);
	const int selectedEvent = gSelectedEvent.get();

	for (int i = 0; i < nbEvents; i++) {
		auto* pEvent = LOAD_POINTER_CAST(ed_event*, pChunk->aEvents[i]);
		if (!pEvent) continue;

		if (gShowOnlyKillPrimitives && !EventHasKillPrim(pEvent)) continue;

		auto* pZone = LOAD_POINTER_CAST(ed_zone_3d*, pEvent->pZone);
		if (!pZone) continue;

		const bool bEventActive = (pEvent->flags & ED_EVENT_FLAG_ACTIVE) != 0;
		const bool bIsSelected  = (i == selectedEvent);

		// pZone->pMatrix (when non-null) transforms the actor's world position into zone-local
		// space before the bounding sphere and primitive tests. In that case the bounding sphere
		// xyz and the primitive matrices are expressed in zone-local space, NOT world space.
		auto* pZoneMatrix = LOAD_POINTER_CAST(edF32MATRIX4*, pZone->pMatrix);

		// Zone bounding spheres
		const bool bZeroVolume = (pZone->boundSphere.w == 0.0f);
		if (gShowZoneSpheres) {
			float r, g, b, a;
			if (bZeroVolume && bIsSelected) {
				r = 0.2f; g = 0.5f; b = 1.0f; a = 1.0f;   // blue = selected (zero-volume)
			} else if (bZeroVolume) {
				r = 1.0f; g = 0.8f; b = 0.0f; a = 1.0f;   // yellow = zero-volume (no sphere cull, always tests primitives)
			} else if (bIsSelected) {
				r = 0.2f; g = 0.5f; b = 1.0f; a = 1.0f;   // blue = selected
			} else if (bEventActive) {
				r = 0.0f; g = 1.0f; b = 0.0f; a = 0.7f;   // green = active
			} else {
				r = 0.5f; g = 0.5f; b = 0.5f; a = 0.4f;   // grey = inactive
			}
			float cx = pZone->boundSphere.x;
			float cy = pZone->boundSphere.y;
			float cz = pZone->boundSphere.z;
			if (pZoneMatrix) {
				// Bounding sphere center is in zone-local space; apply inverse(pZoneMatrix) to
				// bring it back to world space. Assumes rigid body (radius unchanged by transform).
				const glm::vec3 worldCenter(glm::inverse(glm::make_mat4(pZoneMatrix->raw))
				                            * glm::vec4(cx, cy, cz, 1.0f));
				cx = worldCenter.x; cy = worldCenter.y; cz = worldCenter.z;
			}
			const float drawRadius = bZeroVolume ? 0.1f : pZone->boundSphere.w;
			Renderer::Native::DebugShapes::AddSphere(cx, cy, cz, drawRadius, r, g, b, a);
		}

		// Zone primitives: always draw for zero-volume zones (their sphere cull is always skipped,
		// so the primitive is the only shape being tested). For normal zones, draw only for the
		// selected event to avoid clutter, unless gShowAllZonePrimitives is on.
		if (gShowZonePrimitives && (bIsSelected || bZeroVolume || gShowAllZonePrimitives)) {
			const int nbPrims = edEventGetChunkZoneNbInclusivePrimitives(static_cast<int>(selectedChunk), pZone);
			for (int p = 0; p < nbPrims; p++) {
				e_ed_event_prim3d_type primType = 0;
				edF32MATRIX4* pPrimMatrix = edEventGetChunkZonePrimitive(selectedChunk, pZone, static_cast<uint>(p), &primType);
				if (!pPrimMatrix) continue;

				if (bZeroVolume && !bIsSelected) {
					// Yellow-tinted to match the marker sphere; dimmer alpha to reduce noise
					DrawZonePrimitive(pPrimMatrix, pZoneMatrix, primType, 1.0f, 0.8f, 0.0f, 0.6f, static_cast<bool>(gShowZonePrimitivesShaded));
				} else if (!bIsSelected) {
					// Non-selected normal events: grey-tinted to distinguish from selected cyan
					DrawZonePrimitive(pPrimMatrix, pZoneMatrix, primType, 0.5f, 0.5f, 0.5f, 0.4f, static_cast<bool>(gShowZonePrimitivesShaded));
				} else {
					// Selected event primitives in cyan
					DrawZonePrimitive(pPrimMatrix, pZoneMatrix, primType, 0.0f, 0.9f, 1.0f, 1.0f, static_cast<bool>(gShowZonePrimitivesShaded));
				}
			}
		}

		// Collider actor positions. Note: pCollider->worldLocation is NOT a world-space position;
		// it stores the actor's coordinates in primitive-local space (computed by
		// _edEventComputeZeroVolumeZoneAgainstVertex). Use pActorRef->pLocation for the actual
		// world position.
		if (gShowColliderPositions) {
			auto* pCollider = reinterpret_cast<_ed_event_collider_test*>(pEvent + 1);
			for (int c = 0; c < static_cast<int>(pEvent->nbColliders); c++, pCollider++) {
				auto* pActorRef = LOAD_POINTER_CAST(ed_event_actor_ref*, pCollider->pActorRef);
				if (!pActorRef) continue;
				auto* pLocation = LOAD_POINTER_CAST(edF32VECTOR4*, pActorRef->pLocation);
				if (!pLocation) continue;

				const bool bInside = (pCollider->flags & ED_COLLIDER_FLAG_INSIDE) != 0;
				float cr, cg, cb;
				if (bInside) {
					cr = 0.0f;  cg = 1.0f;  cb = 0.3f; // green = inside
				} else {
					cr = 1.0f;  cg = 0.3f;  cb = 0.0f; // red/orange = outside
				}
				Renderer::Native::DebugShapes::AddSphere(
					pLocation->x, pLocation->y, pLocation->z,
					15.0f, cr, cg, cb, 1.0f);
			}
		}
	}
}

namespace Debug {
    MenuRegisterer  sDebugEventMenuReg("Event", Debug::Event::ShowMenu);
    UpdateRegisterer sDebugEventUpdateReg(Debug::Event::DrawEventShapes);
}

