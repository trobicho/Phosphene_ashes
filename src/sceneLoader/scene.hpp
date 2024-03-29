#pragma once
#include "../helper/phosHelper.hpp"
#include "../helper/phosNamedObject.hpp"
#include "../helper/allocator.hpp"
#include "../../shaders/hostDevice.h"
#include "../raytracing/pipelineBuilder.hpp"
#include <cstdint>
#include <string>
#include <vector>
#include "nanovdb/NanoVDB.h"
#include "nanovdb/util/IO.h"

#define PHOS_OBJECT_TYPE_MESH         1
#define PHOS_OBJECT_TYPE_PROCEDURAL   2
#define PHOS_OBJECT_TYPE_VDB					3

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

class   PhosObjectVdb : public PhosNamedObject {
  public:
    PhosObjectVdb(){};

    void      destroy(MemoryAllocator &alloc) {
      alloc.destroyBuffer(aabbBuffer);
			for (auto& grid : grids) {
      	grid.destroy(alloc);
			}
		}
		
		struct VdbGrid : public PhosNamedObject {
			VdbGrid(){};
			VdbGrid(VdbGrid&& vdbGrid) {
				name = vdbGrid.name;
				handle = std::move(vdbGrid.handle);
			}
			VdbGrid(const VdbGrid& vdbGrid) { //SHOULD NOT BE USED except for checking name
				name = vdbGrid.name;
			};

			void      destroy(MemoryAllocator &alloc) {
      	alloc.destroyBuffer(deviceBuffer);
			}

			nanovdb::GridHandle<nanovdb::HostBuffer>	handle;
			BufferWrapper       											deviceBuffer;
		};

    uint32_t  strideAabb(){return(sizeof(VkAabbPositionsKHR));}
    void      createBuffer(MemoryAllocator &alloc);
    
		VkAabbPositionsKHR  	aabb;
		std::vector<VdbGrid>	grids;
    
		BufferWrapper       aabbBuffer;
    VkDeviceAddress     blasDeviceAddress = 0;
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
    void            setVdbHitBindingIndex(uint32_t offset = 0) {m_vdbHitBindingIndex = offset;}
    uint32_t				getVdbHitBindingIndex() {return (m_vdbHitBindingIndex);}
		bool						hasMesh() {return (m_meshs.size() > 0);}
		bool						hasShape() {return (m_proceduralShapes.size() > 0);}
		bool						hasVdb() {return (m_vdbs.size() > 0);}

    void  allocateResources();
    void  update(Pipeline pipeline, bool forceUpdate = false);
    void  update(Pipeline pipeline, const VkCommandBuffer &cmdBuffer, bool forceUpdate = false);

    MemoryAllocator*                  m_alloc;
    std::vector<PhosObjectMesh>       m_meshs;
    std::vector<PhosObjectProcedural> m_proceduralShapes;
    std::vector<PhosObjectVdb>				m_vdbs;
    std::vector<PhosObjectInstance>   m_instances;
    std::vector<Light>                m_lights;
    std::vector<PhosHitShader>        m_hitShaders;
    PhosHitShader											m_vdbDefaultHitShader;
    std::vector<PhosMaterial>         m_materials;

  private:
    MeshDesc    buildMeshDesc(PhosObjectInstance& instance, PhosObjectMesh* mesh);
    ShapeDesc   buildShapeDesc(PhosObjectInstance& instance, PhosObjectProcedural* shape);
		VdbDesc			buildVdbDesc(PhosObjectInstance& instance, PhosObjectVdb* vdb);

    uint32_t  event = 0;
    BufferWrapper           m_lightsBuffer;

    std::vector<MeshDesc>   m_meshDescs;
    std::vector<ShapeDesc>  m_shapeDescs;
    std::vector<VdbDesc>		m_vdbDescs;
    BufferWrapper           m_meshDescsBuffer;
    BufferWrapper           m_shapeDescsBuffer;
    BufferWrapper           m_vdbDescsBuffer;
    BufferWrapper           m_materialsBuffer;

		uint32_t								m_vdbHitBindingIndex = 0;
};
