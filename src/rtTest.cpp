#include "rtTest.hpp"
#include <array>
#include <cstring> //memcpy
#include <iostream>

void  RtTest::init(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex) {
  m_device = device;
  m_physicalDevice = physicalDevice;
  VkPhysicalDeviceProperties2 props2 = {
    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
    .pNext = &m_properties,
  };
  vkGetPhysicalDeviceProperties2(m_physicalDevice, &props2);
  m_alloc.init(m_device, m_physicalDevice, queueFamilyIndex);
  
  {
    pfnVkCreateRayTracingPipelinesKHR =
      reinterpret_cast<PFN_vkCreateRayTracingPipelinesKHR>(vkGetDeviceProcAddr(device, "vkCreateRayTracingPipelinesKHR"));
    pfnVkGetRayTracingShaderGroupHandlesKHR =
      reinterpret_cast<PFN_vkGetRayTracingShaderGroupHandlesKHR>(vkGetDeviceProcAddr(device, "vkGetRayTracingShaderGroupHandlesKHR"));
    pfnVkDestroyAccelerationStructureKHR =
      reinterpret_cast<PFN_vkDestroyAccelerationStructureKHR>(vkGetDeviceProcAddr(device, "vkDestroyAccelerationStructureKHR"));
    pfnVkCmdTraceRaysKHR =
      reinterpret_cast<PFN_vkCmdTraceRaysKHR>(vkGetDeviceProcAddr(device, "vkCmdTraceRaysKHR"));
  }
}

void  RtTest::raytrace(VkCommandBuffer commandBuffer, uint32_t width, uint32_t height) {
  std::vector<VkDescriptorSet> descSets{m_descSet};
  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, m_pipeline);
  vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR
      , m_pipelineLayout, 0, descSets.size(), descSets.data(), 0, nullptr);
  vkCmdPushConstants(commandBuffer, m_pipelineLayout
      , VK_SHADER_STAGE_RAYGEN_BIT_KHR
        | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR
        | VK_SHADER_STAGE_MISS_BIT_KHR
      , 0, sizeof(PushConstantRay), &m_pcRay);
  pfnVkCmdTraceRaysKHR(commandBuffer, &m_rgenRegion, &m_missRegion, &m_hitRegion, &m_callRegion, width, height, 1);
}

void  RtTest::destroy() {
  pfnVkDestroyAccelerationStructureKHR(m_device, m_blas, nullptr);
  pfnVkDestroyAccelerationStructureKHR(m_device, m_tlas, nullptr);
  vkFreeMemory(m_device, m_SBTBuffer.memory, nullptr);
  vkDestroyDescriptorSetLayout(m_device, m_descSetLayout, nullptr);
  vkDestroyDescriptorPool(m_device, m_descPool, nullptr);
  vkDestroyPipeline(m_device, m_pipeline, nullptr);
  vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);
  //vkDestroyBuffer(m_device, m_blasBuffer.buffer, nullptr);
  //vkFreeMemory(m_device, m_blasBuffer.memory, nullptr);
  //vkDestroyBuffer(m_device, m_tlasBuffer.buffer, nullptr);
  //vkFreeMemory(m_device, m_tlasBuffer.memory, nullptr);
}

