#include "rtBuilder.hpp"
#include <iostream>

namespace RtBuilder {

void  Pipeline::destroy() {
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

void  Pipeline::updateDescSet(const std::string name, const DescriptorSetUpdateInfo* info, uint32_t count) {
  DescriptorSetWrapper  *descSetPtr = nullptr;

  for (auto& descSet : m_descSets) {
    if (descSet.name == name)
      descSetPtr = &descSet;
  }
  if (descSetPtr == nullptr)
    return ;

  std::vector<VkWriteDescriptorSet>  writes;
  writes.reserve(count);
  for (uint32_t idx = 0; idx < count; idx++) {
    uint32_t  typeIdx = 0;
    for (auto& layout : descSetPtr->layoutBinds) {
      if (layout.descriptorType == info[idx].type) {
        if (typeIdx == info[idx].index) {
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
          break ;
        }
        typeIdx++;
      }
    }
  }
  vkUpdateDescriptorSets(m_device, writes.size(), writes.data(), 0, nullptr);
}

}
