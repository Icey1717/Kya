#include <gtest/gtest.h>

#include <string.h>
#include <fstream>
#include <iostream>
#include <vector>

#include "edC/edCBank.h"
#include "edC/edCBankBuffer.h"
#include "edText.h"
#include "edMem.h"
#include "edSystem.h"
#include "Kya.h"

#include "renderer.h"

//#include "pointer_conv.h"
#include "DebugRenderer.h"
#include "Rendering/edCTextFont.h"
#include "../../../src/MathOps.h"
#include "../../Windows/Renderer/Vulkan/src/TextureCache.h"
#include "../../../src/port/pointer_conv.h"

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

TEST(IO, FontOpen) {
	edCBankInstall bankHeader;
	edCBankBufferEntry* BootData_BankBufferEntry;
	edCBankBuffer BootData_BankBuffer = { 0 };

	/* The menu BNK contains images for all button icons, the main Medium.fon. Icon for saves, money
	   and a map. */
	memset(&bankHeader, 0, sizeof(edCBankInstall));
	BootData_BankBuffer.initialize(0x32000, 1, &bankHeader);
	/* Set the bank header to point towards 'CDEURO/menu/Messages.bnk' */
	bankHeader.filePath = "CDEURO/menu/MenuData.bnk";
	BootData_BankBufferEntry = BootData_BankBuffer.get_free_entry();
	EXPECT_NE(BootData_BankBufferEntry, nullptr);
	EXPECT_EQ(BootData_BankBufferEntry->load(&bankHeader), true);

	/* Init Medium.Fon */
	int index = BootData_BankBufferEntry->get_index("medium.fon");

	EXPECT_EQ(index, 0x17);

	if (index == -1) {
		BootDataFont = (edCTextFont*)0x0;
	}
	else {
		BootDataFont = (edCTextFont*)get_element(BootData_BankBufferEntry, index);
	}
	EXPECT_NE(BootDataFont, nullptr);
	EXPECT_EQ(edTextInstallFont(BootDataFont), true);
}

edDList_material gMenuSplashMaterial;

TEST(IO, MenuSplashOpen) {
	edFILEH* pLoadedFile;
	ed_g2d_material* pMaterialSection;
	int iStack4;

	char* filePath = "CDEURO/Frontend/kyatitle.g2d";
	char* pTextureFileData = nullptr;
	ed_g2d_manager textureInfo;

	uint alignedSize;
	int fileSize;

	edMemSetFlags(TO_HEAP(H_MAIN), 0x100);
	pLoadedFile = edFileOpen(filePath, 9);
	if (pLoadedFile != (edFILEH*)0x0) {
		fileSize = edFileLoadSize(pLoadedFile);
		alignedSize = fileSize + 0x7ffU & 0xfffff800;
		pTextureFileData = (char*)edMemAllocAlign(TO_HEAP(H_MAIN), (long)(int)alignedSize, 0x40);
		if (pTextureFileData != (char*)0x0) {
			edFileRead(pLoadedFile, pTextureFileData, alignedSize);
			fileSize = pLoadedFile->nbQueuedActions;
			while (fileSize != 0) {
				edFileNoWaitStackFlush();
				fileSize = pLoadedFile->nbQueuedActions;
			}
		}
		edFileClose(pLoadedFile);
	}

	if (pTextureFileData != (char*)0x0) {
		ed3DInstallG2D(pTextureFileData, *(int*)(pTextureFileData + 8), &iStack4, &textureInfo, 0);
		pMaterialSection = ed3DG2DGetG2DMaterialFromIndex(&textureInfo, 0);
		edDListCreatMaterialFromIndex(&gMenuSplashMaterial, 0, &textureInfo, 2);
	}
	edMemClearFlags(TO_HEAP(H_MAIN), 0x100);
	EXPECT_NE(edTextInstallFont(BootDataFont), true);
}

TEST(Image, ReadFont) {
	EXPECT_NE(BootDataFont, nullptr);

	FontPacked_2C* pPackedData = (FontPacked_2C*)LOAD_SECTION(BootDataFont->pSubData);
	
	auto debugMaterial = PS2::GSTexValue(DebugMenu::LoadTextureData(&pPackedData->materialInfo), 0x380);
	EXPECT_EQ(debugMaterial.image.imageData.canvasWidth, 0x100);
	EXPECT_EQ(debugMaterial.image.imageData.canvasHeight, 0x100);

	DumpVectorToFile(debugMaterial.image.readBuffer, "font.bin");

	EXPECT_EQ(TestVector("font.bin", debugMaterial.image.readBuffer), true);
}

TEST(Image, ReadMenuSplash) {
	auto debugMaterial = PS2::GSTexValue(DebugMenu::LoadTextureData(&gMenuSplashMaterial), 0x380);
	EXPECT_EQ(debugMaterial.image.imageData.canvasWidth, 0x100);
	EXPECT_EQ(debugMaterial.image.imageData.canvasHeight, 0x100);

	DumpVectorToFile(debugMaterial.image.readBuffer, "splash.bin");

	EXPECT_EQ(TestVector("splash.bin", debugMaterial.image.readBuffer), true);
}

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
	edF32Matrix4GetTransposeHard(&matrix, &transposed);

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
	MainInit(argc, argv);
	return RUN_ALL_TESTS();
}