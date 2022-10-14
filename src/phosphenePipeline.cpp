#include "phosphene.hpp"
#include <iostream>

void  Phosphene::updateRtImage() {
  VkDescriptorImageInfo imageInfo{{}, m_offscreenImageView, VK_IMAGE_LAYOUT_GENERAL};
  RtBuilder::DescriptorSetUpdateInfo  updateInfo = {
    .type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
    .index = 0,
    .pInfo = &imageInfo,
  };
  m_rtPipeline.updateDescSet("common", updateInfo);
}

void  Phosphene::updateRtTlas(AccelKHR& tlas) {
  VkWriteDescriptorSetAccelerationStructureKHR  asInfo = {
    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR,
    .accelerationStructureCount = 1,
    .pAccelerationStructures = &tlas.accel,
  };
  RtBuilder::DescriptorSetUpdateInfo  updateInfo = {
    .type = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,
    .index = 0,
    .pInfo = &asInfo,
  };
  m_rtPipeline.updateDescSet("common", updateInfo);
}

void  Phosphene::updateRtGlobalUBO() {
  VkDescriptorBufferInfo bufferInfo = {
    .buffer = m_globalUBO.buffer,
    .offset = 0,
    .range = VK_WHOLE_SIZE,
  };
  RtBuilder::DescriptorSetUpdateInfo  updateInfo = {
    .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
    .index = 0,
    .pInfo = &bufferInfo,
  };
  m_rtPipeline.updateDescSet("globalUBO", updateInfo);
}

void  Phosphene::updateRtGlobalUBO(const VkCommandBuffer &cmdBuffer) {
  auto     uboUsageStages = VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR;

  VkBufferMemoryBarrier beforeBarrier{VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER};
  beforeBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
  beforeBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
  beforeBarrier.buffer        = m_globalUBO.buffer;
  beforeBarrier.offset        = 0;
  beforeBarrier.size          = sizeof(m_globalUniform);
  vkCmdPipelineBarrier(cmdBuffer, uboUsageStages, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_DEPENDENCY_DEVICE_GROUP_BIT, 0,
                       nullptr, 1, &beforeBarrier, 0, nullptr);


  // Schedule the host-to-device upload. (hostUBO is copied into the cmd
  // buffer so it is okay to deallocate when the function returns).
  vkCmdUpdateBuffer(cmdBuffer, m_globalUBO.buffer, 0, sizeof(GlobalUniforms), &m_globalUniform);

  // Making sure the updated UBO will be visible.
  VkBufferMemoryBarrier afterBarrier{VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER};
  afterBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
  afterBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
  afterBarrier.buffer        = m_globalUBO.buffer;
  afterBarrier.offset        = 0;
  afterBarrier.size          = sizeof(m_globalUniform);
  vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, uboUsageStages, VK_DEPENDENCY_DEVICE_GROUP_BIT, 0,
                       nullptr, 1, &afterBarrier, 0, nullptr);
}

void  Phosphene::buildRtPipelineBasic() {
  RtBuilder::PipelineBuilder  builder;
  builder.init(m_device, m_physicalDevice, &m_alloc, m_graphicsQueueFamilyIndex);

  std::vector<RtBuilder::DescriptorSetWrapper>  descSets = {
    (RtBuilder::DescriptorSetWrapper) {
      .name = "common",
      .layoutBinds = {
        (VkDescriptorSetLayoutBinding) {
          .binding = 0,
          .descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,
          .descriptorCount = 1,
          .stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR,
        },
        (VkDescriptorSetLayoutBinding) {
          .binding = 1,
          .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
          .descriptorCount = 1,
          .stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR,
        }
      }
    },
    (RtBuilder::DescriptorSetWrapper) {
      .name = "globalUBO",
      .layoutBinds = {
        (VkDescriptorSetLayoutBinding) {
          .binding = 0,
          .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
          .descriptorCount = 1,
          .stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR,
        }
      }
    }
  };
  RtBuilder::PushConstant pushConsant = {
    .range = (VkPushConstantRange) {
      .stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR
        | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR
        | VK_SHADER_STAGE_MISS_BIT_KHR,
      .offset = 0,
      .size = sizeof(m_pcRay),
    },
    .pValues = &m_pcRay,
  };
  builder.addPushConstant(pushConsant);
  builder.addDescSet(descSets);
  builder.setMaxRecursion(1);
  m_rtPipeline.destroy();
  updateRtGlobalUBO();
}
