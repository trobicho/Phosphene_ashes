#include "sceneBuilder.hpp"
#include <iostream>

namespace RtBuilder {

void  SceneBuilder::init(VkDevice device, MemoryAllocator* alloc, uint32_t queueFamilyIndex) {
  m_device = device;
  m_alloc = alloc;
  m_cmdPool.init(device, queueFamilyIndex);
}

void  SceneBuilder::destroy() {
  m_cmdPool.destroy();
  vkDestroyAccelerationStructureKHR(m_device, m_tlas.accel, nullptr);
  m_alloc->destroyBuffer(m_tlas.buffer);
  for (auto& blas : m_blas) {
    vkDestroyAccelerationStructureKHR(m_device, blas.accel, nullptr);
    m_alloc->destroyBuffer(blas.buffer);
  }
}

void  SceneBuilder::buildBlas(PhosScene& scene, VkBuildAccelerationStructureFlagsKHR flags) {
  VkDeviceSize  asTotalSize{0};
  VkDeviceSize  maxScratchSize{0};

  for (auto& mesh : scene.m_meshs) {
    modelToVkGeometry(mesh);
  }
  for (auto& shape : scene.m_proceduralShapes) {
    modelToVkGeometry(shape);
  }
  for (auto& vdb: scene.m_vdbs) {
    modelToVkGeometry(vdb);
  }

  uint32_t  nbBlas = static_cast<uint32_t>(m_blasInput.size());
  std::vector<BuildAccelerationStructure>  buildAs(nbBlas);

  for (uint32_t idx = 0; idx < nbBlas; idx++) {
    buildAs[idx].buildInfo = {
      .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
      .type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
      .flags = m_blasInput[idx].flags | flags,
      .geometryCount = 1,
      .pGeometries = &m_blasInput[idx].asGeometry,
    };
    buildAs[idx].rangeInfo = &m_blasInput[idx].asBuildOffsetInfo;
    uint32_t  maxPrimitiveCount = m_blasInput[idx].asBuildOffsetInfo.primitiveCount;

    vkGetAccelerationStructureBuildSizesKHR(m_device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR
        , &buildAs[idx].buildInfo, &maxPrimitiveCount, &buildAs[idx].sizeInfo);

    asTotalSize += buildAs[idx].sizeInfo.accelerationStructureSize;
    maxScratchSize = std::max(maxScratchSize, buildAs[idx].sizeInfo.buildScratchSize);
  }

  BufferWrapper scratchBuffer;
  m_alloc->createBuffer(maxScratchSize
      , static_cast<VkBufferUsageFlagBits>(VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)
      , VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
      , scratchBuffer);
  VkDeviceAddress scratchAddress = m_alloc->getBufferDeviceAddress(scratchBuffer);

  std::vector<uint32_t> indices;
  VkDeviceSize          batchSize;
  VkDeviceSize          batchLimit{256'000'000}; //256 MB

  for (uint32_t idx = 0; idx < nbBlas; idx++) {
    indices.push_back(idx);
    batchSize += buildAs[idx].sizeInfo.accelerationStructureSize;
    if (batchSize >= batchLimit || idx == nbBlas - 1) {
      VkCommandBuffer cmdBuffer = m_cmdPool.createCommandBuffer();
      VkCommandBufferBeginInfo  beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = 0,
        .pInheritanceInfo = nullptr,
      };
      if (vkBeginCommandBuffer(cmdBuffer, &beginInfo))
        throw PhosHelper::FatalVulkanInitError("Failed to begin recording Command Buffer For BLAS!");
      cmdCreateBlas(cmdBuffer, indices, buildAs, scratchAddress);
      vkEndCommandBuffer(cmdBuffer);
      m_cmdPool.submitAndWait(cmdBuffer);
      batchSize = 0;
      indices.clear();
    }
  }
  uint32_t  meshIdx = 0;
  uint32_t  shapeIdx = 0;
  uint32_t  vdbIdx = 0;
  for (uint32_t idx = 0; idx < nbBlas; idx++) {
		if (m_blasInput[idx].asGeometry.geometryType == VK_GEOMETRY_TYPE_TRIANGLES_KHR) {
      scene.m_meshs[meshIdx].blasDeviceAddress = m_alloc->getAccelerationStructureDeviceAddress(m_blas[idx]);
      meshIdx++;
    }
		else if (m_blasInput[idx].asGeometry.geometryType == VK_GEOMETRY_TYPE_AABBS_KHR && shapeIdx < scene.m_proceduralShapes.size()) {
      scene.m_proceduralShapes[shapeIdx].blasDeviceAddress = m_alloc->getAccelerationStructureDeviceAddress(m_blas[idx]);
      shapeIdx++;
    }
		else {
      scene.m_vdbs[vdbIdx].blasDeviceAddress = m_alloc->getAccelerationStructureDeviceAddress(m_blas[idx]);
      vdbIdx++;
		}
  }
  m_alloc->destroyBuffer(scratchBuffer);
}

