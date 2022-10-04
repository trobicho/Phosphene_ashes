#include "phosphene.hpp"

Phosphene::Phosphene(GLFWwindow *window): m_window(window) {
  m_instance = PhosStartVk::createInstance();
  if (glfwCreateWindowSurface(m_instance, m_window, nullptr, &m_surface) != VK_SUCCESS)
    throw PhosHelper::FatalVulkanInitError("Failed to create Surface");
  m_physicalDevice = PhosStartVk::choosePhysicalDevice(m_instance);
  PhosStartVk::createLogicalDeviceAndQueue(m_device, m_physicalDevice, m_surface, m_graphicsQueue, m_graphicsQueueFamilyIndex);

  int width = 0;
  int height = 0;
  glfwGetWindowSize(m_window, &width, &height);
  if (width <= 0 || height <= 0)
    throw PhosHelper::FatalError("Windows as invalid size");
  m_width = static_cast<uint32_t>(width);
  m_height = static_cast<uint32_t>(height);

  {
    m_vkImpl.init(m_device, m_physicalDevice, m_graphicsQueueFamilyIndex, m_graphicsQueue);
    m_vkImpl.createSwapchain(m_surface, m_width, m_height);
    m_vkImpl.createRenderPass();
    m_vkImpl.createCommandPool();
    m_vkImpl.createFramebuffer();
    m_vkImpl.createPipeline();
  }
}

void  Phosphene::destroy() {
  if (m_instance != VK_NULL_HANDLE) {
    deviceWait();

    m_vkImpl.destroy();

    vkDestroyDevice(m_device, nullptr);
    vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
    vkDestroyInstance(m_instance, nullptr);
    m_instance = VK_NULL_HANDLE;
  }
}
