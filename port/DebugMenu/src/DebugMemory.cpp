#include "DebugMemory.h"

#include <imgui.h>
#include <fstream>
#include <cassert>

#include "edMem.h"
#include "VulkanRenderer.h"

// Helper to visualize allocator blocks in ImGui
// Helper to visualize allocator blocks in ImGui
static void ImGui_ShowAllocatorBlocks()
{
	extern MasterMemoryBlk MemoryMasterBlock;
	if (!MemoryMasterBlock.aBlocks || MemoryMasterBlock.nbTotalBlocks <= 0)
		return;

	// --- Highlight and navigation state ---
	static int selectedBlock = -1;
	static float highlightTime = 0.0f;
	const float highlightDuration = 2.0f; // seconds

	if (ImGui::CollapsingHeader("Allocator Blocks")) {
		ImGui::Text("Total Blocks: %d", MemoryMasterBlock.nbTotalBlocks);
		ImGui::Text("Used Blocks: %d", MemoryMasterBlock.nbUsedBlocks);
		ImGui::Text("Free Blocks: %d", MemoryMasterBlock.nbFreeBlocks);
		ImGui::Separator();

		if (ImGui::BeginTable("Blocks", 13, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY)) {
			ImGui::TableSetupScrollFreeze(0, 1);

			ImGui::TableSetupColumn("Idx");
			ImGui::TableSetupColumn("Flags");
			ImGui::TableSetupColumn("Status");
			ImGui::TableSetupColumn("FreeBytes");
			ImGui::TableSetupColumn("StartAddr");
			ImGui::TableSetupColumn("Align");
			ImGui::TableSetupColumn("Offset");
			ImGui::TableSetupColumn("Prev");
			ImGui::TableSetupColumn("Next");
			ImGui::TableSetupColumn("Parent");
			ImGui::TableSetupColumn("BlockIdx");
			ImGui::TableSetupColumn("FreeListHead");
			ImGui::TableSetupColumn("freeListHeadAllocatedTo");
			ImGui::TableHeadersRow();

			// Update highlight timer
			if (highlightTime > 0.0f)
				highlightTime -= ImGui::GetIO().DeltaTime;

			if (highlightTime < 0.0f)
				highlightTime = 0.0f;

			for (short i = 0; i < MemoryMasterBlock.nbTotalBlocks; ++i) {
				const S_MAIN_MEMORY_HEADER& blk = MemoryMasterBlock.aBlocks[i];
				ImGui::TableNextRow();

				// Highlight row if selected
				if (selectedBlock == i && highlightTime > 0.0f) {
					// Scroll the table to the selected block
					ImGui::SetScrollHereY(0.5f);

					ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, IM_COL32(255, 255, 0, 64));
				}

				ImGui::TableSetColumnIndex(0); ImGui::Text("%d", i);
				ImGui::TableSetColumnIndex(1); ImGui::Text("0x%04x", blk.flags);
				ImGui::TableSetColumnIndex(2); ImGui::TextUnformatted((blk.flags & 2) ? "Free" : "Used");
				ImGui::TableSetColumnIndex(3); ImGui::Text("0x%x", blk.freeBytes);
				ImGui::TableSetColumnIndex(4); ImGui::Text("%p", blk.pStartAddr);
				ImGui::TableSetColumnIndex(5); ImGui::Text("%d", blk.align);
				ImGui::TableSetColumnIndex(6); ImGui::Text("%d", blk.offset);

				// --- Clickable Prev Block ---
				ImGui::TableSetColumnIndex(7);
				if (blk.prevFreeBlock >= 0 && blk.prevFreeBlock < MemoryMasterBlock.nbTotalBlocks) {
					char label[32];
					snprintf(label, sizeof(label), "%d##prev_%d", blk.prevFreeBlock, i);
					if (ImGui::Selectable(label, false, ImGuiSelectableFlags_AllowDoubleClick, ImVec2(0, 0))) {
						selectedBlock = blk.prevFreeBlock;
						highlightTime = highlightDuration;
					}
				}
				else {
					ImGui::Text("%d", blk.nextFreeBlock);
				}

				// --- Clickable Next block ---
				ImGui::TableSetColumnIndex(8);
				if (blk.nextFreeBlock >= 0 && blk.nextFreeBlock < MemoryMasterBlock.nbTotalBlocks) {
					char label[32];
					snprintf(label, sizeof(label), "%d##next_%d", blk.nextFreeBlock, i);
					if (ImGui::Selectable(label, false, ImGuiSelectableFlags_AllowDoubleClick, ImVec2(0,0))) {
						selectedBlock = blk.nextFreeBlock;
						highlightTime = highlightDuration;
					}
				} else {
					ImGui::Text("%d", blk.nextFreeBlock);
				}

				// --- Clickable Parent block ---
				ImGui::TableSetColumnIndex(9);
				if (blk.parentBlockIndex >= 0 && blk.parentBlockIndex < MemoryMasterBlock.nbTotalBlocks) {
					char label[32];
					snprintf(label, sizeof(label), "%d##parent_%d", blk.parentBlockIndex, i);
					if (ImGui::Selectable(label, false, ImGuiSelectableFlags_AllowDoubleClick, ImVec2(0,0))) {
						selectedBlock = blk.parentBlockIndex;
						highlightTime = highlightDuration;
					}
				} else {
					ImGui::Text("%d", blk.parentBlockIndex);
				}

				ImGui::TableSetColumnIndex(10); ImGui::Text("%d", blk.blockIndex);

				// --- Clickable FreeListHead block ---
				ImGui::TableSetColumnIndex(11);
				if (blk.freeListHead >= 0 && blk.freeListHead < MemoryMasterBlock.nbTotalBlocks) {
					char label[32];
					snprintf(label, sizeof(label), "%d##flh_%d", blk.freeListHead, i);
					if (ImGui::Selectable(label, false, ImGuiSelectableFlags_AllowDoubleClick, ImVec2(0,0))) {
						selectedBlock = blk.freeListHead;
						highlightTime = highlightDuration;
					}
				} else {
					ImGui::Text("%d", blk.freeListHead);
				}

				ImGui::TableSetColumnIndex(12); ImGui::Text("%d", blk.freeListHeadAllocatedTo);

				// Optional: Tooltip for more info
				if (ImGui::IsItemHovered()) {
					ImGui::BeginTooltip();
					ImGui::Text("funcTableID: %d", blk.funcTableID);
					ImGui::Text("maxStackLevel: %d", blk.maxStackLevel);
					ImGui::EndTooltip();
				}
			}
			ImGui::EndTable();
		}
	}
}

