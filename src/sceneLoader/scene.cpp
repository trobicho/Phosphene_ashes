#include "scene.hpp"

void  PhosObjectMesh::createBuffer(MemoryAllocator &alloc) {
  size_t  sizeVertex = static_cast<size_t>(strideVertex() * m_vertices.size()); 
  alloc.createBuffer(sizeVertex
      , static_cast<VkBufferUsageFlagBits>(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT
          | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
          | VK_BUFFER_USAGE_TRANSFER_DST_BIT
          | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR
          | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)
      , VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
      , m_vertexBuffer);
  alloc.stagingMakeAndCopy(sizeVertex, m_vertexBuffer, (void*)m_vertices.data());

  size_t  sizeIndex = static_cast<size_t>(sizeof(m_indices[0]) * m_indices.size());
  alloc.createBuffer(sizeIndex
      , static_cast<VkBufferUsageFlagBits>(VK_BUFFER_USAGE_INDEX_BUFFER_BIT
          | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
          | VK_BUFFER_USAGE_TRANSFER_DST_BIT
          | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR
          | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)
      , VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
      , m_indexBuffer);
  alloc.stagingMakeAndCopy(sizeIndex, m_indexBuffer, (void*)m_indices.data());
}

void* PhosScene::getInstanceObject(PhosObjectInstance &instance) {
  if (instance.objectType == PHOS_OBJECT_TYPE_MESH) {
    for (auto& mesh : m_meshs) {
      if (instance.objectName == mesh.m_name)
        return (&mesh);
    }
  }
  return (nullptr);
}

void  PhosScene::createLightsBuffer() {
  size_t  sizeLights = static_cast<size_t>(sizeof(m_lights[0]) * m_lights.size()); 
  if (sizeLights > 0) {
    m_alloc->createBuffer(sizeLights
        , static_cast<VkBufferUsageFlagBits>(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT)
        , VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        , m_lightsBuffer);
  }
}

void  PhosScene::updateLightsBuffer(const VkCommandBuffer &cmdBuffer) {
  auto     uboUsageStages = VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR;

  size_t  sizeLights = static_cast<size_t>(sizeof(m_lights[0]) * m_lights.size()); 
  VkBufferMemoryBarrier beforeBarrier = {
    .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
    .srcAccessMask = VK_ACCESS_SHADER_READ_BIT,
    .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
    .buffer        = m_lightsBuffer.buffer,
    .offset        = 0,
    .size          = sizeLights,
  };
  vkCmdPipelineBarrier(cmdBuffer, uboUsageStages
      , VK_PIPELINE_STAGE_TRANSFER_BIT, VK_DEPENDENCY_DEVICE_GROUP_BIT, 0
      , nullptr, 1, &beforeBarrier, 0, nullptr);


  vkCmdUpdateBuffer(cmdBuffer, m_lightsBuffer.buffer, 0, sizeof(GlobalUniforms), &m_lights);

  VkBufferMemoryBarrier afterBarrier = {
    .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
    .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
    .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
    .buffer        = m_lightsBuffer.buffer,
    .offset        = 0,
    .size          = sizeLights,
  };
  vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT
      , uboUsageStages, VK_DEPENDENCY_DEVICE_GROUP_BIT, 0
      , nullptr, 1, &afterBarrier, 0, nullptr);
}
