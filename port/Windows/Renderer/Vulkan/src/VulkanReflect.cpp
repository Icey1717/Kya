#include "VulkanReflect.h"
#include "spirv_reflect.h"
#include "common.h"
#include <algorithm>

#define ENABLE_LOG 0

// Returns the size in bytes of the provided VkFormat.
// As this is only intended for vertex attribute formats, not all VkFormats are
// supported.
static uint32_t FormatSize(VkFormat format) {
	uint32_t result = 0;
	switch (format) {
	case VK_FORMAT_UNDEFINED:
		result = 0;
		break;
	case VK_FORMAT_R4G4_UNORM_PACK8:
		result = 1;
		break;
	case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
		result = 2;
		break;
	case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
		result = 2;
		break;
	case VK_FORMAT_R5G6B5_UNORM_PACK16:
		result = 2;
		break;
	case VK_FORMAT_B5G6R5_UNORM_PACK16:
		result = 2;
		break;
	case VK_FORMAT_R5G5B5A1_UNORM_PACK16:
		result = 2;
		break;
	case VK_FORMAT_B5G5R5A1_UNORM_PACK16:
		result = 2;
		break;
	case VK_FORMAT_A1R5G5B5_UNORM_PACK16:
		result = 2;
		break;
	case VK_FORMAT_R8_UNORM:
		result = 1;
		break;
	case VK_FORMAT_R8_SNORM:
		result = 1;
		break;
	case VK_FORMAT_R8_USCALED:
		result = 1;
		break;
	case VK_FORMAT_R8_SSCALED:
		result = 1;
		break;
	case VK_FORMAT_R8_UINT:
		result = 1;
		break;
	case VK_FORMAT_R8_SINT:
		result = 1;
		break;
	case VK_FORMAT_R8_SRGB:
		result = 1;
		break;
	case VK_FORMAT_R8G8_UNORM:
		result = 2;
		break;
	case VK_FORMAT_R8G8_SNORM:
		result = 2;
		break;
	case VK_FORMAT_R8G8_USCALED:
		result = 2;
		break;
	case VK_FORMAT_R8G8_SSCALED:
		result = 2;
		break;
	case VK_FORMAT_R8G8_UINT:
		result = 2;
		break;
	case VK_FORMAT_R8G8_SINT:
		result = 2;
		break;
	case VK_FORMAT_R8G8_SRGB:
		result = 2;
		break;
	case VK_FORMAT_R8G8B8_UNORM:
		result = 3;
		break;
	case VK_FORMAT_R8G8B8_SNORM:
		result = 3;
		break;
	case VK_FORMAT_R8G8B8_USCALED:
		result = 3;
		break;
	case VK_FORMAT_R8G8B8_SSCALED:
		result = 3;
		break;
	case VK_FORMAT_R8G8B8_UINT:
		result = 3;
		break;
	case VK_FORMAT_R8G8B8_SINT:
		result = 3;
		break;
	case VK_FORMAT_R8G8B8_SRGB:
		result = 3;
		break;
	case VK_FORMAT_B8G8R8_UNORM:
		result = 3;
		break;
	case VK_FORMAT_B8G8R8_SNORM:
		result = 3;
		break;
	case VK_FORMAT_B8G8R8_USCALED:
		result = 3;
		break;
	case VK_FORMAT_B8G8R8_SSCALED:
		result = 3;
		break;
	case VK_FORMAT_B8G8R8_UINT:
		result = 3;
		break;
	case VK_FORMAT_B8G8R8_SINT:
		result = 3;
		break;
	case VK_FORMAT_B8G8R8_SRGB:
		result = 3;
		break;
	case VK_FORMAT_R8G8B8A8_UNORM:
		result = 4;
		break;
	case VK_FORMAT_R8G8B8A8_SNORM:
		result = 4;
		break;
	case VK_FORMAT_R8G8B8A8_USCALED:
		result = 4;
		break;
	case VK_FORMAT_R8G8B8A8_SSCALED:
		result = 4;
		break;
	case VK_FORMAT_R8G8B8A8_UINT:
		result = 4;
		break;
	case VK_FORMAT_R8G8B8A8_SINT:
		result = 4;
		break;
	case VK_FORMAT_R8G8B8A8_SRGB:
		result = 4;
		break;
	case VK_FORMAT_B8G8R8A8_UNORM:
		result = 4;
		break;
	case VK_FORMAT_B8G8R8A8_SNORM:
		result = 4;
		break;
	case VK_FORMAT_B8G8R8A8_USCALED:
		result = 4;
		break;
	case VK_FORMAT_B8G8R8A8_SSCALED:
		result = 4;
		break;
	case VK_FORMAT_B8G8R8A8_UINT:
		result = 4;
		break;
	case VK_FORMAT_B8G8R8A8_SINT:
		result = 4;
		break;
	case VK_FORMAT_B8G8R8A8_SRGB:
		result = 4;
		break;
	case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
		result = 4;
		break;
	case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
		result = 4;
		break;
	case VK_FORMAT_A8B8G8R8_USCALED_PACK32:
		result = 4;
		break;
	case VK_FORMAT_A8B8G8R8_SSCALED_PACK32:
		result = 4;
		break;
	case VK_FORMAT_A8B8G8R8_UINT_PACK32:
		result = 4;
		break;
	case VK_FORMAT_A8B8G8R8_SINT_PACK32:
		result = 4;
		break;
	case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
		result = 4;
		break;
	case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
		result = 4;
		break;
	case VK_FORMAT_A2R10G10B10_SNORM_PACK32:
		result = 4;
		break;
	case VK_FORMAT_A2R10G10B10_USCALED_PACK32:
		result = 4;
		break;
	case VK_FORMAT_A2R10G10B10_SSCALED_PACK32:
		result = 4;
		break;
	case VK_FORMAT_A2R10G10B10_UINT_PACK32:
		result = 4;
		break;
	case VK_FORMAT_A2R10G10B10_SINT_PACK32:
		result = 4;
		break;
	case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
		result = 4;
		break;
	case VK_FORMAT_A2B10G10R10_SNORM_PACK32:
		result = 4;
		break;
	case VK_FORMAT_A2B10G10R10_USCALED_PACK32:
		result = 4;
		break;
	case VK_FORMAT_A2B10G10R10_SSCALED_PACK32:
		result = 4;
		break;
	case VK_FORMAT_A2B10G10R10_UINT_PACK32:
		result = 4;
		break;
	case VK_FORMAT_A2B10G10R10_SINT_PACK32:
		result = 4;
		break;
	case VK_FORMAT_R16_UNORM:
		result = 2;
		break;
	case VK_FORMAT_R16_SNORM:
		result = 2;
		break;
	case VK_FORMAT_R16_USCALED:
		result = 2;
		break;
	case VK_FORMAT_R16_SSCALED:
		result = 2;
		break;
	case VK_FORMAT_R16_UINT:
		result = 2;
		break;
	case VK_FORMAT_R16_SINT:
		result = 2;
		break;
	case VK_FORMAT_R16_SFLOAT:
		result = 2;
		break;
	case VK_FORMAT_R16G16_UNORM:
		result = 4;
		break;
	case VK_FORMAT_R16G16_SNORM:
		result = 4;
		break;
	case VK_FORMAT_R16G16_USCALED:
		result = 4;
		break;
	case VK_FORMAT_R16G16_SSCALED:
		result = 4;
		break;
	case VK_FORMAT_R16G16_UINT:
		result = 4;
		break;
	case VK_FORMAT_R16G16_SINT:
		result = 4;
		break;
	case VK_FORMAT_R16G16_SFLOAT:
		result = 4;
		break;
	case VK_FORMAT_R16G16B16_UNORM:
		result = 6;
		break;
	case VK_FORMAT_R16G16B16_SNORM:
		result = 6;
		break;
	case VK_FORMAT_R16G16B16_USCALED:
		result = 6;
		break;
	case VK_FORMAT_R16G16B16_SSCALED:
		result = 6;
		break;
	case VK_FORMAT_R16G16B16_UINT:
		result = 6;
		break;
	case VK_FORMAT_R16G16B16_SINT:
		result = 6;
		break;
	case VK_FORMAT_R16G16B16_SFLOAT:
		result = 6;
		break;
	case VK_FORMAT_R16G16B16A16_UNORM:
		result = 8;
		break;
	case VK_FORMAT_R16G16B16A16_SNORM:
		result = 8;
		break;
	case VK_FORMAT_R16G16B16A16_USCALED:
		result = 8;
		break;
	case VK_FORMAT_R16G16B16A16_SSCALED:
		result = 8;
		break;
	case VK_FORMAT_R16G16B16A16_UINT:
		result = 8;
		break;
	case VK_FORMAT_R16G16B16A16_SINT:
		result = 8;
		break;
	case VK_FORMAT_R16G16B16A16_SFLOAT:
		result = 8;
		break;
	case VK_FORMAT_R32_UINT:
		result = 4;
		break;
	case VK_FORMAT_R32_SINT:
		result = 4;
		break;
	case VK_FORMAT_R32_SFLOAT:
		result = 4;
		break;
	case VK_FORMAT_R32G32_UINT:
		result = 8;
		break;
	case VK_FORMAT_R32G32_SINT:
		result = 8;
		break;
	case VK_FORMAT_R32G32_SFLOAT:
		result = 8;
		break;
	case VK_FORMAT_R32G32B32_UINT:
		result = 12;
		break;
	case VK_FORMAT_R32G32B32_SINT:
		result = 12;
		break;
	case VK_FORMAT_R32G32B32_SFLOAT:
		result = 12;
		break;
	case VK_FORMAT_R32G32B32A32_UINT:
		result = 16;
		break;
	case VK_FORMAT_R32G32B32A32_SINT:
		result = 16;
		break;
	case VK_FORMAT_R32G32B32A32_SFLOAT:
		result = 16;
		break;
	case VK_FORMAT_R64_UINT:
		result = 8;
		break;
	case VK_FORMAT_R64_SINT:
		result = 8;
		break;
	case VK_FORMAT_R64_SFLOAT:
		result = 8;
		break;
	case VK_FORMAT_R64G64_UINT:
		result = 16;
		break;
	case VK_FORMAT_R64G64_SINT:
		result = 16;
		break;
	case VK_FORMAT_R64G64_SFLOAT:
		result = 16;
		break;
	case VK_FORMAT_R64G64B64_UINT:
		result = 24;
		break;
	case VK_FORMAT_R64G64B64_SINT:
		result = 24;
		break;
	case VK_FORMAT_R64G64B64_SFLOAT:
		result = 24;
		break;
	case VK_FORMAT_R64G64B64A64_UINT:
		result = 32;
		break;
	case VK_FORMAT_R64G64B64A64_SINT:
		result = 32;
		break;
	case VK_FORMAT_R64G64B64A64_SFLOAT:
		result = 32;
		break;
	case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
		result = 4;
		break;
	case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32:
		result = 4;
		break;

	default:
		break;
	}
	return result;
}

