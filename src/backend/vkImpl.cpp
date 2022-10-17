#include  "vkImpl.hpp"

void  VkImpl::destroy() {
  cleanupSwapchain();

  vkDestroySampler(m_device, m_sampler, nullptr); 
  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    vkDestroySemaphore(m_device, m_semaphoreAvailable[i], nullptr);
    vkDestroySemaphore(m_device, m_semaphoreFinish[i], nullptr);
    vkDestroyFence(m_device, m_fences[i], nullptr);
  }
  vkDestroyCommandPool(m_device, m_commandPool, nullptr);
  vkDestroyRenderPass(m_device, m_renderPass, nullptr);
  {
    vkDestroyPipeline(m_device, m_postPipeline, nullptr);
    vkDestroyPipelineLayout(m_device, m_postPipelineLayout, nullptr);
    vkDestroyDescriptorSetLayout(m_device, m_postDescSetLayout, nullptr);
    vkDestroyDescriptorPool(m_device, m_postDescPool, nullptr);
  }
}

void  VkImpl::setup(VkDevice &device
                  , VkPhysicalDevice &physicalDevice
                  , uint32_t queueFamilyIndex
                  , VkQueue defaultQueue) {
  m_device = device;
  m_physicalDevice = physicalDevice;
  m_queueFamilyIndex = queueFamilyIndex;
  m_queue = defaultQueue;
  if (defaultQueue == VK_NULL_HANDLE)
    vkGetDeviceQueue(m_device, queueFamilyIndex, 0, &m_queue);
}

VkCommandBuffer&  VkImpl::getCommandBuffer(VkSemaphore &semaphoreWait, VkSemaphore &semaphoreSignal) {
  semaphoreWait = m_semaphoreAvailable[m_currentFrame];
  semaphoreSignal = m_semaphoreFinish[m_currentFrame];
  return (m_commandBuffers[m_currentFrame]);
}

VkResult          VkImpl::acquireNextImage(uint32_t &imageIndex, VkFence &fence) {
  vkWaitForFences(m_device, 1, &m_fences[m_currentFrame], VK_TRUE, UINT64_MAX);
  vkResetFences(m_device, 1, &m_fences[m_currentFrame]);
  fence = m_fences[m_currentFrame];
  VkResult  result = vkAcquireNextImageKHR(m_device
      , m_swapchainWrap.chain, UINT64_MAX
      , m_semaphoreAvailable[m_currentFrame]
      , VK_NULL_HANDLE, &m_currentImageIndex);
  imageIndex = m_currentImageIndex;
  return (result);
}

void              VkImpl::recordCommandBuffer(VkCommandBuffer &commandBuffer) {
  VkClearValue clearValue = (VkClearValue){0.0f, 0.0f, 0.0f, 1.0f};
  VkRenderPassBeginInfo     renderPassInfo = {
    .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
    .renderPass = m_renderPass,
    .framebuffer = m_swapchainWrap.framebuffer[m_currentImageIndex],
    .renderArea = (VkRect2D) {
      .offset = (VkOffset2D){0, 0},
      .extent = m_swapchainWrap.extent,
    },
    .clearValueCount = 1,
    .pClearValues = &clearValue,
  };
  vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_postPipeline);

  vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS
      , m_postPipelineLayout, 0, 1, &m_postDescSet, 0, nullptr);

  VkViewport viewport{
    .x = 0.0f,
    .y = 0.0f,
    .width = static_cast<float>(m_swapchainWrap.extent.width),
    .height= static_cast<float>(m_swapchainWrap.extent.height),
    .minDepth = 0.0f,
    .maxDepth = 1.0f,
  };
  vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

  VkRect2D scissor{
    .offset = {0, 0},
    .extent = m_swapchainWrap.extent,
  };
  vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

  vkCmdDraw(commandBuffer, 4, 1, 0, 0);
  vkCmdEndRenderPass(commandBuffer);
}

