#pragma once
#include "../helper/phosHelper.hpp"
#include "../helper/allocator.hpp"
#include "../../shaders/hostDevice.h"
#include "../raytracing/pipelineBuilder.hpp"
#include <string>
#include <vector>

#define PHOS_OBJECT_TYPE_MESH         1
#define PHOS_OBJECT_TYPE_PROCEDURAL   2

struct  PhosHitShader {
  std::string           name;
  std::string           pName = "main";
  std::string           spv;
  VkShaderStageFlagBits type;
};

class   PhosObjectMesh {
  public:
    PhosObjectMesh(){};

    void      destroy(MemoryAllocator &alloc) {
      alloc.destroyBuffer(m_vertexBuffer);
      alloc.destroyBuffer(m_indexBuffer);
    }
    uint32_t  strideVertex(){return(sizeof(Vertex));}
    void      createBuffer(MemoryAllocator &alloc);

    std::string           m_name = "";
    std::vector<Vertex>   m_vertices;
    std::vector<uint32_t> m_indices;
    BufferWrapper         m_vertexBuffer;
    BufferWrapper         m_indexBuffer;
    VkDeviceAddress       m_blasDeviceAddress = 0;
};

class   PhosObjectProcedural { //TODO: m_ or not m_ ?
  public:
    PhosObjectProcedural(){};
    void      destroy(MemoryAllocator &alloc) {
      alloc.destroyBuffer(aabbBuffer);
    }
    uint32_t  strideAabb(){return(sizeof(VkAabbPositionsKHR));}
    void      createBuffer(MemoryAllocator &alloc);

    std::string         m_name = "";
    std::string         intersectionShaderName = "";
    VkAabbPositionsKHR  aabb;
    BufferWrapper       aabbBuffer;
    VkDeviceAddress     m_blasDeviceAddress = 0;
    uint32_t            hitShaderBindingIndex = 0;
};

class   PhosObjectInstance {
  public:
    PhosObjectInstance(){};

    std::string name;
    std::string objectName;
    uint32_t    objectType = 0;
    glm::mat4   transform;
    uint32_t    customIndex = 0;

    //MATERIAL
};

class   PhosScene {
  using Pipeline = RtBuilder::Pipeline;
  public:
    PhosScene(){};
    void  init(MemoryAllocator *alloc) {
      m_alloc = alloc;
    }

    void  destroy();
    void* getInstanceObject(uint32_t idx) {
      if (idx < m_instances.size())
        return (getInstanceObject(m_instances[idx]));
      return (nullptr);
    }
    void*           getInstanceObject(PhosObjectInstance &instance);
    uint32_t        getLightCount() {return (m_lights.size());}
    PhosHitShader*  getShader(const std::string name, uint32_t* index = nullptr);
    void            setShapesHitBindingIndex(uint32_t offset = 0);

    void  allocateResources();
    void  update(Pipeline pipeline, bool forceUpdate = false);
    void  update(Pipeline pipeline, const VkCommandBuffer &cmdBuffer, bool forceUpdate = false);

    MemoryAllocator*                  m_alloc;
    std::vector<PhosObjectMesh>       m_meshs;
    std::vector<PhosObjectProcedural> m_proceduraShapes;
    std::vector<PhosObjectInstance>   m_instances;
    std::vector<Light>                m_lights;
    std::vector<PhosHitShader>        m_hitShaders;

  private:
    uint32_t  event = 0;
    BufferWrapper           m_lightsBuffer;
    std::vector<MeshDesc>   m_meshDescs;
    BufferWrapper           m_meshDescsBuffer;
};
