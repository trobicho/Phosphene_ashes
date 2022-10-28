#include "scene.hpp"
#include <iostream>

void  PhosObjectMesh::createBuffer(MemoryAllocator &alloc) {
  size_t  sizeVertex = static_cast<size_t>(strideVertex() * vertices.size()); 
  alloc.createBuffer(sizeVertex
      , static_cast<VkBufferUsageFlagBits>(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT
          | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
          | VK_BUFFER_USAGE_TRANSFER_DST_BIT
          | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR
          | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)
      , VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
      , vertexBuffer);
  alloc.stagingMakeAndCopy(sizeVertex, vertexBuffer, (void*)vertices.data());

  size_t  sizeIndex = static_cast<size_t>(sizeof(indices[0]) * indices.size());
  alloc.createBuffer(sizeIndex
      , static_cast<VkBufferUsageFlagBits>(VK_BUFFER_USAGE_INDEX_BUFFER_BIT
          | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
          | VK_BUFFER_USAGE_TRANSFER_DST_BIT
          | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR
          | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)
      , VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
      , indexBuffer);
  alloc.stagingMakeAndCopy(sizeIndex, indexBuffer, (void*)indices.data());
}

void  PhosObjectProcedural::createBuffer(MemoryAllocator &alloc) {
  alloc.createBuffer(strideAabb()
      , static_cast<VkBufferUsageFlagBits>(VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
          | VK_BUFFER_USAGE_TRANSFER_DST_BIT
          | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR
          | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)
      , VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
      , aabbBuffer);
  alloc.stagingMakeAndCopy(strideAabb(), aabbBuffer, (void*)&aabb);
}
    
void  PhosScene::destroy() {
  for (auto& mesh : m_meshs) {
    mesh.destroy(*m_alloc);
  }
  for (auto& shape: m_proceduraShapes) {
    shape.destroy(*m_alloc);
  }
  m_alloc->destroyBuffer(m_lightsBuffer);
  m_alloc->destroyBuffer(m_meshDescsBuffer);
  m_alloc->destroyBuffer(m_shapeDescsBuffer);
  m_alloc->destroyBuffer(m_materialsBuffer);
}

void* PhosScene::getInstanceObject(PhosObjectInstance &instance) {
  if (instance.objectType == PHOS_OBJECT_TYPE_MESH)
    return (PhosNamedObject::getObjectFromName(m_meshs, instance.objectName));
  else if (instance.objectType == PHOS_OBJECT_TYPE_PROCEDURAL)
    return (PhosNamedObject::getObjectFromName(m_proceduraShapes, instance.objectName));
  throw PhosHelper::FatalError(std::string("Invalid Instance(" + instance.name
        + ") objectType{" + std::to_string(instance.objectType)
        + "} ObjectName(" + instance.objectName + ")"));
}

void  PhosScene::setShapesHitBindingIndex(uint32_t offset) {
  for (auto& shape : m_proceduraShapes) {
    uint32_t        index = 0;
    PhosHitShader*  shader;
    shader = PhosNamedObject::getObjectFromName(m_hitShaders, shape.intersectionShaderName, &index);
    if (shader == nullptr || shader->type != VK_SHADER_STAGE_INTERSECTION_BIT_KHR) {
      throw PhosHelper::FatalError(
          std::string("Invalid intersection shader: " + shape.intersectionShaderName));
    }
    shape.hitShaderBindingIndex = offset + index;
  }
}

void  PhosScene::allocateResources() {
  for (auto& mesh : m_meshs) {
    mesh.createBuffer(*m_alloc);
  }
  for (auto& shape : m_proceduraShapes) {
    shape.createBuffer(*m_alloc);
  }
  uint32_t meshCustomIndex = 0;
  uint32_t shapeCustomIndex = 0;
  uint32_t proceduralShapeCustomIndex = 0;
  for (auto& instance : m_instances) {
    void* obj = getInstanceObject(instance);
    if (obj == nullptr) {
      throw PhosHelper::FatalError(
          std::string("Invalid object: " + instance.objectName));
    }
    if (instance.objectType == PHOS_OBJECT_TYPE_MESH) {
      PhosObjectMesh* mesh = static_cast<PhosObjectMesh*>(obj);
      auto meshDesc = buildMeshDesc(instance, mesh);
      m_meshDescs.push_back(meshDesc);
      instance.customIndex = meshCustomIndex++;
    }
    else if (instance.objectType == PHOS_OBJECT_TYPE_PROCEDURAL) {
      PhosObjectProcedural* shape = static_cast<PhosObjectProcedural*>(obj);
      auto shapeDesc = buildShapeDesc(instance, shape);
      m_shapeDescs.push_back(shapeDesc);
      instance.customIndex = shapeCustomIndex++;
    }
  }
  size_t  sizeLights = static_cast<size_t>(sizeof(m_lights[0]) * m_lights.size()); 
  std::cout << "strideLight = " << sizeof(m_lights[0]) << std::endl;
  if (sizeLights > 0) {
    m_alloc->createBuffer(sizeLights
        , static_cast<VkBufferUsageFlagBits>(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT)
        , VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        , m_lightsBuffer);
    m_alloc->stagingMakeAndCopy(sizeLights, m_lightsBuffer, m_lights.data());
  }
  size_t  sizeMaterials = static_cast<size_t>(sizeof(Material) * m_materials.size()); 
  if (sizeMaterials > 0) {
    m_alloc->createBuffer(sizeMaterials
        , static_cast<VkBufferUsageFlagBits>(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT)
        , VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        , m_materialsBuffer);
    std::vector<Material> matHostBuffer;
    matHostBuffer.reserve(m_materials.size());
    for (auto& mat : m_materials)
      matHostBuffer.push_back(mat);
    m_alloc->stagingMakeAndCopy(sizeMaterials, m_materialsBuffer, matHostBuffer.data());
  }
  size_t  sizeMeshDescs = static_cast<size_t>(sizeof(m_meshDescs[0]) * m_meshDescs.size()); 
  if (sizeMeshDescs > 0) {
    m_alloc->createBuffer(sizeMeshDescs
        , static_cast<VkBufferUsageFlagBits>(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT)
        , VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        , m_meshDescsBuffer);
    m_alloc->stagingMakeAndCopy(sizeMeshDescs, m_meshDescsBuffer, m_meshDescs.data());
  }
  size_t  sizeShapeDescs = static_cast<size_t>(sizeof(m_shapeDescs[0]) * m_shapeDescs.size()); 
  if (sizeShapeDescs > 0) {
    m_alloc->createBuffer(sizeShapeDescs
        , static_cast<VkBufferUsageFlagBits>(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT)
        , VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        , m_shapeDescsBuffer);
    m_alloc->stagingMakeAndCopy(sizeShapeDescs, m_shapeDescsBuffer, m_shapeDescs.data());
  }
}

