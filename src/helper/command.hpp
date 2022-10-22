#pragma once
#include "../helper/phosHelper.hpp"

class CommandPool {
  public:
    CommandPool() {};

    void  init(VkDevice device
              , uint32_t queueFamilyIndex
              , VkQueue defaultQueue = VK_NULL_HANDLE
              , VkCommandPoolCreateFlagBits flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);

    void  destroy();

    VkCommandBuffer createCommandBuffer();
    void            submit(VkCommandBuffer& cmdBuffer);
    void            submitAndWait(VkCommandBuffer& cmdBuffer);
    void            beginRecord(VkCommandBuffer& cmdBuffer) {
      VkCommandBufferBeginInfo  beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = nullptr,
      };
      vkBeginCommandBuffer(cmdBuffer, &beginInfo);
    }

  private:
    VkDevice        m_device = VK_NULL_HANDLE;
    VkCommandPool   m_commandPool = VK_NULL_HANDLE;
    VkQueue         m_queue = VK_NULL_HANDLE;
};
