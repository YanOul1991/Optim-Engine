#pragma once

#include "OpVkCommon/Minimal.h"
#include "OpVkTypes/SwapChainContext.h"

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

inline std::vector<vk::PipelineShaderStageCreateInfo> CreateVkPipelineShaderStageCreateInfoList(const vk::raii::ShaderModule& shaderModule)
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

  return {
    vertexShaderStageInfo,
    fragmentShaderStageInfo
  };
}

/**
 * Vulkan Tutorial:
 * https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/02_Graphics_pipeline_basics/02_Fixed_functions.html
 */
inline vk::raii::Pipeline CreateVulkanPipeline(
  const SwapChainContext&                         swapChainCtx,
  std::vector<vk::PipelineShaderStageCreateInfo>& stagesInfo,
  const vk::raii::Device&                         device)
{
  std::vector<vk::DynamicState> dynamicStates = {
    vk::DynamicState::eViewport,
    vk::DynamicState::eScissor
  };

  vk::PipelineDynamicStateCreateInfo dynamicState{
    .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
    .pDynamicStates    = dynamicStates.data()
  };

  // VERTEX INPUT
  vk::PipelineVertexInputStateCreateInfo vertexInputInfo;

  // INPUT ASSEMBLY
  vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo{
    .topology = vk::PrimitiveTopology::eTriangleList,
  };

  // VIEWPORTS & SCISSORS
  vk::Viewport viewport{ 0.0f, 0.0f, static_cast<float>(swapChainCtx.swapChainExtent.width), static_cast<float>(swapChainCtx.swapChainExtent.height), 0.0f, 1.0f };
  vk::Rect2D   scissor{
    vk::Offset2D{ 0, 0 },
    swapChainCtx.swapChainExtent
  };

  vk::PipelineViewportStateCreateInfo viewportState{
    .viewportCount = 1,
    .scissorCount  = 1
  };

  // RASTERIZER
  vk::PipelineRasterizationStateCreateInfo rasterizer{
    .depthClampEnable        = vk::False,
    .rasterizerDiscardEnable = vk::False,
    .polygonMode             = vk::PolygonMode::eFill,
    .cullMode                = vk::CullModeFlagBits::eBack,
    .frontFace               = vk::FrontFace::eClockwise,
    .depthBiasEnable         = vk::False,
    .lineWidth               = 1.0f
  };

  // MULTISAMPLING
  vk::PipelineMultisampleStateCreateInfo multisampling{
    .rasterizationSamples = vk::SampleCountFlagBits::e1,
    .sampleShadingEnable  = vk::False
  };

  // DEPTH & STENCIL TESTING
  vk::PipelineDepthStencilStateCreateInfo depthStencil;

  // COLOR BLENDING
  vk::PipelineColorBlendAttachmentState colorBlendAttachement{
    .blendEnable         = vk::True,
    .srcColorBlendFactor = vk::BlendFactor::eSrcAlpha,
    .dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
    .colorBlendOp        = vk::BlendOp::eAdd,
    .srcAlphaBlendFactor = vk::BlendFactor::eOne,
    .dstAlphaBlendFactor = vk::BlendFactor::eZero,
    .colorWriteMask      = vk::ColorComponentFlagBits::eR |
                           vk::ColorComponentFlagBits::eG |
                           vk::ColorComponentFlagBits::eB |
                           vk::ColorComponentFlagBits::eA
  };

  vk::PipelineColorBlendStateCreateInfo colorBlending{
    .logicOpEnable   = vk::False,
    .logicOp         = vk::LogicOp::eCopy,
    .attachmentCount = 1,
    .pAttachments    = &colorBlendAttachement
  };

  // PIPELINE LAYOUT
  vk::raii::PipelineLayout     pipelineLayout = nullptr;
  vk::PipelineLayoutCreateInfo pipelineLayoutInfo{ .setLayoutCount = 0, .pushConstantRangeCount = 0 };
  pipelineLayout = vk::raii::PipelineLayout(device, pipelineLayoutInfo);

  // Pipeline rendering create info
  vk::PipelineRenderingCreateInfo pipelineRenderingCreateInfo{
    .colorAttachmentCount    = 1,
    .pColorAttachmentFormats = &swapChainCtx.swapChainImageFormat.format
  };

  vk::StructureChain<vk::GraphicsPipelineCreateInfo, vk::PipelineRenderingCreateInfo> pipelineCreateInfoChain = {
    { .stageCount          = 2,
     .pStages             = stagesInfo.data(),
     .pVertexInputState   = &vertexInputInfo,
     .pInputAssemblyState = &inputAssemblyInfo,
     .pViewportState      = &viewportState,
     .pRasterizationState = &rasterizer,
     .pMultisampleState   = &multisampling,
     .pColorBlendState    = &colorBlending,
     .pDynamicState       = &dynamicState,
     .layout              = pipelineLayout,
     .renderPass          = nullptr },
    { .colorAttachmentCount = 1, .pColorAttachmentFormats = &swapChainCtx.swapChainImageFormat.format }
  };

  // std::cout << "Pipeline created ???\n";

  return vk::raii::Pipeline(device, nullptr, pipelineCreateInfoChain.get<vk::GraphicsPipelineCreateInfo>());
}

} // namespace Optim::VKPipeline