#pragma once
#include "vkImpl.hpp"
#include "command.hpp"
#include "phosStartVk.hpp"
#include "camera.hpp"

//TESTING
#include "rtTest.hpp"

class Phosphene {
  public:
    Phosphene(GLFWwindow *window);
    ~Phosphene() {destroy();}

    void  destroy();

    void  renderLoop();
    void  deviceWait() {vkDeviceWaitIdle(m_device);}

    void  callbackWindowResize(int width, int height);

  private:
    void  draw();

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

    Camera  m_camera;


    void  createOffscreenRender();
    VkImage           m_offscreenColor{VK_NULL_HANDLE};
    VkFormat          m_offscreenColorFormat{VK_FORMAT_R32G32B32A32_SFLOAT};
    VkImageView       m_offscreenImageView{VK_NULL_HANDLE};
    VkDeviceMemory    m_offscreenImageMemory{VK_NULL_HANDLE};

    VkSemaphore       m_semaphoreRTFinish;
    VkCommandPool     m_commandPool;
    VkCommandBuffer   m_commandBuffer;

    //TESTING
    RtTest  m_rtTest;
    MemoryAllocator m_alloc;
};