void  RtTest::createDescriptorSet(const VkImageView &imageView) {
  m_descSetLayoutBinds.resize(2);
  m_descSetLayoutBinds[0] = {
    .binding = 0,
    .descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,
    .descriptorCount = 1,
    .stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR,
  };
  m_descSetLayoutBinds[1] = {
    .binding = 1,
    .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
    .descriptorCount = 1,
    .stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR,
  };

  VkDescriptorPoolSize        descPoolSize[2] = {
    (VkDescriptorPoolSize){
      .type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
      .descriptorCount = 1,
    },
    (VkDescriptorPoolSize){
      .type = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,
      .descriptorCount = 1,
    },
  };
  VkDescriptorPoolCreateInfo  descPoolInfo = {
    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
    .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT | VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
    .maxSets = 1,
    .poolSizeCount = 2,
    .pPoolSizes = descPoolSize,
  };
  if(vkCreateDescriptorPool(m_device, &descPoolInfo, nullptr, &m_descPool) != VK_SUCCESS)
    throw PhosHelper::FatalVulkanInitError("Failed to create RT descriptor pool !");

  VkDescriptorSetLayoutCreateInfo descSetLayoutInfo = {
    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
    .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
    .bindingCount = 2,
    .pBindings = m_descSetLayoutBinds.data(),
  };

  if (vkCreateDescriptorSetLayout(m_device, &descSetLayoutInfo, nullptr, &m_descSetLayout) != VK_SUCCESS)
    throw PhosHelper::FatalVulkanInitError("Failed to create RT descriptor set layout !");

  VkDescriptorSetAllocateInfo descSetAllocInfo = {
    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
    .descriptorPool = m_descPool,
    .descriptorSetCount = 1,
    .pSetLayouts = &m_descSetLayout,
  };

  if (vkAllocateDescriptorSets(m_device, &descSetAllocInfo, &m_descSet) != VK_SUCCESS)
    throw PhosHelper::FatalVulkanInitError("Failed to Allocate RT descriptor set !");

  VkWriteDescriptorSetAccelerationStructureKHR  descASInfo = {
    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR,
    .accelerationStructureCount = 1,
    .pAccelerationStructures = &m_tlas,
  };
  VkDescriptorImageInfo imageInfo = {
    .imageView = imageView,
    .imageLayout = VK_IMAGE_LAYOUT_GENERAL,
    //.sampler = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
  };

  std::vector<VkWriteDescriptorSet> writes = {
    (VkWriteDescriptorSet){
      .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .pNext = &descASInfo,
      .dstSet = m_descSet,
      .dstBinding = 0,
      .dstArrayElement = 0,
      .descriptorCount = 1,
      .descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,
    },
    (VkWriteDescriptorSet){
      .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .dstSet = m_descSet,
      .dstBinding = 1,
      .dstArrayElement = 0,
      .descriptorCount = 1,
      .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
      .pImageInfo = &imageInfo,
    },
  };
  vkUpdateDescriptorSets(m_device, writes.size(), writes.data(), 0, nullptr);
}

void  RtTest::updateDescriptorSet(const VkImageView &imageView) {
  VkDescriptorImageInfo imageInfo{{}, imageView, VK_IMAGE_LAYOUT_GENERAL};
  VkWriteDescriptorSet  writeDescSet = {
    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
    .dstSet = m_descSet,
    .dstBinding = 1,
    .dstArrayElement = 0,
    .descriptorCount = 1,
    .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
    .pImageInfo = &imageInfo,
  };
  vkUpdateDescriptorSets(m_device, 1, &writeDescSet, 0, nullptr);
}

void  RtTest::createPipeline() {
  enum StageIndices {
    eRaygen,
    eMiss,
    eClosestHit,
    eShaderGroupCount 
  };

  // All stages
  VkShaderModule  rgenShaderModule = PhosHelper::createShaderModuleFromFile(m_device, "./spv/raytrace.rgen.spv");
  VkShaderModule  rmissShaderModule = PhosHelper::createShaderModuleFromFile(m_device, "./spv/raytrace.rmiss.spv");
  VkShaderModule  rchitShaderModule = PhosHelper::createShaderModuleFromFile(m_device, "./spv/raytrace.rchit.spv");

  std::array<VkPipelineShaderStageCreateInfo, eShaderGroupCount> stages{
    (VkPipelineShaderStageCreateInfo){
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR,
      .module = rgenShaderModule,
      .pName = "main",
    },
    (VkPipelineShaderStageCreateInfo){
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage = VK_SHADER_STAGE_MISS_BIT_KHR,
      .module = rmissShaderModule,
      .pName = "main",
    },
    (VkPipelineShaderStageCreateInfo){
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR,
      .module = rchitShaderModule,
      .pName = "main",
    },
  };
  VkRayTracingShaderGroupCreateInfoKHR  shaderGroup = {
    .sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR,
    .generalShader = VK_SHADER_UNUSED_KHR,
    .closestHitShader = VK_SHADER_UNUSED_KHR,
    .anyHitShader = VK_SHADER_UNUSED_KHR,
    .intersectionShader = VK_SHADER_UNUSED_KHR,
  };
  shaderGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
  shaderGroup.generalShader = eRaygen;
  m_shaderGroups.push_back(shaderGroup);
  shaderGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
  shaderGroup.generalShader = eMiss;
  m_shaderGroups.push_back(shaderGroup);
  shaderGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
  shaderGroup.generalShader = VK_SHADER_UNUSED_KHR;
  shaderGroup.closestHitShader = eClosestHit;
  m_shaderGroups.push_back(shaderGroup);

  VkPushConstantRange pushConstantRange = {
    .stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR,
    .offset = 0,
    .size = sizeof(PushConstantRay),
  };
  std::vector<VkDescriptorSetLayout> descSetLayouts = {m_descSetLayout};
  VkPipelineLayoutCreateInfo  pipelineLayoutCreateInfo = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
    .setLayoutCount = static_cast<uint32_t>(descSetLayouts.size()),
    .pSetLayouts = descSetLayouts.data(),
    .pushConstantRangeCount = 1,
    .pPushConstantRanges = &pushConstantRange,
  };

  if (vkCreatePipelineLayout(m_device, &pipelineLayoutCreateInfo
        , nullptr, &m_pipelineLayout) != VK_SUCCESS)
    throw PhosHelper::FatalError("failed to create RayTracing pipeline layout!");
  VkRayTracingPipelineCreateInfoKHR rayPipelineInfo = {
    .sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR,
    .stageCount = static_cast<uint32_t>(stages.size()),
    .pStages = stages.data(),
    .groupCount = static_cast<uint32_t>(m_shaderGroups.size()),
    .pGroups = m_shaderGroups.data(),
    .maxPipelineRayRecursionDepth = 1,
    .layout = m_pipelineLayout,
  };
  if (pfnVkCreateRayTracingPipelinesKHR(m_device, {}, {}, 1, &rayPipelineInfo
        , nullptr, &m_pipeline) != VK_SUCCESS)
    throw PhosHelper::FatalError("failed to create RayTracing pipeline!");
  for(auto& s : stages)
    vkDestroyShaderModule(m_device, s.module, nullptr);
}

