#include <winsock2.h>
#include <ws2tcpip.h>

#include "DebugSocket.h"
#include "DebugMenu.h"
#include "DebugMenuToolbar.h"
#include "DebugCamera.h"
#include "DrawTrace.h"
#include "DebugSetting.h"
#include "DebugSaveScreenshot.h"
#include "DebugMeshViewer.h"
#include "renderer.h"
#include "VulkanRenderer.h"
#include "Native/NativeRenderer.h"
#include "CameraViewManager.h"
#include "ActorManager.h"
#include "Actor.h"
#include "ActorCheckpointManager.h"
#include "SectorManager.h"
#include "imgui.h"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <deque>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

namespace Debug::Socket
{
	namespace
	{
		Setting<bool> enabled("Enable Debug Socket", false);
		Setting<int> configuredPort("Debug Socket Port", 18438);
		SOCKET listener = INVALID_SOCKET;
		uint16_t boundPort = 0;
		bool winsockStarted = false;
		uint64_t frame = 0;
		std::string lastError;
		using Clock = std::chrono::steady_clock;
		uint64_t nextSequence = 1, connections = 0, completed = 0, errors = 0, dropped = 0;
		uint64_t bytesSent = 0;
		std::deque<RequestInfo> history;
		struct Client
		{
			SOCKET socket;
			std::string input;
			std::vector<char> output;
			size_t sent = 0;
			Clock::time_point activity = Clock::now();
			Clock::time_point connected = Clock::now();
			RequestInfo info;
		};
		std::vector<Client> clients;
		constexpr size_t maxRequest = 8192;
		constexpr size_t maxHistory = 128;

		void Finish(Client& client, const char* reason = nullptr)
		{
			auto& info = client.info;
			info.finished = true;
			info.elapsedMs = std::chrono::duration<double, std::milli>(Clock::now() - client.connected).count();
			if (reason) {
				info.state = reason;
				++dropped;
			}
			else if (!info.error.empty()) {
				info.state = "Error";
				++errors;
			}
			else {
				info.state = "Completed";
				++completed;
			}
			history.push_front(info);
			if (history.size() > maxHistory) history.pop_back();
		}

		bool Nonblocking(SOCKET socket)
		{
			u_long mode = 1;
			return ioctlsocket(socket, FIONBIO, &mode) == 0;
		}

		void Reply(Client& client, nlohmann::json header, const std::vector<char>& payload = {})
		{
			header["bytes"] = payload.size();
			const auto line = header.dump() + "\n";
			client.output.assign(line.begin(), line.end());
			client.output.insert(client.output.end(), payload.begin(), payload.end());
			client.info.state = "Sending";
			client.info.error = header.value("error", std::string{});
			client.info.responseBytes = client.output.size();
		}

