#include <gtest/gtest.h>

#include <algorithm>
#include <string.h>
#include <fstream>
#include <iostream>
#include <vector>

#include "edC/edCBank.h"
#include "edC/edCBankBuffer.h"
#include "edText.h"
#include "edMem.h"
#include "EdenLib/edSys/sources/EdSystem.h"

#include "renderer.h"

//#include "pointer_conv.h"
#include "DebugRenderer.h"
#include "Rendering/edCTextFont.h"
#include "../../../src/b-witch/MathOps.h"
#include "../../Windows/Renderer/Vulkan/src/pcsx2/TextureUpload/src/TextureUpload.h"
#include "../../../src/port/pointer_conv.h"
#include "edSysTransferService.h"
#include "edSoundStreamService.h"
#include "port/NativeProjection.h"
#include "CameraViewManager.h"
#include "ed3D.h"
#include "ed3D/ed3DSceneManager.h"

TEST(QuaternionInterpolation, NearbyRotationsPreserveUnitLength)
{
	for (float angle : { 0.002f, 0.003f, 0.005f, 0.01f }) {
		for (float alpha : { 0.0f, 0.25f, 0.5f, 0.75f, 1.0f }) {
			for (float sign : { -1.0f, 1.0f }) {
				edF32VECTOR4 current = { std::sin(0.7f), 0.0f, 0.0f, std::cos(0.7f) };
				edF32VECTOR4 target = { sign * std::sin(0.7f + angle), 0.0f, 0.0f, sign * std::cos(0.7f + angle) };
				edF32VECTOR4 result;
				edQuatShortestSLERPAccurate(alpha, &result, &current, &target);
				EXPECT_NEAR(result.x, std::sin(0.7f + alpha * angle), 1e-5f);
				EXPECT_NEAR(result.w, std::cos(0.7f + alpha * angle), 1e-5f);
				EXPECT_NEAR(result.x * result.x + result.y * result.y + result.z * result.z + result.w * result.w, 1.0f, 1e-5f);
			}
		}
	}
}

// The function to be tested
int Add(int a, int b) {
	return a + b;
}

// The test case
TEST(AddTest, PositiveNumbers) {
	// Test inputs and expected output
	int a = 5;
	int b = 10;
	int expected = 15;

	// Call the function and check the result
	int result = Add(a, b);
	EXPECT_EQ(result, expected);
}

#ifdef PLATFORM_WIN
TEST(NativeProjection, MatchesGameScreenCoordinatesAcrossFovAndAspect)
{
	for (float aspect : { 1.0f, 4.0f / 3.0f, 16.0f / 9.0f }) {
		for (float halfHeightOverDistance : { 0.2f, 0.5f, 1.0f }) {
			edFCamera camera{};
			edF32Matrix4SetIdentityHard(&camera.worldToCamera);
			camera.worldToCamera.da = -2.0f;
			camera.worldToCamera.db = 1.0f;
			camera.worldToCamera.dc = -3.0f;
			edFCameraSetSizeRatioFov(0.05f, aspect, halfHeightOverDistance, &camera);
			ed_3D_Scene scene{};
			scene.pCamera = &camera;
			auto projection = BuildNativeProjection(camera.finalHorizontalHalfFOV,
				camera.baseHorizontalHalfFOV, camera.computedVerticalHalfFOV, -0.1f, -5000.0f);
			for (edF32VECTOR4 position : { edF32VECTOR4{ 2.0f, -1.0f, -10.0f, 1.0f },
				edF32VECTOR4{ -1.0f, 2.0f, -20.0f, 1.0f }, edF32VECTOR4{ 4.0f, -3.0f, -100.0f, 1.0f } }) {
				edF32VECTOR2 screen{};
				ASSERT_TRUE(ed3DComputeSceneCoordinate(&screen, &position, &scene));
				edF32VECTOR4 view{}, clip{};
				edF32Matrix4MulF32Vector4Hard(&view, &camera.worldToCamera, &position);
				edF32Matrix4MulF32Vector4Hard(&clip, &projection, &view);
				ASSERT_GT(clip.w, 0.0f);
				EXPECT_NEAR(clip.x / clip.w, screen.x, 1.0e-6f);
				EXPECT_NEAR(clip.y / clip.w, -screen.y, 1.0e-6f);
			}
		}
	}
}

