#include "phosphene.hpp"

void  Phosphene::guiRender() {
  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
  guiOverlay();
  if (!ImGui::Begin("Pipelines", &m_gui.winPipeline)) {
    ImGui::End();
    return ;
  }
  ImGui::Spacing();
  if (ImGui::BeginListBox("Pipelines:")) {
    ImGui::Spacing();
    if (ImGui::Selectable("Basic")) {
      buildPipeline("basic");
    }
    if (ImGui::Selectable("BasicLight")) {
      buildPipeline("basicLights");
    }
    if (ImGui::Selectable("PathTracing")) {
      buildPipeline("pathTracing");
    }
    ImGui::EndListBox();
  }
  ImGui::End();
}

void  Phosphene::guiOverlay() {
  static int location = 0;
  ImGuiIO& io = ImGui::GetIO();
  ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration
    | ImGuiWindowFlags_AlwaysAutoResize
    | ImGuiWindowFlags_NoSavedSettings
    | ImGuiWindowFlags_NoFocusOnAppearing
    | ImGuiWindowFlags_NoNav;
  if (location >= 0)
  {
    const float PAD = 10.0f;
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 work_pos = viewport->WorkPos;
    ImVec2 work_size = viewport->WorkSize;
    ImVec2 window_pos, window_pos_pivot;
    window_pos.x = (location & 1) ? (work_pos.x + work_size.x - PAD) : (work_pos.x + PAD);
    window_pos.y = (location & 2) ? (work_pos.y + work_size.y - PAD) : (work_pos.y + PAD);
    window_pos_pivot.x = (location & 1) ? 1.0f : 0.0f;
    window_pos_pivot.y = (location & 2) ? 1.0f : 0.0f;
    ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
    window_flags |= ImGuiWindowFlags_NoMove;
  }
  ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
  if (ImGui::Begin("Overlay", &m_gui.overlayActive, window_flags))
  {
    ImGui::Text("Application average\n" "%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::Separator();
    if (ImGui::BeginPopupContextWindow())
    {
      if (ImGui::MenuItem("Custom", nullptr, location == -1)) location = -1;
      if (ImGui::MenuItem("Top-left", nullptr, location == 0)) location = 0;
      if (ImGui::MenuItem("Top-right", nullptr, location == 1)) location = 1;
      if (ImGui::MenuItem("Bottom-left", nullptr, location == 2)) location = 2;
      if (ImGui::MenuItem("Bottom-right", nullptr, location == 3)) location = 3;
      if (m_gui.overlayActive && ImGui::MenuItem("Close"))
        m_gui.overlayActive = false;
      ImGui::EndPopup();
    }
  }
  ImGui::End();
}
