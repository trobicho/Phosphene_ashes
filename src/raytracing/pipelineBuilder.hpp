#pragma once
#include "../helper/phosHelper.hpp"
#include "../helper/allocator.hpp"
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
  VkPipelineShaderStageCreateInfo stageInfo;
  std::string                     pName;
};

struct  HitShaderStage {
  std::string                     name;
  VkPipelineShaderStageCreateInfo stageInfo;
  std::string                     pName;
};

struct  HitGroup {
  VkRayTracingShaderGroupTypeKHR  type;
  std::string                     closestHitName = "";
  std::string                     anyHitName = "";
  std::string                     intersectionName = "";
};

struct  PushConstant {
  VkPushConstantRange range = {0, 0, 0};
  void*               pValues = nullptr;
};

struct  DescriptorSetUpdateInfo {
  VkDescriptorType  type;
  uint32_t          binding;
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
      VkDevice                                          device = VK_NULL_HANDLE;
      std::vector<DescriptorSetWrapper>                 &descSets;
      std::vector<RtBuilder::PushConstant>              &pushConstants;
      std::vector<VkRayTracingShaderGroupCreateInfoKHR> &shaderGroups;
      uint32_t                                          maxRayRecursion;
    };
    Pipeline(){};
    Pipeline(PipelineInitInfo &initInfo) {
      m_device = initInfo.device;
      m_descSets = initInfo.descSets;
      m_pushConstants = initInfo.pushConstants;
      m_shaderGroups = initInfo.shaderGroups;
      m_maxRayRecursion = initInfo.maxRayRecursion;
    }
    void  destroy();

    void  build(std::vector<VkPipelineShaderStageCreateInfo> stages);
    void  setShaderBindingTable(ShaderBindingTable &shaderBindingTable) {
      m_shaderBindingTable = shaderBindingTable;
    }
    
    void  raytrace(VkCommandBuffer commandBuffer, uint32_t width, uint32_t height);

    void  updateDescSet(const std::string name, const DescriptorSetUpdateInfo& info) {
      updateDescSet(name, &info, 1);
    }
    void  updateDescSet(const std::string name, const std::vector<DescriptorSetUpdateInfo>& info) {
      updateDescSet(name, info.data(), static_cast<uint32_t>(info.size()));
    }
    void  updateDescSet(const std::string name, const DescriptorSetUpdateInfo* info, uint32_t count);
    void  updateUBO(const VkCommandBuffer &cmdBuffer, const uint32_t size, BufferWrapper &deviceUBO, void* hostUBO);
    
    VkPipeline  m_pipeline = VK_NULL_HANDLE;

  private:
    void  buildDescriptorSets();

    VkDevice                                          m_device = VK_NULL_HANDLE;
    VkPipelineLayout                                  m_pipelineLayout = VK_NULL_HANDLE;
    VkDescriptorPool                                  m_descPool = VK_NULL_HANDLE;
    std::vector<DescriptorSetWrapper>                 m_descSets;
    std::vector<RtBuilder::PushConstant>              m_pushConstants;
    std::vector<VkRayTracingShaderGroupCreateInfoKHR> m_shaderGroups;
    RtBuilder::ShaderBindingTable                     m_shaderBindingTable;
    uint32_t                                          m_maxRayRecursion;
};

class   PipelineBuilder {
  public:
    PipelineBuilder() {};
    ~PipelineBuilder();
    
    void  init(VkDevice device, VkPhysicalDevice physicalDevice, MemoryAllocator* alloc, uint32_t queueFamilyIndex);

    void  build(Pipeline &pipeline);
    void  destroyModules();

    //Pipeline set Stages
    void  setRayGenStage(const std::string moduleFilename, const std::string pName = "main") {
      VkShaderModule module = PhosHelper::createShaderModuleFromFile(m_device, moduleFilename);
      setRayGenStage(module, pName);
    }
    void  setRayGenStage(VkShaderModule module, const std::string pName = "main");
    void  setRayGenStage(ShaderStage stageInfo) {
      m_rayGenStage = stageInfo;
    }

    void  addMissStage(const std::string moduleFilename, const std::string pName = "main") {
      VkShaderModule module = PhosHelper::createShaderModuleFromFile(m_device, moduleFilename);
      addMissStage(module, pName);
    }
    void  addMissStage(VkShaderModule module, const std::string pName = "main");
    void  addMissStage(ShaderStage stageInfo) {
      m_missStages.push_back(stageInfo);
    }

    void  addHitShader(const std::string name, const std::string moduleFilename, VkShaderStageFlagBits type, const std::string pName = "main") {
      VkShaderModule module = PhosHelper::createShaderModuleFromFile(m_device, moduleFilename);
      addHitShader(name, module, type, pName);
    }
    void  addHitShader(const std::string name, VkShaderModule module, VkShaderStageFlagBits type, const std::string pName = "main");
    void  addHitShader(const std::string name, ShaderStage stageInfo) {
      m_hitStages.push_back(HitShaderStage{.name = name, .stageInfo = stageInfo.stageInfo, .pName = stageInfo.pName});
    }

    void  addHitGroup(const RtBuilder::HitGroup &hitGroup) {
      m_hitGroups.push_back(hitGroup);
    }
    void  addHitGroup(const std::vector<RtBuilder::HitGroup> &hitGroups) {
      m_hitGroups.insert(m_hitGroups.end(), hitGroups.begin(), hitGroups.end());
    }

    void  addDescSet(const DescriptorSetWrapper &descSet) {
      m_descSets.push_back(descSet);
    }
    void  addDescSet(const std::vector<DescriptorSetWrapper> &descSets) {
      m_descSets.insert(m_descSets.end(), descSets.begin(), descSets.end());
    }
    void  addPushConstant(const RtBuilder::PushConstant &pushConstant) {
      m_pushConstants.push_back(pushConstant);
    }
    void  addPushConstant(const std::vector<RtBuilder::PushConstant> &pushConstants) {
      m_pushConstants.insert(m_pushConstants.end(), pushConstants.begin(), pushConstants.end());
    }
    void  setMaxRecursion(uint32_t max = 1);

  private:
    void  createShaderBindingTable(VkPipeline pipeline, std::vector<VkRayTracingShaderGroupCreateInfoKHR>& shaderGroups);

    VkDevice            m_device = VK_NULL_HANDLE;
    VkPhysicalDevice    m_physicalDevice = VK_NULL_HANDLE;
    MemoryAllocator*    m_alloc;
    CommandPool         m_cmdPool;
    ShaderBindingTable  m_shaderBindingTable;

    VkPhysicalDeviceRayTracingPipelinePropertiesKHR m_rtProperties {
      VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR
    };

    std::vector<DescriptorSetWrapper>     m_descSets;
    std::vector<RtBuilder::PushConstant>  m_pushConstants;

    ShaderStage                             m_rayGenStage;
    std::vector<ShaderStage>                m_missStages;
    std::vector<RtBuilder::HitShaderStage>  m_hitStages;
    std::vector<RtBuilder::HitGroup>        m_hitGroups;

    uint32_t  m_maxRecursion = 1;
};

}
