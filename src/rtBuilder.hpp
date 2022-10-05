#pragma once
#include "phosHelper.hpp"
#include <vector>

namespace RtBuilder {

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

}
