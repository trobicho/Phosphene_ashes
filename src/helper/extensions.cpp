#include "extensions.hpp"

static PFN_vkCreateRayTracingPipelinesKHR pfn_vkCreateRayTracingPipelinesKHR = 0;
static PFN_vkCreateAccelerationStructureKHR pfn_vkCreateAccelerationStructureKHR = 0;
static PFN_vkDestroyAccelerationStructureKHR pfn_vkDestroyAccelerationStructureKHR = 0;
static PFN_vkGetRayTracingShaderGroupHandlesKHR pfn_vkGetRayTracingShaderGroupHandlesKHR = 0;
static PFN_vkGetAccelerationStructureBuildSizesKHR  pfn_vkGetAccelerationStructureBuildSizesKHR = 0;
static PFN_vkCmdBuildAccelerationStructuresKHR  pfn_vkCmdBuildAccelerationStructuresKHR = 0;
static PFN_vkCmdTraceRaysKHR  pfn_vkCmdTraceRaysKHR = 0;

void  PhosHelper::loadRtExtension(VkDevice &device) {
  pfn_vkCreateRayTracingPipelinesKHR =
    reinterpret_cast<PFN_vkCreateRayTracingPipelinesKHR>(vkGetDeviceProcAddr(device, "vkCreateRayTracingPipelinesKHR"));
  pfn_vkGetRayTracingShaderGroupHandlesKHR =
    reinterpret_cast<PFN_vkGetRayTracingShaderGroupHandlesKHR>(vkGetDeviceProcAddr(device, "vkGetRayTracingShaderGroupHandlesKHR"));
  pfn_vkCreateAccelerationStructureKHR =
    reinterpret_cast<PFN_vkCreateAccelerationStructureKHR>(vkGetDeviceProcAddr(device, "vkCreateAccelerationStructureKHR"));
  pfn_vkDestroyAccelerationStructureKHR =
    reinterpret_cast<PFN_vkDestroyAccelerationStructureKHR>(vkGetDeviceProcAddr(device, "vkDestroyAccelerationStructureKHR"));
  pfn_vkGetAccelerationStructureBuildSizesKHR = 
    reinterpret_cast<PFN_vkGetAccelerationStructureBuildSizesKHR>(vkGetDeviceProcAddr(device, "vkGetAccelerationStructureBuildSizesKHR"));
  pfn_vkCmdBuildAccelerationStructuresKHR = 
    reinterpret_cast<PFN_vkCmdBuildAccelerationStructuresKHR>(vkGetDeviceProcAddr(device, "vkCmdBuildAccelerationStructuresKHR"));
  pfn_vkCmdTraceRaysKHR =
    reinterpret_cast<PFN_vkCmdTraceRaysKHR>(vkGetDeviceProcAddr(device, "vkCmdTraceRaysKHR"));
}

VKAPI_ATTR VkResult VKAPI_CALL  vkCreateRayTracingPipelinesKHR(VkDevice device
                                  , VkDeferredOperationKHR deferredOperation
                                  , VkPipelineCache pipelineCache
                                  , uint32_t createInfoCount
                                  , const VkRayTracingPipelineCreateInfoKHR *pCreateInfos
                                  , const VkAllocationCallbacks *pAllocator
                                  , VkPipeline *pPipelines)
{
  return (pfn_vkCreateRayTracingPipelinesKHR(device, deferredOperation, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines));
}

VKAPI_ATTR VkResult VKAPI_CALL  vkCreateAccelerationStructureKHR(VkDevice device
                                  , const VkAccelerationStructureCreateInfoKHR *pCreateInfo
                                  , const VkAllocationCallbacks *pAllocator
                                  , VkAccelerationStructureKHR *pAccelerationStructure)
{
  return (pfn_vkCreateAccelerationStructureKHR(device, pCreateInfo, pAllocator, pAccelerationStructure));
}

VKAPI_ATTR void VKAPI_CALL      vkDestroyAccelerationStructureKHR(VkDevice device
                                  , VkAccelerationStructureKHR accelerationStructure
                                  , const VkAllocationCallbacks *pAllocator)
{
  pfn_vkDestroyAccelerationStructureKHR(device, accelerationStructure, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL  vkGetRayTracingShaderGroupHandlesKHR(VkDevice device
                                  , VkPipeline pipeline
                                  , uint32_t firstGroup
                                  , uint32_t groupCount
                                  , size_t dataSize
                                  , void *pData)
{
  return(pfn_vkGetRayTracingShaderGroupHandlesKHR(device, pipeline, firstGroup, groupCount, dataSize, pData));
}

VKAPI_ATTR void VKAPI_CALL      vkGetAccelerationStructureBuildSizesKHR(VkDevice device
                                  , VkAccelerationStructureBuildTypeKHR buildType
                                  , const VkAccelerationStructureBuildGeometryInfoKHR *pBuildInfo
                                  , const uint32_t *pMaxPrimitiveCounts
                                  , VkAccelerationStructureBuildSizesInfoKHR *pSizeInfo)
{
  pfn_vkGetAccelerationStructureBuildSizesKHR(device, buildType, pBuildInfo, pMaxPrimitiveCounts, pSizeInfo);
}

VKAPI_ATTR void VKAPI_CALL      vkCmdBuildAccelerationStructuresKHR(VkCommandBuffer commandBuffer
                                  , uint32_t infoCount
                                  , const VkAccelerationStructureBuildGeometryInfoKHR *pInfos
                                  , const VkAccelerationStructureBuildRangeInfoKHR *const *ppBuildRangeInfos)
{
  pfn_vkCmdBuildAccelerationStructuresKHR(commandBuffer, infoCount, pInfos, ppBuildRangeInfos);
}

VKAPI_ATTR void VKAPI_CALL      vkCmdTraceRaysKHR(VkCommandBuffer commandBuffer
                                  , const VkStridedDeviceAddressRegionKHR *pRaygenShaderBindingTable
                                  , const VkStridedDeviceAddressRegionKHR *pMissShaderBindingTable
                                  , const VkStridedDeviceAddressRegionKHR *pHitShaderBindingTable
                                  , const VkStridedDeviceAddressRegionKHR *pCallableShaderBindingTable
                                  , uint32_t width, uint32_t height, uint32_t depth)
{
  pfn_vkCmdTraceRaysKHR(commandBuffer
      , pRaygenShaderBindingTable
      , pMissShaderBindingTable
      , pHitShaderBindingTable
      , pCallableShaderBindingTable
      , width, height, depth);
}