void  SceneBuilder::buildTlas(PhosScene& scene, VkBuildAccelerationStructureFlagsKHR flags) {
  std::vector<VkAccelerationStructureInstanceKHR> tlasInstance;
  uint32_t  instanceCount = 0;
  for (auto& instance : scene.m_instances) {
    VkDeviceAddress               blasAddr = 0;
    void*                         objectAddr = scene.getInstanceObject(instance);
    VkGeometryInstanceFlagBitsKHR flags{};
    uint32_t                      hitShaderBinding = 0;

    if (instance.objectType == PHOS_OBJECT_TYPE_MESH) {
      auto meshAddr = static_cast<PhosObjectMesh*>(objectAddr);
      if (meshAddr == nullptr) {
        std::string error = "unable to get mesh address for: " + instance.objectName;
        throw PhosHelper::FatalError(error);
      }
      blasAddr = meshAddr->blasDeviceAddress;
      flags =  VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
    }
    else if (instance.objectType == PHOS_OBJECT_TYPE_PROCEDURAL) {
      auto shapeAddr = static_cast<PhosObjectProcedural*>(objectAddr);
      if (shapeAddr == nullptr) {
        std::string error = "unable to get shape address for: " + instance.objectName;
        throw PhosHelper::FatalError(error);
      }
      blasAddr = shapeAddr->blasDeviceAddress;
      flags =  VK_GEOMETRY_INSTANCE_FORCE_OPAQUE_BIT_KHR;
      hitShaderBinding = shapeAddr->hitShaderBindingIndex;
    }
    else if (instance.objectType == PHOS_OBJECT_TYPE_VDB) {
      auto vdbAddr = static_cast<PhosObjectVdb*>(objectAddr);
      if (vdbAddr == nullptr) {
        std::string error = "unable to get vdb address for: " + instance.objectName;
        throw PhosHelper::FatalError(error);
      }
      blasAddr = vdbAddr->blasDeviceAddress;
      flags =  VK_GEOMETRY_INSTANCE_FORCE_OPAQUE_BIT_KHR;
    }
    if (blasAddr != 0) {
      instanceCount++;
      VkAccelerationStructureInstanceKHR  inst = {
        .transform = PhosHelper::matrixToVkTransformMatrix(instance.transform),
        .instanceCustomIndex = instance.customIndex,
        .mask = 0xFF,
        .instanceShaderBindingTableRecordOffset = hitShaderBinding,
        .flags = flags,
        .accelerationStructureReference = blasAddr,
      };
      tlasInstance.push_back(inst);
    }
  }

  if (tlasInstance.empty())
    throw PhosHelper::BasicError("Empty TLAS !");

  BufferWrapper instanceBuffer;
  size_t        instanceBufferSize = sizeof(tlasInstance[0]) * tlasInstance.size();
  m_alloc->createBuffer(instanceBufferSize 
      , static_cast<VkBufferUsageFlagBits>(VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR
        | VK_BUFFER_USAGE_TRANSFER_DST_BIT
        | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)
      , VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
      , instanceBuffer);
  auto instanceBufferAddr = m_alloc->getBufferDeviceAddress(instanceBuffer);
  m_alloc->stagingMakeAndCopy(instanceBufferSize, instanceBuffer, tlasInstance.data());

  VkAccelerationStructureGeometryInstancesDataKHR asGeomData = {
    .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR,
    .data = {
      .deviceAddress = instanceBufferAddr,
    },
  };
  VkAccelerationStructureGeometryKHR  asGeometry = {
    .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
    .geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR,
    .geometry = {
      .instances = asGeomData,
    },
  };
  VkAccelerationStructureBuildGeometryInfoKHR buildGeomInfo = {
    .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
    .type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
    .flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR,
    .mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
    .srcAccelerationStructure = VK_NULL_HANDLE,
    .geometryCount = 1,
    .pGeometries = &asGeometry,
  };
  VkAccelerationStructureBuildSizesInfoKHR  buildSizeInfo = {
    .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR,
  };
  vkGetAccelerationStructureBuildSizesKHR(m_device
      , VK_ACCELERATION_STRUCTURE_BUILD_TYPE_HOST_OR_DEVICE_KHR
      , &buildGeomInfo, &instanceCount, &buildSizeInfo);

  m_alloc->createBuffer(buildSizeInfo.accelerationStructureSize
      , static_cast<VkBufferUsageFlagBits>(VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
        | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR) 
      , VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
      , m_tlas.buffer);
  BufferWrapper scratchBuffer;
  m_alloc->createBuffer(buildSizeInfo.buildScratchSize
      , static_cast<VkBufferUsageFlagBits>(VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
        | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)
      , VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
      , scratchBuffer);

  VkAccelerationStructureCreateInfoKHR  asInfo = {
    .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
    .createFlags = 0,
    .buffer = m_tlas.buffer.buffer,
    .offset = VkDeviceSize{0},
    .size = buildSizeInfo.accelerationStructureSize,
    .type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
    .deviceAddress = 0,
  };
  if (vkCreateAccelerationStructureKHR(m_device, &asInfo, nullptr, &m_tlas.accel) != VK_SUCCESS)
    throw PhosHelper::FatalVulkanInitError("Failed to Build TLAS!");
  VkAccelerationStructureBuildRangeInfoKHR offset = {
    .primitiveCount = instanceCount,
  };
  buildGeomInfo.dstAccelerationStructure = m_tlas.accel;
  buildGeomInfo.scratchData.deviceAddress = m_alloc->getBufferDeviceAddress(scratchBuffer);
  {
    auto cmdBuffer = m_cmdPool.createCommandBuffer();
    VkCommandBufferBeginInfo  beginInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .flags = 0,
      .pInheritanceInfo = nullptr,
    };
    if (vkBeginCommandBuffer(cmdBuffer, &beginInfo))
      throw PhosHelper::FatalVulkanInitError("Failed to begin recording Command Buffer For TLAS!");

    VkAccelerationStructureBuildRangeInfoKHR  *buildRange[] = {&offset};

    VkMemoryBarrier barrier{VK_STRUCTURE_TYPE_MEMORY_BARRIER};
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
    vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, 0, 1, &barrier, 0, nullptr, 0, nullptr);
    vkCmdBuildAccelerationStructuresKHR(cmdBuffer, 1, &buildGeomInfo, buildRange);
    if (vkEndCommandBuffer(cmdBuffer) != VK_SUCCESS)
      throw PhosHelper::FatalVulkanInitError("Failed to end recording Command Buffer For TLAS!");
    m_cmdPool.submitAndWait(cmdBuffer);
  }
  m_alloc->destroyBuffer(scratchBuffer);
  m_alloc->destroyBuffer(instanceBuffer);
}

