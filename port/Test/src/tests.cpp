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

#include "pointer_conv.h"
#include "DebugHelpers.h"
#include "Rendering/edCTextFont.h"

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
	initialize(&BootData_BankBuffer, 0x32000, 1, &bankHeader);
	/* Set the bank header to point towards 'CDEURO/menu/Messages.bnk' */
	bankHeader.filePath = "CDEURO/menu/MenuData.bnk";
	BootData_BankBufferEntry = get_free_entry(&BootData_BankBuffer);
	EXPECT_NE(BootData_BankBufferEntry, nullptr);
	EXPECT_EQ(load(BootData_BankBufferEntry, &bankHeader), true);

	/* Init Medium.Fon */
	int index = get_index(BootData_BankBufferEntry, "medium.fon");

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
		fileSize = GetFileSize_0025bd70(pLoadedFile);
		alignedSize = fileSize + 0x7ffU & 0xfffff800;
		pTextureFileData = (char*)edMemAllocAlign(TO_HEAP(H_MAIN), (long)(int)alignedSize, 0x40);
		if (pTextureFileData != (char*)0x0) {
			SetRead_0025be80(pLoadedFile, pTextureFileData, alignedSize);
			fileSize = pLoadedFile->count_0x228;
			while (fileSize != 0) {
				edFileNoWaitStackFlush();
				fileSize = pLoadedFile->count_0x228;
			}
		}
		SetClose_0025bf60(pLoadedFile);
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
	
	auto debugMaterial = DebugHelpers::DebugMaterial(&pPackedData->materialInfo, false);
	EXPECT_EQ(debugMaterial.texture.width, 0x100);
	EXPECT_EQ(debugMaterial.texture.height, 0x100);

	//DumpVectorToFile(debugMaterial.texture.readBuffer, "font.bin");

	EXPECT_EQ(TestVector("font.bin", debugMaterial.texture.readBuffer), true);
}

TEST(Image, ReadMenuSplash) {
	auto debugMaterial = DebugHelpers::DebugMaterial(&gMenuSplashMaterial, false);
	EXPECT_EQ(debugMaterial.texture.width, 0x100);
	EXPECT_EQ(debugMaterial.texture.height, 0x100);

	//DumpVectorToFile(debugMaterial.texture.readBuffer, "splash.bin");

	EXPECT_EQ(TestVector("splash.bin", debugMaterial.texture.readBuffer), true);
}

// The main function to run the tests
int main(int argc, char* argv[]) {
	::testing::InitGoogleTest(&argc, argv);
	Renderer::SetHeadless(true);
	MainInit(argc, argv);
	return RUN_ALL_TESTS();
}