		void Handle(Client& client)
		{
			const auto started = Clock::now();
			client.info.frame = frame;
			client.info.command = "(invalid request)";
			nlohmann::json id = nullptr;
			try {
				const auto request = nlohmann::json::parse(client.input);
				if (!request.is_object()) throw std::runtime_error("Request must be a JSON object");
				id = request.value("id", nlohmann::json(nullptr));
				client.info.requestId = id.dump();
				const auto command = request.at("command").get<std::string>();
				client.info.command = command;
				const auto extent = Renderer::gHeadless ? VkExtent2D{} : Renderer::Native::GetFrameBufferSize();
				nlohmann::json response = { {"ok", true}, {"id", id}, {"protocol", 1},
					{"pid", GetCurrentProcessId()}, {"frame", frame}, {"offscreen", Renderer::IsRenderingOffscreen()},
					{"width", extent.width}, {"height", extent.height} };
				namespace Trace = Renderer::DrawTrace;
				const auto drawStatus = Trace::GetStatus();
				response["simulation_paused"] = IsSimulationPaused();
				response["draw_capture_pending"] = drawStatus.pending || drawStatus.capturing;
				response["actor_override_matches"] = drawStatus.matches;
				response["actor_override"] = drawStatus.action == Trace::Action::BindPose ? "bind_pose" :
					drawStatus.action == Trace::Action::BindPoseTextureOnly ? "texture_only" :
					drawStatus.action == Trace::Action::IsolateBindPose ? "isolate_bind_pose" : "none";
				if (const CCameraManager* camera = CCameraManager::_gThis) {
					const auto& transform = camera->displayTransformMatrix;
					response["camera"] = {
						{"position", {transform.rowT.x, transform.rowT.y, transform.rowT.z}},
						{"forward", {transform.rowZ.x, transform.rowZ.y, transform.rowZ.z}},
						{"up", {transform.rowY.x, transform.rowY.y, transform.rowY.z}},
						{"horizontal_fov_radians", 2.0 * std::atan2(double(_gDisplayCamera.finalHorizontalHalfFOV), double(_gDisplayCamera.computedVerticalHalfFOV))}
					};
				}
				if (CScene::_pinstance && CScene::ptable.g_SectorManager_00451670) {
					const CSectorManager* sectors = CScene::ptable.g_SectorManager_00451670;
					response["base_sector"] = sectors->baseSector.sectID;
					response["loaded_sectors"] = nlohmann::json::array();
					auto addSector = [&](const CSector& sector) {
						if (sector.sectID >= 0)
							response["loaded_sectors"].push_back({{"id", sector.sectID}, {"stage", sector.loadStage_0x8},
								{"installed", sector.sectorMesh.fileBufferStart != nullptr}});
					};
					addSector(sectors->baseSector);
					for (int i = 0; sectors->aSectors && i < sectors->nbSectors; ++i) addSector(sectors->aSectors[i]);
				}
				if (command == "status") {
					Reply(client, response);
				}
				else if (command == "pause" || command == "resume" || command == "step") {
					if (command == "step") {
						if (!StepSimulationFrame()) throw std::runtime_error("Pause simulation before stepping");
					}
					else if (!SetSimulationPaused(command == "pause")) throw std::runtime_error("Simulation timer unavailable");
					response["simulation_paused"] = IsSimulationPaused();
					Reply(client, response);
				}
				else if (command == "capture_draws") {
					Trace::RequestCapture(false);
					response["draw_capture_pending"] = true;
					Reply(client, response);
				}
				else if (command == "camera_reset") {
					Camera::ResetFreeCamera();
					Reply(client, response);
				}
				else if (command == "camera") {
					const auto position = request.at("position");
					edF32VECTOR4 pose = {};
					pose.x = position.at(0).get<float>();
					pose.y = position.at(1).get<float>();
					pose.z = position.at(2).get<float>();
					pose.w = 1.0f;
					const float yaw = request.at("yaw").get<float>();
					const float pitch = request.at("pitch").get<float>();
					Camera::SetFreeCameraPose(pose, yaw, pitch);
					response["camera_set"] = true;
					response["camera_position"] = { pose.x, pose.y, pose.z };
					response["camera_yaw"] = yaw;
					response["camera_pitch"] = pitch;
					Reply(client, response);
				}
				else if (command == "checkpoint") {
					const std::string managerName = request.at("manager").get<std::string>();
					const int checkpointIndex = request.at("index").get<int>();
					CActorCheckpointManager* pManager = nullptr;
					const auto* manager = CScene::_pinstance ? CScene::ptable.g_ActorManager_004516a4 : nullptr;
					if (manager && manager->aActors)
						for (int actorIndex = 0; actorIndex < manager->nbActors; ++actorIndex)
						{
							CActor* pActor = manager->aActors[actorIndex];
							if (pActor && pActor->typeID == CHECKPOINT_MANAGER && managerName == pActor->name)
							{
								pManager = dynamic_cast<CActorCheckpointManager*>(pActor);
								break;
							}
						}
					if (!pManager) throw std::runtime_error("Checkpoint manager not found: " + managerName);
					if (checkpointIndex < 0 || checkpointIndex >= pManager->checkpointCount)
						throw std::runtime_error("Checkpoint index out of range");
					const int sectorId = pManager->aCheckpoints[checkpointIndex].sectorId;
					pManager->ActivateCheckpoint(checkpointIndex);
					response["checkpoint_manager"] = managerName;
					response["checkpoint_index"] = checkpointIndex;
					response["checkpoint_sector"] = sectorId;
					Reply(client, response);
				}
				else if (command == "actor_info") {
					const std::string actorName = request.at("name").get<std::string>();
					const auto* manager = CScene::_pinstance ? CScene::ptable.g_ActorManager_004516a4 : nullptr;
					CActor* found = nullptr;
					if (manager && manager->aActors)
						for (int actorIndex = 0; actorIndex < manager->nbActors; ++actorIndex)
						{
							CActor* pActor = manager->aActors[actorIndex];
							if (pActor && actorName == pActor->name) { found = pActor; break; }
						}
					if (!found) throw std::runtime_error("Actor not found: " + actorName);
					response["actor_name"] = actorName;
					response["actor_type"] = int(found->typeID);
					response["actor_sector"] = found->sectorId;
					response["actor_scale"] = { found->scale.x, found->scale.y, found->scale.z };
					response["actor_position"] = { found->currentLocation.x, found->currentLocation.y, found->currentLocation.z };
					Reply(client, response);
				}
				else if (command == "focus_actor") {
					const int index = request.at("actor").get<int>();
					const auto* manager = CScene::_pinstance ? CScene::ptable.g_ActorManager_004516a4 : nullptr;
					CActor* actor = manager && manager->aActors && index >= 0 && index < manager->nbActors
						? manager->aActors[index] : nullptr;
					if (!actor || !CCameraManager::_gThis)
						throw std::runtime_error("Actor or camera is no longer available");
					Camera::FocusActor(actor);
					response["actor"] = index;
					Reply(client, response);
				}
				else if (command == "actors" || command == "actor_override" || command == "actor_lighting") {
					const auto snapshot = Trace::GetSnapshot();
					if (command == "actor_override" && request.at("mode") == "none") {
						Trace::ClearOverrides();
						response["actor_override"] = "none";
						response["actor_override_matches"] = 0;
					}
					else {
						if (drawStatus.pending || drawStatus.capturing || !snapshot || snapshot->generation != drawStatus.generation)
							throw std::runtime_error("Request capture_draws and wait for draw_capture_pending to clear");
						response["draw_capture_frame"] = snapshot->frame;
						if (command == "actors") {
							std::map<int, const Trace::Source*> actors;
							for (const auto& source : snapshot->sources)
								if (source.flushed && source.owner && source.ownerIndex >= 0)
									actors.try_emplace(source.ownerIndex, &source);
							response["actors"] = nlohmann::json::array();
							for (const auto& [index, source] : actors)
								response["actors"].push_back({{"index", index}, {"name", source->ownerName.data()},
									{"type", source->ownerType}, {"sector", source->sector}});
							response["draw_capture_truncated"] = snapshot->truncated;
						}
						else {
							const int index = request.at("actor").get<int>();
							const auto source = std::find_if(snapshot->sources.begin(), snapshot->sources.end(), [&](const auto& candidate) {
								return candidate.flushed && candidate.owner && candidate.ownerIndex == index && Trace::IsSourceLive(candidate);
							});
							if (index < 0 || source == snapshot->sources.end())
								throw std::runtime_error("Actor has no live draw in the capture");
							if (command == "actor_lighting") {
								const auto* manager = CScene::_pinstance ? CScene::ptable.g_ActorManager_004516a4 : nullptr;
								const CActor* actor = manager && manager->aActors && index < manager->nbActors ? manager->aActors[index] : nullptr;
								if (actor && reinterpret_cast<uintptr_t>(actor) == source->owner) {
									response["actor_position"] = {actor->currentLocation.x, actor->currentLocation.y, actor->currentLocation.z};
									response["actor_lighting_flags"] = actor->lightingFlags;
									response["actor_lighting_multiplier"] = actor->lightingFloat_0xe0;
									response["actor_ground_light"] = actor->field_0xf4;
								}
								response["submissions"] = nlohmann::json::array();
								std::map<uint64_t, const Trace::Draw*> draws;
								for (const auto& draw : snapshot->draws)
									if (draw.recorded) draws.try_emplace(draw.submissionId, &draw);
								for (const auto& submission : snapshot->submissions) {
									const auto owner = std::find_if(snapshot->sources.begin(), snapshot->sources.end(),
										[&](const auto& candidate) { return candidate.id == submission.sourceId; });
									if (owner == snapshot->sources.end() || owner->owner != source->owner) continue;
									const auto draw = draws.find(submission.id);
									response["submissions"].push_back({{"mesh", submission.mesh.data()},
										{"texture", draw == draws.end() ? "" : draw->second->texture.data()},
										{"model", submission.model},
										{"render_flags", submission.flags}, {"strip_flags", submission.stripFlags},
										{"ambient", submission.lightAmbient}, {"directions", submission.lightDirections},
										{"colors", submission.lightColors}, {"first_vertex_color", submission.firstVertexColor},
										{"first_vertex_normal", submission.firstVertexNormal}, {"first_vertex_flags", submission.firstVertexFlags}});
								}
							}
							else {
								const auto mode = request.at("mode").get<std::string>();
								if (mode != "bind_pose" && mode != "texture_only" && mode != "isolate_bind_pose")
									throw std::runtime_error("Unknown actor override mode: " + mode);
								const auto action = mode == "bind_pose" ? Trace::Action::BindPose :
									mode == "texture_only" ? Trace::Action::BindPoseTextureOnly : Trace::Action::IsolateBindPose;
								Trace::SetOverride(*source, nullptr, action, Trace::Match::Actor);
								response["actor_override"] = mode;
								response["actor_override_matches"] = 0;
							}
							response["actor"] = index;
						}
					}
					Reply(client, response);
				}
				else if (command == "screenshot") {
					const auto target = request.value("target", std::string("game"));
					if (target != "game" && target != "mesh") throw std::runtime_error("Unknown screenshot target: " + target);
					const bool mesh = target == "mesh";
					auto png = SaveLoad::CaptureScreenshotPng(mesh);
					if (png.empty()) throw std::runtime_error(mesh ? "Mesh viewer is unavailable; select a mesh first" : "Game framebuffer is unavailable");
					response["target"] = target;
					if (mesh) {
						response["width"] = DebugMeshViewer::gWidth;
						response["height"] = DebugMeshViewer::gHeight;
						response.erase("camera");
					}
					response["encoding"] = "png";
					Reply(client, response, png);
				}
				else throw std::runtime_error("Unknown command: " + command);
			}
			catch (const std::exception& error) {
				Reply(client, {{"ok", false}, {"id", id}, {"error", error.what()}});
			}
			client.info.processingMs = std::chrono::duration<double, std::milli>(Clock::now() - started).count();
		}

