#include "DebugDrawInspector.h"
#include "DebugMenu.h"
#include "DebugCallstackPreviewer.h"
#include "DebugMeshViewer.h"
#include "DebugTexture.h"
#include "DrawTrace.h"
#include "imgui.h"
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

namespace Debug::DrawInspector {
namespace {
namespace Trace = Renderer::DrawTrace;
bool open = false, collectStacks = false, includeUnrecorded = false;
int typeFilter = 0, passFilter = 0, matchMode = 0;
ImGuiTextFilter meshFilter, textureFilter, ownerFilter;
uint64_t selectedSource = 0, selectedSubmission = 0, selectedDraw = 0, snapshotFrame = 0;
std::unordered_map<uint32_t, CallstackPreviewerEntry> stackViews;
std::string viewerMessage;
struct Row {
    const Trace::Source* source;
    const Trace::Submission* submission;
    const Trace::Draw* draw;
};
const char* KindName(Trace::Kind kind) { return kind == Trace::Kind::Strip ? "Strip" : "Sprite"; }
const char* PassName(int pass) {
    switch (pass) { case 0: return "Main"; case 1: return "Shadow mask"; case 2: return "Shadow receiver"; default: return "Unknown"; }
}
const char* Stage(const Row& row) {
    if (row.draw) {
        if (row.draw->suppressed) return "Suppressed by override";
        if (row.draw->recorded) return "Recorded";
        if (!row.draw->texture[0]) return "No texture binding";
        if (!row.draw->indexCount) return "Zero indices";
        return "Not recorded";
    }
    if (row.submission) return "Submitted; no recorded draw observed";
    return row.source->flushed ? "Flushed; no native submission observed" : "Linked; no flush observed";
}
const char* MeshName(const Row& row) { return row.submission ? row.submission->mesh.data() : ""; }
const char* TextureName(const Row& row) { return row.draw ? row.draw->texture.data() : ""; }
void Select(const Row& row) {
    viewerMessage.clear();
    selectedSource = row.source->id;
    selectedSubmission = row.submission ? row.submission->id : 0;
    selectedDraw = row.draw ? row.draw->id : 0;
}
bool Selected(const Row& row) {
    return selectedSource == row.source->id && selectedSubmission == (row.submission ? row.submission->id : 0) &&
        selectedDraw == (row.draw ? row.draw->id : 0);
}
void Matrix(const char* label, const Trace::Matrix& matrix) {
    if (ImGui::TreeNode(label)) {
        for (int i = 0; i < 4; ++i) ImGui::Text("% .5f  % .5f  % .5f  % .5f", matrix[i * 4], matrix[i * 4 + 1], matrix[i * 4 + 2], matrix[i * 4 + 3]);
        ImGui::TreePop();
    }
}
void Address(const char* label, Trace::Address address) {
    ImGui::Text("%s: 0x%llX", label, static_cast<unsigned long long>(address));
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Captured address only; never dereferenced. Right-click to copy.");
    ImGui::PushID(label);
    if (ImGui::BeginPopupContextItem("Copy address")) {
        if (ImGui::MenuItem("Copy address")) {
            char buffer[32]; std::snprintf(buffer, sizeof(buffer), "0x%llX", static_cast<unsigned long long>(address));
            ImGui::SetClipboardText(buffer);
        }
        ImGui::EndPopup();
    }
    ImGui::PopID();
}
void ShowStack(const Trace::Snapshot& snapshot, const char* label, uint32_t index) {
    ImGui::PushID(label);
    ImGui::SeparatorText(label);
    if (index && index <= snapshot.stacks.size()) {
        auto& entry = stackViews[index];
        if (entry.backtrace.empty()) {
            const auto& stack = snapshot.stacks[index - 1];
            entry.backtrace.assign(stack.addresses.begin(), stack.addresses.begin() + stack.count);
        }
        CallstackPreviewer::Show(entry);
    }
    else ImGui::TextDisabled(snapshot.stacksEnabled ? "No stack available." : "Enable Collect call stacks before capturing.");
    ImGui::PopID();
}
void CopyDetails(const Trace::Snapshot& snapshot, const Row& row) {
    const auto& source = *row.source;
    std::ostringstream text;
    text << "Frame " << snapshot.frame << " source " << source.id << " " << KindName(source.kind)
        << "\nMesh: " << MeshName(row) << "\nTexture: " << TextureName(row) << "\nStage: " << Stage(row)
        << "\nScene: " << source.sceneName.data() << "\nOwner: " << source.ownerName.data() << " index " << source.ownerIndex << " type " << source.ownerType
        << " sector " << source.sector << "\nSource 0x" << std::hex << source.object << " node 0x" << source.node
        << " hierarchy 0x" << source.hierarchy << " viewport 0x" << source.viewport << " list 0x" << source.list
        << " bank 0x" << source.bank << "\nFlags 0x" << source.flags << " node flags 0x" << source.nodeFlags << std::dec
        << "\nMaterial " << source.material << "\nBounds ";
    for (auto value : source.bounds) text << value << ' ';
    if (row.submission) {
        const auto& s = *row.submission;
        text << "\nSubmission " << s.id << " layer " << s.layer << " render flags 0x" << std::hex << s.flags
            << " primitive 0x" << s.primitive << std::dec << "\nModel matrix ";
        for (auto value : s.model) text << value << ' ';
    }
    if (row.draw) {
        const auto& d = *row.draw;
        text << "\nDraw " << d.id << " pass " << PassName(d.pass) << " material/layer " << d.material << '/' << d.layer
            << " indices " << d.indexStart << '+' << d.indexCount << " vertex offset " << d.vertexStart
            << "\nBlend " << d.blend << " depth write " << d.depthWrite << " color write " << d.colorWrite
            << "\nALPHA 0x" << std::hex << d.alpha << " TEST 0x" << d.test << " TEX 0x" << d.tex << " CLAMP 0x" << d.clamp;
    }
    ImGui::SetClipboardText(text.str().c_str());
}
void Details(const Trace::Snapshot& snapshot, const Row& row, const std::vector<Row>& rows) {
    const auto& source = *row.source;
    ImGui::TextWrapped("%s", MeshName(row)[0] ? MeshName(row) : "Source without a native mesh");
    ImGui::TextWrapped("%s", TextureName(row));
    ImGui::Text("Source #%llu", source.id);
    ImGui::TextWrapped("Linked -> %s", Stage(row));
    if (row.submission) ImGui::Text("Submission #%llu / requested layer %d", row.submission->id, row.submission->layer);
    if (row.draw) ImGui::Text("Draw #%llu / %s", row.draw->id, PassName(row.draw->pass));
    if (ImGui::Button("Copy details")) CopyDetails(snapshot, row);
    const bool live = Trace::IsSourceLive(source);
    ImGui::BeginDisabled(!live || !row.submission || !row.draw || source.kind != Trace::Kind::Strip);
    if (ImGui::Button("Open mesh viewer")) {
        viewerMessage = DebugMeshViewer::OpenDraw(source, *row.submission, *row.draw) ? "" :
            "Mesh preview unavailable: the draw's texture is missing, ambiguous, or a framebuffer capture.";
    }
    ImGui::EndDisabled();
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
        ImGui::SetTooltip("Open the selected strip and texture layer in Mesh Previewer. Sprite meshes are not supported by this viewer.");
    ImGui::SameLine();
    ImGui::BeginDisabled(!live || !row.draw || !row.draw->texture[0] || row.draw->framebuffer);
    if (ImGui::Button("Open texture viewer")) {
        viewerMessage = Debug::Texture::OpenDraw(source, *row.draw) ? "" :
            "Texture preview unavailable: no unique matching texture is currently loaded.";
    }
    ImGui::EndDisabled();
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
        ImGui::SetTooltip("Open the bound material, layer and texture in the texture viewer. Framebuffer captures are not texture-library assets.");
    if (!viewerMessage.empty()) ImGui::TextWrapped("%s", viewerMessage.c_str());
    ImGui::Separator();
    ImGui::SetNextItemWidth(190);
    ImGui::Combo("Match", &matchMode, "Source instance\0All uses of asset\0");
    ImGui::BeginDisabled(!live || (matchMode == 1 && !row.submission));
    const auto match = matchMode == 0 ? Trace::Match::Source : Trace::Match::Asset;
    if (ImGui::Button("Hide")) Trace::SetOverride(source, row.submission, Trace::Action::Hide, match);
    ImGui::SameLine();
    if (ImGui::Button("Isolate")) Trace::SetOverride(source, row.submission, Trace::Action::Isolate, match);
    ImGui::SameLine();
    if (ImGui::Button("Highlight bounds")) Trace::SetOverride(source, row.submission, Trace::Action::Highlight, match);
    ImGui::EndDisabled();
    if (!live) ImGui::TextDisabled("Source unavailable. Capture again to refresh.");
    ImGui::TextWrapped("Source matching includes every layer, pass and viewport of this object/hierarchy in its source scene. Repeated submissions with that identity are grouped; the live match count shows native draw instances.");
    ImGui::TextDisabled("Bounds are an approximation, not a skinned silhouette.");
    if (ImGui::BeginTabBar("DrawDetails")) {
        if (ImGui::BeginTabItem("Source")) {
            ImGui::Text("%s / material %d / render list %d", KindName(source.kind), source.material, source.renderList);
            ImGui::Text("Owner: %s", source.ownerName[0] ? source.ownerName.data() : "Unknown");
            ImGui::Text("Actor index/type: %d / %d   Sector: %d", source.ownerIndex, source.ownerType, source.sector);
            ImGui::Text("Scene: %s", source.sceneName[0] ? source.sceneName.data() : "Unknown");
            Address("Source scene", source.scene);
            Address("Owner", source.owner); Address("Source", source.object); Address("PS2 list node", source.node);
            Address("Hierarchy", source.hierarchy); Address("Viewport", source.viewport); Address("List", source.list); Address("Material bank", source.bank);
            ImGui::Text("Source flags: 0x%08X   Linked node flags: 0x%08X", source.flags, source.nodeFlags);
            if (source.kind == Trace::Kind::Strip) {
                ImGui::Text("Meshes: %d   Shadow cast/receive: 0x%X / 0x%X", source.meshCount, source.shadowCast, source.shadowReceive);
                if (source.flags & 0x10000) ImGui::TextUnformatted("Animated strip (0x10000)");
            }
            else ImGui::Text("Batches: %d   Remainder rectangles: %d", source.batches, source.remainder);
            ImGui::Text("Local bounds: %.3f %.3f %.3f / radius %.3f", source.bounds[0], source.bounds[1], source.bounds[2], source.bounds[3]);
            if (ImGui::TreeNode("Related draws from this source instance")) {
                for (const auto& related : rows) {
                    if (related.source->object != source.object || related.source->hierarchy != source.hierarchy ||
                        related.source->owner != source.owner || related.source->scene != source.scene ||
                        related.source->material != source.material) continue;
                    ImGui::PushID(static_cast<int>(&related - rows.data()));
                    char label[200];
                    std::snprintf(label, sizeof(label), "Source %llu / submission %llu / draw %llu / %s", related.source->id,
                        related.submission ? related.submission->id : 0, related.draw ? related.draw->id : 0, Stage(related));
                    if (ImGui::Selectable(label, Selected(related))) Select(related);
                    ImGui::PopID();
                }
                ImGui::TreePop();
            }
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Material/state")) {
            if (row.submission) {
                const auto& s = *row.submission;
                ImGui::Text("Render flags: 0x%08X   PRIM: 0x%08X", s.flags, s.primitive);
                ImGui::Text("Global alpha: %u   Override flag (0x20): %s", s.globalAlpha, s.flags & 0x20 ? "set" : "clear");
                ImGui::Text("Model/animation/lighting/anim-ST indices: %u / %u / %u / %u", s.modelIndex, s.animationIndex, s.lightingIndex, s.animStIndex);
            }
            if (row.draw) {
                const auto& d = *row.draw;
                ImGui::Text("Bound material/layer: %d / %d", d.material, d.layer);
                ImGui::Text("Index start/count: %d / %d   Vertex offset: %d", d.indexStart, d.indexCount, d.vertexStart);
                ImGui::Text("Blend: %s   Depth write: %s", d.blend ? "on" : "off", d.depthWrite ? "on" : "off");
                ImGui::Text("Depth compare: %s", d.depthGreaterEqual ? "Greater or equal" : "Greater");
                ImGui::Text("Color write: %s   RGBA mask: 0x%X", d.colorWrite ? "on" : "off", d.colorMask);
                ImGui::Text("Alpha test: %s   ATST/AREF/AFAIL: %u / %u / %u", d.alphaTest ? "on" : "off", d.alphaAtst, d.alphaAref, d.alphaAfail);
                ImGui::Text("Framebuffer material: %s   Depth-only copy: %s", d.framebuffer ? "yes" : "no", d.zOnly ? "yes" : "no");
                ImGui::Text("Effective ALPHA: 0x%016llX", d.alpha);
                ImGui::Text("Raw TEST: 0x%016llX", d.test); ImGui::Text("Raw TEX: 0x%016llX", d.tex); ImGui::Text("Raw CLAMP: 0x%016llX", d.clamp);
            }
            else ImGui::TextDisabled("No native draw state was observed.");
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Transforms")) {
            if (source.hasModel) Matrix("Model at linking", source.model);
            if (row.submission) Matrix("Submitted model", row.submission->model);
            if (row.draw) { Matrix("View", row.draw->view); Matrix("Projection", row.draw->projection); }
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Stacks")) {
            ShowStack(snapshot, "Viewport linking", source.linkStack);
            ShowStack(snapshot, "Flushing", source.flushStack);
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
}

void DrawWindow() {
    const auto status = Trace::GetStatus();
    if (status.action != Trace::Action::None) {
        ImGui::Begin("Draw overrides", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Text("Overrides active: %s / %u matching draws", status.action == Trace::Action::Hide ? "Hide" : status.action == Trace::Action::Isolate ? "Isolate" : "Highlight bounds", status.matches);
        if (ImGui::Button("Open inspector")) open = true;
        ImGui::SameLine(); if (ImGui::Button("Clear overrides")) Trace::ClearOverrides();
        ImGui::End();
    }
    if (!open) return;
    ImGui::SetNextWindowSize(ImVec2(1150, 700), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Draw Inspector", &open)) { ImGui::End(); return; }
    ImGui::BeginDisabled(status.pending || status.capturing);
    if (ImGui::Button("Capture next frame")) Trace::RequestCapture(collectStacks);
    ImGui::SameLine(); ImGui::Checkbox("Collect call stacks", &collectStacks);
    ImGui::EndDisabled();
    ImGui::SameLine(); if (ImGui::Button("Clear snapshot")) Trace::ClearSnapshot();
    ImGui::SameLine(); if (ImGui::Button("Clear overrides")) Trace::ClearOverrides();
    if (status.pending || status.capturing) ImGui::TextUnformatted(status.capturing ? "Capturing; waiting for frame recording to finish..." : "Capture armed for the next complete frame...");
    auto snapshot = Trace::GetSnapshot();
    if (!snapshot) {
        ImGui::TextWrapped("Capture a frame to inspect strips and sprites. Search the mesh or texture names from renderer debug labels. This is a game-frame snapshot, independent of external captures.");
        ImGui::End(); return;
    }
    if (snapshotFrame != snapshot->frame) {
        snapshotFrame = snapshot->frame; selectedSource = selectedSubmission = selectedDraw = 0; stackViews.clear();
    }
    ImGui::Text("Frame %llu: %zu sources / %zu submissions / %zu draw attempts", snapshot->frame, snapshot->sources.size(), snapshot->submissions.size(), snapshot->draws.size());
    if (snapshot->truncated) ImGui::TextColored(ImVec4(1, 0.65f, 0.15f, 1), "Capture truncated at the 64 MiB collection budget. Missing rows may be incomplete.");
    meshFilter.Draw("Mesh", 240); ImGui::SameLine(); textureFilter.Draw("Texture", 280);
    ownerFilter.Draw("Owner", 240); ImGui::SameLine(); ImGui::SetNextItemWidth(100); ImGui::Combo("Type", &typeFilter, "All\0Strip\0Sprite\0");
    ImGui::SameLine(); ImGui::SetNextItemWidth(145); ImGui::Combo("Pass", &passFilter, "All\0Main\0Shadow mask\0Shadow receiver\0");
    ImGui::Checkbox("Include linked/submitted sources without recorded draws", &includeUnrecorded);

    std::unordered_map<uint64_t, const Trace::Source*> sources;
    std::unordered_map<uint64_t, const Trace::Submission*> submissions;
    std::unordered_set<uint64_t> drawn, submitted;
    std::vector<Row> rows;
    for (const auto& source : snapshot->sources) sources[source.id] = &source;
    for (const auto& submission : snapshot->submissions) { submissions[submission.id] = &submission; submitted.insert(submission.sourceId); }
    for (const auto& draw : snapshot->draws) {
        auto s = submissions.find(draw.submissionId);
        if (s == submissions.end()) continue;
        auto source = sources.find(s->second->sourceId);
        if (source == sources.end()) continue;
        rows.push_back({source->second, s->second, &draw}); drawn.insert(draw.submissionId);
    }
    for (const auto& submission : snapshot->submissions) if (!drawn.count(submission.id) && sources.count(submission.sourceId)) rows.push_back({sources[submission.sourceId], &submission, nullptr});
    for (const auto& source : snapshot->sources) if (!submitted.count(source.id)) rows.push_back({&source, nullptr, nullptr});
    std::vector<size_t> visible;
    for (size_t i = 0; i < rows.size(); ++i) {
        const auto& row = rows[i];
        if (!includeUnrecorded && (!row.draw || (!row.draw->recorded && !row.draw->suppressed))) continue;
        if (typeFilter && static_cast<int>(row.source->kind) != typeFilter - 1) continue;
        if (passFilter && (!row.draw || row.draw->pass != passFilter - 1)) continue;
        if (!meshFilter.PassFilter(MeshName(row)) || !textureFilter.PassFilter(TextureName(row)) || !ownerFilter.PassFilter(row.source->ownerName.data())) continue;
        visible.push_back(i);
    }
    ImGui::Text("%zu matching rows", visible.size());
    if (!selectedSource && !visible.empty()) Select(rows[visible.front()]);
    const float listWidth = std::max(350.0f, ImGui::GetContentRegionAvail().x * 0.52f);
    ImGui::BeginChild("Draw list", ImVec2(listWidth, 0), true);
    if (ImGui::BeginTable("Draws", 9, ImGuiTableFlags_Resizable | ImGuiTableFlags_Sortable | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY | ImGuiTableFlags_ScrollX)) {
        ImGui::TableSetupColumn("Order", ImGuiTableColumnFlags_DefaultSort); ImGui::TableSetupColumn("Type");
        ImGui::TableSetupColumn("Mesh"); ImGui::TableSetupColumn("Texture"); ImGui::TableSetupColumn("m / l");
        ImGui::TableSetupColumn("Pass"); ImGui::TableSetupColumn("Owner");
        ImGui::TableSetupColumn("Indices"); ImGui::TableSetupColumn("State");
        ImGui::TableSetupScrollFreeze(0, 1); ImGui::TableHeadersRow();
        if (auto* sort = ImGui::TableGetSortSpecs(); sort && sort->SpecsCount) {
            const auto spec = sort->Specs[0];
            std::stable_sort(visible.begin(), visible.end(), [&](size_t a, size_t b) {
                const auto& x = rows[a]; const auto& y = rows[b]; int comparison = 0;
                switch (spec.ColumnIndex) {
                case 1: comparison = static_cast<int>(x.source->kind) - static_cast<int>(y.source->kind); break;
                case 2: comparison = std::strcmp(MeshName(x), MeshName(y)); break;
                case 3: comparison = std::strcmp(TextureName(x), TextureName(y)); break;
                case 4:
                    comparison = (x.draw ? x.draw->material : x.source->material) - (y.draw ? y.draw->material : y.source->material);
                    if (!comparison) comparison = (x.draw ? x.draw->layer : x.submission ? x.submission->layer : -1) - (y.draw ? y.draw->layer : y.submission ? y.submission->layer : -1);
                    break;
                case 5: comparison = (x.draw ? x.draw->pass : -1) - (y.draw ? y.draw->pass : -1); break;
                case 6: comparison = std::strcmp(x.source->ownerName.data(), y.source->ownerName.data()); break;
                case 7: comparison = (x.draw ? x.draw->indexCount : 0) - (y.draw ? y.draw->indexCount : 0); break;
                case 8: comparison = std::strcmp(Stage(x), Stage(y)); break;
                default: comparison = (a > b) - (a < b); break;
                }
                if (!comparison) comparison = (a > b) - (a < b);
                return spec.SortDirection == ImGuiSortDirection_Ascending ? comparison < 0 : comparison > 0;
            });
            sort->SpecsDirty = false;
        }
        ImGuiListClipper clipper; clipper.Begin(static_cast<int>(visible.size()));
        while (clipper.Step()) for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i) {
            const auto& row = rows[visible[i]];
            ImGui::PushID(static_cast<int>(visible[i])); ImGui::TableNextRow(); ImGui::TableNextColumn();
            char label[48]; std::snprintf(label, sizeof(label), "%zu%s", visible[i] + 1, row.draw && row.draw->suppressed ? " (hidden)" : "");
            if (ImGui::Selectable(label, Selected(row), ImGuiSelectableFlags_SpanAllColumns)) Select(row);
            ImGui::TableNextColumn(); ImGui::TextUnformatted(KindName(row.source->kind));
            ImGui::TableNextColumn(); ImGui::TextUnformatted(MeshName(row));
            ImGui::TableNextColumn(); ImGui::TextUnformatted(TextureName(row));
            ImGui::TableNextColumn(); ImGui::Text("%d / %d", row.draw ? row.draw->material : row.source->material, row.draw ? row.draw->layer : row.submission ? row.submission->layer : -1);
            ImGui::TableNextColumn(); ImGui::TextUnformatted(row.draw ? PassName(row.draw->pass) : "--");
            ImGui::TableNextColumn(); ImGui::TextUnformatted(row.source->ownerName[0] ? row.source->ownerName.data() : "Unknown");
            ImGui::TableNextColumn(); if (row.draw) ImGui::Text("%d", row.draw->indexCount); else ImGui::TextUnformatted("--");
            ImGui::TableNextColumn(); ImGui::TextUnformatted(Stage(row));
            ImGui::PopID();
        }
        ImGui::EndTable();
    }
    ImGui::EndChild(); ImGui::SameLine(); ImGui::BeginChild("Draw details", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);
    auto selection = std::find_if(rows.begin(), rows.end(), Selected);
    if (selection != rows.end()) Details(*snapshot, *selection, rows);
    else ImGui::TextWrapped("Select a draw to trace its source and inspect the state used by the native renderer.");
    ImGui::EndChild(); ImGui::End();
}
Debug::UpdateRegisterer update(DrawWindow);
}
void ShowMenu(bool* pOpen) {
    open = *pOpen;
    DrawWindow();
    *pOpen = open;
}
void DrawLauncher() {
    if (ImGui::Button("Draw Inspector")) open = true;
    if (Trace::GetStatus().action != Trace::Action::None) {
        ImGui::SameLine(); ImGui::TextUnformatted("Draw overrides active");
    }
}
}
