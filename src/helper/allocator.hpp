#pragma once
#include "command.hpp"

struct  ImageWrapper {
  ImageWrapper(){};
  ImageWrapper(VkFormat argFormat): format(argFormat){};

  VkImage           image{VK_NULL_HANDLE};
  VkFormat          format;
  VkImageView       imageView{VK_NULL_HANDLE};
  VkDeviceMemory    memory{VK_NULL_HANDLE};
};

struct  BufferWrapper {
  VkBuffer        buffer = VK_NULL_HANDLE;
  VkDeviceMemory  memory = VK_NULL_HANDLE;

  void  destroy(VkDevice device) {
    vkDestroyBuffer(device, buffer, nullptr);
    vkFreeMemory(device, memory, nullptr);
  }
};

struct  AccelKHR {
  VkAccelerationStructureKHR  accel = VK_NULL_HANDLE;
  BufferWrapper               buffer;
};

class MemoryAllocator { //TODO: Real allocator
  public:
    MemoryAllocator(){};

    void  destroy();

    void  init(VkDevice device
              , VkPhysicalDevice physicalDevice
              , uint32_t queueFamilyIndex
              , VkQueue defaultQueue = VK_NULL_HANDLE);
    
    void  createBuffer(size_t size, VkBufferUsageFlagBits usage
                      , VkMemoryPropertyFlags propertyFlags
                      , VkBuffer &buffer, VkDeviceMemory &bufferMemory);
    
    void  createBuffer(size_t size, VkBufferUsageFlagBits usage
                      , VkMemoryPropertyFlags propertyFlags
                      , BufferWrapper &buffer) {
      createBuffer(size, usage, propertyFlags, buffer.buffer, buffer.memory);
    };
    void  destroyBuffer(BufferWrapper &buffer);

    void  createImage(const VkExtent3D& extent
                      , VkImageUsageFlagBits usage
                      , VkImageLayout layout
                      , VkImageAspectFlagBits aspect
                      , VkComponentMapping components
                      , ImageWrapper &image);
    void  destroyImage(ImageWrapper &image);
    
    VkDeviceAddress getBufferDeviceAddress(BufferWrapper &buffer);
    VkDeviceAddress getAccelerationStructureDeviceAddress(AccelKHR &accel);

    void  stagingMakeAndCopy(size_t size, BufferWrapper &buffer, void *data);



  private:
    uint32_t  findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags propertyFlags);

    VkDevice          m_device = VK_NULL_HANDLE;
    VkPhysicalDevice  m_physicalDevice = VK_NULL_HANDLE;
    VkQueue           m_queue = VK_NULL_HANDLE;
    uint32_t          m_queueFamilyIndex;
    CommandPool       m_cmdPool;
};
