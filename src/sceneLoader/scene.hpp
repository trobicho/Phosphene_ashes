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
    
    void      destroy(VkDevice &device) {
      vkDestroyBuffer(device, m_vertexBuffer.buffer, nullptr);
      vkFreeMemory(device, m_vertexBuffer.memory, nullptr);
      vkDestroyBuffer(device, m_indexBuffer.buffer, nullptr);
      vkFreeMemory(device, m_indexBuffer.memory, nullptr);
    }
    uint32_t  strideVertex(){return(sizeof(Vertex));}
    void      createBuffer() {
    }

    std::string           m_name = "";
    std::vector<Vertex>   m_vertices;
    std::vector<uint32_t> m_indices;
    BufferWrapper         m_vertexBuffer;
    BufferWrapper         m_indexBuffer;
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

    std::vector<PhosObjectMesh>       m_meshs;
    std::vector<PhosObjectProcedural> m_proceduraShapes;
    std::vector<PhosObjectInstance>   m_instances;
    std::vector<PhosLight>            m_lights;
};
