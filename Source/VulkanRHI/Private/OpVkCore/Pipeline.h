#pragma once

#include "OpVkCommon/Minimal.h"

/**
 * Vulkan tutorial simple pipeline creation documentaion:
 * https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/02_Graphics_pipeline_basics/00_Introduction.html
 * 
 * 
 * NOTES:
 *  - Could add `LoadCompiledShader` code directly inside of `CreateVkShaderModule`. 
 */

namespace Optim
{

/**
 * @brief
 * Get the binary data of a compiled shader (.spv) inside of a
 * `std::vector<char>` object. If the file cannot be
 * opened throws an exception.
 */
[[nodiscard]]
std::vector<char> LoadCompiledShader(const char* filename);

/**
 * @brief
 * Create a shader module from compiled SPIR-V byte code.
 */
[[nodiscard]]
vk::raii::ShaderModule CreateVkShaderModule(const std::vector<char>& bytecode, const vk::raii::Device& device);

/**
 * @brief
 * Create a list of shader stages info, from a given `VkShaderModule` object.
 *
 * With SPIR-V, shaders can and should normally have all shader stages
 * be defined in the same bytecode unit.
 *
 * For now only supports fertex and fragement shader stages.
 */
[[nodiscard]]
std::vector<vk::PipelineShaderStageCreateInfo> CreateVkPipelineShaderStageCreateInfoList(const vk::raii::ShaderModule& shaderModule);

/**
 * @brief
 * Vulkan Tutorial:
 * https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/02_Graphics_pipeline_basics/02_Fixed_functions.html
 *
 * Create a `VkPipeline` object for given shader stages.
 */
[[nodiscard]]
vk::raii::Pipeline CreateVkPipeline(
  const vk::raii::Device& device, 
  const vk::Format& colorAttachmentFormat, 
  const std::vector<vk::PipelineShaderStageCreateInfo>& stagesInfo,
  const vk::raii::PipelineLayout& pipelineLayout
);

} // namespace Optim::Vk