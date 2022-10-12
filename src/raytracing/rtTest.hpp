#pragma once
#include "../helper/phosHelper.hpp"
#include "../helper/allocator.hpp"
#include "../../shaders/hostDevice.h"

class RtTest {
  public:
    RtTest(){};

    void  init(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex);
    void  raytrace(VkCommandBuffer commandBuffer, uint32_t width, uint32_t height);
    void  destroy();
    void  createDescriptorSet(const VkImageView &imageView, const VkAccelerationStructureKHR tlas = VK_NULL_HANDLE);
    void  updateDescriptorSet(const VkImageView &imageView);
    void  updateDescriptorSet(const VkAccelerationStructureKHR &tlas);
    void  updateUniformBuffer(const VkCommandBuffer &cmdBuffer, GlobalUniforms &uniform);
    void  createPipeline();
    void  createShaderBindingTable();

    VkDevice          m_device = VK_NULL_HANDLE;
    VkPhysicalDevice  m_physicalDevice = VK_NULL_HANDLE;
    MemoryAllocator   m_alloc;

    BufferWrapper     m_globalUBO;


    VkPhysicalDeviceRayTracingPipelinePropertiesKHR m_properties {
      VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR
    };

    VkPipeline        m_pipeline = VK_NULL_HANDLE;
    VkPipelineLayout  m_pipelineLayout = VK_NULL_HANDLE;

   std::vector<VkDescriptorSetLayoutBinding>
      m_descSetLayoutBinds;
    std::vector<VkDescriptorSetLayoutBinding>
      m_descSetLayoutGlobalBinds;
    VkDescriptorPool            m_descPool;
    VkDescriptorSetLayout       m_descSetLayout;
    VkDescriptorSetLayout       m_descSetLayoutGlobal;
    VkDescriptorSet             m_descSet;
    VkDescriptorSet             m_descSetGlobal;

    std::vector<VkRayTracingShaderGroupCreateInfoKHR>
      m_shaderGroups;

    PushConstantRay             m_pcRay{};

    //SHADER BINDNG TABLE
    BufferWrapper                     m_SBTBuffer;
    VkStridedDeviceAddressRegionKHR   m_rgenRegion{};
    VkStridedDeviceAddressRegionKHR   m_missRegion{};
    VkStridedDeviceAddressRegionKHR   m_hitRegion{};
    VkStridedDeviceAddressRegionKHR   m_callRegion{};
};
