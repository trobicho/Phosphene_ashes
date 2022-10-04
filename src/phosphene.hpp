#pragma once
#include "vkImpl.hpp"
#include "command.hpp"
#include "phosStartVk.hpp"

class Phosphene {
  public:
    Phosphene(GLFWwindow *window);
    ~Phosphene() {destroy();}

    void  destroy();
    void  deviceWait() {vkDeviceWaitIdle(m_device);}

    void  callbackWindowResize(int width, int height);

  private:
    GLFWwindow  *m_window;
    uint32_t    m_width;
    uint32_t    m_height;

    VkInstance        m_instance = VK_NULL_HANDLE;
    VkSurfaceKHR      m_surface;
    VkDevice          m_device = VK_NULL_HANDLE;
    VkPhysicalDevice  m_physicalDevice = VK_NULL_HANDLE;
    VkQueue           m_graphicsQueue = VK_NULL_HANDLE;
    uint32_t          m_graphicsQueueFamilyIndex;

    VkImpl  m_vkImpl;
};
