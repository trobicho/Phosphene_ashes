#include "phosStartVk.hpp"
#include <fstream>
#include <iostream>
#include <sstream>

namespace PhosStartVk {

void              getQueueFamilyIndex(VkPhysicalDevice device, VkSurfaceKHR surface, uint32_t &queueFamilyIndex) {
  uint32_t queueCount;
  VkBool32  presentSupport;

  std::vector<VkQueueFamilyProperties> queueProps;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueCount, nullptr);
  if (queueCount == 0)
    throw PhosHelper::FatalVulkanInitError("No Queue found !");
  queueProps.resize(queueCount);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueCount, queueProps.data());
  for (int i = 0; i < queueCount; ++i) {
    vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
    if (queueProps[i].queueCount > 0
        && queueProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT
        && presentSupport) {
      queueFamilyIndex = i;
      return ;
    }
  }
  throw PhosHelper::FatalVulkanInitError("No Queue found !");
}

VkInstance        createInstance() {
  VkApplicationInfo appInfo = {
    .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
    .pApplicationName = "Phosphene",
    .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
    .pEngineName = "Phosphene",
    .engineVersion = VK_MAKE_VERSION(1, 0, 0),
    .apiVersion = VK_API_VERSION_1_3
  };

  auto exts = PhosStartVk::instanceExtensions();
  auto layers = PhosStartVk::validationLayers();
  //TODO: Check valLayers exts and return VK_SUCCESS
  VkInstanceCreateInfo createInfo = {
    .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
    .pApplicationInfo = &appInfo,
    .enabledLayerCount = (uint32_t)layers.size(),
    .ppEnabledLayerNames = layers.data(),
    .enabledExtensionCount = (uint32_t)exts.size(),
    .ppEnabledExtensionNames = exts.data(),
  };

  VkInstance  instance;
  if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
    throw PhosHelper::FatalVulkanInitError("Failed to create Instance");
  return (instance);
}

VkPhysicalDevice  choosePhysicalDevice(VkInstance &instance) { //TODO: make the thing
  uint32_t  deviceCount;
  std::vector<VkPhysicalDevice>   device;

  vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
  device.resize(deviceCount);
  vkEnumeratePhysicalDevices(instance, &deviceCount, device.data());

  VkPhysicalDeviceProperties  props;
  vkGetPhysicalDeviceProperties(device[0], &props);
  std::cout << props.deviceName << std::endl;

  return (device[0]);
}

void              createLogicalDeviceAndQueue(VkDevice &device
                                              , const VkPhysicalDevice &physicalDevice
                                              , const VkSurfaceKHR &surface
                                              , VkQueue &queue
                                              , uint32_t &queueFamilyIndex) {
  float queuePriority = 1.0f;

  PhosHelper::infoRaytracingProperties(physicalDevice);

  PhosStartVk::getQueueFamilyIndex(physicalDevice, surface, queueFamilyIndex);
  VkDeviceQueueCreateInfo queueInfo = {
    .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
    .queueFamilyIndex = queueFamilyIndex,
    .queueCount = 1,
    .pQueuePriorities = &queuePriority,
  };
  auto  deviceExtentions = PhosStartVk::deviceExtensions();
  //VkPhysicalDeviceFeatures  deviceFeature = {};

  //FEATURES
  VkPhysicalDeviceScalarBlockLayoutFeatures     scalarBlockFeatures = {
    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SCALAR_BLOCK_LAYOUT_FEATURES,
    .scalarBlockLayout = VK_TRUE,
  };
  //RAYTRACING FEATURES
  VkPhysicalDeviceBufferDeviceAddressFeatures   deviceAddressFeatures = {
    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES,
    .pNext = &scalarBlockFeatures,
    .bufferDeviceAddress = VK_TRUE,
  };
  VkPhysicalDeviceRayTracingPipelineFeaturesKHR rtpFeatures = {
    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR,
    .pNext = &deviceAddressFeatures,
    .rayTracingPipeline = VK_TRUE,
  };
  VkPhysicalDeviceAccelerationStructureFeaturesKHR  asFeatures = {
    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR,
    .pNext = &rtpFeatures,
    .accelerationStructure = VK_TRUE,
  };
  VkPhysicalDeviceFeatures2KHR  features = {
    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2_KHR,
    .pNext = &asFeatures,
    .features = {
      .shaderInt64 = VK_TRUE,
    },
  };
  //-------------------------

  VkDeviceCreateInfo        deviceInfo = {
    .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
    .pNext = &features,
    .queueCreateInfoCount = 1,
    .pQueueCreateInfos = &queueInfo,
    .enabledExtensionCount = (uint32_t)deviceExtentions.size(),
    .ppEnabledExtensionNames = deviceExtentions.data(),
    //.pEnabledFeatures = &deviceFeature,
  };
	VkResult result;
  if ((result = vkCreateDevice(physicalDevice, &deviceInfo
      , nullptr, &device)) != VK_SUCCESS) {
		std::stringstream error; 
		error << "Failed to create logical device! (";
		error << PhosHelper::VkResultToString(result) << ")";
    throw PhosHelper::FatalVulkanInitError(error.str());
	}
  vkGetDeviceQueue(device, queueFamilyIndex, 0, &queue);
}

}