		void ShowMenu(bool* open)
		{
			ImGui::SetNextWindowSize(ImVec2(820, 440), ImGuiCond_FirstUseEver);
			if (!ImGui::Begin("Debug Socket", open)) {
				ImGui::End();
				return;
			}
			if (enabled.DrawImguiControl()) {
				if (enabled.get()) Start(static_cast<uint16_t>(std::clamp(configuredPort.get(), 1, 65535)));
				else Stop();
			}
			if (configuredPort.DrawImguiControl()) {
				configuredPort = std::clamp(configuredPort.get(), 1, 65535);
				if (enabled.get()) Start(static_cast<uint16_t>(configuredPort.get()));
			}
			if (boundPort) ImGui::Text("Listening on 127.0.0.1:%u (%zu clients)", boundPort, clients.size());
			else ImGui::TextUnformatted("Stopped");
			if (!lastError.empty()) ImGui::TextWrapped("%s", lastError.c_str());
			ImGui::TextWrapped("Commands: status, screenshot, pause, resume, step, capture_draws, actors, actor_override, focus_actor, camera_reset. Enable Render When Minimized to service requests while minimized.");
			ImGui::Separator();
			ImGui::Text("Connections: %llu   Completed: %llu   Errors: %llu   Dropped: %llu   Sent: %.1f KiB",
				connections, completed, errors, dropped, double(bytesSent) / 1024.0);
			static ImGuiTextFilter filter;
			static bool errorsOnly = false;
			filter.Draw("Filter", 220);
			ImGui::SameLine();
			ImGui::Checkbox("Errors only", &errorsOnly);
			ImGui::SameLine();
			if (ImGui::Button("Clear history")) ClearHistory();
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Clears completed rows. Live requests and session counters are retained.");
			ImGui::TextDisabled("Newest first. Last %zu closed connections retained. Hover a result for details.", maxHistory);
			if (ImGui::BeginTable("Requests", 6, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH |
				ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY, ImVec2(0, 0))) {
				ImGui::TableSetupScrollFreeze(0, 1);
				ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed, 40);
				ImGui::TableSetupColumn("Command / ID", ImGuiTableColumnFlags_WidthStretch);
				ImGui::TableSetupColumn("Result", ImGuiTableColumnFlags_WidthFixed, 90);
				ImGui::TableSetupColumn("Handle ms", ImGuiTableColumnFlags_WidthFixed, 75);
				ImGui::TableSetupColumn("Total ms", ImGuiTableColumnFlags_WidthFixed, 75);
				ImGui::TableSetupColumn("Sent / total", ImGuiTableColumnFlags_WidthFixed, 130);
				ImGui::TableHeadersRow();
				for (const auto& request : GetRequests()) {
					const bool failed = !request.error.empty() || (request.finished && request.state != "Completed");
					if (errorsOnly && !failed) continue;
					const auto searchable = request.command + " " + request.requestId + " " + request.state + " " + request.error;
					if (!filter.PassFilter(searchable.c_str())) continue;
					ImGui::TableNextRow();
					ImGui::TableNextColumn(); ImGui::Text("%llu", request.sequence);
					ImGui::TableNextColumn(); ImGui::Text("%s  %s", request.command.c_str(), request.requestId.c_str());
					ImGui::TableNextColumn();
					const ImVec4 color = failed ? ImVec4(1, 0.45f, 0.4f, 1) :
						request.finished ? ImVec4(0.5f, 0.85f, 0.6f, 1) : ImVec4(0.95f, 0.8f, 0.4f, 1);
					ImGui::TextColored(color, "%s", request.state.c_str());
					if (ImGui::IsItemHovered()) {
						ImGui::BeginTooltip();
						ImGui::PushTextWrapPos(ImGui::GetFontSize() * 38);
						ImGui::Text("Peer: %s\nFrame: %llu", request.peer.c_str(), request.frame);
						if (!request.error.empty()) ImGui::TextUnformatted(request.error.c_str());
						ImGui::PopTextWrapPos();
						ImGui::EndTooltip();
					}
					ImGui::TableNextColumn(); ImGui::Text("%.2f", request.processingMs);
					ImGui::TableNextColumn(); ImGui::Text("%.2f", request.elapsedMs);
					ImGui::TableNextColumn(); ImGui::Text("%zu / %zu B", request.sentBytes, request.responseBytes);
				}
				ImGui::EndTable();
			}
			ImGui::End();
		}

