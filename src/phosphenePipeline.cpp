#include "phosphene.hpp"
#include <iostream>
#include "../shaders/hostDevice.h"

bool  Phosphene::buildPipeline(std::string name) {
  deviceWait();
  if (name == "basicLights")
    buildRtPipelineBasicLights();
  else if (name == "pathTracing")
    buildRtPipelinePathTracing();
  else
    return (false);
  m_scene.update(m_rtPipeline, true);
  m_pcRay.nbConsecutiveRay = 0;
  return (true);
}

void  Phosphene::updateRtImage() {
  VkDescriptorImageInfo imageInfo{{}, m_offscreenImage.imageView, VK_IMAGE_LAYOUT_GENERAL};
  RtBuilder::DescriptorSetUpdateInfo  updateInfo = {
    .type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
    .binding = BindingsRtx::eImageOut,
    .pInfo = &imageInfo,
  };
  m_rtPipeline.updateDescSet("rtx", updateInfo);
  VkDescriptorImageInfo colorInfo{{}, m_gbuffer.color.imageView, VK_IMAGE_LAYOUT_GENERAL};
  VkDescriptorImageInfo normalInfo{{}, m_gbuffer.normal.imageView, VK_IMAGE_LAYOUT_GENERAL};
  VkDescriptorImageInfo depthInfo{{}, m_gbuffer.depth.imageView, VK_IMAGE_LAYOUT_GENERAL};
  VkDescriptorImageInfo materialInfo{{}, m_gbuffer.material.imageView, VK_IMAGE_LAYOUT_GENERAL};
  std::vector<RtBuilder::DescriptorSetUpdateInfo>  gbufferUpdateInfo = {
    {
      .type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
      .binding = BindingsRtx::eImageOut,
      .pInfo = &colorInfo,
    },
    {
      .type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
      .binding = BindingsRtx::eImageOut,
      .pInfo = &normalInfo,
    },
    {
      .type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
      .binding = BindingsRtx::eImageOut,
      .pInfo = &depthInfo,
    },
    {
      .type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
      .binding = BindingsRtx::eImageOut,
      .pInfo = &materialInfo,
    }
  };
  m_gbufferPipeline.updateDescSet("gbuffer", updateInfo);
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
  m_gbufferPipeline.updateDescSet("rtx", updateInfo);
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
  m_gbufferPipeline.updateDescSet("common", updateInfo);
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
          .binding = BindingsScene::eMeshDescs,
          .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
          .descriptorCount = 1,
          .stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR,
        },
        (VkDescriptorSetLayoutBinding) {
          .binding = BindingsScene::eShapeDescs,
          .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
          .descriptorCount = 1,
          .stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_INTERSECTION_BIT_KHR,
        },
        (VkDescriptorSetLayoutBinding) {
          .binding = BindingsScene::eMaterials,
          .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
          .descriptorCount = 1,
          .stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_RAYGEN_BIT_KHR,
        }
      }
    }
  });
}

