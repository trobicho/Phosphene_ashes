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
