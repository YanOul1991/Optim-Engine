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

class GpuBuffer final
{
 private:
 vk::raii::Buffer       buffer       = nullptr;
 vk::raii::DeviceMemory deviceMemory = nullptr;
 
 static uint32 FindMemoryTypeIndex(const vk::raii::PhysicalDevice& physicalDevice, uint32 typeFilter, vk::MemoryPropertyFlags properties);

 public:
  OPTIM_VK_RAII_TYPE(GpuBuffer)

  GpuBuffer(
    vk::raii::Device&         device,
    vk::raii::PhysicalDevice& physicalDevice,
    vk::DeviceSize            deviceSize,
    vk::BufferUsageFlags      usageFlags,
    vk::MemoryPropertyFlags   memoryPorpertyFlag,
    void*                     pInitalData,
    vk::SharingMode           sharingMode = vk::SharingMode::eExclusive);

  inline void Map(const void* pData, vk::DeviceSize size)
  {
    void* data = deviceMemory.mapMemory(0, size);
    memcpy(data, pData, size);
    deviceMemory.unmapMemory();
  }

  // Getters
  bool IsValid() const noexcept          { return buffer != nullptr && deviceMemory != nullptr; }
  vk::raii::Buffer& Buffer()             { return buffer; }
  vk::raii::DeviceMemory& DeviceMemory() { return deviceMemory; }
};

class VertexBuffer final
{
 private:
  GpuBuffer gpuBuffer;
  uint32    vertexCount = 0;

 public:
  OPTIM_VK_RAII_TYPE(VertexBuffer)

  VertexBuffer(std::nullptr_t) {}

  static constexpr vk::BufferUsageFlags usageFlags = {
    vk::BufferUsageFlagBits::eVertexBuffer |
    vk::BufferUsageFlagBits::eTransferDst
  };
  static constexpr vk::MemoryPropertyFlags memoryFlags = { // Optimize vertex buffer resource by making it only GPU accessible 
    // vk::MemoryPropertyFlagBits::eHostVisible |
    // vk::MemoryPropertyFlagBits::eHostCoherent
    vk::MemoryPropertyFlagBits::eDeviceLocal
  };
  
  VertexBuffer(
    vk::raii::Device&         device,
    vk::raii::PhysicalDevice& physicalDevice,
    std::vector<Vertex>&      vertices,
    vk::BufferUsageFlags      inUsageFlags  = usageFlags,
    vk::MemoryPropertyFlags   inMemoryFlags = memoryFlags) :
      gpuBuffer(device, physicalDevice, BinSize(vertices), inUsageFlags, inMemoryFlags, nullptr),
      vertexCount(vertices.size())
  {}

  inline uint32 VertexCount() const noexcept             { return vertexCount; }
  inline GpuBuffer& GpuBuffer() noexcept                 { return gpuBuffer; }
  inline vk::raii::Buffer& Buffer() noexcept             { return gpuBuffer.Buffer(); }
  inline vk::raii::DeviceMemory& DeviceMemory() noexcept { return gpuBuffer.DeviceMemory(); }
};