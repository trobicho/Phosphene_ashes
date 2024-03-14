#include "pipelineBuilder.hpp"
#include <iostream>
#include <cstring> //memcpy

namespace RtBuilder {

void  PipelineBuilder::init(VkDevice device, VkPhysicalDevice physicalDevice, MemoryAllocator* alloc, uint32_t queueFamilyIndex) {
  m_device = device;
  m_alloc = alloc;
  m_cmdPool.init(device, queueFamilyIndex);
  m_physicalDevice = physicalDevice;
  VkPhysicalDeviceProperties2 props2 = {
    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
    .pNext = &m_rtProperties,
  };
  vkGetPhysicalDeviceProperties2(m_physicalDevice, &props2);
}

PipelineBuilder::~PipelineBuilder() {
  m_cmdPool.destroy();
}

void  PipelineBuilder::destroyModules() {
  vkDestroyShaderModule(m_device, m_rayGenStage.stageInfo.module, nullptr);
  for (auto& missStage : m_missStages)
    vkDestroyShaderModule(m_device, missStage.stageInfo.module, nullptr);
  for (auto& hitStage : m_hitStages)
    vkDestroyShaderModule(m_device, hitStage.stageInfo.module, nullptr);
}

void  PipelineBuilder::setMaxRecursion(uint32_t max) {
  if (m_rtProperties.maxRayRecursionDepth < max) {
    std::string error = "MaxRayRecursion is inferior to: " + std::to_string(max) + " (" + std::to_string(m_rtProperties.maxRayRecursionDepth) + ")";
    throw PhosHelper::BasicError(error);
  }
  else
    m_maxRecursion = max;
}

void  PipelineBuilder::build(Pipeline &pipeline) {
  std::vector<VkPipelineShaderStageCreateInfo>  stages;
  std::vector<VkRayTracingShaderGroupCreateInfoKHR> shaderGroups;
  {
    m_rayGenStage.stageInfo.pName = m_rayGenStage.pName.c_str();
    stages.push_back(m_rayGenStage.stageInfo);
    for (auto& missStage: m_missStages) {
      missStage.stageInfo.pName = missStage.pName.c_str();
      stages.push_back(missStage.stageInfo);
    }
    for (auto& hitStage : m_hitStages) {
      hitStage.stageInfo.pName = hitStage.pName.c_str();
      stages.push_back(hitStage.stageInfo);
    }
    VkRayTracingShaderGroupCreateInfoKHR  shaderGroup = {
      .sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR,
      .generalShader = VK_SHADER_UNUSED_KHR,
      .closestHitShader = VK_SHADER_UNUSED_KHR,
      .anyHitShader = VK_SHADER_UNUSED_KHR,
      .intersectionShader = VK_SHADER_UNUSED_KHR,
    };
    shaderGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
    shaderGroup.generalShader = 0;
    shaderGroups.push_back(shaderGroup);
    for (uint32_t idx = 0; idx < m_missStages.size(); idx++) {
      shaderGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
      shaderGroup.generalShader = idx + 1;
      shaderGroups.push_back(shaderGroup);
    }
    auto getIdFromName = [&] (std::string name, uint32_t baseHitId) {
      for(uint32_t idx = 0; idx < m_hitStages.size(); idx++) {
        if (m_hitStages[idx].name == name) {
          return idx + baseHitId;
        }
      }
      return VK_SHADER_UNUSED_KHR;
    };
    uint32_t baseHitId = 1 + m_missStages.size();
    shaderGroup.generalShader = VK_SHADER_UNUSED_KHR;
    for (auto& hitGroup : m_hitGroups) {
      VkRayTracingShaderGroupCreateInfoKHR  shaderHitGroup = {
        .sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR,
        .type = hitGroup.type,
        .generalShader = VK_SHADER_UNUSED_KHR,
        .closestHitShader = VK_SHADER_UNUSED_KHR,
        .anyHitShader = VK_SHADER_UNUSED_KHR,
        .intersectionShader = VK_SHADER_UNUSED_KHR,
      };
      if (hitGroup.closestHitName != "")
        shaderHitGroup.closestHitShader = getIdFromName(hitGroup.closestHitName, baseHitId);
      if (hitGroup.anyHitName != "")
        shaderHitGroup.anyHitShader = getIdFromName(hitGroup.anyHitName, baseHitId);
      if (hitGroup.intersectionName != "")
        shaderHitGroup.intersectionShader = getIdFromName(hitGroup.intersectionName, baseHitId);
      if (shaderHitGroup.closestHitShader != VK_SHADER_UNUSED_KHR
          || shaderHitGroup.anyHitShader != VK_SHADER_UNUSED_KHR
          || shaderHitGroup.intersectionShader != VK_SHADER_UNUSED_KHR)
        shaderGroups.push_back(shaderHitGroup);
    }
  }
  
  Pipeline::PipelineInitInfo  info = {
    .device = m_device,
    .descSets = m_descSets,
    .pushConstants = m_pushConstants,
    .shaderGroups = shaderGroups,
    .maxRayRecursion = m_maxRecursion,
  };
  pipeline = Pipeline(info);
  pipeline.build(stages);
  createShaderBindingTable(pipeline.m_pipeline, shaderGroups);
  pipeline.setShaderBindingTable(m_shaderBindingTable);
}

void  PipelineBuilder::setRayGenStage(VkShaderModule module, const std::string pName) {
  ShaderStage shaderStage = {
    .pName = pName,
  };
  shaderStage.stageInfo = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
    .stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR,
    .module = module,
  };
  setRayGenStage(shaderStage);
}