void  Phosphene::buildRtPipelineBasicLights() {
  std::vector<RtBuilder::DescriptorSetWrapper>  descSets = commonBindings();
  descSets.push_back((RtBuilder::DescriptorSetWrapper) {
    .name = "sceneOther",
    .layoutBinds = {
    (VkDescriptorSetLayoutBinding) {
      .binding = BindingsSceneOther::eLights,
      .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
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
  builder.addHitShader("cHit", "./spv/raytraceMeshShadow.rchit.spv", VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR);
  builder.addHitShader("shapeCHit", "./spv/raytraceShapeShadow.rchit.spv", VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR);
  RtBuilder::HitGroup hitGroup = {
    .type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR,
    .closestHitName = "cHit",
  };
  builder.addHitGroup(hitGroup);
  for (auto& hitShader : m_scene.m_hitShaders) {
    builder.addHitShader(hitShader.name, "./spv/" + hitShader.spv, hitShader.type);
    RtBuilder::HitGroup hitGroup = {
      .type = VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_KHR,
      .closestHitName = "shapeCHit",
      .intersectionName = hitShader.name,
    };
    builder.addHitGroup(hitGroup);
  }
  builder.setMaxRecursion(2);
  m_rtPipeline.destroy();
  builder.build(m_rtPipeline);
  builder.destroyModules();

  updateRtGlobalUBO();
  updateRtImage();
  updateRtTlas();
}

void  Phosphene::buildRtPipelinePathTracing() {
  std::vector<RtBuilder::DescriptorSetWrapper>  descSets = commonBindings();
  RtBuilder::PushConstant pushConsant = {
    .range = (VkPushConstantRange) {
      .stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR
        | VK_SHADER_STAGE_RAYGEN_BIT_KHR
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
  builder.setRayGenStage("./spv/pathtrace.rgen.spv");
  builder.addMissStage("./spv/pathtrace.rmiss.spv");
  builder.addHitShader("cHit", "./spv/pathtraceMesh.rchit.spv", VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR);
  builder.addHitShader("shapeCHit", "./spv/pathtraceShape.rchit.spv", VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR);
  RtBuilder::HitGroup hitGroup = {
    .type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR,
    .closestHitName = "cHit",
  };
  builder.addHitGroup(hitGroup);
  for (auto& hitShader : m_scene.m_hitShaders) {
    builder.addHitShader(hitShader.name, "./spv/" + hitShader.spv, hitShader.type);
    RtBuilder::HitGroup hitGroup = {
      .type = VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_KHR,
      .closestHitName = "shapeCHit",
      .intersectionName = hitShader.name,
    };
    builder.addHitGroup(hitGroup);
  }
  builder.setMaxRecursion(1);
  m_rtPipeline.destroy();
  builder.build(m_rtPipeline);
  builder.destroyModules();

  updateRtGlobalUBO();
  updateRtImage();
  updateRtTlas();
}

void  Phosphene::buildGBufferPipeline() {
  std::vector<RtBuilder::DescriptorSetWrapper>  descSets = {
    (RtBuilder::DescriptorSetWrapper) {
      .name = "rtx",
      .layoutBinds = {
        (VkDescriptorSetLayoutBinding) {
          .binding = BindingsRtx::eTlas,
          .descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,
          .descriptorCount = 1,
          .stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR,
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
      .name = "gbuffer",
      .layoutBinds = {
        (VkDescriptorSetLayoutBinding) {
          .binding = BindingsGBuffer::eGBufferColor,
          .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
          .descriptorCount = 1,
          .stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR,
        },
        (VkDescriptorSetLayoutBinding) {
          .binding = BindingsGBuffer::eGBufferNormal,
          .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
          .descriptorCount = 1,
          .stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR,
        },
        (VkDescriptorSetLayoutBinding) {
          .binding = BindingsGBuffer::eGBufferDepth,
          .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
          .descriptorCount = 1,
          .stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR,
        },
        (VkDescriptorSetLayoutBinding) {
          .binding = BindingsGBuffer::eGBufferMaterial,
          .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
          .descriptorCount = 1,
          .stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR,
        },
      }
    },
    (RtBuilder::DescriptorSetWrapper) {
      .name = "scene",
      .layoutBinds = {
        (VkDescriptorSetLayoutBinding) {
          .binding = BindingsScene::eMeshDescs,
          .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
          .descriptorCount = 1,
          .stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR,
        },
        (VkDescriptorSetLayoutBinding) {
          .binding = BindingsScene::eShapeDescs,
          .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
          .descriptorCount = 1,
          .stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_INTERSECTION_BIT_KHR,
        },
        (VkDescriptorSetLayoutBinding) {
          .binding = BindingsScene::eMaterials,
          .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
          .descriptorCount = 1,
          .stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_RAYGEN_BIT_KHR,
        }
      }
    }
  };
  RtBuilder::PushConstant pushConsant = {
    .range = (VkPushConstantRange) {
      .stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR
        | VK_SHADER_STAGE_RAYGEN_BIT_KHR
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
  builder.setRayGenStage("./spv/gBuffer.rgen.spv");
  builder.addMissStage("./spv/gBuffer.rmiss.spv");
  builder.addHitShader("cHit", "./spv/gBufferMesh.rchit.spv", VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR);
  builder.addHitShader("shapeCHit", "./spv/gBufferShape.rchit.spv", VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR);
  RtBuilder::HitGroup hitGroup = {
    .type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR,
    .closestHitName = "cHit",
  };
  builder.addHitGroup(hitGroup);
  for (auto& hitShader : m_scene.m_hitShaders) {
    builder.addHitShader(hitShader.name, "./spv/" + hitShader.spv, hitShader.type);
    RtBuilder::HitGroup hitGroup = {
      .type = VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_KHR,
      .closestHitName = "shapeCHit",
      .intersectionName = hitShader.name,
    };
    builder.addHitGroup(hitGroup);
  }
  builder.setMaxRecursion(1);
  m_gbufferPipeline.destroy();
  builder.build(m_gbufferPipeline);
  builder.destroyModules();

  updateRtGlobalUBO();
  updateRtImage();
  updateRtTlas();
}