TEST(NativeProjection, PreservesReverseDepthAndFrustumEdges)
{
	auto projection = BuildNativeProjection(0.08f, 0.05f, 0.1f, -0.1f, -5000.0f);
	edF32VECTOR4 nearPoint{ 0.0f, 0.0f, -0.1f, 1.0f }, farPoint{ 0.0f, 0.0f, -5000.0f, 1.0f };
	edF32VECTOR4 nearClip{}, farClip{};
	edF32Matrix4MulF32Vector4Hard(&nearClip, &projection, &nearPoint);
	edF32Matrix4MulF32Vector4Hard(&farClip, &projection, &farPoint);
	EXPECT_NEAR(nearClip.z / nearClip.w, 1.0f, 1.0e-6f);
	EXPECT_NEAR(farClip.z / farClip.w, 0.0f, 1.0e-6f);
	edF32VECTOR4 corner{ 8.0f, 5.0f, -10.0f, 1.0f }, clip{};
	edF32Matrix4MulF32Vector4Hard(&clip, &projection, &corner);
	EXPECT_NEAR(clip.x / clip.w, 1.0f, 1.0e-6f);
	EXPECT_NEAR(clip.y / clip.w, -1.0f, 1.0e-6f);
	EXPECT_GT(clip.z / clip.w, 0.0f);
	EXPECT_LT(clip.z / clip.w, 1.0f);
}

TEST(NativeShadowSettings, NormalizesExtentAndSampleBudget)
{
	Renderer::Native::ShadowPassSettings settings{};
	settings.width = 0;
	settings.height = 0;
	settings.blurSamples = 100;
	settings.blurRadius = 7;
	const auto normalized = Renderer::Native::NormalizeShadowPassSettings(settings);
	EXPECT_EQ(normalized.width, 1u);
	EXPECT_EQ(normalized.height, 1u);
	EXPECT_EQ(normalized.blurSamples, 32u);
	EXPECT_EQ(normalized.blurRadius, 7u);
}

TEST(AudioTransferService, OwnsDataAndReturnsSoundHandle)
{
	Audio::Reset();
	std::uint8_t source[] = {1, 2, 3, 4};
	std::uint32_t result = 0;
	const auto index = Audio::Submit(source, sizeof(source), 0x40, Audio::TransferFlags::HighMem, 1, 2, 3, &result, 4, 0, nullptr);
	source[0] = 9;
	EXPECT_EQ(Audio::PumpThrough(index), index);
	Audio::LoadedDataInfo data;
	ASSERT_TRUE(Audio::LookupLoadedData(result, data));
	EXPECT_EQ(data.size, sizeof(source));
	EXPECT_EQ(data.data[0], 1);
	EXPECT_EQ(data.alignment, 0x40u);
	EXPECT_EQ(data.flags, Audio::TransferFlags::HighMem);
	EXPECT_TRUE(Audio::ReleaseLoadedData(result));
}
TEST(AudioTransferService, CompletesFifo)
{
	Audio::Reset();
	std::uint8_t source = 7;
	const auto first = Audio::Submit(&source, 1, 1, Audio::TransferFlags::None, 0, 0, 0, nullptr, 0, 0, nullptr);
	const auto second = Audio::Submit(&source, 1, 1, Audio::TransferFlags::None, 0, 0, 0, nullptr, 0, 0, nullptr);
	EXPECT_EQ(Audio::PumpThrough(second), second);
	EXPECT_EQ(first + 1, second);
}
TEST(AudioTransferService, ZeroSizeDoesNotQueue)
{
	Audio::Reset();
	EXPECT_EQ(Audio::Submit(nullptr, 0, 0, Audio::TransferFlags::None, 0, 0, 0, nullptr, 0, 0, nullptr), 0u);
	EXPECT_EQ(Audio::PumpAll(), 0u);
}
TEST(AudioTransferService, ResetReleasesData)
{
	Audio::Reset();
	std::uint8_t source = 1;
	std::uint32_t result = 0;
	Audio::Submit(&source, 1, 1, Audio::TransferFlags::None, 1, 2, 3, &result, 4, 0, nullptr);
	Audio::PumpAll();
	Audio::Reset();
	Audio::LoadedDataInfo data;
	EXPECT_FALSE(Audio::LookupLoadedData(result, data));
	EXPECT_EQ(Audio::PumpAll(), 0u);
}

