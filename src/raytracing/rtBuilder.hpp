#pragma once
#include "../helper/phosHelper.hpp"
#include "../sceneLoader/scene.hpp"
#include <vector>

namespace RtBuilder {

/*
enum    ShaderType {
  eRaygen,
  eMiss,
  eClosestHit,
  eIntersection,
};


class SBTable {
  public:
    VkStridedDeviceAddressRegionKHR   rgenRegion{};
};
*/

struct  ShaderStage {
  std::string                     moduleFileName;
  VkPipelineShaderStageCreateInfo stageInfo;
  VkShaderStageFlagBits           type;
};

struct  PushConstant {
  VkPushConstantRange range = {0, 0, 0};
  void*               pValues = nullptr;
};

struct  DescriptorSetUpdateInfo {
  VkDescriptorType  type;
  uint32_t          index = 0;
  void*             pInfo = nullptr;
};

struct  DescriptorSetWrapper {
  std::string                               name = "";
  VkDescriptorSet                           set = VK_NULL_HANDLE;
  VkDescriptorSetLayout                     layout = VK_NULL_HANDLE;
  std::vector<VkDescriptorSetLayoutBinding> layoutBinds;

  void  destroy(VkDevice &device) {
    vkDestroyDescriptorSetLayout(device, layout, nullptr);
    layout = VK_NULL_HANDLE;
  }
};

struct  ShaderBindingTable {
  BufferWrapper                     buffer;
  VkStridedDeviceAddressRegionKHR   rgenRegion{};
  VkStridedDeviceAddressRegionKHR   missRegion{};
  VkStridedDeviceAddressRegionKHR   hitRegion{};
  VkStridedDeviceAddressRegionKHR   callRegion{};
};

class   Pipeline {
  public:
    struct PipelineInitInfo {
      VkDevice                              device = VK_NULL_HANDLE;
      VkPipeline                            pipeline = VK_NULL_HANDLE;
      VkPipelineLayout                      pipelineLayout = VK_NULL_HANDLE;
      VkDescriptorPool                      descPool = VK_NULL_HANDLE;
      std::vector<DescriptorSetWrapper>     &descSets;
      std::vector<RtBuilder::PushConstant>  &pushConstants;
      ShaderBindingTable                    shaderBindingTable;
    };
    Pipeline(){};
    Pipeline(PipelineInitInfo &initInfo) {
      m_device = initInfo.device;
      m_pipeline = initInfo.pipeline;
      m_pipelineLayout = initInfo.pipelineLayout;
      m_descPool = initInfo.descPool;
      m_descSets = initInfo.descSets;
      m_pushConstants = initInfo.pushConstants;
      m_shaderBindingTable = initInfo.shaderBindingTable;
    }
    void  destroy();
    
    void  raytrace(VkCommandBuffer commandBuffer, uint32_t width, uint32_t height);

    void  updateDescSet(const std::string name, const DescriptorSetUpdateInfo& info) {
      updateDescSet(name, &info, 1);
    }
    void  updateDescSet(const std::string name, const std::vector<DescriptorSetUpdateInfo>& info) {
      updateDescSet(name, info.data(), static_cast<uint32_t>(info.size()));
    }
    void  updateDescSet(const std::string name, const DescriptorSetUpdateInfo* info, uint32_t count);


  private:
    VkDevice                              m_device = VK_NULL_HANDLE;
    VkPipeline                            m_pipeline = VK_NULL_HANDLE;
    VkPipelineLayout                      m_pipelineLayout = VK_NULL_HANDLE;
    VkDescriptorPool                      m_descPool = VK_NULL_HANDLE;
    std::vector<DescriptorSetWrapper>     m_descSets;
    std::vector<RtBuilder::PushConstant>  m_pushConstants;
    RtBuilder::ShaderBindingTable         m_shaderBindingTable;
};

class   PipelineBuilder {
  public:
    PipelineBuilder() {};
    
    void  init(VkDevice device, VkPhysicalDevice physicalDevice, MemoryAllocator* alloc, uint32_t queueFamilyIndex);

    Pipeline  build();

    void  addDescSet(const DescriptorSetWrapper &descSet) {
      m_descSets.push_back(descSet);
    }
    void  addDescSet(const std::vector<DescriptorSetWrapper> &descSets) {
      m_descSets.insert(m_descSets.end(), descSets.begin(), descSets.end());
    }
    void  buildDescriptorSets();
    void  addPushConstant(const RtBuilder::PushConstant &pushConstant) {
      m_pushConstants.push_back(pushConstant);
    }
    void  addPushConstant(const std::vector<RtBuilder::PushConstant> &pushConstants) {
      m_pushConstants.insert(m_pushConstants.end(), pushConstants.begin(), pushConstants.end());
    }
    void  setMaxRecursion(uint32_t max = 1);

  private:
    VkDevice            m_device = VK_NULL_HANDLE;
    VkPhysicalDevice    m_physicalDevice = VK_NULL_HANDLE;
    MemoryAllocator*    m_alloc;
    CommandPool         m_cmdPool;
    ShaderBindingTable  m_shaderBindingTable;

    VkPhysicalDeviceRayTracingPipelinePropertiesKHR m_rtProperties {
      VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR
    };

    VkDescriptorPool                      m_descPool;
    std::vector<DescriptorSetWrapper>     m_descSets;
    std::vector<RtBuilder::PushConstant>  m_pushConstants;

    std::vector<VkPipelineShaderStageCreateInfo>    m_stages;
  std::vector<VkRayTracingShaderGroupCreateInfoKHR> m_shaderGroups;

    uint32_t  m_maxRecursion = 1;
};

struct  BlasInput {
  VkAccelerationStructureGeometryKHR        asGeometry;
  VkAccelerationStructureBuildRangeInfoKHR  asBuildOffsetInfo;
  VkBuildAccelerationStructureFlagsKHR      flags{0};
};

class   SceneBuilder {
  public:
    SceneBuilder(){};

    void  init(VkDevice device, MemoryAllocator* alloc, uint32_t queueFamilyIndex);
    void  destroy();

    void  buildBlas(PhosScene& scene, VkBuildAccelerationStructureFlagsKHR flags);
    void  buildTlas(PhosScene& scene, VkBuildAccelerationStructureFlagsKHR flags);

    AccelKHR& getTlas() {return (m_tlas);}

  private:
    struct  BuildAccelerationStructure {
      VkAccelerationStructureBuildGeometryInfoKHR     buildInfo{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR};
      VkAccelerationStructureBuildSizesInfoKHR        sizeInfo{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR};
      const VkAccelerationStructureBuildRangeInfoKHR* rangeInfo;
    };

    void  modelToVkGeometry(PhosObjectMesh& model);
    void  cmdCreateBlas(VkCommandBuffer cmdBuffer
                        , std::vector<uint32_t> indices
                        , std::vector<BuildAccelerationStructure> buildAs
                        , VkDeviceAddress scratchAddress);

    VkDevice          m_device = VK_NULL_HANDLE;
    MemoryAllocator*  m_alloc;

    std::vector<BlasInput>  m_blasInput;
    std::vector<AccelKHR>   m_blas;
    AccelKHR                m_tlas;
    CommandPool             m_cmdPool;
};

}
