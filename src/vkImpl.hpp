#pragma once
#include <vulkan/vulkan.h>

class VkImpl {
  public:
    VkImpl() {};

    VkInstance    m_instance = VK_NULL_HANDLE;
    VkDevice      m_device = VK_NULL_HANDLE;
    uint32_t      m_queueFamily;
    VkQueue       m_queue = VK_NULL_HANDLE;
};