TEST(AudioStreamService, LifecyclePreservesPlaybackState)
{
	Audio::ResetStreams();
	Audio::RegisterStream(3, 100, 1000.0f);

	Audio::StreamInfo info;
	ASSERT_TRUE(Audio::GetStreamInfo(3, info));
	EXPECT_TRUE(info.ready);
	EXPECT_FALSE(info.playing);
	EXPECT_EQ(info.channels, 1u);
	EXPECT_EQ(info.position, 0u);

	EXPECT_TRUE(Audio::StartStream(3));
	ASSERT_TRUE(Audio::GetStreamInfo(3, info));
	EXPECT_TRUE(info.playing);

	EXPECT_TRUE(Audio::StopStream(3));
	ASSERT_TRUE(Audio::GetStreamInfo(3, info));
	EXPECT_FALSE(info.playing);
	EXPECT_TRUE(Audio::UnregisterStream(3));
	EXPECT_FALSE(Audio::GetStreamInfo(3, info));
}

TEST(AudioStreamService, DecodesVagAdpcmBlock)
{
	std::vector<std::uint8_t> vag(0x30 + 16, 0);
	vag[0] = 'V';
	vag[1] = 'A';
	vag[2] = 'G';
	vag[3] = 'p';
	vag[0xc] = 0;
	vag[0xd] = 0;
	vag[0xe] = 0;
	vag[0xf] = 16;
	vag[0x10] = 0;
	vag[0x11] = 0;
	vag[0x12] = 0x56;
	vag[0x13] = 0x22;
	vag[0x30] = 0x0c;
	vag[0x31] = 1;

	std::vector<std::int16_t> samples;
	std::uint32_t sampleRate = 0;
	ASSERT_TRUE(Audio::DecodeVag(vag.data(), vag.size(), samples, sampleRate));
	EXPECT_EQ(sampleRate, 22050u);
	EXPECT_EQ(samples.size(), 28u);
	EXPECT_TRUE(std::all_of(samples.begin(), samples.end(), [](std::int16_t sample) { return sample == 0; }));
}

TEST(AudioStreamService, DecodesInterleavedMibAdpcm)
{
	constexpr std::uint32_t channels = 2;
	constexpr std::uint32_t interleaveBlockSize = 16;
	constexpr std::uint32_t sampleRate = 22050;
	std::vector<std::uint8_t> mib(interleaveBlockSize * channels * 2, 0);

	auto setBlock = [&mib](std::size_t offset, std::uint8_t firstNibble) {
		mib[offset] = 0x00;
		mib[offset + 2] = firstNibble;
	};
	setBlock(0, 1);
	setBlock(16, 2);
	setBlock(32, 3);
	setBlock(48, 4);

	std::vector<std::int16_t> samples;
	ASSERT_TRUE(Audio::DecodeMib(mib.data(), mib.size(), channels, interleaveBlockSize, sampleRate, samples));
	EXPECT_EQ(samples.size(), 112u);
	EXPECT_EQ(samples[0], 4096);
	EXPECT_EQ(samples[1], 8192);
	EXPECT_EQ(samples[56], 12288);
	EXPECT_EQ(samples[57], 16384);
	EXPECT_EQ(samples[28], 0);
	EXPECT_EQ(samples[29], 0);
}

class PointerConvTest : public ::testing::Test
{
  protected:
	void SetUp() override
	{
		PointerConv::ResetAll();
	}

	void TearDown() override
	{
		PointerConv::ResetAll();
	}
};

TEST_F(PointerConvTest, NullPointerRoundTripUsesZeroHandle)
{
	EXPECT_EQ(STORE_POINTER(nullptr), 0);
	EXPECT_TRUE(PointerConv::ResolvePointer(0).has_value());
	EXPECT_EQ(LOAD_POINTER(0), nullptr);
	EXPECT_EQ(PointerConv::GetLiveHandleCount(), 0);
}

