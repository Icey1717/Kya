#include <gtest/gtest.h>
#include <array>
#include <cstddef>
#include <new>
#include <vector>
#include "ed3D.h"
#include "ed3D/ed3DG3D.h"
#include "port/pointer_conv.h"
#include "port/NativeProjection.h"
#include "MathOps.h"
#include "LightManager.h"

extern void MTXLightFrustum(float left, float right, float bottom, float top, float zNear,
	float fovY, float scaleY, float offsetX, edF32MATRIX4* matrix, float offsetY);

namespace
{
	struct ShadowHierarchyFixture
	{
		struct ObjectChunk
		{
			ed_Chunck chunk{};
			ed_g3d_object object{};
		};
		alignas(16) std::array<std::byte, sizeof(ed_Chunck) + sizeof(ed_g3d_hierarchy) + 2 * sizeof(ed3DLod) + sizeof(ed_Chunck)> hierarchyData{};
		std::array<ObjectChunk, 2> objects{};
		std::array<ed_hash_code, 2> hashes{};
		std::array<ed_3d_strip, 2> strips{};
		std::vector<int> handles;
		ed_g3d_hierarchy* hierarchy;

		int Store(void* pointer)
		{
			const int handle = STORE_POINTER(pointer);
			handles.push_back(handle);
			return handle;
		}

		ShadowHierarchyFixture()
		{
			auto* chunk = new (hierarchyData.data()) ed_Chunck{};
			chunk->hash = HASH_CODE_HIER;
			chunk->size = sizeof(ed_Chunck) + sizeof(ed_g3d_hierarchy) + 2 * sizeof(ed3DLod);
			hierarchy = new (hierarchyData.data() + sizeof(ed_Chunck)) ed_g3d_hierarchy{};
			hierarchy->lodCount = 2;
			new (hierarchyData.data() + chunk->size) ed_Chunck{}; // Non-HIER terminates the chunk walk.
			for (size_t i = 0; i < objects.size(); ++i) {
				objects[i].object.stripCount = 1;
				objects[i].object.p3DData = Store(&strips[i]);
				hashes[i].pData = Store(&objects[i].chunk);
				auto* lod = new (&hierarchy->aLods[i]) ed3DLod{};
				lod->pObj = Store(&hashes[i]);
				strips[i].shadowCastFlags = 0x10;
				strips[i].shadowReceiveFlags = 0x80;
			}
		}

		~ShadowHierarchyFixture()
		{
			for (const int handle : handles) RELEASE_POINTER(handle);
		}
	};
}

TEST(ShadowCasterFlags, SetsCasterFlagsAcrossLodsWithoutChangingReceiverFlags)
{
	ShadowHierarchyFixture data;
	ed3DG3DHierarchySetStripShadowCastFlag(data.hierarchy, 0x4);
	EXPECT_NE(data.hierarchy->flags_0x9e & 0x200, 0);
	for (const auto& strip : data.strips) {
		EXPECT_EQ(strip.shadowCastFlags, 0x14);
		EXPECT_EQ(strip.shadowReceiveFlags, 0x80);
	}
}

TEST(ShadowCasterFlags, UsesLastLodForActorShadowAndPreservesReceiverFlags)
{
	ShadowHierarchyFixture data;
	data.hierarchy->flags_0x9e = 0x100; // Actor setup selects the last LOD for casting.
	data.hierarchy->pLinkTransformData = data.Store(data.hierarchyData.data());
	ed3DG3DHierarchySetStripShadowCastFlag(data.hierarchy, 0x4);
	EXPECT_EQ(data.strips[0].shadowCastFlags, 0x10);
	EXPECT_EQ(data.strips[1].shadowCastFlags, 0x14);
	for (const auto& strip : data.strips) EXPECT_EQ(strip.shadowReceiveFlags, 0x80);
}

TEST(ShadowCasterFlags, RuntimeHierarchyLodsFollowNativePointerLayout)
{
	ed_3d_hierarchy_node data{};
	data.base.lodCount = 2;
	// The live hierarchy contains native pointers; it cannot be cast to the serialized type.
	EXPECT_NE(sizeof(ed_3d_hierarchy), sizeof(ed_g3d_hierarchy));
	EXPECT_EQ(ed3DHierarcGetLOD(&data.base, 0), &data.aLods[0]);
	EXPECT_EQ(ed3DHierarcGetLOD(&data.base, data.base.lodCount - 1), &data.aLods[1]);
	EXPECT_EQ(ed3DHierarcGetLOD(&data.base, data.base.lodCount), nullptr);
}

