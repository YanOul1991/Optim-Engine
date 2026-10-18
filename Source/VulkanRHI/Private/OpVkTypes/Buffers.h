#pragma once

#include "OpVkCommon/Minimal.h"

template <typename T>
inline constexpr size_t BinSize(const std::vector<T>& v) {
  return sizeof(T) * std::size(v);
}

template <typename T, size_t N>
inline constexpr size_t BinSize(const std::array<T, N>& v) {
  return sizeof(T) * std::size(v);
}

class BufferData final
{
 public:
  OPTIM_VK_RAII_TYPE(BufferData)

  BufferData(
    vk::raii::Device&         device,
    vk::raii::PhysicalDevice& physicalDevice,
    vk::DeviceSize            deviceSize,
    vk::BufferUsageFlagBits   usage,
    vk::MemoryPropertyFlags   memoryPorperties)
  {
    // Buffer creation
    // https://docs.vulkan.org/refpages/latest/refpages/source/VkBufferCreateInfo.html#_document_notes
    vk::BufferCreateInfo bufferInfo{
      .size        = deviceSize,
      .usage       = usage,
      .sharingMode = vk::SharingMode::eExclusive
    };

    buffer = vk::raii::Buffer(device, bufferInfo);

    if (buffer == nullptr) {
      return;
    }

    // Get buffer's memory requirements
    vk::MemoryRequirements memoryReqs = buffer.getMemoryRequirements();

    vk::MemoryAllocateInfo memAllocInfo {
      .allocationSize  = memoryReqs.size,
      .memoryTypeIndex = FindMemoryTypeIndex(physicalDevice, memoryReqs.memoryTypeBits, memoryPorperties)
    };

    deviceMemory = vk::raii::DeviceMemory(device, memAllocInfo);

    if(deviceMemory == nullptr) {
      return;
    }

    buffer.bindMemory(*deviceMemory, 0);
  }

  void Map(const void* pInData, vk::DeviceSize size)
  {
    void* data = deviceMemory.mapMemory(0, size);
    memcpy(data, pInData, size);
    deviceMemory.unmapMemory();
  }

  bool IsValid() const noexcept { return buffer != nullptr && deviceMemory != nullptr; }
  vk::raii::Buffer& GetBuffer() { return buffer; }
  vk::raii::DeviceMemory& GetMemory() { return deviceMemory; }

 private:
  inline uint32 FindMemoryTypeIndex(const vk::raii::PhysicalDevice& physicalDevice, uint32 typeFilter, vk::MemoryPropertyFlags properties)
  {
    vk::PhysicalDeviceMemoryProperties memProperties = physicalDevice.getMemoryProperties();
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
      if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
        return i;
      }
    }
    throw std::runtime_error("failed to find suitable memory type!");
  }

  vk::raii::Buffer       buffer       = nullptr;
  vk::raii::DeviceMemory deviceMemory = nullptr;
};


class VertexBuffer final
{
 private:
  BufferData gpuBuffer;
  uint32     vertexCount = 0;

 public:
  OPTIM_VK_RAII_TYPE(VertexBuffer)

  static constexpr const auto usage       = vk::BufferUsageFlagBits::eVertexBuffer;
  static constexpr const auto memoryFlags = vk::MemoryPropertyFlagBits::eHostVisible |
                                            vk::MemoryPropertyFlagBits::eHostCoherent;

  VertexBuffer(vk::raii::Device& device, vk::raii::PhysicalDevice& physicalDevice)
  {
    // Cannot directly call member initalizer list for BufferData constructor because the vertices
    // buffer will not be instanciated in time, therefore the size value being passed is of 0 which
    // causes a runtime error.
    // For now because of testing, it will be fine, but in the future the list of vertices may
    // not be owner directly by this class, in which case we will see again about initalized list.
    gpuBuffer = BufferData(device, physicalDevice, BinSize(vertices), usage, memoryFlags);
    if(!gpuBuffer.IsValid()) {
      throw std::runtime_error("Failed to create vertex buffer");
    }
    gpuBuffer.Map(vertices.data(), BinSize(vertices));
    vertexCount = static_cast<uint32>(vertices.size());
  }

  uint32 Count() const noexcept { return vertexCount; }

  BufferData& GetBufferData() { return gpuBuffer; }

  // TEST FIELD
  std::vector<Vertex> vertices = {
    { { 0.0f, -0.5f }, { 1.0f, 1.0f, 1.0f } },
    {  { 0.5f, 0.5f }, { 1.0f, 1.0f, 1.0f } },
    { { -0.5f, 0.5f }, { 1.0f, 1.0f, 1.0f } }
  };
};