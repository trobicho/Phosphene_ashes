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
  void  printMatrix(glm::mat4 matrix) {
      std::cout 
        << matrix[0][0] << ", "
        << matrix[0][1] << ", "
        << matrix[0][2] << ", "
        << matrix[0][3] << std::endl
        << matrix[1][0] << ", "
        << matrix[1][1] << ", "
        << matrix[1][2] << ", "
        << matrix[1][3] << std::endl
        << matrix[2][0] << ", "
        << matrix[2][1] << ", "
        << matrix[2][2] << ", "
        << matrix[2][3] << std::endl
        << matrix[3][0] << ", "
        << matrix[3][1] << ", "
        << matrix[3][2] << ", "
        << matrix[3][3] << std::endl;
  }

  VkTransformMatrixKHR  matrixToVkTransformMatrix(glm::mat4 matrix) {
    VkTransformMatrixKHR result;

    result.matrix[0][0] = matrix[0][0]; result.matrix[0][1] = matrix[0][1]; result.matrix[0][2] = matrix[0][2]; result.matrix[0][3] = matrix[0][3];
    result.matrix[1][0] = matrix[1][0]; result.matrix[1][1] = matrix[1][1]; result.matrix[1][2] = matrix[1][2]; result.matrix[1][3] = matrix[1][3];
    result.matrix[2][0] = matrix[2][0]; result.matrix[2][1] = matrix[2][1]; result.matrix[2][2] = matrix[2][2]; result.matrix[2][3] = matrix[2][3];
    return (result);
  }

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

