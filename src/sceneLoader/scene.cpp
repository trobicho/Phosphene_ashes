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

void  PhosScene::allocateResources() {
  size_t  sizeLights = static_cast<size_t>(sizeof(m_lights[0]) * m_lights.size()); 
  if (sizeLights > 0) {
    m_alloc->createBuffer(sizeLights
        , static_cast<VkBufferUsageFlagBits>(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT)
        , VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        , m_lightsBuffer);
  }
}

void  PhosScene::update(Pipeline pipeline, bool forceUpdate) {
  if (forceUpdate) {
    VkDescriptorBufferInfo bufferInfo = {
      .buffer = m_lightsBuffer.buffer,
      .offset = 0,
      .range = VK_WHOLE_SIZE,
    };
    RtBuilder::DescriptorSetUpdateInfo  updateInfo = {
      .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
      .binding = BindingsSceneOther::eLights,
      .pInfo = &bufferInfo,
    };
    pipeline.updateDescSet("sceneOther", updateInfo);
  }
}

void  PhosScene::update(Pipeline pipeline, const VkCommandBuffer &cmdBuffer, bool forceUpdate) {
  if (forceUpdate) {
    size_t  sizeLights = static_cast<size_t>(sizeof(m_lights[0]) * m_lights.size()); 
    pipeline.updateUBO(cmdBuffer, sizeLights, m_lightsBuffer, m_lights.data());
  }
}
