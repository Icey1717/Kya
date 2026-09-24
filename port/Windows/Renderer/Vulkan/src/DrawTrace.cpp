#include "DrawTrace.h"
#include <algorithm>
#include <atomic>
#include <cstring>
#include <mutex>
#include <unordered_map>
#ifdef _WIN32
#include <windows.h>
#endif

namespace Renderer::DrawTrace {
namespace {
constexpr size_t maxBytes = 64 * 1024 * 1024;
std::mutex mutex;
std::atomic<bool> enabled{false};
Snapshot working;
std::shared_ptr<const Snapshot> published;
bool pending = false, pendingStacks = false, capturing = false;
uint64_t frame = 0, generation = 1, nextSource = 0, nextSubmission = 0, nextDraw = 0;
size_t bytes = 0;
std::unordered_map<Address, size_t> nodes;
std::unordered_map<uint64_t, size_t> sourceIndices, submissionIndices;
std::unordered_map<Address, Source> owners;
std::vector<Source> liveSources;
// Tombstones distinguish a freed address from a later allocation at the same address.
std::unordered_map<Address, uint64_t> retiredAt;
thread_local Source currentOwner;
thread_local Address currentBank = 0;
thread_local Address currentScene = 0, currentViewport = 0;
thread_local Name currentSceneName{};
thread_local uint64_t currentSource = 0;
thread_local int currentLayer = 0;
struct Override { Source source; uint64_t assetKey = 0; Action action = Action::None; Match match = Match::Source; };
Override requested, active;
uint32_t matches = 0, lastMatches = 0;

bool Reserve(size_t size) {
    // Conservative charge covers vector growth, the live identity copy and map nodes/buckets.
    const size_t charge = size * 4 + 512;
    if (charge > maxBytes - bytes) { working.truncated = true; return false; }
    bytes += charge;
    return true;
}
bool SameSource(const Source& a, const Source& b) {
    return a.generation == b.generation && a.kind == b.kind && a.object == b.object &&
        a.hierarchy == b.hierarchy && a.owner == b.owner && a.scene == b.scene && a.material == b.material;
}
bool UsesAddress(const Source& source, Address address) {
    return address && (source.object == address || source.hierarchy == address || source.owner == address || source.scene == address);
}
bool Retired(const Source& source) {
    for (const auto address : {source.object, source.hierarchy, source.owner, source.scene}) {
        auto found = retiredAt.find(address);
        if (found != retiredAt.end() && source.id <= found->second) return true;
    }
    return false;
}
bool Matches(const Source& source, const Submission& submission) {
    if (active.match == Match::Source) return SameSource(source, active.source);
    return source.generation == active.source.generation && source.kind == active.source.kind &&
        submission.assetKey == active.assetKey;
}
Source* FindSource(uint64_t id) {
    auto it = sourceIndices.find(id);
    return it == sourceIndices.end() ? nullptr : &working.sources[it->second];
}
uint32_t CollectStack() {
    if (!capturing || !working.stacksEnabled) return 0;
    Stack stack;
#ifdef _WIN32
    void* addresses[64];
    stack.count = CaptureStackBackTrace(2, 64, addresses, nullptr);
    for (uint32_t i = 0; i < stack.count; ++i) stack.addresses[i] = reinterpret_cast<uintptr_t>(addresses[i]);
#endif
    if (!stack.count) return 0;
    for (size_t i = 0; i < working.stacks.size(); ++i)
        if (working.stacks[i].addresses == stack.addresses) return static_cast<uint32_t>(i + 1);
    if (!Reserve(sizeof(Stack))) return 0;
    working.stacks.push_back(stack);
    return static_cast<uint32_t>(working.stacks.size());
}
}

void CopyName(Name& target, const char* name) {
    target.fill(0);
    if (name) {
        size_t length = 0;
        while (length + 1 < target.size() && name[length]) ++length;
        std::memcpy(target.data(), name, length);
    }
}
bool IsEnabled() { return enabled.load(std::memory_order_relaxed); }
uint64_t AssetKey(const char* name) {
    uint64_t hash = 14695981039346656037ull;
    if (name) while (*name) { hash ^= static_cast<unsigned char>(*name++); hash *= 1099511628211ull; }
    return hash;
}
void RequestCapture(bool stacks) { std::lock_guard lock(mutex); pending = true; pendingStacks = stacks; }
void ClearSnapshot() { std::lock_guard lock(mutex); published.reset(); }
std::shared_ptr<const Snapshot> GetSnapshot() { std::lock_guard lock(mutex); return published; }
Status GetStatus() { std::lock_guard lock(mutex); return {pending, capturing, requested.action, lastMatches, generation}; }

void AdvanceFrame() {
    std::lock_guard lock(mutex);
    lastMatches = matches;
    matches = 0;
    // Store lightweight identities for safe UI actions, not live game pointers.
    if (capturing || active.action != Action::None) {
        liveSources.clear();
        for (const auto& source : working.sources) liveSources.push_back(source);
    }
    if (capturing) published = std::make_shared<Snapshot>(std::move(working));
    working = {};
    nodes.clear(); sourceIndices.clear(); submissionIndices.clear(); owners.clear();
    bytes = 0;
    capturing = pending;
    pending = false;
    working.stacksEnabled = capturing && pendingStacks;
    working.frame = ++frame;
    working.generation = generation;
    active = requested;
    enabled.store(capturing || active.action != Action::None, std::memory_order_relaxed);
}

void InvalidateSources() {
    std::lock_guard lock(mutex);
    ++generation;
    requested = {}; active = {};
    liveSources.clear(); owners.clear(); nodes.clear(); retiredAt.clear();
    enabled.store(capturing, std::memory_order_relaxed);
}

void InvalidateSource(Address address) {
    if (!address) return;
    std::lock_guard lock(mutex);
    // Keep lifetime bookkeeping bounded independently of per-frame capture storage.
    if (retiredAt.size() >= 65536 && !retiredAt.count(address)) {
        ++generation; retiredAt.clear(); liveSources.clear(); requested = {}; active = {};
    }
    retiredAt[address] = nextSource;
    if (UsesAddress(requested.source, address)) requested = {};
    if (UsesAddress(active.source, address)) active = {};
    liveSources.erase(std::remove_if(liveSources.begin(), liveSources.end(),
        [&](const Source& source) { return UsesAddress(source, address); }), liveSources.end());
    for (auto it = nodes.begin(); it != nodes.end();) {
        if (UsesAddress(working.sources[it->second], address)) it = nodes.erase(it); else ++it;
    }
    for (auto it = owners.begin(); it != owners.end();) {
        if (it->first == address || it->second.owner == address) it = owners.erase(it); else ++it;
    }
    enabled.store(capturing || active.action != Action::None, std::memory_order_relaxed);
}

void RegisterOwner(Address hierarchy, Address owner, const char* name, int index, int type, int sector) {
    if (!IsEnabled() || !hierarchy) return;
    std::lock_guard lock(mutex);
    if (!owners.count(hierarchy) && !Reserve(sizeof(Source))) return;
    auto& entry = owners[hierarchy];
    entry.owner = owner; entry.ownerIndex = index; entry.ownerType = type; entry.sector = sector;
    CopyName(entry.ownerName, name);
}
OwnerScope::OwnerScope(Address owner, const char* name, int index, int type, int sector) : previous(currentOwner) {
    currentOwner.owner = owner; currentOwner.ownerIndex = index; currentOwner.ownerType = type; currentOwner.sector = sector;
    CopyName(currentOwner.ownerName, name);
}
OwnerScope::~OwnerScope() { currentOwner = previous; }
BankScope::BankScope(Address bank) : previous(currentBank) { currentBank = bank; }
BankScope::~BankScope() { currentBank = previous; }
SceneScope::SceneScope(Address scene, const char* name, Address viewport)
    : previousScene(currentScene), previousViewport(currentViewport), previousName(currentSceneName) {
    currentScene = scene; currentViewport = viewport; CopyName(currentSceneName, name);
}
SceneScope::~SceneScope() {
    currentScene = previousScene; currentViewport = previousViewport; currentSceneName = previousName;
}

void Link(Source source) {
    if (!IsEnabled()) return;
    std::lock_guard lock(mutex);
    // A failed capture must not attribute a reused list node to its previous occupant.
    nodes.erase(source.node);
    if (!Reserve(sizeof(Source))) return;
    source.id = ++nextSource;
    source.generation = generation;
    source.bank = currentBank;
    source.scene = currentScene;
    source.sceneName = currentSceneName;
    if (currentViewport) source.viewport = currentViewport;
    const Source* owner = &currentOwner;
    if (!owner->owner) {
        auto it = owners.find(source.hierarchy);
        if (it != owners.end()) owner = &it->second;
    }
    source.owner = owner->owner; source.ownerIndex = owner->ownerIndex; source.ownerType = owner->ownerType;
    source.sector = owner->sector; source.ownerName = owner->ownerName;
    source.linkStack = CollectStack();
    nodes[source.node] = working.sources.size();
    sourceIndices[source.id] = working.sources.size();
    working.sources.push_back(source);
}

FlushScope::FlushScope(Address node, int layer) : previous(currentSource), previousLayer(currentLayer) {
    currentSource = 0; currentLayer = layer;
    if (!IsEnabled()) return;
    std::lock_guard lock(mutex);
    auto it = nodes.find(node);
    if (it == nodes.end()) return;
    auto& source = working.sources[it->second];
    currentSource = source.id;
    source.flushed = true;
    if (!source.flushStack) source.flushStack = CollectStack();
}
FlushScope::~FlushScope() { currentSource = previous; currentLayer = previousLayer; }

uint64_t Submit(Submission submission) {
    if (!IsEnabled() || !currentSource) return 0;
    std::lock_guard lock(mutex);
    if (!FindSource(currentSource) || !Reserve(sizeof(Submission))) return 0;
    submission.id = ++nextSubmission;
    submission.sourceId = currentSource;
    submission.layer = currentLayer;
    if (!submission.assetKey) submission.assetKey = AssetKey(submission.mesh.data());
    submissionIndices[submission.id] = working.submissions.size();
    working.submissions.push_back(submission);
    return submission.id;
}

bool Record(uint64_t submissionId, Draw draw) {
    if (!submissionId) return true;
    std::lock_guard lock(mutex);
    auto it = submissionIndices.find(submissionId);
    if (it == submissionIndices.end()) return true;
    const auto& submission = working.submissions[it->second];
    const auto* source = FindSource(submission.sourceId);
    const bool match = source && Matches(*source, submission);
    if (match && active.action != Action::None) ++matches;
    const bool suppress = source && active.action != Action::None &&
        ((active.action == Action::Hide && match) || (active.action == Action::Isolate && !match));
    if (capturing && Reserve(sizeof(Draw))) {
        draw.id = ++nextDraw;
        draw.submissionId = submissionId;
        draw.suppressed = suppress;
        draw.recorded = draw.recorded && !suppress;
        working.draws.push_back(draw);
    }
    return !suppress;
}

bool Highlight(uint64_t submissionId, Source& source) {
    if (!submissionId) return false;
    std::lock_guard lock(mutex);
    if (active.action != Action::Highlight) return false;
    auto it = submissionIndices.find(submissionId);
    if (it == submissionIndices.end()) return false;
    const auto& submission = working.submissions[it->second];
    auto* found = FindSource(submission.sourceId);
    if (!found || !Matches(*found, submission)) return false;
    source = *found;
    source.model = submission.model;
    source.hasModel = true;
    return true;
}
bool IsSourceLive(const Source& source) {
    std::lock_guard lock(mutex);
    // Fresh snapshots can be acted on while collection is off. Destruction hooks invalidate the generation.
    return source.generation == generation && !Retired(source) && std::any_of(liveSources.begin(), liveSources.end(),
        [&](const Source& other) { return SameSource(source, other); });
}
void SetOverride(const Source& source, const Submission* submission, Action action, Match match) {
    std::lock_guard lock(mutex);
    if (source.generation != generation || Retired(source) || (match == Match::Asset && !submission)) return;
    requested.source = source; requested.action = action; requested.match = match;
    requested.assetKey = submission ? submission->assetKey : 0;
}
void ClearOverrides() { std::lock_guard lock(mutex); requested = {}; }
}
