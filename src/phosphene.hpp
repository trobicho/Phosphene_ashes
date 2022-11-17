#pragma once
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "imgui.h"
#include "backend/phosStartVk.hpp"
#include "backend/vkImpl.hpp"
#include "helper/command.hpp"
#include "camera.hpp"
#include "sceneLoader/sceneLoader.hpp"
#include "raytracing/pipelineBuilder.hpp"
#include "raytracing/sceneBuilder.hpp"
#include "raytracing/rayPicker.hpp"
#include <mutex>
#include <string>

#define VIEW_RT_PIPELINE
#define VIEW_GBUFFER_COLOR
#define VIEW_GBUFFER_NORMAL
#define VIEW_GBUFFER_DEPTH

struct  GBuffer {
  ImageWrapper  color = ImageWrapper(VK_FORMAT_R32G32B32A32_SFLOAT);
  ImageWrapper  normal = ImageWrapper(VK_FORMAT_R32G32B32A32_SFLOAT);
  ImageWrapper  depth = ImageWrapper(VK_FORMAT_R32_SFLOAT);
  ImageWrapper  material = ImageWrapper(VK_FORMAT_R32G32_SINT);
};

class Phosphene {
  public:
    Phosphene(GLFWwindow *window);
    ~Phosphene() {destroy();}

    void  destroy();

    void  renderLoop();
    void  deviceWait() {
      if (vkDeviceWaitIdle(m_device) != VK_SUCCESS)
        throw PhosHelper::FatalError("Error waiting for device !!!");
    }
    bool  loadScene(const std::string &filename);

    //Callback
    void  callbackWindowResize(int width, int height);
    void  callbackKey(int key, int scancode, int action, int mods);
    void  callbackCursor(double x_pos, double y_pos);
    void  callbackMouseButton(int button, int action, int mod);

  private:
    void  draw();

    //Raytracing pipeline building
    void  buildGBufferPipeline();
    bool  buildPipeline(std::string name);
    void  buildRtPipelineBasicLights();
    void  buildRtPipelinePathTracing();
    void  updateRtImage();
    void  updateRtTlas() {
      updateRtTlas(m_sceneBuilder.getTlas());
    }
    void  updateRtTlas(AccelKHR &tlas);
    void  updateRtGlobalUBO();
    void  updateRtLights();

    std::mutex  m_mutexEvent;

    GLFWwindow  *m_window;
    uint32_t    m_width;
    uint32_t    m_height;
    bool        m_quit = false;
    bool        m_update = true;


    VkInstance        m_instance = VK_NULL_HANDLE;
    VkSurfaceKHR      m_surface;
    VkDevice          m_device = VK_NULL_HANDLE;
    VkPhysicalDevice  m_physicalDevice = VK_NULL_HANDLE;
    VkQueue           m_graphicsQueue = VK_NULL_HANDLE;
    uint32_t          m_graphicsQueueFamilyIndex;

    VkImpl  m_vkImpl;

    Camera              m_camera;
    GlobalUniforms      m_globalUniform;
    BufferWrapper       m_globalUBO;
    PushConstantRay     m_pcRay;
    RayPicker           m_rayPicker;

    PhosScene       m_scene;

    void  createGBuffer();
    void  createOffscreenRender();

    ImageWrapper    m_offscreenImage;
    GBuffer         m_gbuffer;

    MemoryAllocator m_alloc;

    RtBuilder::SceneBuilder m_sceneBuilder;
    RtBuilder::Pipeline     m_rtPipeline;
    RtBuilder::Pipeline     m_gbufferPipeline;

    //GUI Definition

    void  guiRender();

    struct  PhosGui {
      bool      winPipeline = true;
      bool      overlayActive = true;

      ImGuiID   pipelineList = 1;

      VkDescriptorPool  imguiDescPool;
    };
    PhosGui m_gui;         
    bool    m_showGui = true;

    void  guiOverlay();
};
