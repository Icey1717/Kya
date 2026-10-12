#pragma once

#include "log.h"

#define MY_LOG_NAMED_CATEGORY(category, level, format, ...) do { \
	auto& kyaLogEntry = LogCategories::category(); \
	if (kyaLogEntry.bEnabled) { \
		Log::GetInstance().AddLog(level, kyaLogEntry, format, ##__VA_ARGS__); \
	} \
} while (false)

// One lazily resolved handle per category, shared across translation units.
// Entries must not be erased: the debug menu and these handles retain references.
namespace LogCategories
{
#define DEFINE_LOG_CATEGORY(symbol, name) \
	inline LogEntry& symbol() { \
		static LogEntry& entry = Log::GetInstance().GetCategory(name); \
		return entry; \
	}

	DEFINE_LOG_CATEGORY(Actor, "Actor")
	DEFINE_LOG_CATEGORY(ActorHero, "ActorHero")
	DEFINE_LOG_CATEGORY(Animation, "Animation")
	DEFINE_LOG_CATEGORY(Audio, "Audio")
	DEFINE_LOG_CATEGORY(AudioInstances, "AudioInstances")
	DEFINE_LOG_CATEGORY(AutoLoad, "AutoLoad")
	DEFINE_LOG_CATEGORY(Autonomous, "Autonomous")
	DEFINE_LOG_CATEGORY(Camera, "Camera")
	DEFINE_LOG_CATEGORY(Collision, "Collision")
	DEFINE_LOG_CATEGORY(CustomShell, "CustomShell")
	DEFINE_LOG_CATEGORY(Cutscene, "Cutscene")
	DEFINE_LOG_CATEGORY(Debug, "Debug")
	DEFINE_LOG_CATEGORY(Ed3D, "ed3D")
	DEFINE_LOG_CATEGORY(EdDList, "edDList")
	DEFINE_LOG_CATEGORY(EdFCamera, "edFCamera")
	DEFINE_LOG_CATEGORY(EdList, "edList")
	DEFINE_LOG_CATEGORY(EdMem, "edMem")
	DEFINE_LOG_CATEGORY(Event, "event")
	DEFINE_LOG_CATEGORY(Fighter, "Fighter")
	DEFINE_LOG_CATEGORY(General, "General")
	DEFINE_LOG_CATEGORY(Io, "io")
	DEFINE_LOG_CATEGORY(IoLegacy, "IO")
	DEFINE_LOG_CATEGORY(JamGut, "JamGut")
	DEFINE_LOG_CATEGORY(LevelScheduler, "levelScheduler")
	DEFINE_LOG_CATEGORY(LightManager, "LightManager")
	DEFINE_LOG_CATEGORY(MeshLibrary, "MeshLibrary")
	DEFINE_LOG_CATEGORY(MeshPreviewer, "Mesh Previewer")
	DEFINE_LOG_CATEGORY(NativeRenderer, "NativeRenderer")
	DEFINE_LOG_CATEGORY(Particle, "Particle")
	DEFINE_LOG_CATEGORY(PS2, "PS2")
	DEFINE_LOG_CATEGORY(SaveLoad, "SaveLoad")
	DEFINE_LOG_CATEGORY(Sector, "Sector")
	DEFINE_LOG_CATEGORY(Sprite, "Sprite")
	DEFINE_LOG_CATEGORY(TextResources, "textresources")
	DEFINE_LOG_CATEGORY(TextureCache, "TextureCache")
	DEFINE_LOG_CATEGORY(TextureLibrary, "TextureLibrary")
	DEFINE_LOG_CATEGORY(Validation, "Validation")
	DEFINE_LOG_CATEGORY(Video, "Video")
	DEFINE_LOG_CATEGORY(Viewport, "Viewport")
	DEFINE_LOG_CATEGORY(Vulkan, "Vulkan")
	DEFINE_LOG_CATEGORY(VtxTrace, "vtx_trace")

#undef DEFINE_LOG_CATEGORY
}
