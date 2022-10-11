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

VkCommandBuffer CommandPool::createCommandBuffer() {
  VkCommandBuffer commandBuffer;

  VkCommandBufferAllocateInfo allocateInfo = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
    .commandPool = m_commandPool,
    .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    .commandBufferCount = 1,
  };
  if (vkAllocateCommandBuffers(m_device, &allocateInfo
        , &commandBuffer) != VK_SUCCESS)
    throw PhosHelper::FatalVulkanInitError("Failed to allocate Command Buffers !");
  return (commandBuffer);
}

void            CommandPool::submit(VkCommandBuffer& cmdBuffer) {
  VkSubmitInfo info = {
    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
    .commandBufferCount = 1,
    .pCommandBuffers = &cmdBuffer,
  };
  vkQueueSubmit(m_queue, 1, &info, VK_NULL_HANDLE);
}

void            CommandPool::submitAndWait(VkCommandBuffer& cmdBuffer) {
  submit(cmdBuffer);
  VkResult result = vkQueueWaitIdle(m_queue);
  if (result != VK_SUCCESS) {
    std::string error;
    if (result == VK_ERROR_OUT_OF_HOST_MEMORY)
      error = "VK_ERROR_OUT_OF_HOST_MEMORY";
    if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY)
      error = "VK_ERROR_OUT_OF_DEVICE_MEMORY";
    if (result == VK_ERROR_DEVICE_LOST)
      error = "VK_ERROR_DEVICE_LOST";
    throw PhosHelper::FatalError(("Failed to submit and wait: " + error));
  }
}
