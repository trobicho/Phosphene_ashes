#include "phosphene.hpp"
#include "sceneLoader/vdbLoader.hpp"
#include "helper/extensions.hpp"

#include <imgui_impl_vulkan.h>
#include <iostream>

Phosphene::Phosphene(GLFWwindow *window): m_window(window) {
  m_instance = PhosStartVk::createInstance();
  if (glfwCreateWindowSurface(m_instance, m_window, nullptr, &m_surface) != VK_SUCCESS)
    throw PhosHelper::FatalVulkanInitError("Failed to create Surface");
  m_physicalDevice = PhosStartVk::choosePhysicalDevice(m_instance);
  PhosStartVk::createLogicalDeviceAndQueue(m_device, m_physicalDevice, m_surface, m_graphicsQueue, m_graphicsQueueFamilyIndex);
  PhosHelper::loadRtExtension(m_device);
  m_pcRay = (PushConstantRay){
    .clearColor = glm::vec4(0.1, 0.1, 0.6, 1.0),
    .nbLights = 0,
    .nbConsecutiveRay = 0,
    .pathMaxRecursion = 3,
  };

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
    //m_camera.setFov(80.f);
    m_camera.setAspectRatio(m_width, m_height);
    m_alloc.createBuffer(sizeof(GlobalUniforms)
        , static_cast<VkBufferUsageFlagBits>(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT)
        , VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        , m_globalUBO);
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
    createGBuffer();
    m_vkImpl.updatePostDescSet(m_offscreenImage.imageView);
  }

  {
    m_sceneBuilder.init(m_device, &m_alloc, m_graphicsQueueFamilyIndex);
    m_scene.init(&m_alloc);
    buildGBufferPipeline();
    buildPipeline("basicLights");
  }

  {
    IMGUI_CHECKVERSION();
    std::vector<VkDescriptorPoolSize> poolSizes =
    {
      { VK_DESCRIPTOR_TYPE_SAMPLER, 1},
      { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1},
    };
    VkDescriptorPoolCreateInfo        poolInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    poolInfo.maxSets       = poolSizes.size();
    poolInfo.poolSizeCount = poolSizes.size();
    poolInfo.pPoolSizes    = poolSizes.data();
    vkCreateDescriptorPool(m_device, &poolInfo, nullptr, &m_gui.imguiDescPool);

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.IniFilename = nullptr;
    io.DisplaySize = ImVec2(static_cast<float>(m_width), static_cast<float>(m_height));
    ImGui_ImplVulkan_InitInfo init_info = {
      .Instance = m_instance,
      .PhysicalDevice = m_physicalDevice,
      .Device = m_device,
      .QueueFamily = m_graphicsQueueFamilyIndex,
      .Queue = m_graphicsQueue,
      .DescriptorPool = m_gui.imguiDescPool,
      .Subpass = 0,
      .MinImageCount = m_vkImpl.m_swapchainWrap.imageCount,
      .ImageCount = m_vkImpl.m_swapchainWrap.imageCount,
      .MSAASamples = VK_SAMPLE_COUNT_1_BIT,
      .Allocator = nullptr,
      .CheckVkResultFn = nullptr,
    };
    ImGui_ImplVulkan_Init(&init_info, m_vkImpl.m_renderPass);
    ImGui_ImplVulkan_CreateFontsTexture();
    ImGui_ImplVulkan_DestroyFontsTexture();
    ImGui::SetNextWindowSize(ImVec2(static_cast<float>(m_width), static_cast<float>(m_height)));
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui_ImplVulkan_SetMinImageCount(m_vkImpl.m_swapchainWrap.imageCount);
  }

  {
    m_rayPicker.init(m_device, m_physicalDevice, m_graphicsQueueFamilyIndex);
  }
	VdbLoader::initialize();
}