void  PipelineBuilder::addMissStage(VkShaderModule module, const std::string pName) {
  ShaderStage shaderStage = {
    .pName = pName,
  };
  shaderStage.stageInfo = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
    .stage = VK_SHADER_STAGE_MISS_BIT_KHR,
    .module = module,
  };
  addMissStage(shaderStage);
}

void  PipelineBuilder::addHitShader(const std::string name, VkShaderModule module, VkShaderStageFlagBits type, const std::string pName) {
  ShaderStage shaderStage = {
    .pName = pName,
  };
  shaderStage.stageInfo = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
    .stage = type,
    .module = module,
  };
  addHitShader(name, shaderStage);
}

void  PipelineBuilder::createShaderBindingTable(VkPipeline pipeline, std::vector<VkRayTracingShaderGroupCreateInfoKHR>& shaderGroups) {
  uint32_t  missCount = m_missStages.size();
  uint32_t  hitCount = 0;

  for (auto& shaderGroup : shaderGroups) {
    if (shaderGroup.closestHitShader != VK_SHADER_UNUSED_KHR
        || shaderGroup.anyHitShader != VK_SHADER_UNUSED_KHR
        || shaderGroup.intersectionShader != VK_SHADER_UNUSED_KHR)
      hitCount += 1;
  }

  auto      handleCount = 1 + missCount + hitCount;
  uint32_t  handleSize  = m_rtProperties.shaderGroupHandleSize;

  auto alignUp = [](uint32_t addr, uint32_t alignement) {
    return ((addr + (alignement - 1)) & ~(alignement - 1));
  };
  uint32_t  handleSizeAligned = alignUp(handleSize, m_rtProperties.shaderGroupHandleAlignment);

  m_shaderBindingTable = (RtBuilder::ShaderBindingTable){
    .rgenRegion = {
      .stride = alignUp(handleSizeAligned, m_rtProperties.shaderGroupBaseAlignment),
    },
    .missRegion = {
      .stride = handleSizeAligned,
      .size = alignUp(missCount * handleSizeAligned, m_rtProperties.shaderGroupBaseAlignment),
    },
    .hitRegion = {
      .stride = handleSizeAligned,
      .size = alignUp(hitCount * handleSizeAligned, m_rtProperties.shaderGroupBaseAlignment),
    },
  };
  m_shaderBindingTable.rgenRegion.size = m_shaderBindingTable.rgenRegion.stride;

  uint32_t    dataSize = handleCount * handleSize;
  std::vector<uint8_t> handles(dataSize);
  if (vkGetRayTracingShaderGroupHandlesKHR(m_device, pipeline, 0, handleCount, dataSize, handles.data()) != VK_SUCCESS)
    throw PhosHelper::FatalVulkanInitError("Unable get RT shader group handles !");

  VkDeviceSize sbtSize = m_shaderBindingTable.rgenRegion.size
    + m_shaderBindingTable.missRegion.size
    + m_shaderBindingTable.hitRegion.size
    + m_shaderBindingTable.callRegion.size;

  m_alloc->createBuffer(static_cast<size_t>(sbtSize),
      static_cast<VkBufferUsageFlagBits>(
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT
        | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
        | VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR),
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
        | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
      m_shaderBindingTable.buffer);

  VkBufferDeviceAddressInfo info{VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, nullptr, m_shaderBindingTable.buffer.buffer};
  VkDeviceAddress           sbtAddress = vkGetBufferDeviceAddress(m_device, &info);
  m_shaderBindingTable.rgenRegion.deviceAddress = sbtAddress;
  m_shaderBindingTable.missRegion.deviceAddress = sbtAddress + m_shaderBindingTable.rgenRegion.size;
  m_shaderBindingTable.hitRegion.deviceAddress = sbtAddress + m_shaderBindingTable.rgenRegion.size + m_shaderBindingTable.missRegion.size;
  std::cout << "SBT Device Adress = " << m_shaderBindingTable.rgenRegion.deviceAddress << std::endl;

  {
    auto getHandle = [&] (int i) { return handles.data() + i * handleSize; };

    void*  data;
    vkMapMemory(m_device, m_shaderBindingTable.buffer.memory, 0, sbtSize, 0, &data);
    auto*     pSBTBuffer = reinterpret_cast<uint8_t*>(data);
    uint8_t*  pData{nullptr};
    uint32_t  handleIdx{0};
    pData = pSBTBuffer;
    memcpy(pData, getHandle(handleIdx++), handleSize);

    pData = pSBTBuffer + m_shaderBindingTable.rgenRegion.size;
    for(uint32_t c = 0; c < missCount; c++)
    {
      memcpy(pData, getHandle(handleIdx++), handleSize);
      pData += m_shaderBindingTable.missRegion.stride;
    }
    pData = pSBTBuffer + m_shaderBindingTable.rgenRegion.size + m_shaderBindingTable.missRegion.size;
    for(uint32_t c = 0; c < hitCount; c++)
    {
      memcpy(pData, getHandle(handleIdx++), handleSize);
      pData += m_shaderBindingTable.hitRegion.stride;
    }
    vkUnmapMemory(m_device, m_shaderBindingTable.buffer.memory);
    //vkDestroyBuffer(m_device, m_shaderBindingTable.buffer.buffer, nullptr);
    m_shaderBindingTable.buffer.buffer = VK_NULL_HANDLE;
  }
}

}
