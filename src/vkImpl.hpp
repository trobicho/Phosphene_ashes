#pragma once
#include "phosHelper.hpp"
#include <vulkan/vulkan.h>
#include <vector>

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

    void  createSwapchain(const VkSurfaceKHR &surface, uint32_t width, uint32_t height);
    void  createFramebuffer();
    void  cleanupSwapchain();
    void  recreateSwapchain(const VkSurfaceKHR &surface, uint32_t width, uint32_t height);

    void  createRenderPass();

    VkInstance          m_instance = VK_NULL_HANDLE;
    VkDevice            m_device = VK_NULL_HANDLE;
    VkPhysicalDevice    m_physicalDevice = VK_NULL_HANDLE;
    uint32_t            m_queueFamily;
    VkQueue             m_queue = VK_NULL_HANDLE;

    SwapchainWrap   m_swapchainWrap;

    VkRenderPass    m_renderPass;
};
