#include "rtBuilder.hpp"


namespace RtBuilder {

/*
void  PipelineBuilder::build() {

  VkRayTracingPipelineCreateInfoKHR rayPipelineInfo = {
    .sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR,
  }
}
*/

void  SceneBuilder::init(VkDevice device, MemoryAllocator *alloc, uint32_t queueFamilyIndex) {
  m_device = device;
  m_alloc = alloc;
  m_cmdPool.init(device, queueFamilyIndex);
}

void  SceneBuilder::buildBlas(PhosScene &scene, VkBuildAccelerationStructureFlagsKHR flags) {
  VkDeviceSize  asTotalSize{0};
  VkDeviceSize  maxScratchSize{0};

  for (auto& mesh : scene.m_meshs) {
    modelToVkGeometry(mesh);
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

  for (uint32_t idx; idx < nbBlas; idx++) {
    indices.push_back(idx);
    batchSize += buildAs[idx].sizeInfo.accelerationStructureSize;
    if (batchSize >= batchLimit || idx == nbBlas - 1) {
      VkCommandBuffer cmdBuffer = m_cmdPool.createCommandBuffer();
      cmdCreateBlas(cmdBuffer, indices, buildAs, scratchAddress);
      m_cmdPool.submitAndWait(cmdBuffer);
    }
    batchSize = 0;
    indices.clear();
  }
  m_alloc->destroyBuffer(scratchBuffer);
}

void  SceneBuilder::modelToVkGeometry(PhosObjectMesh& model) {
  uint32_t  maxPrimitiveCount = model.m_indices.size() / 3;

  VkDeviceAddress vertexAddress = m_alloc->getBufferDeviceAddress(model.m_vertexBuffer);
  VkDeviceAddress indexAddress = m_alloc->getBufferDeviceAddress(model.m_indexBuffer);

  VkAccelerationStructureGeometryTrianglesDataKHR triangles = {
    .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR,
    .vertexFormat = VK_FORMAT_R32G32B32_SFLOAT,
    .vertexData = {
      .deviceAddress = vertexAddress,
    },
    .vertexStride = model.strideVertex(),
    .maxVertex = static_cast<uint32_t>(model.m_vertices.size()),
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
    VkAccelerationStructureCreateInfoKHR  createInfo = {
      .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
      .size = buildAs[idx].sizeInfo.accelerationStructureSize,
      .type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
    };
    AccelKHR  blas;
    m_alloc->createBuffer(createInfo.size
                          , static_cast<VkBufferUsageFlagBits>(VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR 
                                                              | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)
                          , VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
                          , blas.buffer);
    vkCreateAccelerationStructureKHR(m_device, &createInfo, nullptr, &blas.accel);
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
