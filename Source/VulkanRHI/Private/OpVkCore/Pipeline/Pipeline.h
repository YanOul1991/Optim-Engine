#pragma once

#include "OpVkCommon/Minimal.h"

#include <filesystem>
#include <fstream>

/**
 * Vulkan Pipeline documentaion:
 * https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/02_Graphics_pipeline_basics/00_Introduction.html
 *
 *
 */

namespace Optim::VKPipeline
{

/**
 * Get the binary data of a compiled shader (.spv) inside of a
 * `std::vector<char>` object. If the file cannot be
 * opened throws an exception.
 */
inline std::vector<char> LoadCompiledShader(const char* filename)
{
  // std::ios::ate    -> Start reading at the end of file.
  // std::ios::binary -> Read file as binary format
  std::ifstream file(filename, std::ios::ate | std::ios::binary);

  if (!file.is_open()) {
    throw std::runtime_error("Failed to open shader file.");
  }

  std::vector<char> buffer(file.tellg());
  file.seekg(0, std::ios::beg);
  file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
  file.close();

  return buffer;
}

[[nodiscard]]
inline vk::raii::ShaderModule CreateVkShaderModule(const std::vector<char>& bytecode, const vk::raii::Device& device)
{
  vk::ShaderModuleCreateInfo createInfo{
    .codeSize = bytecode.size() * sizeof(char),
    .pCode    = reinterpret_cast<const uint32*>(bytecode.data())
  };

  return vk::raii::ShaderModule(device, createInfo);
}

void CreateShaderStage(const vk::raii::ShaderModule& shaderModule)
{
  vk::PipelineShaderStageCreateInfo vertexShaderStageInfo{
    .stage  = vk::ShaderStageFlagBits::eVertex,
    .module = shaderModule,
    .pName  = "VertexMain"
  };

  vk::PipelineShaderStageCreateInfo fragmentShaderStageInfo{
    .stage  = vk::ShaderStageFlagBits::eFragment,
    .module = shaderModule,
    .pName  = "FragmentMain"
  };

  vk::PipelineShaderStageCreateInfo shaderStages[] = {
    vertexShaderStageInfo,
    fragmentShaderStageInfo
  };
}

} // namespace Optim::VK