#include "pipelineBuilder.hpp"
#include <iostream>

namespace RtBuilder {

void  Pipeline::destroy() {
  if (m_device != VK_NULL_HANDLE) {
    for (auto& descSet : m_descSets) {
      descSet.destroy(m_device);
    }
    m_descSets.clear();
    vkDestroyDescriptorPool(m_device, m_descPool, nullptr);
    m_descPool = VK_NULL_HANDLE;
    vkDestroyPipeline(m_device, m_pipeline, nullptr);
    m_pipeline = VK_NULL_HANDLE;
    vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);
    m_pipelineLayout = VK_NULL_HANDLE;
    m_shaderBindingTable.buffer.destroy(m_device);
    m_shaderBindingTable = ShaderBindingTable();
    m_shaderGroups.clear();
  }
}

void  Pipeline::raytrace(VkCommandBuffer commandBuffer, uint32_t width, uint32_t height) {
  std::vector<VkDescriptorSet> descSets;
  descSets.reserve(m_descSets.size());
  for (auto& descSet : m_descSets) {
    descSets.push_back(descSet.set);
  }
  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, m_pipeline);
  vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR
      , m_pipelineLayout, 0, descSets.size(), descSets.data(), 0, nullptr);
  for (auto& pushConstant : m_pushConstants) {
    vkCmdPushConstants(commandBuffer, m_pipelineLayout
        , pushConstant.range.stageFlags
        , pushConstant.range.offset
        , pushConstant.range.size
        , pushConstant.pValues);
  }
  vkCmdTraceRaysKHR(commandBuffer
      , &m_shaderBindingTable.rgenRegion
      , &m_shaderBindingTable.missRegion
      , &m_shaderBindingTable.hitRegion
      , &m_shaderBindingTable.callRegion
      , width, height, 1);
}

void  Pipeline::build(std::vector<VkPipelineShaderStageCreateInfo> stages) {
  buildDescriptorSets();
  std::vector<VkDescriptorSetLayout>  descSetLayouts;
  for (auto& descSet : m_descSets)
    descSetLayouts.push_back(descSet.layout);
  std::vector<VkPushConstantRange>    pushConstantRanges;
  for (auto& pushConstant: m_pushConstants)
    pushConstantRanges.push_back(pushConstant.range);

  VkPipelineLayoutCreateInfo  pipelineLayoutCreateInfo = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
    .setLayoutCount = static_cast<uint32_t>(descSetLayouts.size()),
    .pSetLayouts = descSetLayouts.data(),
    .pushConstantRangeCount = static_cast<uint32_t>(pushConstantRanges.size()),
    .pPushConstantRanges = pushConstantRanges.data(),
  };
  if (vkCreatePipelineLayout(m_device, &pipelineLayoutCreateInfo
        , nullptr, &m_pipelineLayout) != VK_SUCCESS)
    throw PhosHelper::FatalError("failed to create RayTracing pipeline layout!");

  VkRayTracingPipelineCreateInfoKHR pipelineCreateInfo = {
    .sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR,
    .stageCount = static_cast<uint32_t>(stages.size()),
    .pStages = stages.data(),
    .groupCount = static_cast<uint32_t>(m_shaderGroups.size()),
    .pGroups = m_shaderGroups.data(),
    .maxPipelineRayRecursionDepth = m_maxRayRecursion,
    .layout = m_pipelineLayout,
  };
  if (vkCreateRayTracingPipelinesKHR(m_device, {}, {}, 1, &pipelineCreateInfo
        , nullptr, &m_pipeline) != VK_SUCCESS)
    throw PhosHelper::FatalError("failed to create RayTracing pipeline!");

}

void  Pipeline::buildDescriptorSets() {
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

void  Pipeline::updateDescSet(const std::string name, const DescriptorSetUpdateInfo* info, uint32_t count) {
  DescriptorSetWrapper  *descSetPtr = nullptr;

  for (auto& descSet : m_descSets) {
    if (descSet.name == name) {
      descSetPtr = &descSet;
      break;
    }
  }
  if (descSetPtr == nullptr)
    return ;

  std::vector<VkWriteDescriptorSet>  writes;
  writes.reserve(count);
  for (uint32_t idx = 0; idx < count; idx++) {
    for (auto& layout : descSetPtr->layoutBinds) {
      if (layout.binding == info[idx].binding) {
        if (layout.descriptorType == info[idx].type) {
          VkWriteDescriptorSet  write = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = descSetPtr->set,
            .dstBinding = layout.binding,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = info[idx].type,
          };
          if (info[idx].type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
              || info[idx].type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
            write.pBufferInfo = static_cast<VkDescriptorBufferInfo*>(info[idx].pInfo);
          else if (info[idx].type == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)
            write.pImageInfo = static_cast<VkDescriptorImageInfo*>(info[idx].pInfo);
          else
            write.pNext = info[idx].pInfo;
          writes.push_back(write);
        }
        break ;
      }
    }
  }
  vkUpdateDescriptorSets(m_device, writes.size(), writes.data(), 0, nullptr);
}

void  Pipeline::updateUBO(const VkCommandBuffer &cmdBuffer, const uint32_t size, BufferWrapper &deviceUBO, void* hostUBO) {
  auto     uboUsageStages = VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR;

  VkBufferMemoryBarrier beforeBarrier{VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER};
  beforeBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
  beforeBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
  beforeBarrier.buffer        = deviceUBO.buffer;
  beforeBarrier.offset        = 0;
  beforeBarrier.size          = size;
  vkCmdPipelineBarrier(cmdBuffer, uboUsageStages, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_DEPENDENCY_DEVICE_GROUP_BIT, 0,
                       nullptr, 1, &beforeBarrier, 0, nullptr);


  // Schedule the host-to-device upload. (hostUBO is copied into the cmd
  // buffer so it is okay to deallocate when the function returns).
  vkCmdUpdateBuffer(cmdBuffer, deviceUBO.buffer, 0, size, hostUBO);

  // Making sure the updated UBO will be visible.
  VkBufferMemoryBarrier afterBarrier{VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER};
  afterBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
  afterBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
  afterBarrier.buffer        = deviceUBO.buffer;
  afterBarrier.offset        = 0;
  afterBarrier.size          = size;
  vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, uboUsageStages, VK_DEPENDENCY_DEVICE_GROUP_BIT, 0,
                       nullptr, 1, &afterBarrier, 0, nullptr);
}

}