namespace PhosHelper {

std::string VkResultToString(VkResult result) {
  switch(result) {
    case VK_SUCCESS:
      return ("VK_SUCCESS");
    case VK_TIMEOUT:
      return ("VK_TIMEOUT");
    case VK_NOT_READY:
      return ("VK_NOT_READY");
    case VK_SUBOPTIMAL_KHR:
      return ("VK_SUBOPTIMAL_KHR");
    case VK_EVENT_SET:
      return ("VK_EVENT_SET");
    case VK_EVENT_RESET:
      return ("VK_EVENT_RESET");
    case VK_INCOMPLETE:
      return ("VK_INCOMPLETE");

    case VK_ERROR_OUT_OF_HOST_MEMORY:
      return ("VK_ERROR_OUT_OF_HOST_MEMORY");
    case VK_ERROR_OUT_OF_DEVICE_MEMORY:
      return ("VK_ERROR_OUT_OF_DEVICE_MEMORY");
    case VK_ERROR_INITIALIZATION_FAILED:
      return ("VK_ERROR_INITIALIZATION_FAILED");
    case VK_ERROR_DEVICE_LOST:
      return ("VK_ERROR_DEVICE_LOST");
    case VK_ERROR_MEMORY_MAP_FAILED:
      return ("VK_ERROR_MEMORY_MAP_FAILED");
    case VK_ERROR_LAYER_NOT_PRESENT:
      return ("VK_ERROR_LAYER_NOT_PRESENT");
    case VK_ERROR_EXTENSION_NOT_PRESENT:
      return ("VK_ERROR_EXTENSION_NOT_PRESENT");
    case VK_ERROR_FEATURE_NOT_PRESENT:
      return ("VK_ERROR_FEATURE_NOT_PRESENT");
    case VK_ERROR_INCOMPATIBLE_DRIVER:
      return ("VK_ERROR_INCOMPATIBLE_DRIVER");
    case VK_ERROR_TOO_MANY_OBJECTS:
      return ("VK_ERROR_TOO_MANY_OBJECTS");
    case VK_ERROR_FORMAT_NOT_SUPPORTED:
      return ("VK_ERROR_FORMAT_NOT_SUPPORTED");
    case VK_ERROR_FRAGMENTED_POOL:
      return ("VK_ERROR_FRAGMENTED_POOL");
    case VK_ERROR_UNKNOWN:
      return ("VK_ERROR_UNKNOWN");
      // Provided by VK_VERSION_1_1:
    case VK_ERROR_OUT_OF_POOL_MEMORY:
      return ("VK_ERROR_OUT_OF_POOL_MEMORY");
      // Provided by VK_VERSION_1_1:
    case VK_ERROR_INVALID_EXTERNAL_HANDLE:
      return ("VK_ERROR_INVALID_EXTERNAL_HANDLE");
      // Provided by VK_VERSION_1_2:
      return ("VK_ERROR_FRAGMENTATION");
      // Provided by VK_VERSION_1_2:
    case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS:
      return ("VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS");
      // Provided by VK_VERSION_1_3:
    case VK_PIPELINE_COMPILE_REQUIRED:
      return ("VK_PIPELINE_COMPILE_REQUIRED");
      // Provided by VK_KHR_surface:
    case VK_ERROR_SURFACE_LOST_KHR:
      return ("VK_ERROR_SURFACE_LOST_KHR");
      // Provided by VK_KHR_surface:
    case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
      return ("VK_ERROR_NATIVE_WINDOW_IN_USE_KHR");
      // Provided by VK_KHR_swapchain:
    case VK_ERROR_OUT_OF_DATE_KHR:
      return ("VK_ERROR_OUT_OF_DATE_KHR");
      // Provided by VK_KHR_display_swapchain:
    case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
      return ("VK_ERROR_INCOMPATIBLE_DISPLAY_KHR");
      // Provided by VK_EXT_debug_report:
    case VK_ERROR_VALIDATION_FAILED_EXT:
      return ("VK_ERROR_VALIDATION_FAILED_EXT");
      // Provided by VK_NV_glsl_shader:
    case VK_ERROR_INVALID_SHADER_NV:
      return ("VK_ERROR_INVALID_SHADER_NV");
      // Provided by VK_EXT_image_drm_format_modifier:
    case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT:
      return ("VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT");
      // Provided by VK_KHR_global_priority:
    case VK_ERROR_NOT_PERMITTED_KHR:
      return ("VK_ERROR_NOT_PERMITTED_KHR");
      // Provided by VK_EXT_full_screen_exclusive:
    case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
      return ("VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT");
      // Provided by VK_KHR_deferred_host_operations:
    case VK_THREAD_IDLE_KHR:
      return ("VK_THREAD_IDLE_KHR");
      // Provided by VK_KHR_deferred_host_operations:
    case VK_THREAD_DONE_KHR:
      return ("VK_THREAD_DONE_KHR");
      // Provided by VK_KHR_deferred_host_operations:
    case VK_OPERATION_DEFERRED_KHR:
      return ("VK_OPERATION_DEFERRED_KHR");
      // Provided by VK_KHR_deferred_host_operations:
    case VK_OPERATION_NOT_DEFERRED_KHR:
      return ("VK_OPERATION_NOT_DEFERRED_KHR");
      // Provided by VK_EXT_image_compression_control:
    case VK_ERROR_COMPRESSION_EXHAUSTED_EXT:
      return ("VK_ERROR_COMPRESSION_EXHAUSTED_EXT");
      // Provided by VK_EXT_descriptor_indexing:
    case VK_ERROR_FRAGMENTATION_EXT:
      return ("VK_ERROR_FRAGMENTATION_EXT");
#ifdef VK_ENABLE_BETA_EXTENSIONS
      return ("VK_ENABLE_BETA_EXTENSIONS");
      // Provided by VK_KHR_video_queue:
    case VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR:
      return ("VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR");
      // Provided by VK_KHR_video_queue:
    case VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR:
      return ("VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR");
      // Provided by VK_KHR_video_queue:
    case VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR:
      return ("VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR");
      // Provided by VK_KHR_video_queue:
    case VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR:
      return ("VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR");
      // Provided by VK_KHR_video_queue:
    case VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR:
      return ("VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR");
      return ("VK_ENABLE_BETA_EXTENSIONS");
      // Provided by VK_KHR_video_queue:
    case VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR:
      return ("VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR");
#endif
    case VK_RESULT_MAX_ENUM:
      return ("VK_RESULT_MAX_ENUM");
  }
  return ("");
}

}//namespace PhosHelper
