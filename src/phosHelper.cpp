#include <iostream>
#include <cstdint>
#include <string>
#include <fstream>
#include "phosHelper.hpp"

static std::string printVKBool(VkBool32 vBool) {
  if (vBool == VK_TRUE)
    return ("TRUE");
  else
    return ("FALSE");
}

namespace PhosHelper {
  void  infoInstance() {
    uint32_t apiVersion;

    vkEnumerateInstanceVersion(&apiVersion);
    std::cout << "API version = " << VK_API_VERSION_VARIANT(apiVersion) << " "
        << VK_API_VERSION_MAJOR(apiVersion) << "."
        << VK_API_VERSION_MINOR(apiVersion) << "."
        << VK_API_VERSION_PATCH(apiVersion) << std::endl;
  }

  void  infoRaytracingProperties(VkPhysicalDevice physicalDevice) {
    VkPhysicalDeviceRayTracingPipelineFeaturesKHR rtpFeatures = {
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR,
      .pNext = nullptr,
    };
    VkPhysicalDeviceAccelerationStructureFeaturesKHR  asFeatures = {
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR,
      .pNext = &rtpFeatures,
    };
    VkPhysicalDeviceFeatures2KHR  features = {
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2_KHR,
      .pNext = &asFeatures,
    };
    vkGetPhysicalDeviceFeatures2(physicalDevice, &features);

    std::cout << "Raytracing Features: " << std::endl; {
      std::cout << "\t" << "Acceleration Structure: " << printVKBool(asFeatures.accelerationStructure) << std::endl
        << "\t\t" << "capture replay: " << printVKBool(asFeatures.accelerationStructureCaptureReplay) << std::endl
        << "\t\t" << "indirect build: " << printVKBool(asFeatures.accelerationStructureIndirectBuild) << std::endl
        << "\t\t" << "host command: " << printVKBool(asFeatures.accelerationStructureHostCommands) << std::endl
        << "\t\t" << "descriptor update after bind: " << printVKBool(asFeatures.descriptorBindingAccelerationStructureUpdateAfterBind) << std::endl;

      std::cout << "\t" << "Raytracing Pipeline: " << printVKBool(rtpFeatures.rayTracingPipeline) << std::endl
        << "\t\t" << "shader group handle capture replay: " << printVKBool(rtpFeatures.rayTracingPipelineShaderGroupHandleCaptureReplay) << std::endl
        << "\t\t" << "shader group handle capture replay mixed: " <<
          printVKBool(rtpFeatures.rayTracingPipelineShaderGroupHandleCaptureReplayMixed) << std::endl
        << "\t\t" << "trace rays indirect: " << printVKBool(rtpFeatures.rayTracingPipelineTraceRaysIndirect) << std::endl
        << "\t\t" << "primitive culling: " << printVKBool(rtpFeatures.rayTraversalPrimitiveCulling) << std::endl;
    }

    VkPhysicalDeviceRayTracingPipelinePropertiesKHR rtpProps = {
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR,
      .pNext = nullptr,
    };
    VkPhysicalDeviceAccelerationStructurePropertiesKHR  asProps = {
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR,
      .pNext = &rtpProps,
    };
    VkPhysicalDeviceProperties2KHR  properties = {
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2_KHR,
      .pNext = &asProps,
    };
    vkGetPhysicalDeviceProperties2(physicalDevice, &properties);

    std::cout << std::endl << "Raytracing Properties: " << std::endl; {
      std::cout << "\t" << "Acceleration Structure: " << std::endl
        << "\t\t" << "max Geometry: " << asProps.maxGeometryCount << std::endl
        << "\t\t" << "max Instance: " << asProps.maxInstanceCount << std::endl
        << "\t\t" << "max Primitive: " << asProps.maxPrimitiveCount << std::endl
        << "\t\t" << "max per stage AS: " << asProps.maxPerStageDescriptorAccelerationStructures << std::endl
        << "\t\t" << "max per stage update after bind AS: " << asProps.maxPerStageDescriptorUpdateAfterBindAccelerationStructures << std::endl
        << "\t\t" << "max descSet AS: " << asProps.maxDescriptorSetAccelerationStructures << std::endl
        << "\t\t" << "max descSet update after bind AS: " << asProps.maxDescriptorSetUpdateAfterBindAccelerationStructures << std::endl
        << "\t\t" << "min AS scratch offset alignement: " << asProps.minAccelerationStructureScratchOffsetAlignment << std::endl;
      std::cout << "\t" << "Raytracing Pipeline: " << std::endl
        << "\t\t" << "shader group handle size: " << rtpProps.shaderGroupHandleSize << std::endl
        << "\t\t" << "max ray recursion depth: " << rtpProps.maxRayRecursionDepth << std::endl
        << "\t\t" << "max shader group stride: " << rtpProps.maxShaderGroupStride << std::endl
        << "\t\t" << "shader group base alignement: " << rtpProps.shaderGroupBaseAlignment << std::endl
        << "\t\t" << "shader group handle capture replay size: " << rtpProps.shaderGroupHandleCaptureReplaySize << std::endl
        << "\t\t" << "max ray dispatch invocation: " << rtpProps.maxRayDispatchInvocationCount << std::endl
        << "\t\t" << "shader group handle alignement: " << rtpProps.shaderGroupHandleAlignment << std::endl
        << "\t\t" << "max ray hit attribute size: " << rtpProps.maxRayHitAttributeSize<< std::endl;
    }
  }
}

namespace PhosHelper {
  std::vector<char>   readBinaryFile(const std::string &filename) {
    std::ifstream	file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open())
      throw PhosHelper::BasicError(std::string("error reading binary file ") + filename);

    size_t  fileSize = (size_t)file.tellg();
    std::vector<char>   buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);

    return (buffer);
  }

  VkShaderModule	createShaderModuleFromCode(VkDevice device, std::vector<char> code) {
    VkShaderModuleCreateInfo  info{};
    VkShaderModule            shaderModule;

    info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    info.codeSize = code.size();
    info.pCode = reinterpret_cast<const uint32_t*>(code.data());
    if (vkCreateShaderModule(device, &info
          , nullptr, &shaderModule) != VK_SUCCESS)
      throw PhosHelper::BasicError("unable to create shader module!");
    return (shaderModule);
  }
}
