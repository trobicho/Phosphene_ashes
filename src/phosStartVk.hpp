#pragma once
#include "phosHelper.hpp"
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

namespace PhosStartVk {
  inline std::vector<const char*>  validationLayers() {
    std::vector<const char*> layers = {
      "VK_LAYER_KHRONOS_validation",
    };
    return (layers);
  }

  inline std::vector<const char*>  instanceExtensions() {
    std::vector<const char*> exts = {VK_EXT_DEBUG_UTILS_EXTENSION_NAME};

    uint32_t glfwExtCount;
    const char **glfwExt = glfwGetRequiredInstanceExtensions(&glfwExtCount);
    for (int i = 0; i < glfwExtCount; ++i) {
      exts.push_back(glfwExt[i]);
    }
    return (exts);
  }

  inline std::vector<const char*>  deviceExtensions() {
    std::vector<const char*> exts = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME,

      VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
      VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
      VK_KHR_RAY_QUERY_EXTENSION_NAME,

      VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME,

      VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
      VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
      VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,

      VK_KHR_SPIRV_1_4_EXTENSION_NAME,
      VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME,
    };
    return (exts);
  }

  void              getQueueFamilyIndex(VkPhysicalDevice device, VkSurfaceKHR surface, uint32_t &queueFamilyIndex);
  VkInstance        createInstance();
  VkPhysicalDevice  choosePhysicalDevice(VkInstance &instance);
  void              createLogicalDeviceAndQueue(VkDevice &device
                                                , const VkPhysicalDevice &physicalDevice
                                                , const VkSurfaceKHR &surface
                                                , VkQueue &queue
                                                , uint32_t &queueFamilyIndex);
};