bool  Phosphene::loadScene(const std::string &filename) {
  SceneLoader sceneLoader(m_scene);
  try {
    sceneLoader.load(filename);
  }
  catch (const char* e) {
    return (false);
  }
  buildPipeline("basicLights");
  buildGBufferPipeline();
  m_scene.setShapesHitBindingIndex(1);
  m_scene.allocateResources();
  m_sceneBuilder.buildBlas(m_scene, 0);
  m_sceneBuilder.buildTlas(m_scene, 0); 
  m_scene.update(m_rtPipeline, true);
  m_scene.update(m_gbufferPipeline, true);
  m_pcRay.nbConsecutiveRay = 0;
  m_pcRay.nbLights = m_scene.getLightCount();
  updateRtTlas();
  return (true);
}

void  Phosphene::renderLoop() {
  uint32_t  frame_n = 1;
  float     camDeltaTime = 0.0;

  m_globalUniform.time = 0.0f;
  while(!glfwWindowShouldClose(m_window) && !m_quit) {
    ImGuiIO& io = ImGui::GetIO();
    glfwPollEvents();
    m_mutexEvent.lock();
    camDeltaTime += io.DeltaTime;
    if (camDeltaTime >= BASE_DELTA_TIME) {
      m_camera.step(camDeltaTime);
      camDeltaTime = 0.0f;
    }
    if (m_camera.buildGlobalUniform(m_globalUniform)) {
      m_pcRay.nbConsecutiveRay = 0;
      //std::cout << std::endl << "VIEW INVERSE:" << std::endl;
      //PhosHelper::printMatrix(m_globalUniform.viewInverse);
      m_update = true;
    }
    draw();
    m_pcRay.nbConsecutiveRay += 1;
    if (m_pcRay.nbConsecutiveRay % 100 == 0)
      std::cout << "ray launch count = " << m_pcRay.nbConsecutiveRay << std::endl;
    m_update = false;
    //m_quit = true;
    m_globalUniform.time += io.DeltaTime;
    m_mutexEvent.unlock();
  }
}

void  Phosphene::draw() {
  VkSemaphore semaphoreWait;
  VkSemaphore semaphoreSignal;
  VkFence     fence;
  uint32_t    imageIndex;

  VkResult result = m_vkImpl.acquireNextImage(imageIndex, fence);
  if (result != VK_SUCCESS)
    return ;
  if (m_showGui) {
    guiRender();
  }
  auto& commandBuffer = m_vkImpl.getCommandBuffer(semaphoreWait, semaphoreSignal);
  vkResetCommandBuffer(commandBuffer, 0);
  VkCommandBufferBeginInfo  beginInfo = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    .pInheritanceInfo = nullptr,
  };
  vkBeginCommandBuffer(commandBuffer, &beginInfo);

  if (m_update) {
    m_gbufferPipeline.updateUBO(commandBuffer, sizeof(m_globalUniform), m_globalUBO, &m_globalUniform);
    m_scene.update(m_gbufferPipeline, commandBuffer, false);
    m_gbufferPipeline.raytrace(commandBuffer, m_width, m_height);
  }

  {
    m_rtPipeline.updateUBO(commandBuffer, sizeof(m_globalUniform), m_globalUBO, &m_globalUniform);
    m_scene.update(m_rtPipeline, commandBuffer, false);
    m_rtPipeline.raytrace(commandBuffer, m_width, m_height);
  }
  {
    VkClearValue clearValue = (VkClearValue){1.0f, 1.0f, 1.0f, 1.0f};
    VkRenderPassBeginInfo     renderPassInfo = {
      .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
      .renderPass = m_vkImpl.m_renderPass,
      .framebuffer = m_vkImpl.getFramebuffer(),
      .renderArea = (VkRect2D) {
        .offset = (VkOffset2D){0, 0},
        .extent = m_vkImpl.m_swapchainWrap.extent,
      },
      .clearValueCount = 1,
      .pClearValues = &clearValue,
    };
    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    m_vkImpl.recordCommandBuffer(commandBuffer);
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
    vkCmdEndRenderPass(commandBuffer);
  }
  vkEndCommandBuffer(commandBuffer);
  VkPipelineStageFlags  waitStage[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  VkSubmitInfo  submitInfo = {
    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
    .waitSemaphoreCount = 1,
    .pWaitSemaphores = &semaphoreWait,
    .pWaitDstStageMask = waitStage,
    .commandBufferCount = 1,
    .pCommandBuffers = &commandBuffer,
    .signalSemaphoreCount = 1,
    .pSignalSemaphores = &semaphoreSignal,
  };
  if (vkQueueSubmit(m_graphicsQueue, 1, &submitInfo
        , fence) != VK_SUCCESS)
    throw PhosHelper::FatalVulkanInitError("Failed to submit Post");

  m_vkImpl.present();
}

