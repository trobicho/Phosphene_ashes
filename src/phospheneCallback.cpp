#include "phosphene.hpp"
#include <iostream>


void  Phosphene::callbackWindowResize(int width, int height) {
  m_mutexEvent.lock();
  deviceWait();
  m_width = width;
  m_height = height;
  m_vkImpl.cleanupSwapchain();
  vkDestroySurfaceKHR(m_instance, m_surface, nullptr);

  if (glfwCreateWindowSurface(m_instance, m_window, nullptr, &m_surface) != VK_SUCCESS)
    throw PhosHelper::FatalVulkanInitError("Failed to create Surface");
  m_vkImpl.recreateSwapchain(m_surface, width, height);

  createOffscreenRender();
  createGBuffer();

  m_camera.eventChangeAspectRatio(m_width, m_height);
  m_vkImpl.updatePostDescSet(m_offscreenImage.imageView);
  ImGuiIO& io = ImGui::GetIO(); (void)io;
  io.DisplaySize = ImVec2(static_cast<float>(m_width), static_cast<float>(m_height));
  ImGui::SetNextWindowSize(ImVec2(static_cast<float>(m_width), static_cast<float>(m_height)));
  ImGui::SetNextWindowPos(ImVec2(0, 0));
  updateRtImage();
  deviceWait();
  m_mutexEvent.unlock();
}

void  Phosphene::callbackKey(int key, int scancode, int action, int mods) {
  m_mutexEvent.lock();
  if (key == GLFW_KEY_ESCAPE)
    m_quit = true;
  if (key == GLFW_KEY_W) {
    if (action == GLFW_PRESS)
      m_camera.setKeyState(CAMERA_KEY_STATE_FORWARD);
    else if (action == GLFW_RELEASE)
      m_camera.unsetKeyState(CAMERA_KEY_STATE_FORWARD);
  }
  if (key == GLFW_KEY_A) {
    if (action == GLFW_PRESS)
      m_camera.setKeyState(CAMERA_KEY_STATE_LEFT);
    else if (action == GLFW_RELEASE)
      m_camera.unsetKeyState(CAMERA_KEY_STATE_LEFT);
  }
  if (key == GLFW_KEY_S) {
    if (action == GLFW_PRESS)
      m_camera.setKeyState(CAMERA_KEY_STATE_BACKWARD);
    else if (action == GLFW_RELEASE)
      m_camera.unsetKeyState(CAMERA_KEY_STATE_BACKWARD);
  }
  if (key == GLFW_KEY_D) {
    if (action == GLFW_PRESS)
      m_camera.setKeyState(CAMERA_KEY_STATE_RIGHT);
    else if (action == GLFW_RELEASE)
      m_camera.unsetKeyState(CAMERA_KEY_STATE_RIGHT);
  }
  if (key == GLFW_KEY_Q) {
    if (action == GLFW_PRESS)
      m_camera.setKeyState(CAMERA_KEY_STATE_ROLL_COUNTER_CLOCKWISE);
    else if (action == GLFW_RELEASE)
      m_camera.unsetKeyState(CAMERA_KEY_STATE_ROLL_COUNTER_CLOCKWISE);
  }
  if (key == GLFW_KEY_E) {
    if (action == GLFW_PRESS)
      m_camera.setKeyState(CAMERA_KEY_STATE_ROLL_CLOCKWISE);
    else if (action == GLFW_RELEASE)
      m_camera.unsetKeyState(CAMERA_KEY_STATE_ROLL_CLOCKWISE);
  }
  m_mutexEvent.unlock();
}

void  Phosphene::callbackCursor(double x_pos, double y_pos) {
  m_mutexEvent.lock();
  m_camera.rotate(x_pos, y_pos);
  m_mutexEvent.unlock();
}

void  Phosphene::callbackMouseButton(int button, int action, int mod) {
  m_mutexEvent.lock();
  if (button == GLFW_MOUSE_BUTTON_LEFT) {
    if (action == GLFW_PRESS) {
      double  x, y;
      glfwGetCursorPos(m_window, &x, &y);
      glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
      m_camera.setAllowRotation(action == GLFW_PRESS, x, y);
    }
    else {
      glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
      m_camera.setAllowRotation(false);
    }
  }
  m_mutexEvent.unlock();
}
