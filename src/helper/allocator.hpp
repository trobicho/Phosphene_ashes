#pragma once
#include "command.hpp"

struct  BufferWrapper {
  VkBuffer        buffer = VK_NULL_HANDLE;
  VkDeviceMemory  memory = VK_NULL_HANDLE;
};

class MemoryAllocator { //TODO: Real allocator
  public:
    MemoryAllocator(){};

    void  createBuffer(size_t size, VkBufferUsageFlagBits usage
                      , VkMemoryPropertyFlags propertyFlags
                      , VkBuffer &buffer, VkDeviceMemory &bufferMemory);
    
    void  createBuffer(size_t size, VkBufferUsageFlagBits usage
                      , VkMemoryPropertyFlags propertyFlags
                      , BufferWrapper &buffer) {
      createBuffer(size, usage, propertyFlags, buffer.buffer, buffer.memory);
    };

    void  init(VkDevice device
              , VkPhysicalDevice physicalDevice
              , uint32_t queueFamilyIndex
              , VkQueue defaultQueue = VK_NULL_HANDLE);

  private:
    uint32_t  findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags propertyFlags);

    VkDevice          m_device = VK_NULL_HANDLE;
    VkPhysicalDevice  m_physicalDevice = VK_NULL_HANDLE;
    VkQueue           m_queue = VK_NULL_HANDLE;
};
