#include "vkImpl.hpp"
#include <algorithm>
#include <limits>

struct  SupportDetails {
  VkSurfaceCapabilitiesKHR          capability;
  std::vector<VkSurfaceFormatKHR>   formats;
  std::vector<VkPresentModeKHR>     presentModes;
};

void  VkImpl::cleanupSwapchain() {
  for (auto& imageView : m_swapchainWrap.imageView)
    vkDestroyImageView(m_device, imageView, nullptr);
  for (auto& framebuffer : m_swapchainWrap.framebuffer)
    vkDestroyFramebuffer(m_device, framebuffer, nullptr);
  vkDestroySwapchainKHR(m_device, m_swapchainWrap.chain, nullptr);
}

void  VkImpl::recreateSwapchain(const VkSurfaceKHR &surface, uint32_t width, uint32_t height) {
  deviceWait();

  cleanupSwapchain();

  createSwapchain(surface, width, height);
  createFramebuffer();
}

static void   getSupportDetails(VkPhysicalDevice device, VkSurfaceKHR surface, SupportDetails &details) {
  uint32_t  count;

  if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface
        , &details.capability) != VK_SUCCESS)
    throw PhosHelper::BasicError("Failed to get device surface capability");
  vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, nullptr);
  if (count != 0) {
    details.formats.resize(count);
    if (vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count
          , details.formats.data()) != VK_SUCCESS)
      throw PhosHelper::BasicError("Failed to get device surface formats");
  }
  vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, nullptr);
  if (count != 0) {
    details.presentModes.resize(count);
    if (vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count
          , details.presentModes.data()) != VK_SUCCESS)
      throw PhosHelper::BasicError("Failed to get device surface present modes");
  }
}

static VkSurfaceFormatKHR chooseFormat(SupportDetails &details) {
  if (details.formats.size() == 1
      && details.formats[0].format == VK_FORMAT_UNDEFINED) {
    return ((VkSurfaceFormatKHR){
        VK_FORMAT_B8G8R8_UNORM,
        VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
        });
  }
  for (auto format : details.formats) {
    if (format.format == VK_FORMAT_B8G8R8A8_UNORM
        && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
      return (format);
  }
  return (details.formats[0]);
}

static VkExtent2D  chooseExtent(SupportDetails &details, VkExtent2D actualExtent) {
  if (details.capability.currentExtent.width
      != std::numeric_limits<uint32_t>::max())
    return (details.capability.currentExtent);

  actualExtent = (VkExtent2D){
    .width = std::max(
        std::min(actualExtent.width, details.capability.maxImageExtent.width)
        , details.capability.minImageExtent.width),
      .height = std::max(
          std::min(actualExtent.height, details.capability.maxImageExtent.height)
          , details.capability.minImageExtent.height),
  };
  return (actualExtent);
}

static inline VkPresentModeKHR    choosePresentMode(SupportDetails &details) {
  return (VK_PRESENT_MODE_FIFO_KHR);
}

void  VkImpl::createSwapchain(const VkSurfaceKHR &surface, uint32_t width, uint32_t height) {
  SupportDetails details;

  VkExtent2D actualExtent = {
    static_cast<uint32_t>(width),
    static_cast<uint32_t>(height)
  };

  getSupportDetails(m_physicalDevice, surface, details);
  auto  format = chooseFormat(details);
  auto  presentMode = choosePresentMode(details);
  auto  extent = chooseExtent(details, actualExtent);

  uint32_t  imageCount = details.capability.minImageCount + 1;
  if (details.capability.maxImageCount > 0
      && imageCount > details.capability.maxImageCount)
    imageCount = details.capability.maxImageCount;

  VkSwapchainCreateInfoKHR  swapInfo = {
    .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
    .surface = surface,
    .minImageCount = imageCount,
    .imageFormat = format.format,
    .imageColorSpace = format.colorSpace,
    .imageExtent = extent,
    .imageArrayLayers = 1,
    .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
    .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
    .queueFamilyIndexCount = 0,
    .pQueueFamilyIndices = nullptr,
    .preTransform = details.capability.currentTransform,
    .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
    .clipped = VK_TRUE,
    .oldSwapchain = VK_NULL_HANDLE,
  };
  if (vkCreateSwapchainKHR(m_device, &swapInfo, nullptr, &m_swapchainWrap.chain)
      != VK_SUCCESS)
    throw PhosHelper::FatalVulkanInitError("Failed to create Swapchain!");

  if (vkGetSwapchainImagesKHR(m_device, m_swapchainWrap.chain
        , &imageCount, nullptr) != VK_SUCCESS)
    throw PhosHelper::FatalVulkanInitError("Failed to optain imageKHR!");

  if (imageCount <= 0)
    throw PhosHelper::FatalVulkanInitError("Failed to optain image count!");
  m_swapchainWrap.image.resize(imageCount);
  if (vkGetSwapchainImagesKHR(m_device, m_swapchainWrap.chain
        , &imageCount, m_swapchainWrap.image.data()) != VK_SUCCESS)
    throw PhosHelper::FatalVulkanInitError("Failed to optain imageKHR!");

  m_swapchainWrap.imageFormat = format.format;
  m_swapchainWrap.extent = extent;
  m_swapchainWrap.imageCount = imageCount;

  m_swapchainWrap.imageView.resize(imageCount);
  for (int i = 0; i < imageCount; ++i) {
    VkImageViewCreateInfo viewInfo = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .image = m_swapchainWrap.image[i],
      .viewType = VK_IMAGE_VIEW_TYPE_2D,
      .format = m_swapchainWrap.imageFormat,
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
    if (vkCreateImageView(m_device, &viewInfo
        , nullptr, &m_swapchainWrap.imageView[i]) != VK_SUCCESS)
      throw PhosHelper::FatalVulkanInitError("Failed to create ImageViews!");
  }
}

void  VkImpl::createFramebuffer() {
  uint32_t  imageCount = m_swapchainWrap.imageCount;

  m_swapchainWrap.framebuffer.resize(imageCount);
  for (int i = 0; i < imageCount; ++i) {
    VkFramebufferCreateInfo framebufferInfo = {
      .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
      .renderPass = m_renderPass,
      .attachmentCount = 1,
      .pAttachments = &m_swapchainWrap.imageView[i],
      .width = m_swapchainWrap.extent.width,
      .height = m_swapchainWrap.extent.height,
      .layers = 1,
    };
    if (vkCreateFramebuffer(m_device, &framebufferInfo
          , nullptr, &m_swapchainWrap.framebuffer[i]) != VK_SUCCESS)
      throw PhosHelper::FatalVulkanInitError("Failed to create Framebuffer!");
  }
}
