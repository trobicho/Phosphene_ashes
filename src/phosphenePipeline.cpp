#include "phosphene.hpp"
#include <iostream>
#include "../shaders/hostDevice.h"

void  Phosphene::updateRtImage() {
  VkDescriptorImageInfo imageInfo{{}, m_offscreenImageView, VK_IMAGE_LAYOUT_GENERAL};
  RtBuilder::DescriptorSetUpdateInfo  updateInfo = {
    .type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
    .binding = BindingsRtx::eImageOut,
    .pInfo = &imageInfo,
  };
  m_rtPipeline.updateDescSet("rtx", updateInfo);
}

void  Phosphene::updateRtTlas(AccelKHR& tlas) {
  VkWriteDescriptorSetAccelerationStructureKHR  asInfo = {
    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR,
    .accelerationStructureCount = 1,
    .pAccelerationStructures = &tlas.accel,
  };
  RtBuilder::DescriptorSetUpdateInfo  updateInfo = {
    .type = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,
    .binding = BindingsRtx::eTlas,
    .pInfo = &asInfo,
  };
  m_rtPipeline.updateDescSet("rtx", updateInfo);
}

void  Phosphene::updateRtGlobalUBO() {
  VkDescriptorBufferInfo bufferInfo = {
    .buffer = m_globalUBO.buffer,
    .offset = 0,
    .range = VK_WHOLE_SIZE,
  };
  RtBuilder::DescriptorSetUpdateInfo  updateInfo = {
    .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
    .binding = BindingsCommon::eGlobals,
    .pInfo = &bufferInfo,
  };
  m_rtPipeline.updateDescSet("common", updateInfo);
}

static std::vector<RtBuilder::DescriptorSetWrapper> commonBindings() {
  return (std::vector<RtBuilder::DescriptorSetWrapper> {
    (RtBuilder::DescriptorSetWrapper) {
      .name = "rtx",
      .layoutBinds = {
        (VkDescriptorSetLayoutBinding) {
          .binding = BindingsRtx::eTlas,
          .descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,
          .descriptorCount = 1,
          .stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR,
        },
        (VkDescriptorSetLayoutBinding) {
          .binding = BindingsRtx::eImageOut,
          .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
          .descriptorCount = 1,
          .stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR,
        }
      }
    },
    (RtBuilder::DescriptorSetWrapper) {
      .name = "common",
      .layoutBinds = {
        (VkDescriptorSetLayoutBinding) {
          .binding = BindingsCommon::eGlobals,
          .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
          .descriptorCount = 1,
          .stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR,
        },
      }
    },
    (RtBuilder::DescriptorSetWrapper) {
      .name = "scene",
      .layoutBinds = {
        (VkDescriptorSetLayoutBinding) {
          .binding = BindingsScene::eObjDescs,
          .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
          .descriptorCount = 1,
          .stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR,
        }
      }
    }
  });
}

void  Phosphene::buildRtPipelineBasic() {
  std::vector<RtBuilder::DescriptorSetWrapper>  descSets = commonBindings();
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
  RtBuilder::PipelineBuilder  builder;
  builder.init(m_device, m_physicalDevice, &m_alloc, m_graphicsQueueFamilyIndex);
  builder.addDescSet(descSets);
  builder.addPushConstant(pushConsant);
  builder.setRayGenStage("./spv/raytrace.rgen.spv");
  builder.addMissStage("./spv/raytrace.rmiss.spv");
  builder.addHitShader("cHit", "./spv/raytrace.rchit.spv", VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR);
  RtBuilder::HitGroup hitGroup = {
    .type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR,
    .closestHitName = "cHit",
  };
  builder.addHitGroup(hitGroup);
  builder.setMaxRecursion(1);
  m_rtPipeline.destroy();
  builder.build(m_rtPipeline);
  builder.destroyModules();

  updateRtGlobalUBO();
}

void  Phosphene::buildRtPipelineBasicLights() {
  std::vector<RtBuilder::DescriptorSetWrapper>  descSets = commonBindings();
  descSets.push_back((RtBuilder::DescriptorSetWrapper) {
    .name = "sceneOther",
    .layoutBinds = {
    (VkDescriptorSetLayoutBinding) {
      .binding = BindingsSceneOther::eLights,
      .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
      .descriptorCount = 1,
      .stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR,
      }
    }
  });
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
  RtBuilder::PipelineBuilder  builder;
  builder.init(m_device, m_physicalDevice, &m_alloc, m_graphicsQueueFamilyIndex);
  builder.addDescSet(descSets);
  builder.addPushConstant(pushConsant);
  builder.setRayGenStage("./spv/raytrace.rgen.spv");
  builder.addMissStage("./spv/raytrace.rmiss.spv");
  builder.addMissStage("./spv/raytraceShadow.rmiss.spv");
  builder.addHitShader("cHit", "./spv/raytraceShadow.rchit.spv", VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR);
  RtBuilder::HitGroup hitGroup = {
    .type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR,
    .closestHitName = "cHit",
  };
  builder.addHitGroup(hitGroup);
  builder.setMaxRecursion(2);
  m_rtPipeline.destroy();
  builder.build(m_rtPipeline);
  builder.destroyModules();

  updateRtGlobalUBO();
}
