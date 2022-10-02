#include "command.hpp"

void  CommandPool::init(VkDevice device
                        , uint32_t familyIndex
                        , VkCommandPoolCreateFlagBits flags
                        , VkQueue defaultQueue) {
  m_device = device;
  m_queue = defaultQueue;
  if (defaultQueue == VK_NULL_HANDLE)
    vkGetDeviceQueue(m_device, familyIndex, 0, &m_queue);

  VkCommandPoolCreateInfo poolInfo = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
    .flags = flags,
    .queueFamilyIndex = familyIndex,
  };
  if (vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_commandPool) != VK_SUCCESS)
    throw PhosHelper::FatalVulkanInitError("Failed to create Command Pool !");
}