static void GetPushConstantRanges(SpvReflectShaderModule& module, std::vector<VkPushConstantRange>& pushConstantRanges, const SpvReflectShaderStageFlagBits& stageFlags) {
	uint32_t pushConstantBlockCount = 0;
	spvReflectEnumeratePushConstantBlocks(&module, &pushConstantBlockCount, NULL);

	std::vector<SpvReflectBlockVariable*> pushConstantBlocks(pushConstantBlockCount);
	spvReflectEnumeratePushConstantBlocks(&module, &pushConstantBlockCount, pushConstantBlocks.data());

	for (const auto& block : pushConstantBlocks) {
		VkPushConstantRange pushConstantRange = {};
		pushConstantRange.stageFlags = stageFlags;
		pushConstantRange.offset = block->offset;
		pushConstantRange.size = block->size;
		pushConstantRanges.push_back(pushConstantRange);
	}
}

ReflectData CreateReflectionData(const std::vector<char>& shaderCode) {
	// Create a ShaderModule instance
	SpvReflectShaderModule module = {};
	SpvReflectResult result =
		spvReflectCreateShaderModule(shaderCode.size(), shaderCode.data(), &module);
	assert(result == SPV_REFLECT_RESULT_SUCCESS);

	uint32_t count = 0;
	result = spvReflectEnumerateDescriptorSets(&module, &count, NULL);
	assert(result == SPV_REFLECT_RESULT_SUCCESS);

	std::vector<SpvReflectDescriptorSet*> sets(count);
	result = spvReflectEnumerateDescriptorSets(&module, &count, sets.data());
	assert(result == SPV_REFLECT_RESULT_SUCCESS);

	ReflectData reflectData;
	auto& set_layouts = reflectData.GetLayouts();
	set_layouts = std::vector<DescriptorSetLayoutData>(sets.size(), DescriptorSetLayoutData{});

	// Demonstrates how to generate all necessary data structures to create a
	// VkDescriptorSetLayout for each descriptor set in this shader.

	for (size_t i_set = 0; i_set < sets.size(); ++i_set) {
		const SpvReflectDescriptorSet& refl_set = *(sets[i_set]);
		DescriptorSetLayoutData& layout = set_layouts[i_set];
		layout.bindings.resize(refl_set.binding_count);
		for (uint32_t i_binding = 0; i_binding < refl_set.binding_count;
			++i_binding) {
			const SpvReflectDescriptorBinding& refl_binding =
				*(refl_set.bindings[i_binding]);
			VkDescriptorSetLayoutBinding& layout_binding = layout.bindings[i_binding];
			layout_binding.binding = refl_binding.binding;
			layout_binding.descriptorType =
				static_cast<VkDescriptorType>(refl_binding.descriptor_type);
			layout_binding.descriptorCount = 1;
			for (uint32_t i_dim = 0; i_dim < refl_binding.array.dims_count; ++i_dim) {
				layout_binding.descriptorCount *= refl_binding.array.dims[i_dim];
			}
			layout_binding.stageFlags =
				static_cast<VkShaderStageFlagBits>(module.shader_stage);
		}
		layout.setNumber = refl_set.set;
	}

	count = 0;
	result = spvReflectEnumerateInputVariables(&module, &count, NULL);
	assert(result == SPV_REFLECT_RESULT_SUCCESS);

	std::vector<SpvReflectInterfaceVariable*> input_vars(count);
	result = spvReflectEnumerateInputVariables(&module, &count, input_vars.data());
	assert(result == SPV_REFLECT_RESULT_SUCCESS);

	if (module.shader_stage == SPV_REFLECT_SHADER_STAGE_VERTEX_BIT) {
		// Demonstrates how to generate all necessary data structures to populate
		// a VkPipelineVertexInputStateCreateInfo structure, given the module's
		// expected input variables.
		//
		// Simplifying assumptions:
		// - All vertex input attributes are sourced from a single vertex buffer,
		//   bound to VB slot 0.
		// - Each vertex's attribute are laid out in ascending order by location.
		// - The format of each attribute matches its usage in the shader;
		//   float4 -> VK_FORMAT_R32G32B32A32_FLOAT, etc. No attribute compression
		//   is applied.
		// - All attributes are provided per-vertex, not per-instance.
		VkVertexInputBindingDescription& binding_description = reflectData.bindingDescription;
		binding_description = {};
		binding_description.binding = 0;
		binding_description.stride = 0;  // computed below
		binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		std::vector<VkVertexInputAttributeDescription>& attribute_descriptions = reflectData.GetAttributes();
		attribute_descriptions.reserve(input_vars.size());
		for (size_t i_var = 0; i_var < input_vars.size(); ++i_var) {
			const SpvReflectInterfaceVariable& refl_var = *(input_vars[i_var]);

#if ENABLE_LOG
			// Print information about the input variable
			printf("\nInput %d: name=%s, type=%s|%s, location=%d, offset: %d\n",
				i_var, refl_var.name, refl_var.type_description->type_name, refl_var.type_description->struct_member_name, refl_var.location, refl_var.word_offset.location);
			PrintInterfaceVariable(std::cout, module.source_language, refl_var, "    ");
#endif

			// ignore built-in variables
			if (refl_var.decoration_flags & SPV_REFLECT_DECORATION_BUILT_IN) {
				continue;
			}
			VkVertexInputAttributeDescription attr_desc{};
			attr_desc.location = refl_var.location;
			attr_desc.binding = binding_description.binding;
			attr_desc.format = static_cast<VkFormat>(refl_var.format);
			attr_desc.offset = 0;  // final offset computed below after sorting.
			attribute_descriptions.push_back(attr_desc);
		}
		// Sort attributes by location
		std::sort(std::begin(attribute_descriptions),
			std::end(attribute_descriptions),
			[](const VkVertexInputAttributeDescription& a,
				const VkVertexInputAttributeDescription& b) {
					return a.location < b.location;
			});
		// Compute final offsets of each attribute, and total vertex stride.
		for (auto& attribute : attribute_descriptions) {
			uint32_t format_size = FormatSize(attribute.format);
			attribute.offset = binding_description.stride;
			binding_description.stride += format_size;
		}
		// Nothing further is done with attribute_descriptions or
		// binding_description in this sample. A real application would probably
		// derive this information from its mesh format(s); a similar mechanism
		// could be used to ensure mesh/shader compatibility.
	}

#if ENABLE_LOG
	PrintModuleInfo(std::cout, module);
#endif

	reflectData.entryPointname = module.entry_point_name;

	GetPushConstantRanges(module, reflectData.pushConstants, module.shader_stage);

	// Log the descriptor set contents to stdout
	const char* t = "  ";
	const char* tt = "    ";

#if ENABLE_LOG
	std::cout << "Descriptor sets:"
		<< "\n";
#endif
	for (size_t index = 0; index < sets.size(); ++index) {
		auto p_set = sets[index];

		// descriptor sets can also be retrieved directly from the module, by set
		// index
		auto p_set2 = spvReflectGetDescriptorSet(&module, p_set->set, &result);
		assert(result == SPV_REFLECT_RESULT_SUCCESS);
		assert(p_set == p_set2);
		(void)p_set2;

#if ENABLE_LOG
		std::cout << t << index << ":"
			<< "\n";
		PrintDescriptorSet(std::cout, *p_set, tt);
		std::cout << "\n\n";
#endif
	}

	spvReflectDestroyShaderModule(&module);

	return reflectData;
}