TEST(ShadowProjection, StqDivisionMatchesNativeMaskTextureCoordinates)
{
	for (float aspect : { 1.0f, 2.0f, 16.0f / 9.0f }) {
		const float halfHeight = 0.05f, halfWidth = halfHeight * aspect, distance = 0.1f;
		const float nearDistance = 0.1f;
		edF32MATRIX4 ps2Projection{}, stqProjection{};
		MTXLightFrustum(-halfHeight * nearDistance / distance, halfHeight * nearDistance / distance,
			-halfWidth * nearDistance / distance, halfWidth * nearDistance / distance,
			nearDistance, 0.5f, 0.5f, 0.5f, &ps2Projection, 0.5f);
		edF32Matrix4GetTransposeHard(&stqProjection, &ps2Projection);
		auto nativeProjection = BuildNativeProjection(halfWidth, halfHeight, distance, -0.1f, -75.0f);
		for (float depth : { 1.0f, 10.0f, 40.0f }) {
			for (float fraction : { -1.0f, 0.0f, 1.0f }) {
				edF32VECTOR4 position{ fraction * halfWidth * depth / distance,
					fraction * halfHeight * depth / distance, -depth, 1.0f };
				edF32VECTOR4 stq{}, clip{};
				edF32Matrix4MulF32Vector4Hard(&stq, &stqProjection, &position);
				edF32Matrix4MulF32Vector4Hard(&clip, &nativeProjection, &position);
				EXPECT_FLOAT_EQ(stq.z, depth);
				EXPECT_FLOAT_EQ(stq.w, 1.0f);
				EXPECT_NEAR(stq.x / stq.z, 0.5f * (clip.x / clip.w + 1.0f), 1.0e-6f);
				EXPECT_NEAR(stq.y / stq.z, 0.5f * (clip.y / clip.w + 1.0f), 1.0e-6f);
			}
		}
	}
}

TEST(ShadowLighting, PreservesAngledLightDirectionWithoutSettingCameraFallbackFlag)
{
	for (edF32VECTOR4 lightDirection : { edF32VECTOR4{ 0.3f, -0.4f, 0.8660254f, 0.0f },
		edF32VECTOR4{ -0.6f, 0.8f, 0.0f, 0.0f } }) {
		edF32VECTOR4 ambient{};
		edF32MATRIX4 directions{}, colors{};
		directions.rowX = lightDirection;
		colors.rowX.x = 100.0f;
		ed_3D_Light_Config config{ &ambient, &directions, &colors };
		edF32VECTOR4 result{};
		CLightConfig::ComputeShadow(&config, &result);
		// Preserve the existing upward seed and absolute-Y weighting, then negate XYZ.
		edF32VECTOR4 expected{ -100.0f * lightDirection.x,
			-(1.0f + 100.0f * std::abs(lightDirection.y)), -100.0f * lightDirection.z, 0.0f };
		edF32Vector4NormalizeHard(&expected, &expected);
		EXPECT_NEAR(result.x, expected.x, 1.0e-6f);
		EXPECT_NEAR(result.y, expected.y, 1.0e-6f);
		EXPECT_NEAR(result.z, expected.z, 1.0e-6f);
		EXPECT_FLOAT_EQ(result.w, 0.0f);
	}
}

