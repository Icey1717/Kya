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

extern ed_event_chunk** pedEventChunks;
extern uint edEventNbChunks;

void EventCallbackSendMessage(edCEventMessage* pEventMessage);

namespace Debug {
	namespace Event {
		static Setting<bool> gShowEventsOverlay("Show Events Overlay", true);
		static Setting<bool> gShowAllZones("Show All Zones", false);
		static Setting<bool> gShowSelectedEvent("Show Selected Event", true);
		static Setting<bool> gShowActiveEvents("Show Active Events", true);

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
				auto* pEvent = LOAD_SECTION_CAST(ed_event*, pEventMessage->pEventChunk->aEvents[i]);

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

			pSendInfo = LOAD_SECTION_CAST(EventSendInfo*, pEventMessage->pEventCollider->aSendInfo[pEventMessage->colliderId]);

			nbActorIndexes = pSendInfo->nbActorIndexes;
			pCurrentActorIndex = reinterpret_cast<int*>(pSendInfo + 1);
			iVar6 = (pSendInfo->field_0x0 + -1) - nbActorIndexes;

			int* pReceiveData = pCurrentActorIndex + nbActorIndexes;

			if (nbActorIndexes == 0) {
				ed_event_actor_ref* pRef = LOAD_SECTION_CAST(ed_event_actor_ref*, pEventMessage->pEventCollider->pActorRef);
				CActor* pActorRef = LOAD_SECTION_CAST(CActor*, pRef->pActor);
				newRecord.receivers.push_back(pActorRef);
			}
			else {
				while (bVar1 = nbActorIndexes != 0, nbActorIndexes = nbActorIndexes + -1, bVar1) {
					iVar3 = *pCurrentActorIndex;

					if (iVar3 == -1) {
						ed_event_actor_ref* pRef = LOAD_SECTION_CAST(ed_event_actor_ref*, pEventMessage->pEventCollider->pActorRef);
						CActor* pActorRef = LOAD_SECTION_CAST(CActor*, pRef->pActor);

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
	static int selectedChunk = 0;

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

	if (ImGui::BeginCombo("Select Chunk", std::to_string(selectedChunk).c_str())) {
		for (int i = 0; i < edEventNbChunks; i++) {
			bool isSelected = selectedChunk == i;
			if (ImGui::Selectable((std::to_string(i) + (pedEventChunks[i] ? "" : " (Invalid)")).c_str(), isSelected)) {
				selectedChunk = i;
			}

			if (isSelected) {
				ImGui::SetItemDefaultFocus();
			}
		}

		ImGui::EndCombo();
	}

	ImGui::Spacing();

	// show chunk details
	if (selectedChunk < edEventNbChunks) {
		auto* pChunk = pedEventChunks[selectedChunk];

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

			if (ImGui::BeginCombo("Select Event", std::to_string(gSelectedEvent).c_str())) {
				for (int i = 0; i < pChunk->nbEvents; i++) {
					bool isSelected = gSelectedEvent == i;
					if (ImGui::Selectable(std::to_string(i).c_str(), isSelected)) {
						gSelectedEvent = i;
					}

					if (isSelected) {
						ImGui::SetItemDefaultFocus();
					}
				}

				ImGui::EndCombo();
			}

			if (gSelectedEvent < pChunk->nbEvents && ImGui::CollapsingHeader("Event Details")) {
				auto* pEvent = LOAD_SECTION_CAST(ed_event*, pChunk->aEvents[gSelectedEvent]);

				ImGui::Text("flags: 0x%x", pEvent->flags);

				DebugHelpers::TextValidValue("pZone %p", pEvent->pZone);

				auto* pZone = LOAD_SECTION_CAST(ed_zone_3d*, pEvent->pZone);

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

				for (int i = 0; i < pEvent->nbColliders; i++) {
					ImGui::Text("Collider %d", i);
					ImGui::Text("worldLocation: %s", pCollider->worldLocation.ToString().c_str());

					ImGui::Spacing();

					ImGui::Text("messageFlags[0]: %d", pCollider->messageFlags[0]);
					ImGui::Text("messageFlags[1]: %d", pCollider->messageFlags[1]);
					ImGui::Text("messageFlags[2]: %d", pCollider->messageFlags[2]);
					ImGui::Text("messageFlags[3]: %d", pCollider->messageFlags[3]);

					ImGui::Spacing();

					ImGui::Text("flags: 0x%x", pCollider->flags);

					auto* pActorRef = LOAD_SECTION_CAST(ed_event_actor_ref*, pCollider->pActorRef);

					if (pActorRef) {
						auto* pActor = LOAD_SECTION_CAST(CActor*, pActorRef->pActor);
						ImGui::Text("pActor (%s)", pActor ? pActor->name : "null");

						auto* pLocation = LOAD_SECTION_CAST(edF32VECTOR4*, pActorRef->pLocation);

						ImGui::Text("pLocation %s", pLocation ? pLocation->ToString().c_str() : "null");
					}

					DebugHelpers::TextValidValue("field_0x1c %p", pCollider->field_0x1c);

					ImGui::Spacing();

					ImGui::Text("aSendInfo - 0: %d", pCollider->aSendInfo[0]);
					ImGui::Text("aSendInfo - 1: %d", pCollider->aSendInfo[1]);
					ImGui::Text("aSendInfo - 2: %d", pCollider->aSendInfo[2]);
					ImGui::Text("aSendInfo - 3: %d", pCollider->aSendInfo[3]);

					for (int j = 0; j < 4; j++) {
						ImGui::Text("aSendInfo - %d: %d", j, pCollider->aSendInfo[j]);

						auto* pSendInfo = LOAD_SECTION_CAST(EventSendInfo*, pCollider->aSendInfo[j]);

						if (pSendInfo && ImGui::CollapsingHeader(std::to_string(j).c_str())) {
							if (pSendInfo->nbActorIndexes == 0) {
								if (pActorRef) {
									auto* pActor = LOAD_SECTION_CAST(CActor*, pActorRef->pActor);
									ImGui::Text("(TA) %s", pActor->name);
								}
							}
							else {
								ImGui::Text("nbActorIndexes: %d", pSendInfo->nbActorIndexes);

								int* pIndex = reinterpret_cast<int*>(pSendInfo + 1);

								for (int k = 0; k < pSendInfo->nbActorIndexes; k++) {
									if (*pIndex == -1) {
										if (pActorRef) {
											auto* pActor = LOAD_SECTION_CAST(CActor*, pActorRef->pActor);
											ImGui::Text("(TA) %s", pActor->name);
										}
									}
									else {
										CActor* pActor = CScene::ptable.g_ActorManager_004516a4->aActors[*pIndex];
										ImGui::Text("Actor Index %d: %s", k, pActor->name);
									}

									pIndex++;
								}
							}

							if (ImGui::Button("Activate")) {
								edCEventMessage msg;
								msg.colliderId = j;
								msg.pEventChunk = pChunk;
								msg.pEventCollider = pCollider;

								EventCallbackSendMessage(&msg);
							}
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

	ImGui::End();

	if (bTrackingEvents) {
		ImGui::Begin("Event Tracking", &bTrackingEvents, ImGuiWindowFlags_AlwaysAutoResize);

		// List the events that have been triggered
		for (auto& record : gEventRecords) {
			std::stringstream ss;
			ss << "Event: 0x" << std::hex << record.first.colliderId << " 0x" << record.first.eventIndex << " 0x" << record.first.colliderIndex;

			if (ImGui::CollapsingHeader(ss.str().c_str())) {
				ImGui::Text("Triggered %d times", record.second.triggers.size());
				
				static int selectedTrigger = 0;

				// Add selector for the trigger with a combo box
				if (ImGui::BeginCombo("Select Trigger", std::to_string(selectedTrigger).c_str())) {
					for (int i = 0; i < record.second.triggers.size(); i++) {
						if (ImGui::Selectable(std::to_string(i).c_str())) {
							selectedTrigger = i;
						}
					}

					ImGui::EndCombo();
				}

				selectedTrigger = std::clamp<int>(selectedTrigger, 0, record.second.triggers.size() - 1);

				if (selectedTrigger < record.second.triggers.size()) {
					auto& trigger = record.second.triggers[selectedTrigger];
					for (auto* pActor : trigger.receivers) {
						ImGui::Text("Receiver: %s", pActor->name);
					}
				}
			}

			ImGui::Separator();
		}

		ImGui::End();
	}

	auto* pEventChunk = pedEventChunks[selectedChunk];

	if (gShowEventsOverlay && pEventChunk) {
		// Remove the title bar and window border and make the window transparent
		ImGui::Begin("Event Overlay", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar);

		// Overlay this window on top of the game window
		ImGui::SetWindowPos(FrameBuffer::GetGameWindowPosition(), ImGuiCond_Always);
		ImGui::SetWindowSize(FrameBuffer::GetGameWindowSize(), ImGuiCond_Always);

		if (gShowAllZones) {
			for (int i = 0; i < edEventGetChunkNbEvents(selectedChunk); i++) {
				const auto* pZone = edEventGetChunkZone(selectedChunk, i);

				ImVec2 screenPos;
				if (Debug::Projection::WorldToScreen(pZone->boundSphere, screenPos)) {
					ImGui::SetCursorPos(screenPos);
					ImGui::Text("Event ID: 0x%x", i);
				}
			}
		}

		if (gShowSelectedEvent) {
			if (gSelectedEvent < pEventChunk->nbEvents) {
				auto* pEvent = LOAD_SECTION_CAST(ed_event*, pEventChunk->aEvents[gSelectedEvent]);

				auto* pZone = LOAD_SECTION_CAST(ed_zone_3d*, pEvent->pZone);

				{
					ImVec2 screenPos;
					if (Debug::Projection::WorldToScreen(pZone->boundSphere, screenPos)) {
						ImGui::SetCursorPos(screenPos);
						ImGui::Text("Zone");
					}
				}

				auto* pCollider = reinterpret_cast<_ed_event_collider_test*>(pEvent + 1);

				for (int i = 0; i < pEvent->nbColliders; i++) {
					auto* pActorRef = LOAD_SECTION_CAST(ed_event_actor_ref*, pCollider->pActorRef);

					if (pActorRef) {
						auto* pActor = LOAD_SECTION_CAST(CActor*, pActorRef->pActor);
						auto* pLocation = LOAD_SECTION_CAST(edF32VECTOR4*, pActorRef->pLocation);

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

				auto* pEvent = LOAD_SECTION_CAST(ed_event*, pEventChunk->aEvents[i]);

				auto* pZone = LOAD_SECTION_CAST(ed_zone_3d*, pEvent->pZone);

				auto* pCollider = reinterpret_cast<_ed_event_collider_test*>(pEvent + 1);

				for (int j = 0; j < pEvent->nbColliders; j++) {
					if (pCollider->flags & 1) {
						bActive = true;

						auto* pActorRef = LOAD_SECTION_CAST(ed_event_actor_ref*, pCollider->pActorRef);
						auto* pActor = LOAD_SECTION_CAST(CActor*, pActorRef->pActor);
						auto* pLocation = LOAD_SECTION_CAST(edF32VECTOR4*, pActorRef->pLocation);

						bool bTriggerSelf = false;

						for (int k = 0; k < 4; k++) {
							auto* pSendInfo = LOAD_SECTION_CAST(EventSendInfo*, pCollider->aSendInfo[k]);

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
