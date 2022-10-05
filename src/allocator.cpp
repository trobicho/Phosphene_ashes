#include "allocator.hpp"


void  MemoryAllocator::init(VkDevice device
                            , VkPhysicalDevice physicalDevice
                            , uint32_t queueFamilyIndex
                            , VkQueue defaultQueue) {
  m_device = device;
  m_physicalDevice = physicalDevice;
  m_queue = defaultQueue;
  if (defaultQueue == VK_NULL_HANDLE)
    vkGetDeviceQueue(m_device, queueFamilyIndex, 0, &m_queue);
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
