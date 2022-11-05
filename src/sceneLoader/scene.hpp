#pragma once
#include "../helper/phosHelper.hpp"
#include "../helper/phosNamedObject.hpp"
#include "../helper/allocator.hpp"
#include "../../shaders/hostDevice.h"
#include "../raytracing/pipelineBuilder.hpp"
#include <string>
#include <vector>

#define PHOS_OBJECT_TYPE_MESH         1
#define PHOS_OBJECT_TYPE_PROCEDURAL   2

#define PHOS_DEFAULT_MAT_NAME           "phosDefaultMaterial"
#define PHOS_DEFAULT_RINT_ENTRY_POINT   "main"//"phosDefaultEntryPoint"

struct  PhosHitShader : public PhosNamedObject {
  std::string           pName = "main";
  std::string           spv;
  VkShaderStageFlagBits type;
};

struct  PhosMaterial : public PhosNamedObject, public Material {
};

struct  PhosTexture : public PhosNamedObject {
};

class   PhosObjectMesh : public PhosNamedObject {
  public:
    PhosObjectMesh(){};

    void      destroy(MemoryAllocator &alloc) {
      alloc.destroyBuffer(vertexBuffer);
      alloc.destroyBuffer(indexBuffer);
    }
    uint32_t  strideVertex(){return(sizeof(Vertex));}
    void      createBuffer(MemoryAllocator &alloc);

    std::vector<Vertex>   vertices;
    std::vector<uint32_t> indices;

    BufferWrapper         vertexBuffer;
    BufferWrapper         indexBuffer;
    VkDeviceAddress       blasDeviceAddress = 0;
};

class   PhosObjectProcedural : public PhosNamedObject { //TODO: m_ or not m_ ?
  public:
    PhosObjectProcedural(){};
    void      destroy(MemoryAllocator &alloc) {
      alloc.destroyBuffer(aabbBuffer);
    }
    uint32_t  strideAabb(){return(sizeof(VkAabbPositionsKHR));}
    void      createBuffer(MemoryAllocator &alloc);

    std::string intersectionShaderName = "";

    VkAabbPositionsKHR  aabb;

    BufferWrapper       aabbBuffer;
    VkDeviceAddress     blasDeviceAddress = 0;
    uint32_t            hitShaderBindingIndex = 0;
};

class   PhosObjectInstance : public PhosNamedObject {
  public:
    PhosObjectInstance(){};

    std::string objectName;
    uint32_t    objectType = 0;
    glm::mat4   transform;
    uint32_t    customIndex = 0;

    //MATERIAL
    std::string textureName = "";
    std::string materialName = "";

    //PROCEDURAL SPECIFIC
    float       marchingMinDist = 0.001;
    int         marchingMaxStep = 100;
};

class   PhosScene {
  using Pipeline = RtBuilder::Pipeline;
  public:
    PhosScene() {
      PhosMaterial  default_mat;
      default_mat.name = PHOS_DEFAULT_MAT_NAME;
      default_mat.ambient = glm::vec3(0.0);
      default_mat.diffuse = glm::vec3(0.2);
      default_mat.transmittance = glm::vec3(0.5);
      default_mat.emission = glm::vec3(0.0);
      default_mat.refractionIndex = 0.0;
      default_mat.shininess = 2.0;
      default_mat.specular = 0.3;
      default_mat.dissolve = 0.0;
      default_mat.intensity = 0.0;
      m_materials.push_back(default_mat);
    }
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
    std::vector<PhosMaterial>         m_materials;

  private:
    MeshDesc    buildMeshDesc(PhosObjectInstance& instance, PhosObjectMesh* mesh);
    ShapeDesc   buildShapeDesc(PhosObjectInstance& instance, PhosObjectProcedural* shape);

    uint32_t  event = 0;
    BufferWrapper           m_lightsBuffer;

    std::vector<MeshDesc>   m_meshDescs;
    std::vector<ShapeDesc>  m_shapeDescs;
    BufferWrapper           m_meshDescsBuffer;
    BufferWrapper           m_shapeDescsBuffer;
    BufferWrapper           m_materialsBuffer;
};
