#pragma once
#include "../helper/phosHelper.hpp"
#include "../helper/allocator.hpp"
#include "../../shaders/hostDevice.h"
#include "../../shaders/hostDevicePicker.h"
#include <array>

class RayPicker {
  public:
    void  init(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex);
    void  destroy();

    void  build();

    void  updateCamera(BufferWrapper& cameraUBO);
    void  updateTlas(AccelKHR& tlas);

  private:
    void  buildDescSet();

    VkDevice          m_device = VK_NULL_HANDLE;
    VkPhysicalDevice  m_physicalDevice = VK_NULL_HANDLE;
    MemoryAllocator   m_alloc;
    CommandPool       m_cmdPool;

    VkPipeline          m_pipeline;
    VkPipelineLayout    m_pipelineLayout;
    VkDescriptorPool    m_descPool = VK_NULL_HANDLE;
    PushConstantPickRay m_pcPickRay;

    VkDescriptorSet                             m_descSet = VK_NULL_HANDLE;
    VkDescriptorSetLayout                       m_descSetLayout = VK_NULL_HANDLE;
    std::array<VkDescriptorSetLayoutBinding, 3> m_descSetLayoutBinds;
};