TEST(ShadowLighting, AutomaticSlotsPreserveEachLightsFlagsAndFeedShadowDirection)
{
	CLightManager manager;
	auto* previousManager = CScene::ptable.g_LightManager_004516b0;
	CScene::ptable.g_LightManager_004516b0 = &manager;
	CLightSun sun;
	CLightDirectional directional;
	CLightAmbient ambient;
	sun.colour_0x4.rgba = 0x410F0105;
	directional.colour_0x4.rgba = 0x410F0206;
	ambient.colour_0x4.rgba = 0x410F0307;
	sun.baseShape.direction = { 0.549232f, -0.614251f, 0.566605f, 1.0f };
	directional.baseShape.direction = sun.baseShape.direction;
	sun.colorModel.color = { 144.0f, 102.0f, 70.0f, 0.0f };
	manager.Activate(&sun, -1);
	manager.Activate(&directional, -1);
	manager.Activate(&ambient, -3);
	const uint sunFlags = sun.colour_0x4.rgba;
	const uint directionalFlags = directional.colour_0x4.rgba;
	manager.aSectorLights[0] = &sun;
	manager.aSectorLights[1] = &directional;
	manager.aSectorLights[2] = &ambient;
	manager.sectorLightCount = 3;
	manager.BuildActiveList();
	EXPECT_EQ(manager.activeLightCount, 3);
	EXPECT_EQ(sun.colour_0x4.rgba & 0xFF0FFFFF, (sunFlags | 0x02000000) & 0xFF0FFFFF);
	EXPECT_EQ(directional.colour_0x4.rgba & 0xFF0FFFFF, (directionalFlags | 0x02000000) & 0xFF0FFFFF);
	const int slot = sun.colour_0x4.b >> 4;
	EXPECT_LT(slot, 4);
	EXPECT_LT(directional.colour_0x4.b >> 4, 4);
	if (slot < 4) {
		manager.lightDirections = gF32Matrix4Zero;
		manager.lightColorMatrix = gF32Matrix4Zero;
		sun.Manage();
		EXPECT_FLOAT_EQ(manager.lightColorMatrix.vector[slot].x, 144.0f);
		CLightConfig::Validate(&manager.lightConfig, true);
		edF32VECTOR4 shadowDirection{};
		CLightConfig::ComputeShadow(&manager.lightConfig, &shadowDirection);
		EXPECT_GT(std::abs(shadowDirection.x), 0.1f);
		EXPECT_GT(std::abs(shadowDirection.z), 0.1f);
		EXPECT_FLOAT_EQ(shadowDirection.w, 0.0f);
		// The hero's per-position lighting also requires the preserved enabled bit.
		// Directional DoLighting is unimplemented; its slot-assignment path is checked above.
		manager.aActiveLights[1] = &ambient;
		manager.activeLightCount = 2;
		edF32VECTOR4 localAmbient{}, location{};
		edF32MATRIX4 localDirections{}, localColors{};
		ed_3D_Light_Config localConfig{ &localAmbient, &localDirections, &localColors };
		manager.ComputeLighting(&location, &localConfig);
		EXPECT_FLOAT_EQ(localColors.vector[slot].x, 144.0f);
	}
	CScene::ptable.g_LightManager_004516b0 = previousManager;
	delete[] manager.aLights;
	delete[] manager.aLightConfigs;
	delete[] manager.field_0xe0;
}

TEST(ShadowLighting, SunManagerUsesAuthoredDirectionAndRgbColour)
{
	CLightManager manager;
	auto* previousManager = CScene::ptable.g_LightManager_004516b0;
	CScene::ptable.g_LightManager_004516b0 = &manager;
	manager.lightDirections = gF32Matrix4Zero;
	manager.lightColorMatrix = gF32Matrix4Zero;
	CLightSun sun;
	sun.colour_0x4.rgba = 0;
	sun.colour_0x4.b = 0x20; // Slot 2, distinct from the default slot.
	sun.baseShape.direction = { 0.3f, -0.4f, 0.8660254f, 0.0f };
	sun.colorModel.color = { 100.0f, 80.0f, 60.0f, 0.0f };
	sun.colorModel.ambientColor = { 4.0f, 5.0f, 6.0f, 0.0f };
	sun.colorModel.field_0x20 = { 7.0f, 8.0f, 9.0f, 0.0f };
	edF32VECTOR4 direction{}, color{}, ambient{};
	LightingContext context{};
	context.pLightDirection = &direction;
	context.pLightColor = &color;
	context.pLightAmbient = &ambient;
	sun.DoLighting(&context);
	sun.Manage();
	for (size_t component = 0; component < 4; ++component) {
		EXPECT_FLOAT_EQ(manager.lightDirections.vector[2].raw[component], direction.raw[component]);
		EXPECT_FLOAT_EQ(manager.lightColorMatrix.vector[2].raw[component], color.raw[component]);
		EXPECT_FLOAT_EQ(manager.lightDirections.vector[0].raw[component], 0.0f);
		EXPECT_FLOAT_EQ(manager.lightColorMatrix.vector[0].raw[component], 0.0f);
	}
	CLightConfig::Validate(&manager.lightConfig, true);
	edF32VECTOR4 shadowDirection{};
	CLightConfig::ComputeShadow(&manager.lightConfig, &shadowDirection);
	EXPECT_GT(std::abs(shadowDirection.x), 0.1f);
	EXPECT_GT(std::abs(shadowDirection.z), 0.1f);
	EXPECT_FLOAT_EQ(shadowDirection.w, 0.0f);
	CScene::ptable.g_LightManager_004516b0 = previousManager;
	delete[] manager.aLights;
	delete[] manager.aLightConfigs;
	delete[] manager.field_0xe0;
}