void  RtTest::createShaderBindingTable() {
  uint32_t  missCount{1};
  uint32_t  hitCount{1};
  auto      handleCount = 1 + missCount + hitCount;
  uint32_t  handleSize  = m_properties.shaderGroupHandleSize;

  auto alignUp = [](uint32_t addr, uint32_t alignement) {
    return ((addr + (alignement - 1)) & ~(alignement - 1));
  };
  uint32_t  handleSizeAligned = alignUp(handleSize, m_properties.shaderGroupHandleAlignment);

  m_rgenRegion.stride = alignUp(handleSizeAligned, m_properties.shaderGroupBaseAlignment);
  m_rgenRegion.size = m_rgenRegion.stride;
  m_missRegion.stride = handleSizeAligned;
  m_missRegion.size = alignUp(missCount * handleSizeAligned, m_properties.shaderGroupBaseAlignment);
  m_hitRegion.stride = handleSizeAligned;
  m_hitRegion.size = alignUp(hitCount * handleSizeAligned, m_properties.shaderGroupBaseAlignment);
  std::cout << "HANDLE_SIZE_ALIGNED = " << handleSizeAligned << ", " << handleSize << ", " << m_rgenRegion.stride << std::endl;

  uint32_t    dataSize = handleCount * handleSize;
  std::vector<uint8_t> handles(dataSize);
  if (pfnVkGetRayTracingShaderGroupHandlesKHR(m_device, m_pipeline, 0, handleCount, dataSize, handles.data()) != VK_SUCCESS)
    throw PhosHelper::FatalVulkanInitError("Unable get RT shader group handles !");

  VkDeviceSize sbtSize = m_rgenRegion.size + m_missRegion.size + m_hitRegion.size + m_callRegion.size;
  m_alloc.createBuffer(static_cast<size_t>(sbtSize),
      static_cast<VkBufferUsageFlagBits>(
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT
        | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
        | VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR),
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
        | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
      m_SBTBuffer.buffer, m_SBTBuffer.memory);

  VkBufferDeviceAddressInfo info{VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, nullptr, m_SBTBuffer.buffer};
  VkDeviceAddress           sbtAddress = vkGetBufferDeviceAddress(m_device, &info);
  m_rgenRegion.deviceAddress = sbtAddress;
  m_missRegion.deviceAddress = sbtAddress + m_rgenRegion.size;
  m_hitRegion.deviceAddress = sbtAddress + m_rgenRegion.size + m_missRegion.size;
  std::cout << "SBT Device Adress = " << m_hitRegion.deviceAddress << std::endl;

  auto getHandle = [&] (int i) { return handles.data() + i * handleSize; };

  void*  data;
  vkMapMemory(m_device, m_SBTBuffer.memory, 0, sbtSize, 0, &data);
  auto*     pSBTBuffer = reinterpret_cast<uint8_t*>(data);
  uint8_t*  pData{nullptr};
  uint32_t  handleIdx{0};
  pData = pSBTBuffer;
  memcpy(pData, getHandle(handleIdx++), handleSize);

  pData = pSBTBuffer + m_rgenRegion.size;
  for(uint32_t c = 0; c < missCount; c++)
  {
    memcpy(pData, getHandle(handleIdx++), handleSize);
    pData += m_missRegion.stride;
  }
  pData = pSBTBuffer + m_rgenRegion.size + m_missRegion.size;
  for(uint32_t c = 0; c < hitCount; c++)
  {
    memcpy(pData, getHandle(handleIdx++), handleSize);
    pData += m_hitRegion.stride;
  }
  vkUnmapMemory(m_device, m_SBTBuffer.memory);
  vkDestroyBuffer(m_device, m_SBTBuffer.buffer, nullptr);
}
