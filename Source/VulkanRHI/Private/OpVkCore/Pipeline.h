#pragma once

#include "OpVkCommon/Minimal.h"
#include "OpVkTypes/SwapChainContext.h"

#include <filesystem>
#include <fstream>

/**
 * Vulkan Pipeline documentaion:
 * https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/02_Graphics_pipeline_basics/00_Introduction.html
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

/**
 * @brief
 * Create a shader module from compiled SPIR-V byte code.
 */
[[nodiscard]]
inline vk::raii::ShaderModule CreateVkShaderModule(const std::vector<char>& bytecode, const vk::raii::Device& device)
{
  vk::ShaderModuleCreateInfo createInfo{
    .codeSize = bytecode.size() * sizeof(char),
    .pCode    = reinterpret_cast<const uint32*>(bytecode.data())
  };

  return vk::raii::ShaderModule(device, createInfo);
}

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
inline std::vector<vk::PipelineShaderStageCreateInfo> CreateVkPipelineShaderStageCreateInfoList(const vk::raii::ShaderModule& shaderModule)
{
  // The shader stages entrypoints names are fixed and program
  // looks for those specefic ones:
  //  Vertex Shader         -> VertexMain
  //  Fragment/Pixel Shader -> FragmentMain
  //
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
 * @brief
 * Vulkan Tutorial:
 * https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/02_Graphics_pipeline_basics/02_Fixed_functions.html
 *
 * Create a `VkPipeline` object for given shader stages.
 */
inline vk::raii::Pipeline CreateVulkanPipeline(
  const SwapChainContext&                         swapChainCtx,
  std::vector<vk::PipelineShaderStageCreateInfo>& stagesInfo,
  const vk::raii::Device&                         device)
{
  // Dynamic States allows to change pipeline configurations when we record
  // commands.
  //
  // For example, settings the viewport and scissors as dyanmic state, allows
  // to call some specefic functions such as `vkCmdSetViewport` or
  // `vkCmdSetScissor` after calling `CommandBuffer::begin`.
  //
  // It will allow to update those without having to recreate the whole pipeline,
  // when the render surface's size does change.
  std::vector<vk::DynamicState> dynamicStates = {
    vk::DynamicState::eViewport,
    vk::DynamicState::eScissor
  };
  vk::PipelineDynamicStateCreateInfo dynamicState{
    .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
    .pDynamicStates    = dynamicStates.data()
  };

  /**
   * VERTEX INPUT
   *
   * REFERENCE: https://docs.vulkan.org/refpages/latest/refpages/source/VkPipelineVertexInputStateCreateInfo.html
   *
   * For now used as is.
   */
  vk::PipelineVertexInputStateCreateInfo vertexInputInfo;

  /**
   * INPUT ASSEMBLY
   *
   * REFERENCE: https://docs.vulkan.org/refpages/latest/refpages/source/VkPipelineInputAssemblyStateCreateInfo.html
   */
  vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo{
    .topology = vk::PrimitiveTopology::eTriangleList,
  };

  /**
   * VEIEWPORTS & SCISSORS
   *
   * REFERENCE: https://docs.vulkan.org/refpages/latest/refpages/source/VkPipelineViewportStateCreateInfo.html
   *
   * Because we are using dynamic states, we dont need to use  `pViewports` and
   * `pScissors` to the `PipelineViewportStateCreateInfo` struct, as it will
   * be ignored by Vulkan anyway.
   *
   * Commented the viewport and scissor structs, they can be ignored, because
   * of dyanmic state use.
   */

  // vk::Viewport viewport{ 0.0f, 0.0f, static_cast<float>(swapChainCtx.swapChainExtent.width), static_cast<float>(swapChainCtx.swapChainExtent.height), 0.0f, 1.0f };
  // vk::Rect2D   scissor {
  //   vk::Offset2D{ 0, 0 },
  //   swapChainCtx.swapChainExtent
  // };
  vk::PipelineViewportStateCreateInfo viewportState{
    .viewportCount = 1,
    .scissorCount  = 1
  };

  /**
   * RASTERIZER
   *
   * REFERENCE: https://docs.vulkan.org/refpages/latest/refpages/source/VkPipelineRasterizationStateCreateInfo.html
   */
  vk::PipelineRasterizationStateCreateInfo rasterizer{
    .depthClampEnable        = vk::False,
    .rasterizerDiscardEnable = vk::False,
    .polygonMode             = vk::PolygonMode::eFill,
    .cullMode                = vk::CullModeFlagBits::eBack,
    .frontFace               = vk::FrontFace::eClockwise,
    .depthBiasEnable         = vk::False,
    .depthBiasConstantFactor = 0.0f,
    .depthBiasClamp          = 0.0f,
    .depthBiasSlopeFactor    = 0.0f,
    .lineWidth               = 1.0f
  };

  /**
   * MULTISAMPLING
   *
   * REFRRENCE: https://docs.vulkan.org/refpages/latest/refpages/source/VkPipelineMultisampleStateCreateInfo.html
   */
  vk::PipelineMultisampleStateCreateInfo multisampling{
    .rasterizationSamples = vk::SampleCountFlagBits::e1,
    .sampleShadingEnable  = vk::False
  };

  /**
   * DEPTH & STENCIL TESTING
   *
   * REFERENCE: https://registry.khronos.org/vulkan/specs/latest/man/html/VkPipelineDepthStencilStateCreateInfo.html
   *
   * @todo
   * Come back to this eventually
   */
  vk::PipelineDepthStencilStateCreateInfo depthStencil;

  /**
   * COLOR BLENDING
   *
   * REFERENCE: https://docs.vulkan.org/refpages/latest/refpages/source/VkPipelineColorBlendAttachmentState.html
   *
   */
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
  vk::PipelineLayoutCreateInfo pipelineLayoutInfo = { 
    .setLayoutCount = 0, 
    .pushConstantRangeCount = 0 
  };
  pipelineLayout = vk::raii::PipelineLayout(device, pipelineLayoutInfo);

  // GraphicsPipelineCreateInfo
  vk::GraphicsPipelineCreateInfo graphicsPipelineCreateInfo = {
    .stageCount          = static_cast<uint32_t>(stagesInfo.size()),
    .pStages             = stagesInfo.data(),
    .pVertexInputState   = &vertexInputInfo,
    .pInputAssemblyState = &inputAssemblyInfo,
    .pViewportState      = &viewportState,
    .pRasterizationState = &rasterizer,
    .pMultisampleState   = &multisampling,
    .pColorBlendState    = &colorBlending,
    .pDynamicState       = &dynamicState,
    .layout              = pipelineLayout,
    .renderPass          = nullptr
  };
  // Pipeline rendering create info
  vk::PipelineRenderingCreateInfo pipelineRenderingCreateInfo = {
    .colorAttachmentCount    = 1,
    .pColorAttachmentFormats = &swapChainCtx.swapChainImageFormat.format
  };

  vk::StructureChain<vk::GraphicsPipelineCreateInfo, vk::PipelineRenderingCreateInfo> pipelineCreateInfoChain = {
    graphicsPipelineCreateInfo,
    pipelineRenderingCreateInfo
  };
  // pipelineCreateInfoChain.get<vk::GraphicsPipelineCreateInfo>()  = graphicsPipelineCreateInfo;
  // pipelineCreateInfoChain.get<vk::PipelineRenderingCreateInfo>() = pipelineRenderingCreateInfo;

  // vk::StructureChain<vk::GraphicsPipelineCreateInfo, vk::PipelineRenderingCreateInfo> pipelineCreateInfoChain = {
  //   { .stageCount          = static_cast<uint32_t>(stagesInfo.size()),
  //    .pStages             = stagesInfo.data(),
  //    .pVertexInputState   = &vertexInputInfo,
  //    .pInputAssemblyState = &inputAssemblyInfo,
  //    .pViewportState      = &viewportState,
  //    .pRasterizationState = &rasterizer,
  //    .pMultisampleState   = &multisampling,
  //    .pColorBlendState    = &colorBlending,
  //    .pDynamicState       = &dynamicState,
  //    .layout              = pipelineLayout,
  //    .renderPass          = nullptr },
  //   { .colorAttachmentCount = 1, .pColorAttachmentFormats = &swapChainCtx.swapChainImageFormat.format }
  // };

  return vk::raii::Pipeline(device, nullptr, pipelineCreateInfoChain.get<vk::GraphicsPipelineCreateInfo>());
}

} // namespace Optim::VKPipeline