TEST_F(PointerConvTest, PersistentHandlesDeduplicateAndReferenceCount)
{
	int value = 123;
	int handleA = STORE_POINTER(&value);
	int handleB = STORE_POINTER(&value);

	EXPECT_EQ(handleA, handleB);
	EXPECT_EQ(LOAD_POINTER(handleA), &value);
	EXPECT_EQ(PointerConv::GetLiveHandleCount(), 1);

	RELEASE_POINTER(handleA);
	EXPECT_EQ(LOAD_POINTER(handleB), &value);
	EXPECT_EQ(PointerConv::GetLiveHandleCount(), 1);

	RELEASE_POINTER(handleB);
	EXPECT_FALSE(PointerConv::ResolvePointer(handleA).has_value());
	EXPECT_EQ(PointerConv::GetLiveHandleCount(), 0);
}

TEST_F(PointerConvTest, ReleasedSlotsReuseIndexWithNewGeneration)
{
	int valueA = 1;
	int valueB = 2;
	int handleA = STORE_POINTER(&valueA);
	EXPECT_EQ(PointerConv::GetCapacity(), 1);

	RELEASE_POINTER(handleA);
	EXPECT_FALSE(PointerConv::ResolvePointer(handleA).has_value());

	int handleB = STORE_POINTER(&valueB);
	EXPECT_NE(handleA, handleB);
	EXPECT_EQ(PointerConv::GetCapacity(), 1);
	EXPECT_EQ(LOAD_POINTER(handleB), &valueB);
}

TEST_F(PointerConvTest, TransientHandlesResetSeparately)
{
	int persistentValue = 10;
	int transientValue = 20;
	int persistentHandle = STORE_POINTER(&persistentValue);
	int transientHandle = STORE_TRANSIENT_POINTER(&transientValue);

	EXPECT_EQ(PointerConv::GetLiveHandleCount(), 2);

	POINTERCONV_RESET_TRANSIENT();

	EXPECT_EQ(LOAD_POINTER(persistentHandle), &persistentValue);
	EXPECT_FALSE(PointerConv::ResolvePointer(transientHandle).has_value());
	EXPECT_EQ(PointerConv::GetLiveHandleCount(), 1);
}

TEST_F(PointerConvTest, ResetAllClearsAllHandles)
{
	int valueA = 1;
	int valueB = 2;
	STORE_POINTER(&valueA);
	STORE_TRANSIENT_POINTER(&valueB);

	EXPECT_EQ(PointerConv::GetLiveHandleCount(), 2);
	EXPECT_GE(PointerConv::GetCapacity(), PointerConv::GetLiveHandleCount());

	POINTERCONV_RESET_ALL();

	EXPECT_EQ(PointerConv::GetLiveHandleCount(), 0);
	EXPECT_EQ(PointerConv::GetCapacity(), 0);
}
#endif

// Function to dump vector contents to a file
void DumpVectorToFile(const std::vector<uint8_t>& data, const std::string& filename) {
	std::string filepath = std::string(TEST_DATA_DIRECTORY) + filename;
	std::ofstream file(filepath, std::ios::binary);
	if (file.is_open()) {
		file.write(reinterpret_cast<const char*>(data.data()), data.size());
		file.close();
	}
}

// Function to read file contents into a vector
std::vector<uint8_t> ReadFileToVector(const std::string& filename) {
	std::vector<uint8_t> data;

	std::string filepath = std::string(TEST_DATA_DIRECTORY) + filename;

	std::ifstream file(filepath, std::ios::binary);
	if (file.is_open()) {
		// Get file size
		file.seekg(0, std::ios::end);
		size_t fileSize = file.tellg();
		file.seekg(0, std::ios::beg);

		// Read file contents into the vector
		data.resize(fileSize);
		file.read(reinterpret_cast<char*>(data.data()), fileSize);

		file.close();
	}
	return data;
}

bool TestVector(std::string filename, const std::vector<uint8_t>& testData) {
	// Read file contents into a new vector
	std::vector<uint8_t> readData = ReadFileToVector(filename);

	// Verify the contents are the same
	if (testData.size() != readData.size()) {
		return false;
	}

	for (size_t i = 0; i < testData.size(); ++i) {
		if (testData[i] != readData[i]) {
			return false;
		}
	}

	return true;
}

