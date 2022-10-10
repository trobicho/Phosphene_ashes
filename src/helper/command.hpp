#pragma once
#include "../helper/phosHelper.hpp"

class CommandPool {
  public:
    CommandPool() {};

    void  init(VkDevice device
              , uint32_t queueFamilyIndex
              , VkCommandPoolCreateFlagBits flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT
              , VkQueue defaultQueue = VK_NULL_HANDLE);

    void  destroy();

    VkCommandBuffer createCommandBuffer();
    void            submit(VkCommandBuffer& cmdBuffer);
    void            submitAndWait(VkCommandBuffer& cmdBuffer);

  private:
    VkDevice        m_device = VK_NULL_HANDLE;
    VkCommandPool   m_commandPool = VK_NULL_HANDLE;
    VkQueue         m_queue = VK_NULL_HANDLE;
};