void Debug::Memory::ShowMenu(bool* bOpen)
{
	ImGui::Begin("Memory", bOpen);

	ImGui::Text("Main Heap Size: 0x%x", edMemGetMemoryAvailable(TO_HEAP(H_MAIN)));
	ImGui::Text("Memkit Heap Size: 0x%x", edMemGetMemoryAvailable(TO_HEAP(H_MEMKIT)));
	ImGui::Text("Scratchpad Heap Size: 0x%x", edMemGetMemoryAvailable(TO_HEAP(H_SCRATCHPAD)));
	ImGui::Text("Video Heap Size: 0x%x", edMemGetMemoryAvailable(TO_HEAP(H_VIDEO)));

	if (ImGui::Button("Dump Memory")) {
		std::ofstream inFile("memory_dump.bin", std::ios::binary);
		assert(inFile.is_open());

		const char* pMem = reinterpret_cast<const char*>(edmemGetMainHeader());
		inFile.write(pMem, sizeof(VirtualMemory));
		inFile.close();
	}

	if (ImGui::Button("Load Memory")) {
		std::ifstream inFile("memory_dump.bin", std::ios::binary);
		assert(inFile.is_open());

		char* pMem = reinterpret_cast<char*>(edmemGetMainHeader());
		inFile.read(pMem, sizeof(VirtualMemory));
		inFile.close();
	}

	ImGui::Spacing();
	ImGui::Spacing();

	ImGui_ShowAllocatorBlocks();

	if (ImGui::CollapsingHeader("Vulkan")) {
		ImGui::Text("Allocations:");
		ImGui::Text("VkAllocations VK_SYSTEM_ALLOCATION_SCOPE_COMMAND: %u", GetAllocationCount(VK_SYSTEM_ALLOCATION_SCOPE_COMMAND));
		ImGui::Text("VkAllocations VK_SYSTEM_ALLOCATION_SCOPE_OBJECT: %u", GetAllocationCount(VK_SYSTEM_ALLOCATION_SCOPE_OBJECT));
		ImGui::Text("VkAllocations VK_SYSTEM_ALLOCATION_SCOPE_CACHE: %u", GetAllocationCount(VK_SYSTEM_ALLOCATION_SCOPE_CACHE));
		ImGui::Text("VkAllocations VK_SYSTEM_ALLOCATION_SCOPE_DEVICE: %u", GetAllocationCount(VK_SYSTEM_ALLOCATION_SCOPE_DEVICE));
		ImGui::Text("VkAllocations VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE: %u", GetAllocationCount(VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE));
	}

	ImGui::End();
}