static edCTextFont* BootDataFont = NULL;

//TEST(IO, FontOpen) {
//	edCBankInstall bankHeader;
//	edCBankBufferEntry* BootData_BankBufferEntry;
//	edCBankBuffer BootData_BankBuffer = { 0 };
//
//	/* The menu BNK contains images for all button icons, the main Medium.fon. Icon for saves, money
//	   and a map. */
//	memset(&bankHeader, 0, sizeof(edCBankInstall));
//	BootData_BankBuffer.initialize(0x32000, 1, &bankHeader);
//	/* Set the bank header to point towards 'CDEURO/menu/Messages.bnk' */
//	bankHeader.filePath = "CDEURO/menu/MenuData.bnk";
//	BootData_BankBufferEntry = BootData_BankBuffer.get_free_entry();
//	EXPECT_NE(BootData_BankBufferEntry, nullptr);
//	EXPECT_EQ(BootData_BankBufferEntry->load(&bankHeader), true);
//
//	/* Init Medium.Fon */
//	int index = BootData_BankBufferEntry->get_index("medium.fon");
//
//	EXPECT_EQ(index, 0x17);
//
//	if (index == -1) {
//		BootDataFont = (edCTextFont*)0x0;
//	}
//	else {
//		BootDataFont = (edCTextFont*)get_element(BootData_BankBufferEntry, index);
//	}
//	EXPECT_NE(BootDataFont, nullptr);
//	EXPECT_EQ(edTextInstallFont(BootDataFont), true);
//}
//
//edDList_material gMenuSplashMaterial;
//
//TEST(IO, MenuSplashOpen) {
//	edFILEH* pLoadedFile;
//	ed_g2d_material* pMaterialSection;
//	int iStack4;
//
//	char* filePath = "CDEURO/Frontend/kyatitle.g2d";
//	char* pTextureFileData = nullptr;
//	ed_g2d_manager textureInfo;
//
//	uint alignedSize;
//	int fileSize;
//
//	edMemSetFlags(TO_HEAP(H_MAIN), 0x100);
//	pLoadedFile = edFileOpen(filePath, 9);
//	if (pLoadedFile != (edFILEH*)0x0) {
//		fileSize = edFileLoadSize(pLoadedFile);
//		alignedSize = fileSize + 0x7ffU & 0xfffff800;
//		pTextureFileData = (char*)edMemAllocAlign(TO_HEAP(H_MAIN), (long)(int)alignedSize, 0x40);
//		if (pTextureFileData != (char*)0x0) {
//			edFileRead(pLoadedFile, pTextureFileData, alignedSize);
//			fileSize = pLoadedFile->nbQueuedActions;
//			while (fileSize != 0) {
//				edFileNoWaitStackFlush();
//				fileSize = pLoadedFile->nbQueuedActions;
//			}
//		}
//		edFileClose(pLoadedFile);
//	}
//
//	if (pTextureFileData != (char*)0x0) {
//		ed3DInstallG2D(pTextureFileData, *(int*)(pTextureFileData + 8), &iStack4, &textureInfo, 0);
//		pMaterialSection = ed3DG2DGetG2DMaterialFromIndex(&textureInfo, 0);
//		edDListCreatMaterialFromIndex(&gMenuSplashMaterial, 0, &textureInfo, 2);
//	}
//	edMemClearFlags(TO_HEAP(H_MAIN), 0x100);
//	EXPECT_NE(edTextInstallFont(BootDataFont), true);
//}
//
//TEST(Image, ReadFont) {
//	EXPECT_NE(BootDataFont, nullptr);
//
//	FontPacked_2C* pPackedData = (FontPacked_2C*)LOAD_POINTER(BootDataFont->pSubData);
//	
//	auto debugMaterial = PS2::GSTexValue(DebugMenu::LoadTextureData(&pPackedData->materialInfo), 0x380);
//	EXPECT_EQ(debugMaterial.image.imageData.canvasWidth, 0x100);
//	EXPECT_EQ(debugMaterial.image.imageData.canvasHeight, 0x100);
//
//	DumpVectorToFile(debugMaterial.image.readBuffer, "font.bin");
//
//	EXPECT_EQ(TestVector("font.bin", debugMaterial.image.readBuffer), true);
//}
//
//TEST(Image, ReadMenuSplash) {
//	auto debugMaterial = PS2::GSTexValue(DebugMenu::LoadTextureData(&gMenuSplashMaterial), 0x380);
//	EXPECT_EQ(debugMaterial.image.imageData.canvasWidth, 0x100);
//	EXPECT_EQ(debugMaterial.image.imageData.canvasHeight, 0x100);
//
//	DumpVectorToFile(debugMaterial.image.readBuffer, "splash.bin");
//
//	EXPECT_EQ(TestVector("splash.bin", debugMaterial.image.readBuffer), true);
//}

