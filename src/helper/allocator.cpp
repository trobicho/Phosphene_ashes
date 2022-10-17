#include "allocator.hpp"
#include <cstring> //memcpy


void  MemoryAllocator::init(VkDevice device
                            , VkPhysicalDevice physicalDevice
                            , uint32_t queueFamilyIndex
                            , VkQueue defaultQueue) {
  m_device = device;
  m_physicalDevice = physicalDevice;
  m_queue = defaultQueue;
  m_queueFamilyIndex = queueFamilyIndex;

  m_cmdPool.init(m_device, m_queueFamilyIndex);

  if (defaultQueue == VK_NULL_HANDLE)
    vkGetDeviceQueue(m_device, queueFamilyIndex, 0, &m_queue);
}

void  MemoryAllocator::destroy() {
  m_cmdPool.destroy();
}

void  MemoryAllocator::createBuffer(size_t size, VkBufferUsageFlagBits usage
                                    , VkMemoryPropertyFlags propertyFlags
                                    , VkBuffer &buffer, VkDeviceMemory &bufferMemory) {

  VkBufferCreateInfo  bufferInfo  = {
    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
    .size = size,
    .usage = usage,
    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
  };
  if (vkCreateBuffer(m_device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
    throw PhosHelper::FatalVulkanInitError("Failed to Create Vertex Buffer !");

  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(m_device, buffer, &memRequirements);

  VkMemoryAllocateFlagsInfo allocateFlags = {
    .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO,
    .flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT,
  };

  VkMemoryAllocateInfo memAllocInfo = {
    .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
    .pNext = &allocateFlags,
    .allocationSize = memRequirements.size,
    .memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, propertyFlags),
  };
  if (vkAllocateMemory(m_device, &memAllocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
    throw PhosHelper::FatalVulkanInitError("failed to allocate vertex buffer memory!");

  vkBindBufferMemory(m_device, buffer, bufferMemory, 0);
}

uint32_t  MemoryAllocator::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags propertyFlags) {

  VkPhysicalDeviceMemoryProperties  memProps;

  vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memProps);
  for (uint32_t i = 0; i < memProps.memoryTypeCount; i++) {
    if (typeFilter & (1 << i)
        && (memProps.memoryTypes[i].propertyFlags & propertyFlags))
      return (i);
  }
  throw PhosHelper::FatalVulkanInitError("Unable to find memory type !");
}

void  MemoryAllocator::destroyBuffer(BufferWrapper &buffer) {
  vkDestroyBuffer(m_device, buffer.buffer, nullptr);
  vkFreeMemory(m_device, buffer.memory, nullptr);
}

VkDeviceAddress MemoryAllocator::getBufferDeviceAddress(BufferWrapper &buffer) {
  VkBufferDeviceAddressInfo addrInfo = {
    .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
    .buffer = buffer.buffer,
  };

  return (vkGetBufferDeviceAddress(m_device, &addrInfo));
}

VkDeviceAddress MemoryAllocator::getAccelerationStructureDeviceAddress(AccelKHR &accel) {
  VkAccelerationStructureDeviceAddressInfoKHR addrInfo = {
    .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR,
    .accelerationStructure = accel.accel,
  };

  return (vkGetAccelerationStructureDeviceAddressKHR(m_device, &addrInfo));
}

void  MemoryAllocator::stagingMakeAndCopy(size_t size, BufferWrapper &buffer, void *data) {
  BufferWrapper staging;
  createBuffer(size
      , VK_BUFFER_USAGE_TRANSFER_SRC_BIT
      , VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
      , staging);

  void* stagingData;
  vkMapMemory(m_device, staging.memory, 0, size, 0, &stagingData);
  memcpy(stagingData, data, size);
  vkUnmapMemory(m_device, staging.memory);

  auto cmdBuffer = m_cmdPool.createCommandBuffer();
  VkCommandBufferBeginInfo beginInfo = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
  };

  vkBeginCommandBuffer(cmdBuffer, &beginInfo);
  VkBufferCopy copyRegion = {
    .srcOffset = 0,
    .dstOffset = 0,
    .size = size,
  };
  vkCmdCopyBuffer(cmdBuffer, staging.buffer, buffer.buffer, 1, &copyRegion);
  vkEndCommandBuffer(cmdBuffer);
  m_cmdPool.submitAndWait(cmdBuffer);
  destroyBuffer(staging);
}