		void Init()
		{
			Renderer::GetFrameSubmittedDelegate() += [] {
				UpdateSingleStepState();
				Poll();
			};
			Renderer::GetCleanupDelegate() += Stop;
			if (enabled.get()) Start(static_cast<uint16_t>(std::clamp(configuredPort.get(), 1, 65535)));
		}
		MenuRegisterer menu("Debug Socket", ShowMenu);
		StartupRegisterer startup(Init);
	}

	void Stop()
	{
		for (auto& client : clients) {
			Finish(client, "Stopped");
			closesocket(client.socket);
		}
		clients.clear();
		if (listener != INVALID_SOCKET) closesocket(listener);
		listener = INVALID_SOCKET;
		boundPort = 0;
		if (winsockStarted) WSACleanup();
		winsockStarted = false;
	}

	bool Start(uint16_t port)
	{
		Stop();
		lastError.clear();
		WSADATA data;
		int error = WSAStartup(MAKEWORD(2, 2), &data);
		if (!error) {
			winsockStarted = true;
			listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			BOOL exclusive = TRUE;
			sockaddr_in address{};
			address.sin_family = AF_INET;
			address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
			address.sin_port = htons(port);
			if (listener != INVALID_SOCKET &&
				setsockopt(listener, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, reinterpret_cast<const char*>(&exclusive), sizeof(exclusive)) == 0 &&
				bind(listener, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == 0 &&
				listen(listener, 4) == 0 && Nonblocking(listener)) {
				int size = sizeof(address);
				if (getsockname(listener, reinterpret_cast<sockaddr*>(&address), &size) == 0) {
					boundPort = ntohs(address.sin_port);
					return true;
				}
			}
			error = WSAGetLastError();
		}
		lastError = "Could not start debug socket (Winsock " + std::to_string(error) + ")";
		Stop();
		return false;
	}

	uint16_t GetPort() { return boundPort; }

	std::vector<RequestInfo> GetRequests()
	{
		std::vector<RequestInfo> requests(history.begin(), history.end());
		for (const auto& client : clients) {
			auto info = client.info;
			info.elapsedMs = std::chrono::duration<double, std::milli>(Clock::now() - client.connected).count();
			requests.push_back(std::move(info));
		}
		std::sort(requests.begin(), requests.end(), [](const RequestInfo& a, const RequestInfo& b) { return a.sequence > b.sequence; });
		return requests;
	}

	void ClearHistory() { history.clear(); }

	void Poll()
	{
		++frame;
		if (listener == INVALID_SOCKET) return;
		// Bound all socket work per frame; a slow client never blocks the game.
		for (int i = 0; i < 4; ++i) {
			sockaddr_in peer{};
			int peerSize = sizeof(peer);
			const auto socket = accept(listener, reinterpret_cast<sockaddr*>(&peer), &peerSize);
			if (socket == INVALID_SOCKET) break;
			++connections;
			Client client{socket};
			client.info.sequence = nextSequence++;
			client.info.frame = frame;
			client.info.command = "(waiting)";
			client.info.state = "Receiving";
			client.info.peer = "127.0.0.1:" + std::to_string(ntohs(peer.sin_port));
			if (clients.size() == 4 || !Nonblocking(socket)) {
				Finish(client, "Rejected");
				closesocket(socket);
			}
			else clients.push_back(std::move(client));
		}
		bool captured = false;
		for (auto it = clients.begin(); it != clients.end();) {
			auto& client = *it;
			bool close = Clock::now() - client.activity > std::chrono::seconds(15);
			const char* closeReason = close ? "Timed out" : nullptr;
			if (!close && client.output.empty() && client.input.find('\n') == std::string::npos) {
				char buffer[8192];
				const int count = recv(client.socket, buffer, sizeof(buffer), 0);
				if (count > 0) {
					client.activity = Clock::now();
					client.input.append(buffer, count);
				}
				else if (count == 0 || WSAGetLastError() != WSAEWOULDBLOCK) {
					close = true;
					closeReason = "Disconnected";
				}
				if (client.input.size() > maxRequest) {
					client.info.command = "(oversized request)";
					Reply(client, {{"ok", false}, {"error", "Request exceeds 8192 bytes"}});
				}
				else if (client.input.find('\n') != std::string::npos) client.info.state = "Queued";
			}
			if (!close && client.output.empty() && client.input.find('\n') != std::string::npos && !captured) {
				client.input.resize(client.input.find('\n'));
				Handle(client);
				captured = true; // At most one command (and GPU readback) per frame.
			}
			if (!close && !client.output.empty()) {
				const int count = send(client.socket, client.output.data() + client.sent,
					static_cast<int>(std::min<size_t>(256 * 1024, client.output.size() - client.sent)), 0);
				if (count > 0) {
					client.sent += count;
					client.info.sentBytes = client.sent;
					bytesSent += count;
					client.activity = Clock::now();
					close = client.sent == client.output.size();
				}
				else if (count == 0 || WSAGetLastError() != WSAEWOULDBLOCK) {
					close = true;
					closeReason = "Disconnected";
				}
			}
			if (close) {
				Finish(client, closeReason);
				closesocket(client.socket);
				it = clients.erase(it);
			}
			else ++it;
		}
	}
}
