#include  "vkImpl.hpp"

void  VkImpl::destroy() {
  cleanupSwapchain();

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
void  VkImpl::init(VkDevice &device
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
  VkSampler           sampler;
  vkCreateSampler(m_device, &samplerInfo, nullptr, &sampler);
  VkDescriptorImageInfo imageInfo = {
    .sampler = sampler,
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
