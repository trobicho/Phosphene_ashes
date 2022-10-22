#pragma once
#include "../helper/phosHelper.hpp"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "imgui.h"
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

class PhosGui {
  public:
    PhosGui(){};

    void  destroy();
    void  render();

  private:
    bool    m_guiActive = true;
    ImGuiIO m_io;
};
