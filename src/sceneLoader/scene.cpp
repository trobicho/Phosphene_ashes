#include "scene.hpp"
#include <iostream>

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
  for (auto& mesh : m_meshs) {
    mesh.createBuffer(*m_alloc);
  }
  uint32_t meshCustomIndex = 0;
  uint32_t proceduralShapeCustomIndex = 0;
  for (auto& instance : m_instances) {
    void* obj = getInstanceObject(instance);
    if (instance.objectType == PHOS_OBJECT_TYPE_MESH) {
      PhosObjectMesh* objMesh = static_cast<PhosObjectMesh*>(obj);
      MeshDesc  meshDesc = {
        .textOffset = 0,
        .vertexAddress = m_alloc->getBufferDeviceAddress(objMesh->m_vertexBuffer),
        .indexAddress = m_alloc->getBufferDeviceAddress(objMesh->m_indexBuffer),
      };
      m_meshDescs.push_back(meshDesc);
      instance.customIndex = meshCustomIndex++;
    }
  }
  size_t  sizeLights = static_cast<size_t>(sizeof(m_lights[0]) * m_lights.size()); 
  std::cout << "strideLight = " << sizeof(m_lights[0]) << std::endl;
  if (sizeLights > 0) {
    m_alloc->createBuffer(sizeLights
        , static_cast<VkBufferUsageFlagBits>(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT)
        , VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        , m_lightsBuffer);
  }
  size_t  sizeMeshDescs = static_cast<size_t>(sizeof(m_meshDescs[0]) * m_meshDescs.size()); 
  if (sizeMeshDescs > 0) {
    m_alloc->createBuffer(sizeMeshDescs
        , static_cast<VkBufferUsageFlagBits>(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT)
        , VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        , m_meshDescsBuffer);
  }
}

void  PhosScene::update(Pipeline pipeline, bool forceUpdate) {
  if (forceUpdate) {
    VkDescriptorBufferInfo bufferInfo = {
      .buffer = m_meshDescsBuffer.buffer,
      .offset = 0,
      .range = VK_WHOLE_SIZE,
    };
    RtBuilder::DescriptorSetUpdateInfo  updateInfo = {
      .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
      .binding = BindingsScene::eMeshDescs,
      .pInfo = &bufferInfo,
    };
    pipeline.updateDescSet("scene", updateInfo);
  }
  if (forceUpdate) {
    VkDescriptorBufferInfo bufferInfo = {
      .buffer = m_lightsBuffer.buffer,
      .offset = 0,
      .range = VK_WHOLE_SIZE,
    };
    RtBuilder::DescriptorSetUpdateInfo  updateInfo = {
      .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
      .binding = BindingsSceneOther::eLights,
      .pInfo = &bufferInfo,
    };
    pipeline.updateDescSet("sceneOther", updateInfo);
  }
}

void  PhosScene::update(Pipeline pipeline, const VkCommandBuffer &cmdBuffer, bool forceUpdate) {
  if (forceUpdate) {
    size_t  sizeLights = static_cast<size_t>(sizeof(m_lights[0]) * m_lights.size()); 
    if (sizeLights > 0)
      pipeline.updateUBO(cmdBuffer, sizeLights, m_lightsBuffer, m_lights.data());
  }
  if (forceUpdate) {
    size_t  sizeMeshDescs = static_cast<size_t>(sizeof(m_meshDescs[0]) * m_meshDescs.size()); 
    if (sizeMeshDescs > 0)
      pipeline.updateUBO(cmdBuffer, sizeMeshDescs, m_meshDescsBuffer, m_meshDescs.data());
  }
}
