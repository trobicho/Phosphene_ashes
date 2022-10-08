#pragma once
#include "../helper/phosHelper.hpp"
#include "../helper/allocator.hpp"
#include <string>
#include <vector>

enum  ObjectType {
  eMesh,
  eProceduralShape,
};

class ObjectContainer {
  public:
    ObjectContainer(){};

    std::string m_name = "";
    ObjectType  m_type;
};

template<class T>
class PhosObjectMesh: public ObjectContainer {
  public:
    PhosObjectMesh(){m_type = eMesh;}
    
    void      destroy(VkDevice &device) {
      vkDestroyBuffer(device, m_vertexBuffer.buffer, nullptr);
      vkFreeMemory(device, m_vertexBuffer.memory, nullptr);
      vkDestroyBuffer(device, m_indexBuffer.buffer, nullptr);
      vkFreeMemory(device, m_indexBuffer.memory, nullptr);
    }
    uint32_t  strideVertex(){return(sizeof(T));}
    void      createBuffer() {
    }

    std::vector<T>        m_vertices;
    std::vector<uint32_t> m_indices;
    BufferWrapper         m_vertexBuffer;
    BufferWrapper         m_indexBuffer;
};

class PhosObjectProcedural: public ObjectContainer {
  public:
    PhosObjectProcedural(){m_type = eProceduralShape;}

    std::string intersectionShaderName;
};

class PhosObjectInstance {
  public:
    PhosObjectInstance(){};

    std::string name;
    uint32_t    index;
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

    std::vector<ObjectContainer>    m_objects;
    std::vector<PhosObjectInstance> m_instances;
    std::vector<PhosLight>          m_lights;
};