void  SceneBuilder::modelToVkGeometry(PhosObjectProcedural& shape) {
  VkDeviceAddress aabbAddress = m_alloc->getBufferDeviceAddress(shape.aabbBuffer);
  
  VkAccelerationStructureGeometryAabbsDataKHR aabbs = {
    .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_AABBS_DATA_KHR,
    .data = {
      .deviceAddress = aabbAddress,
    },
    .stride = shape.strideAabb(),
  };

  VkAccelerationStructureGeometryKHR  asGeom = {
    .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
    .geometryType = VK_GEOMETRY_TYPE_AABBS_KHR,
    .geometry = {
      .aabbs = aabbs,
    },
    .flags = VK_GEOMETRY_OPAQUE_BIT_KHR,
  };

  VkAccelerationStructureBuildRangeInfoKHR buildOffsetInfo = {
    .primitiveCount = 1,
    .primitiveOffset = 0,
    .firstVertex = 0,
    .transformOffset = 0,
  };

  BlasInput input = {
    .asGeometry = asGeom,
    .asBuildOffsetInfo = buildOffsetInfo,
  };
  m_blasInput.push_back(input);
}

void  SceneBuilder::modelToVkGeometry(PhosObjectVdb& vdb) {
  VkDeviceAddress aabbAddress = m_alloc->getBufferDeviceAddress(vdb.aabbBuffer);
  
  VkAccelerationStructureGeometryAabbsDataKHR aabbs = {
    .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_AABBS_DATA_KHR,
    .data = {
      .deviceAddress = aabbAddress,
    },
    .stride = vdb.strideAabb(),
  };

  VkAccelerationStructureGeometryKHR  asGeom = {
    .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
    .geometryType = VK_GEOMETRY_TYPE_AABBS_KHR,
    .geometry = {
      .aabbs = aabbs,
    },
    .flags = VK_GEOMETRY_OPAQUE_BIT_KHR,
  };

  VkAccelerationStructureBuildRangeInfoKHR buildOffsetInfo = {
    .primitiveCount = 1,
    .primitiveOffset = 0,
    .firstVertex = 0,
    .transformOffset = 0,
  };

  BlasInput input = {
    .asGeometry = asGeom,
    .asBuildOffsetInfo = buildOffsetInfo,
  };
  m_blasInput.push_back(input);
}

