#include "rtBuilder.hpp"
#include <iostream>

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

void  PipelineBuilder::setMaxRecursion(uint32_t max) {
  if (m_rtProperties.maxRayRecursionDepth < max) {
    std::string error = "MaxRayRecursion is inferior to: " + std::to_string(max) + " (" + std::to_string(m_rtProperties.maxRayRecursionDepth) + ")";
    throw PhosHelper::BasicError(error);
  }
  else
    m_maxRecursion = max;
}

Pipeline  PipelineBuilder::build() {
  VkPipelineLayout  pipelineLayout = VK_NULL_HANDLE;
  VkPipeline        pipeline = VK_NULL_HANDLE;

  std::vector<VkDescriptorSetLayout>  descSetLayouts;
  for (auto& descSet : m_descSets) {
    descSetLayouts.push_back(descSet.layout);
  }
  VkPipelineLayoutCreateInfo  pipelineLayoutCreateInfo = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
    .setLayoutCount = static_cast<uint32_t>(descSetLayouts.size()),
    .pSetLayouts = descSetLayouts.data(),
    .pushConstantRangeCount = static_cast<uint32_t>(m_pushConstants.size()),
  };
  if (vkCreatePipelineLayout(m_device, &pipelineLayoutCreateInfo
        , nullptr, &pipelineLayout) != VK_SUCCESS)
    throw PhosHelper::FatalError("failed to create RayTracing pipeline layout!");
  
  VkRayTracingPipelineCreateInfoKHR pipelineCreateInfo = {
    .sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR,
    .stageCount = static_cast<uint32_t>(m_stages.size()),
    .pStages = m_stages.data(),
    .groupCount = static_cast<uint32_t>(m_shaderGroups.size()),
    .pGroups = m_shaderGroups.data(),
    .maxPipelineRayRecursionDepth = m_maxRecursion,
    .layout = pipelineLayout,
  };
  if (vkCreateRayTracingPipelinesKHR(m_device, {}, {}, 1, &pipelineCreateInfo
        , nullptr, &pipeline) != VK_SUCCESS)
    throw PhosHelper::FatalError("failed to create RayTracing pipeline!");
  Pipeline::PipelineInitInfo  info = {
    .device = m_device,
    .pipeline = pipeline,
    .pipelineLayout = pipelineLayout,
    .descPool = m_descPool,
    .descSets = m_descSets,
    .pushConstants = m_pushConstants,
    .shaderBindingTable = m_shaderBindingTable,
  };
  return (Pipeline(info));
}

void  PipelineBuilder::buildDescriptorSets() {
  std::vector<VkDescriptorPoolSize> descPoolSize;

  for (auto& descSet : m_descSets) {
    for (auto& binding : descSet.layoutBinds) {
      descPoolSize.push_back((VkDescriptorPoolSize){
        .type = binding.descriptorType,
        .descriptorCount = 1,
      });
    }
  }
  VkDescriptorPoolCreateInfo  descPoolInfo = {
    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
    .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT | VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
    .maxSets = static_cast<uint32_t>(m_descSets.size()),
    .poolSizeCount = static_cast<uint32_t>(descPoolSize.size()),
    .pPoolSizes = descPoolSize.data(),
  };
  if(vkCreateDescriptorPool(m_device, &descPoolInfo, nullptr, &m_descPool) != VK_SUCCESS)
    throw PhosHelper::FatalVulkanInitError("Failed to create RT descriptor pool !");

  for (auto& descSet : m_descSets) {
    VkDescriptorSetLayoutCreateInfo descSetLayoutInfo = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
      .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
      .bindingCount = static_cast<uint32_t>(descSet.layoutBinds.size()),
      .pBindings = descSet.layoutBinds.data(),
    };
    if (vkCreateDescriptorSetLayout(m_device, &descSetLayoutInfo, nullptr, &descSet.layout) != VK_SUCCESS)
      throw PhosHelper::FatalVulkanInitError("Failed to create RT descriptor set layout !");
    VkDescriptorSetAllocateInfo descSetAllocInfo = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
      .descriptorPool = m_descPool,
      .descriptorSetCount = 1,
      .pSetLayouts = &descSet.layout,
    };
    if (vkAllocateDescriptorSets(m_device, &descSetAllocInfo, &descSet.set) != VK_SUCCESS)
      throw PhosHelper::FatalVulkanInitError("Failed to Allocate RT descriptor set !");
  }
}

}
