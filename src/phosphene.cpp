#include "phosphene.hpp"
#include "helper/extensions.hpp"
#include <iostream>

Phosphene::Phosphene(GLFWwindow *window): m_window(window) {
  m_instance = PhosStartVk::createInstance();
  if (glfwCreateWindowSurface(m_instance, m_window, nullptr, &m_surface) != VK_SUCCESS)
    throw PhosHelper::FatalVulkanInitError("Failed to create Surface");
  m_physicalDevice = PhosStartVk::choosePhysicalDevice(m_instance);
  PhosStartVk::createLogicalDeviceAndQueue(m_device, m_physicalDevice, m_surface, m_graphicsQueue, m_graphicsQueueFamilyIndex);
  PhosHelper::loadRtExtension(m_device);

  int width = 0;
  int height = 0;
  glfwGetWindowSize(m_window, &width, &height);
  if (width <= 0 || height <= 0)
    throw PhosHelper::FatalError("Windows as invalid size");
  m_width = static_cast<uint32_t>(width);
  m_height = static_cast<uint32_t>(height);
  m_alloc.init(m_device, m_physicalDevice, m_graphicsQueueFamilyIndex, m_graphicsQueue);

  {
    m_camera.setAllowMoving(true);
    m_camera.setFov(80.f);
    m_camera.setAspectRatio(m_width, m_height);
  }

  {
    m_vkImpl.setup(m_device, m_physicalDevice, m_graphicsQueueFamilyIndex, m_graphicsQueue);
    m_vkImpl.createSwapchain(m_surface, m_width, m_height);
    m_vkImpl.createRenderPass();
    m_vkImpl.createCommandPool();
    m_vkImpl.createFramebuffer();
    m_vkImpl.createPipeline();
  }

  {
    createOffscreenRender();
    m_vkImpl.updatePostDescSet(m_offscreenImageView);
  }

  {
    VkCommandPoolCreateInfo poolInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
      .queueFamilyIndex = m_graphicsQueueFamilyIndex,
    };
  if (vkCreateCommandPool(m_device, &poolInfo
          , nullptr, &m_commandPool) != VK_SUCCESS)
      throw PhosHelper::FatalVulkanInitError("Failed to create Raytracing Command Pool !");

    VkCommandBufferAllocateInfo allocateInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .commandPool = m_commandPool,
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = 1,
    };
    if (vkAllocateCommandBuffers(m_device, &allocateInfo, &m_commandBuffer) != VK_SUCCESS)
        throw PhosHelper::FatalVulkanInitError("Failed to create Rasytracing commandBuffer !");

    VkSemaphoreCreateInfo semaphoreInfo = {
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };
    if (vkCreateSemaphore(m_device, &semaphoreInfo, nullptr
          , &m_semaphoreRTFinish) != VK_SUCCESS)
        throw PhosHelper::FatalVulkanInitError("Failed to create Raytracing Semaphore !");
  }

  {
    m_sceneBuilder.init(m_device, &m_alloc, m_graphicsQueueFamilyIndex);
    m_rtTest.init(m_device, m_physicalDevice, m_graphicsQueueFamilyIndex);
    m_rtTest.createDescriptorSet(m_offscreenImageView);
    m_rtTest.createPipeline();
    m_rtTest.createShaderBindingTable();
  }
}

void  Phosphene::loadScene(const std::string &filename) {
  SceneLoader test(m_scene);
  test.test(filename);
  for (auto& mesh : m_scene.m_meshs) {
    mesh.createBuffer(m_alloc);
  }
  m_sceneBuilder.buildBlas(m_scene, 0);
}

void  Phosphene::renderLoop() {
  uint32_t  frame_n = 1;

  while(!glfwWindowShouldClose(m_window) && !m_quit) {
    glfwPollEvents();
    m_camera.step();
    if (m_camera.buildGlobalUniform(m_globalUniform)) {
      std::cout << std::endl << "VIEW INVERSE:" << std::endl;
      //PhosHelper::printMatrix(m_globalUniform.viewInverse);
      m_update = true;
    }
    draw();
    m_update = false;
  }
}