// Test case for matrix multiplication
TEST(MatrixMultiplicationTest, MultiplyMatrices) {
	edF32MATRIX4 matrix1 = { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f };
	edF32MATRIX4 matrix2 = { 17.0f, 18.0f, 19.0f, 20.0f, 21.0f, 22.0f, 23.0f, 24.0f, 25.0f, 26.0f, 27.0f, 28.0f, 29.0f, 30.0f, 31.0f, 32.0f };

	edF32MATRIX4 expected = { 250.0f, 260.0f, 270.0f, 280.0f, 618.0f, 644.0f, 670.0f, 696.0f, 986.0f, 1028.0f, 1070.0f, 1112.0f, 1354.0f, 1412.0f, 1470.0f, 1528.0f };

	{
		edF32MATRIX4 result;
		edF32Matrix4MulF32Matrix4Hard(&result, &matrix1, &matrix2);

		// Check if the result matches the expected matrix
		EXPECT_EQ(result.aa, expected.aa);
		EXPECT_EQ(result.ab, expected.ab);
		EXPECT_EQ(result.ac, expected.ac);
		EXPECT_EQ(result.ad, expected.ad);
		EXPECT_EQ(result.ba, expected.ba);
		EXPECT_EQ(result.bb, expected.bb);
		EXPECT_EQ(result.bc, expected.bc);
		EXPECT_EQ(result.bd, expected.bd);
		EXPECT_EQ(result.ca, expected.ca);
		EXPECT_EQ(result.cb, expected.cb);
		EXPECT_EQ(result.cc, expected.cc);
		EXPECT_EQ(result.cd, expected.cd);
		EXPECT_EQ(result.da, expected.da);
		EXPECT_EQ(result.db, expected.db);
		EXPECT_EQ(result.dc, expected.dc);
		EXPECT_EQ(result.dd, expected.dd);
	}
}

TEST(MatrixTest, TransposeHard) {
	edF32MATRIX4 matrix{ 1.0f, 2.0f, 3.0f, 4.0f,
		5.0f, 6.0f, 7.0f, 8.0f,
		9.0f, 10.0f, 11.0f, 12.0f,
		13.0f, 14.0f, 15.0f, 16.0f };

	edF32MATRIX4 transposed;
	edF32Matrix4GetTransposeHard(&transposed, &matrix);

	EXPECT_EQ(transposed.aa, 1.0f);
	EXPECT_EQ(transposed.ab, 5.0f);
	EXPECT_EQ(transposed.ac, 9.0f);
	EXPECT_EQ(transposed.ad, 13.0f);
	EXPECT_EQ(transposed.ba, 2.0f);
	EXPECT_EQ(transposed.bb, 6.0f);
	EXPECT_EQ(transposed.bc, 10.0f);
	EXPECT_EQ(transposed.bd, 14.0f);
	EXPECT_EQ(transposed.ca, 3.0f);
	EXPECT_EQ(transposed.cb, 7.0f);
	EXPECT_EQ(transposed.cc, 11.0f);
	EXPECT_EQ(transposed.cd, 15.0f);
	EXPECT_EQ(transposed.da, 4.0f);
	EXPECT_EQ(transposed.db, 8.0f);
	EXPECT_EQ(transposed.dc, 12.0f);
	EXPECT_EQ(transposed.dd, 16.0f);
}

TEST(VU1, Run) {
	
}

// The main function to run the tests
int main(int argc, char* argv[]) {
	::testing::InitGoogleTest(&argc, argv);
	Renderer::SetHeadless(true);
	return RUN_ALL_TESTS();
}