void  SceneBuilder::modelToVkGeometry(PhosObjectMesh& model) {
  uint32_t  maxPrimitiveCount = model.indices.size() / 3;

  VkDeviceAddress vertexAddress = m_alloc->getBufferDeviceAddress(model.vertexBuffer);
  VkDeviceAddress indexAddress = m_alloc->getBufferDeviceAddress(model.indexBuffer);

  VkAccelerationStructureGeometryTrianglesDataKHR triangles = {
    .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR,
    .vertexFormat = VK_FORMAT_R32G32B32_SFLOAT,
    .vertexData = {
      .deviceAddress = vertexAddress,
    },
    .vertexStride = model.strideVertex(),
    .maxVertex = static_cast<uint32_t>(model.vertices.size()),
    .indexType = VK_INDEX_TYPE_UINT32,
    .indexData = {
      .deviceAddress = indexAddress,
    },
  };

  VkAccelerationStructureGeometryKHR  asGeom = {
    .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
    .geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR,
    .geometry = {
      .triangles = triangles,
    },
    .flags = VK_GEOMETRY_OPAQUE_BIT_KHR,
  };

  VkAccelerationStructureBuildRangeInfoKHR buildOffsetInfo = {
    .primitiveCount = maxPrimitiveCount,
    .primitiveOffset = 0,
    .firstVertex = 0,
    .transformOffset = 0,
  };

  BlasInput input = {
    .asGeometry = asGeom,
    .asBuildOffsetInfo = buildOffsetInfo,
  };
  m_blasInput.push_back(input);
}

void  SceneBuilder::cmdCreateBlas(VkCommandBuffer cmdBuffer
    , std::vector<uint32_t> indices
    , std::vector<BuildAccelerationStructure> buildAs
    , VkDeviceAddress scratchAddress) {
  for (const auto& idx : indices) {
    BufferWrapper blasBuffer;
    AccelKHR  blas;
    m_alloc->createBuffer(buildAs[idx].sizeInfo.accelerationStructureSize
        , static_cast<VkBufferUsageFlagBits>(VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR 
          | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)
        , VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        , blas.buffer);
    VkAccelerationStructureCreateInfoKHR  createInfo = {
      .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
      .buffer = blas.buffer.buffer,
      .size = buildAs[idx].sizeInfo.accelerationStructureSize,
      .type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
    };
    vkCreateAccelerationStructureKHR(m_device, &createInfo, nullptr, &blas.accel);
    buildAs[idx].buildInfo.dstAccelerationStructure = blas.accel;
    buildAs[idx].buildInfo.scratchData.deviceAddress = scratchAddress;
    vkCmdBuildAccelerationStructuresKHR(cmdBuffer, 1, &buildAs[idx].buildInfo, &buildAs[idx].rangeInfo);
    VkMemoryBarrier barrier = {
      .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER,
      .srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR,
      .dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR,
    };
    vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR
        , VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR
        , 0, 1, &barrier, 0, nullptr, 0, nullptr);
    m_blas.push_back(blas);
  }
}

}
