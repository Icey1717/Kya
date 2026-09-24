#pragma once

// CPU-only bridge: no game layouts, Vulkan objects or ImGui dependencies.
#include <array>
#include <cstdint>
#include <memory>
#include <vector>

namespace Renderer::DrawTrace {
using Address = uintptr_t;
using Name = std::array<char, 256>;
using Matrix = std::array<float, 16>;
enum class Kind { Strip, Sprite };
enum class Action { None, Hide, Isolate, Highlight };
enum class Match { Source, Asset };

struct Source {
    uint64_t id = 0, generation = 0;
    Kind kind = Kind::Strip;
    Address node = 0, object = 0, hierarchy = 0, viewport = 0, scene = 0, list = 0, bank = 0, owner = 0;
    int material = -1, ownerIndex = -1, ownerType = -1, sector = -1, renderList = -1;
    uint32_t flags = 0, nodeFlags = 0;
    int meshCount = 0, batches = 0, remainder = 0, shadowCast = 0, shadowReceive = 0;
    Name ownerName{};
    Name sceneName{};
    Matrix model{};
    std::array<float, 4> bounds{};
    bool hasModel = false, flushed = false;
    uint32_t linkStack = 0, flushStack = 0;
};
struct Submission {
    uint64_t id = 0, sourceId = 0;
    uint64_t assetKey = 0;
    Name mesh{};
    int layer = 0;
    uint32_t flags = 0, primitive = 0, modelIndex = 0, animationIndex = 0, lightingIndex = 0, animStIndex = 0;
    uint32_t globalAlpha = 0;
    Matrix model{};
};
struct Draw {
    uint64_t id = 0, submissionId = 0;
    Name texture{};
    int material = -1, layer = -1, pass = 0;
    int indexStart = 0, indexCount = 0, vertexStart = 0;
    uint64_t alpha = 0, test = 0, tex = 0, clamp = 0;
    uint32_t alphaAtst = 0, alphaAref = 0, alphaAfail = 0, colorMask = 15;
    bool alphaTest = false, blend = false, depthWrite = false, colorWrite = true;
    bool depthGreaterEqual = false, framebuffer = false, zOnly = false, suppressed = false, recorded = false;
    Matrix view{}, projection{};
};
struct Stack { std::array<uint64_t, 64> addresses{}; uint32_t count = 0; };
struct Snapshot {
    uint64_t frame = 0, generation = 0;
    bool truncated = false, stacksEnabled = false;
    std::vector<Source> sources;
    std::vector<Submission> submissions;
    std::vector<Draw> draws;
    std::vector<Stack> stacks;
};
struct Status {
    bool pending = false, capturing = false;
    Action action = Action::None;
    uint32_t matches = 0;
    uint64_t generation = 0;
};
void CopyName(Name& target, const char* name);
uint64_t AssetKey(const char* name);
bool IsEnabled();
void RequestCapture(bool stacks);
void ClearSnapshot();
std::shared_ptr<const Snapshot> GetSnapshot();
Status GetStatus();
// Called on the producer thread after the render thread has finished the frame.
void AdvanceFrame();
void InvalidateSources();
void InvalidateSource(Address address);
void Link(Source source);
void RegisterOwner(Address hierarchy, Address owner, const char* name, int index, int type, int sector);

class OwnerScope {
public:
    OwnerScope(Address owner, const char* name, int index, int type, int sector);
    ~OwnerScope();
private:
    Source previous;
};
class BankScope {
public:
    explicit BankScope(Address bank);
    ~BankScope();
private:
    Address previous;
};
class SceneScope {
public:
    SceneScope(Address scene, const char* name, Address viewport);
    ~SceneScope();
private:
    Address previousScene, previousViewport;
    Name previousName;
};
class FlushScope {
public:
    explicit FlushScope(Address node, int layer = 0);
    ~FlushScope();
private:
    uint64_t previous;
    int previousLayer;
};
uint64_t Submit(Submission submission);
// Returns whether this draw should be emitted. Recorded copies get distinct IDs.
bool Record(uint64_t submissionId, Draw draw);
bool Highlight(uint64_t submissionId, Source& source);
void SetOverride(const Source& source, const Submission* submission, Action action, Match match);
void ClearOverrides();
bool IsSourceLive(const Source& source);
}
