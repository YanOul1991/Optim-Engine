#include "OpVkTypes/Buffers.h"

GpuBuffer::GpuBuffer(
  vk::raii::Device&         device,
  vk::raii::PhysicalDevice& physicalDevice,
  vk::DeviceSize            deviceSize,
  vk::BufferUsageFlags      usageFlags,
  vk::MemoryPropertyFlags   memoryPorpertyFlag,
  void*                     pInitalData,
  vk::SharingMode           sharingMode)
{
  // Buffer creation struct
  vk::BufferCreateInfo bufferCreateInfo{
    .size        = deviceSize,
    .usage       = usageFlags,
    .sharingMode = sharingMode
  };

  // Create buffer handle
  buffer = vk::raii::Buffer(device, bufferCreateInfo);

  // Validate buffer
  if (buffer == nullptr) {
    return;
  }

  // Get buffer's memory requirements
  vk::MemoryRequirements memoryReqs = buffer.getMemoryRequirements();

  vk::MemoryAllocateInfo memAllocInfo{
    .allocationSize  = memoryReqs.size,
    // Helper function to get correct index for requirements
    .memoryTypeIndex = FindMemoryTypeIndex(physicalDevice, memoryReqs.memoryTypeBits, memoryPorpertyFlag)
  };

  // Create device memory 
  deviceMemory = vk::raii::DeviceMemory(device, memAllocInfo);

  // Validate
  if (deviceMemory == nullptr) {
    return;
  }

  // Bind memory
  buffer.bindMemory(*deviceMemory, 0);
}

uint32 GpuBuffer::FindMemoryTypeIndex(const vk::raii::PhysicalDevice& physicalDevice, uint32 typeFilter, vk::MemoryPropertyFlags properties)
{
  vk::PhysicalDeviceMemoryProperties memProperties = physicalDevice.getMemoryProperties();
  for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
    if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
      return i;
    }
  }
  throw std::runtime_error("failed to find suitable memory type!");
}