void  Phosphene::destroy() {
  if (m_instance != VK_NULL_HANDLE) {
    std::cout << std::endl << "DESTROY" << std::endl;
    deviceWait();

    {
      ImGui_ImplVulkan_Shutdown();
      ImGui_ImplGlfw_Shutdown();
      ImGui::DestroyContext();
      vkDestroyDescriptorPool(m_device, m_gui.imguiDescPool, nullptr);
    }
    {
      m_alloc.destroyImage(m_gbuffer.color);
      m_alloc.destroyImage(m_gbuffer.normal);
      m_alloc.destroyImage(m_gbuffer.depth);
      m_alloc.destroyImage(m_gbuffer.material);
    }
    m_vkImpl.destroy();
    m_rtPipeline.destroy();
    m_gbufferPipeline.destroy();
    m_sceneBuilder.destroy();
    m_scene.destroy();
    m_globalUBO.destroy(m_device);
    m_rayPicker.destroy();

    m_alloc.destroyImage(m_offscreenImage);
    
    m_alloc.destroy();

    vkDestroyDevice(m_device, nullptr);
    vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
    vkDestroyInstance(m_instance, nullptr);
    m_instance = VK_NULL_HANDLE;
  }
}

void  Phosphene::createOffscreenRender() {
  m_alloc.destroyImage(m_offscreenImage);

  m_offscreenImage.format = m_vkImpl.m_swapchainWrap.imageFormat;
  VkExtent3D extent = {
    .width = m_width,
    .height = m_height, 
    .depth = 1,
  };
  VkImageUsageFlagBits  usage = static_cast<VkImageUsageFlagBits>(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
      | VK_IMAGE_USAGE_SAMPLED_BIT
      | VK_IMAGE_USAGE_STORAGE_BIT);

  VkComponentMapping  components = {
    .r = VK_COMPONENT_SWIZZLE_IDENTITY,
    .g = VK_COMPONENT_SWIZZLE_IDENTITY,
    .b = VK_COMPONENT_SWIZZLE_IDENTITY,
    .a = VK_COMPONENT_SWIZZLE_IDENTITY,
  };

  m_alloc.createImage(extent, usage, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_ASPECT_COLOR_BIT, components, m_offscreenImage);
}

void  Phosphene::createGBuffer() {
  m_alloc.destroyImage(m_gbuffer.color);
  m_alloc.destroyImage(m_gbuffer.normal);
  m_alloc.destroyImage(m_gbuffer.depth);
  m_alloc.destroyImage(m_gbuffer.material);

  VkExtent3D extent = {
    .width = m_width,
    .height = m_height, 
    .depth = 1,
  };
  VkImageUsageFlagBits  usage = static_cast<VkImageUsageFlagBits>(VK_IMAGE_USAGE_SAMPLED_BIT
      | VK_IMAGE_USAGE_STORAGE_BIT);
  VkComponentMapping  components = {
    .r = VK_COMPONENT_SWIZZLE_IDENTITY,
    .g = VK_COMPONENT_SWIZZLE_IDENTITY,
    .b = VK_COMPONENT_SWIZZLE_IDENTITY,
    .a = VK_COMPONENT_SWIZZLE_IDENTITY,
  };

  m_alloc.createImage(extent, usage, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_ASPECT_COLOR_BIT, components, m_gbuffer.color);
  m_alloc.createImage(extent, usage, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_ASPECT_COLOR_BIT, components, m_gbuffer.normal);
  m_alloc.createImage(extent, usage, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_ASPECT_COLOR_BIT, components, m_gbuffer.depth);
  m_alloc.createImage(extent, usage, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_ASPECT_COLOR_BIT, components, m_gbuffer.material);
}