void              VkImpl::present() {
    VkPresentInfoKHR  presentInfo = {
    .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
    .waitSemaphoreCount = 1,
    .pWaitSemaphores = &m_semaphoreFinish[m_currentFrame],
    .swapchainCount = 1,
    .pSwapchains = &m_swapchainWrap.chain,
    .pImageIndices = &m_currentImageIndex,
    .pResults = nullptr,
  };
  vkQueuePresentKHR(m_queue, &presentInfo);
  ++m_currentFrame %= MAX_FRAMES_IN_FLIGHT;
}

void  VkImpl::createCommandPool() {
  VkCommandPoolCreateInfo poolInfo {
    .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
    .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
    .queueFamilyIndex = m_queueFamilyIndex,
  };
  if (vkCreateCommandPool(m_device, &poolInfo
        , nullptr, &m_commandPool) != VK_SUCCESS)
    throw PhosHelper::FatalVulkanInitError("Failed to create Command Pool !");

  VkCommandBufferAllocateInfo allocateInfo = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
    .commandPool = m_commandPool,
    .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    .commandBufferCount = (uint32_t) m_commandBuffers.size(),
  };
  if (vkAllocateCommandBuffers(m_device, &allocateInfo
        , m_commandBuffers.data()) != VK_SUCCESS)
    throw PhosHelper::FatalVulkanInitError("Failed to allocate Command Buffers !");

  m_currentFrame = 0;
  VkSemaphoreCreateInfo semaphoreInfo = {
    .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
  };
  VkFenceCreateInfo     fenceInfo = {
    .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
    .flags = VK_FENCE_CREATE_SIGNALED_BIT,
  };
  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
    if (vkCreateSemaphore(m_device, &semaphoreInfo, nullptr
          , &m_semaphoreAvailable[i]) != VK_SUCCESS
        || vkCreateSemaphore(m_device, &semaphoreInfo, nullptr
          , &m_semaphoreFinish[i]) != VK_SUCCESS
        || vkCreateFence(m_device, &fenceInfo, nullptr
          , &m_fences[i]) != VK_SUCCESS)
      throw PhosHelper::FatalVulkanInitError("Failed to create Semaphores or Fences!");
  }
}

void  VkImpl::createRenderPass() {
  VkAttachmentDescription attachmentDescription = {
    .format = m_swapchainWrap.imageFormat,
    .samples = VK_SAMPLE_COUNT_1_BIT,
    .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
    .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
    .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
  };
  VkAttachmentReference   colorAttachmentReference = {
    .attachment = 0,
    .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
  };
  VkSubpassDescription    subpassDescription = {
    .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
    .colorAttachmentCount = 1,
    .pColorAttachments = &colorAttachmentReference,
  };
  VkSubpassDependency     subpassDependency = {
    .srcSubpass = VK_SUBPASS_EXTERNAL,
    .dstSubpass = 0,
    .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    .srcAccessMask = 0,
    .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT
      | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
  };
  VkRenderPassCreateInfo  renderPassInfo = {
    .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
    .attachmentCount = 1,
    .pAttachments = &attachmentDescription,
    .subpassCount = 1,
    .pSubpasses = &subpassDescription,
    .dependencyCount = 1,
    .pDependencies = &subpassDependency,
  };
  if (vkCreateRenderPass(m_device, &renderPassInfo
        , nullptr, &m_renderPass) != VK_SUCCESS)
    throw PhosHelper::FatalVulkanInitError("Failed to create RenderPass");
}

void  VkImpl::updatePostDescSet(VkImageView &offscreenImageView) {
  VkSamplerCreateInfo samplerInfo{VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
  vkDestroySampler(m_device, m_sampler, nullptr);
  vkCreateSampler(m_device, &samplerInfo, nullptr, &m_sampler);
  VkDescriptorImageInfo imageInfo = {
    .sampler = m_sampler,
    .imageView = offscreenImageView,
    .imageLayout = VK_IMAGE_LAYOUT_GENERAL,
  };

  std::vector<VkWriteDescriptorSet> writes = {
    (VkWriteDescriptorSet){
      .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .dstSet = m_postDescSet,
      .dstBinding = 0,
      .dstArrayElement = 0,
      .descriptorCount = 1,
      .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
      .pImageInfo = &imageInfo,
    },
  };
  vkUpdateDescriptorSets(m_device, writes.size(), writes.data(), 0, nullptr);
}
