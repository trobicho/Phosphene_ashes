#pragma once
#include "../helper/phosHelper.hpp"
#include "../helper/allocator.hpp"
#include "../../shaders/hostDevice.h"
#include <string>
#include <vector>

#define PHOS_OBJECT_TYPE_MESH         1
#define PHOS_OBJECT_TYPE_PROCEDURAL   2

class PhosObjectMesh {
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
    VkDeviceAddress       m_blasDeviceAddress;
};

class PhosObjectProcedural {
  public:
    PhosObjectProcedural(){};

    std::string m_name = "";
    std::string intersectionShaderName;
};

class PhosObjectInstance {
  public:
    PhosObjectInstance(){};

    std::string name;
    std::string objectName;
    uint32_t    objectType;
    glm::mat4   transform;

    //MATERIAL
};

struct  PhosLight {
  public:
    PhosLight(){};

    float     intensity;
    glm::vec3 color;
    glm::vec3 position;
};

class PhosScene {
  public:
    PhosScene(){};

    void  destroy(MemoryAllocator &alloc) {
      for (auto& mesh : m_meshs) {
        mesh.destroy(alloc);
      }
    }
    void* getInstanceObject(uint32_t idx) {
      if (idx < m_instances.size())
        return (getInstanceObject(m_instances[idx]));
      return (nullptr);
    }
    void* getInstanceObject(PhosObjectInstance &instance);

    std::vector<PhosObjectMesh>       m_meshs;
    std::vector<PhosObjectProcedural> m_proceduraShapes;
    std::vector<PhosObjectInstance>   m_instances;
    std::vector<PhosLight>            m_lights;
};