void  PhosScene::update(Pipeline pipeline, bool forceUpdate) {
  if (forceUpdate) {
    if (m_meshDescs.size() > 0) {
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
    if (m_shapeDescs.size() > 0) {
      VkDescriptorBufferInfo bufferInfo = {
        .buffer = m_shapeDescsBuffer.buffer,
        .offset = 0,
        .range = VK_WHOLE_SIZE,
      };
      RtBuilder::DescriptorSetUpdateInfo  updateInfo = {
        .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .binding = BindingsScene::eShapeDescs,
        .pInfo = &bufferInfo,
      };
      pipeline.updateDescSet("scene", updateInfo);
    }
    if (m_materials.size() > 0) {
      VkDescriptorBufferInfo bufferInfo = {
        .buffer = m_materialsBuffer.buffer,
        .offset = 0,
        .range = VK_WHOLE_SIZE,
      };
      RtBuilder::DescriptorSetUpdateInfo  updateInfo = {
        .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .binding = BindingsScene::eMaterials,
        .pInfo = &bufferInfo,
      };
      pipeline.updateDescSet("scene", updateInfo);
    }
  }
  if (forceUpdate) {
    if (m_lights.size() > 0) {
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
}

void  PhosScene::update(Pipeline pipeline, const VkCommandBuffer &cmdBuffer, bool forceUpdate) {
  static bool need_update = true;

  if (forceUpdate || need_update) {
    size_t  sizeLights = static_cast<size_t>(sizeof(m_lights[0]) * m_lights.size()); 
    if (sizeLights > 0)
      pipeline.updateUBO(cmdBuffer, sizeLights, m_lightsBuffer, m_lights.data());
  }
  if (forceUpdate || need_update) {
    size_t  sizeMeshDescs = static_cast<size_t>(sizeof(m_meshDescs[0]) * m_meshDescs.size()); 
    if (sizeMeshDescs > 0)
      pipeline.updateUBO(cmdBuffer, sizeMeshDescs, m_meshDescsBuffer, m_meshDescs.data());
  }
  need_update = false;
}

MeshDesc  PhosScene::buildMeshDesc(PhosObjectInstance& instance, PhosObjectMesh* mesh) {
  if (instance.materialName == "")
    instance.materialName = PHOS_DEFAULT_MAT_NAME;
  uint32_t matId = PhosNamedObject::getIdFromName(m_materials, instance.materialName);
  MeshDesc  meshDesc = {
    .textureId = -1,
    .materialId = matId,
    .vertexAddress = m_alloc->getBufferDeviceAddress(mesh->vertexBuffer),
    .indexAddress = m_alloc->getBufferDeviceAddress(mesh->indexBuffer),
  };
  return (meshDesc);
}

ShapeDesc PhosScene::buildShapeDesc(PhosObjectInstance& instance, PhosObjectProcedural* shape) {
  if (instance.materialName == "")
    instance.materialName = PHOS_DEFAULT_MAT_NAME;
  uint32_t matId = PhosNamedObject::getIdFromName(m_materials, instance.materialName);
  ShapeDesc shapeDesc = {
    .textureId = -1,
    .materialId = matId,
    .marchingMinDist = instance.marchingMinDist,
    .marchingMaxStep = instance.marchingMaxStep,
    .aabb = {
      .min = glm::vec3(shape->aabb.minX, shape->aabb.minY, shape->aabb.minZ),
      .max = glm::vec3(shape->aabb.maxX, shape->aabb.maxY, shape->aabb.maxZ),
    },
  };
  return (shapeDesc);
}
