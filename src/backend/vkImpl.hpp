#pragma once
#include "../helper/phosHelper.hpp"
#include <vulkan/vulkan.h>
#include <vector>
#include <array>

#define MAX_FRAMES_IN_FLIGHT  3

struct  SwapchainWrap {
  VkSwapchainKHR  chain;
  uint32_t        imageCount;
  VkFormat        imageFormat;
  VkExtent2D      extent;
  uint32_t        imageCurrent = 0;

  std::vector<VkImage>        image;
  std::vector<VkImageView>    imageView;
  std::vector<VkFramebuffer>  framebuffer;
};

class VkImpl {
  public:
    VkImpl() {};

    void  deviceWait() {vkDeviceWaitIdle(m_device);}
    void  destroy();

    void  setup(VkDevice &device
              , VkPhysicalDevice &physicalDevice
              , uint32_t queueFamilyIndex
              , VkQueue defaultqueue = VK_NULL_HANDLE);

    void  createSwapchain(const VkSurfaceKHR &surface, uint32_t width, uint32_t height);
    void  createFramebuffer();
    void  cleanupSwapchain();
    void  recreateSwapchain(const VkSurfaceKHR &surface, uint32_t width, uint32_t height);

    void  createRenderPass();
    void  createCommandPool();

    void  createPipeline() {
      createPostDescriptorSet();
      createPostPipeline();
    }
    void  updatePostDescSet(VkImageView &offscreenImageView);

    VkCommandBuffer&  getCommandBuffer(VkSemaphore &semaphoreWait, VkSemaphore &semaphoreSignal);
    VkFramebuffer&  getFramebuffer(){
      return (m_swapchainWrap.framebuffer[m_currentImageIndex]);
    }
    VkResult          acquireNextImage(uint32_t &imageIndex, VkFence &fence);
    void              recordCommandBuffer(VkCommandBuffer &commandBuffer);
    void              present();

    VkInstance          m_instance = VK_NULL_HANDLE;
    VkDevice            m_device = VK_NULL_HANDLE;
    VkPhysicalDevice    m_physicalDevice = VK_NULL_HANDLE;
    uint32_t            m_queueFamilyIndex;
    VkQueue             m_queue = VK_NULL_HANDLE;

    SwapchainWrap       m_swapchainWrap;

    VkRenderPass        m_renderPass;
    VkPipeline          m_postPipeline = VK_NULL_HANDLE;

  private:
    void  createPostDescriptorSet();
    void  createPostPipeline();
    void  createSynchronisationObjects();

    VkSampler               m_sampler = VK_NULL_HANDLE;

    VkDescriptorSet         m_postDescSet = VK_NULL_HANDLE;
    VkDescriptorPool        m_postDescPool = VK_NULL_HANDLE;
    VkDescriptorSetLayout   m_postDescSetLayout = VK_NULL_HANDLE;
    std::array<VkDescriptorSetLayoutBinding, 1>
                            m_postDescSetLayoutBinds;
    VkPipelineLayout        m_postPipelineLayout = VK_NULL_HANDLE;

    VkCommandPool                                       m_commandPool = VK_NULL_HANDLE;
    std::array<VkCommandBuffer, MAX_FRAMES_IN_FLIGHT>   m_commandBuffers;
    std::array<VkFence, MAX_FRAMES_IN_FLIGHT>           m_fences{VK_NULL_HANDLE};
    std::array<VkSemaphore, MAX_FRAMES_IN_FLIGHT>       m_semaphoreAvailable{VK_NULL_HANDLE};
    std::array<VkSemaphore, MAX_FRAMES_IN_FLIGHT>       m_semaphoreFinish{VK_NULL_HANDLE};
    uint32_t                                            m_currentFrame = 0;
    uint32_t                                            m_currentImageIndex = 0;
};
