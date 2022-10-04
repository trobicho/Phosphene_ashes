#include "command.hpp"

void  CommandPool::init(VkDevice device
                        , uint32_t queueFamilyIndex
                        , VkCommandPoolCreateFlagBits flags
                        , VkQueue defaultQueue) {
  m_device = device;
  m_queue = defaultQueue;
  if (defaultQueue == VK_NULL_HANDLE)
    vkGetDeviceQueue(m_device, queueFamilyIndex, 0, &m_queue);

  VkCommandPoolCreateInfo poolInfo = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
    .flags = flags,
    .queueFamilyIndex = queueFamilyIndex,
  };
  if (vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_commandPool) != VK_SUCCESS)
    throw PhosHelper::FatalVulkanInitError("Failed to create Command Pool !");
}

void  CommandPool::destroy() {
  vkDestroyCommandPool(m_device, m_commandPool, nullptr);
}
