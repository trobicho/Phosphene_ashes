#pragma once
#include "phosHelper.hpp"

class CommandPool {
  public:
    CommandPool() {};

    void  init(VkDevice device
              , uint32_t familyIndex
              , VkCommandPoolCreateFlagBits flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT
              , VkQueue defaultQueue = VK_NULL_HANDLE);

    void  destroy();

  private:
    VkDevice        m_device = VK_NULL_HANDLE;
    VkCommandPool   m_commandPool = VK_NULL_HANDLE;
    VkQueue         m_queue;
};