void  Phosphene::draw() {
  VkSemaphore semaphoreWait;
  VkSemaphore semaphoreSignal;
  VkFence     fence;
  uint32_t    imageIndex;

  VkResult result = m_vkImpl.acquireNextImage(imageIndex, fence);
  auto& commandBufferPost = m_vkImpl.getCommandBuffer(semaphoreWait, semaphoreSignal);
  vkResetCommandBuffer(m_commandBuffer, 0);
  vkResetCommandBuffer(commandBufferPost, 0);
  VkCommandBufferBeginInfo  beginInfo = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    .pInheritanceInfo = nullptr,
  };

  {
    vkBeginCommandBuffer(m_commandBuffer, &beginInfo);
    m_rtTest.updateUniformBuffer(m_commandBuffer, m_globalUniform);
    m_rtTest.raytrace(m_commandBuffer, m_width, m_height);
    vkEndCommandBuffer(m_commandBuffer);
    VkPipelineStageFlags  waitStage[] = {VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR};
    VkSubmitInfo  submitInfo = {
      .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = &semaphoreWait,
      .pWaitDstStageMask = waitStage,
      .commandBufferCount = 1,
      .pCommandBuffers = &m_commandBuffer,
      .signalSemaphoreCount = 1,
      .pSignalSemaphores = &m_semaphoreRTFinish,
    };
    if (vkQueueSubmit(m_graphicsQueue, 1, &submitInfo
          , VK_NULL_HANDLE) != VK_SUCCESS)
      throw PhosHelper::FatalVulkanInitError("Failed to submit Raytrace");
  }
  {
    vkBeginCommandBuffer(commandBufferPost, &beginInfo);
    m_vkImpl.recordCommandBuffer();
    vkEndCommandBuffer(commandBufferPost);
    VkPipelineStageFlags  waitStage[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSubmitInfo  submitInfo = {
      .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = &m_semaphoreRTFinish,
      //.pWaitSemaphores = &semaphoreWait,
      .pWaitDstStageMask = waitStage,
      .commandBufferCount = 1,
      .pCommandBuffers = &commandBufferPost,
      .signalSemaphoreCount = 1,
      .pSignalSemaphores = &semaphoreSignal,
    };
    if (vkQueueSubmit(m_graphicsQueue, 1, &submitInfo
          , fence) != VK_SUCCESS)
      throw PhosHelper::FatalVulkanInitError("Failed to submit Post");
  }

  m_vkImpl.present();
}

void  Phosphene::destroy() {
  if (m_instance != VK_NULL_HANDLE) {
    std::cout << std::endl << "DESTROY" << std::endl;
    deviceWait();

    m_vkImpl.destroy();
    m_rtTest.destroy();
    m_scene.destroy(m_alloc);

    {
      vkDestroyCommandPool(m_device, m_commandPool, nullptr);
      vkDestroySemaphore(m_device, m_semaphoreRTFinish, nullptr);
      vkDestroyImage(m_device, m_offscreenColor, nullptr);
      vkFreeMemory(m_device, m_offscreenImageMemory, nullptr);
      vkDestroyImageView(m_device, m_offscreenImageView, nullptr);
    }
    
    m_alloc.destroy();

    vkDestroyDevice(m_device, nullptr);
    vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
    vkDestroyInstance(m_instance, nullptr);
    m_instance = VK_NULL_HANDLE;
  }
}

void  Phosphene::createOffscreenRender() {
  m_offscreenColorFormat = m_vkImpl.m_swapchainWrap.imageFormat;
  VkImageCreateInfo colorInfo  = {
    .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
    .flags = 0,
    .imageType = VK_IMAGE_TYPE_2D,
    .format = m_offscreenColorFormat,
    .extent = VkExtent3D{
      .width = m_width,
      .height = m_height, 
      .depth = 1,
    },
    .mipLevels = 1,
    .arrayLayers = 1,
    .samples = VK_SAMPLE_COUNT_1_BIT,
    .tiling = VK_IMAGE_TILING_OPTIMAL,
    .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
      | VK_IMAGE_USAGE_SAMPLED_BIT
      | VK_IMAGE_USAGE_STORAGE_BIT,
    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    .queueFamilyIndexCount = 1,
    .pQueueFamilyIndices = &m_graphicsQueueFamilyIndex,
    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
  };
  if (vkCreateImage(m_device, &colorInfo, nullptr, &m_offscreenColor) != VK_SUCCESS)
    throw PhosHelper::FatalVulkanInitError("Failed to begin Create offscreen Color Image !");
  VkMemoryRequirements  memoryReqs;
  vkGetImageMemoryRequirements(m_device, m_offscreenColor, &memoryReqs);
  VkMemoryAllocateInfo  allocImageInfo = {
    .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
    .allocationSize = memoryReqs.size,
    .memoryTypeIndex = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
  };
  vkAllocateMemory(m_device, &allocImageInfo, nullptr, &m_offscreenImageMemory);
  vkBindImageMemory(m_device, m_offscreenColor, m_offscreenImageMemory, 0);

  VkImageViewCreateInfo viewInfo = {
    .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
    .image = m_offscreenColor,
    .viewType = VK_IMAGE_VIEW_TYPE_2D,
    .format = m_offscreenColorFormat,
    .components = VkComponentMapping {
      .r = VK_COMPONENT_SWIZZLE_IDENTITY,
      .g = VK_COMPONENT_SWIZZLE_IDENTITY,
      .b = VK_COMPONENT_SWIZZLE_IDENTITY,
      .a = VK_COMPONENT_SWIZZLE_IDENTITY,
    },
    .subresourceRange = VkImageSubresourceRange {
      .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
      .baseMipLevel = 0,
      .levelCount = 1,
      .baseArrayLayer = 0,
      .layerCount = 1,
    },
  };
  if (vkCreateImageView(m_device, &viewInfo, nullptr, &m_offscreenImageView) != VK_SUCCESS)
    throw PhosHelper::FatalVulkanInitError("Failed to create Offscreen ImageView !");
}
