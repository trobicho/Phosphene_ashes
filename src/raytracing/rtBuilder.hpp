#pragma once
#include "../helper/phosHelper.hpp"
#include "../sceneLoader/scene.hpp"
#include <vector>

namespace RtBuilder {

/*
enum    ShaderType {
  eRaygen,
  eMiss,
  eClosestHit,
  eIntersection,
};

struct  ShaderStage {
  VkPipelineShaderStageCreateInfo stageInfo;
  ShaderType                      type;
};

class   PipelineBuilder {
  public:
    PipelineBuilder(VkDevice  &device): m_device(device){};

  private:
    VkDevice  m_device;

    std::vector<VkPushConstantRange>              pushConstantRange;
    std::vector<VkPipelineShaderStageCreateInfo>  stages;
};

class SBTable {
  public:
    VkStridedDeviceAddressRegionKHR   rgenRegion{};
};
*/

struct  BlasInput {
  VkAccelerationStructureGeometryKHR        asGeometry;
  VkAccelerationStructureBuildRangeInfoKHR  asBuildOffsetInfo;
  VkBuildAccelerationStructureFlagsKHR      flags{0};
};

class SceneBuilder {
  public:
    SceneBuilder(){};

    void  init(VkDevice device, MemoryAllocator *alloc, uint32_t queueFamilyIndex);

    void  buildBlas(PhosScene &scene, VkBuildAccelerationStructureFlagsKHR flags);
    void  buildTlas();

  private:

    struct  BuildAccelerationStructure {
      VkAccelerationStructureBuildGeometryInfoKHR     buildInfo{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR};
      VkAccelerationStructureBuildSizesInfoKHR        sizeInfo{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR};
      const VkAccelerationStructureBuildRangeInfoKHR* rangeInfo;
    };

    void  modelToVkGeometry(PhosObjectMesh& model);
    void  cmdCreateBlas(VkCommandBuffer cmdBuffer
                        , std::vector<uint32_t> indices
                        , std::vector<BuildAccelerationStructure> buildAs
                        , VkDeviceAddress scratchAddress);

    VkDevice          m_device = VK_NULL_HANDLE;
    MemoryAllocator   *m_alloc;

    std::vector<BlasInput>  m_blasInput;
    std::vector<AccelKHR>   m_blas;
    AccelKHR                m_tlas;
    CommandPool             m_cmdPool;
};

}
