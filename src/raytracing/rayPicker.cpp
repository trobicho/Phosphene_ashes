#include "rayPicker.hpp"

void  RayPicker::init(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex) {
  m_device = device;
  m_cmdPool.init(device, queueFamilyIndex);
  m_physicalDevice = physicalDevice;
  m_alloc.init(device, physicalDevice, queueFamilyIndex);
}

void  RayPicker::destroy() {
  vkDestroyPipeline(m_device, m_pipeline, nullptr);
  vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);
  vkDestroyDescriptorSetLayout(m_device, m_descSetLayout, nullptr);
  vkDestroyDescriptorPool(m_device, m_descPool, nullptr);
  m_alloc.destroy();
  m_cmdPool.destroy();
}

void  RayPicker::updateCamera(BufferWrapper& cameraUBO) {
  std::vector<VkWriteDescriptorSet>  writes;
  VkDescriptorBufferInfo bufferInfo = {
    .buffer = cameraUBO.buffer,
    .offset = 0,
    .range = VK_WHOLE_SIZE,
  };
  VkWriteDescriptorSet  write = {
    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
    .dstSet = 0,
    .dstBinding = BindingsPicker::ePickerTlas,
    .dstArrayElement = 0,
    .descriptorCount = 1,
    .descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,
    .pBufferInfo = &bufferInfo,
  };
  vkUpdateDescriptorSets(m_device, writes.size(), writes.data(), 0, nullptr);
}

void  RayPicker::updateTlas(AccelKHR& tlas) {
  std::vector<VkWriteDescriptorSet>  writes;
  VkWriteDescriptorSetAccelerationStructureKHR  asInfo = {
    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR,
    .accelerationStructureCount = 1,
    .pAccelerationStructures = &tlas.accel,
  };
  VkWriteDescriptorSet  write = {
    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
    .pNext = &asInfo,
    .dstSet = 0,
    .dstBinding = BindingsPicker::ePickerTlas,
    .dstArrayElement = 0,
    .descriptorCount = 1,
    .descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,
  };
  vkUpdateDescriptorSets(m_device, writes.size(), writes.data(), 0, nullptr);
}

void  RayPicker::build() {
  buildDescSet();

  VkPushConstantRange pushConstantRange = {
    .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
    .offset = 0,
    .size = sizeof(m_pcPickRay),
  };

  VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
    .setLayoutCount = 1,
    .pSetLayouts = &m_descSetLayout,
    .pushConstantRangeCount = 1,
    .pPushConstantRanges = &pushConstantRange,
  };

  VkShaderModule  computeModule = PhosHelper::createShaderModuleFromFile(m_device, "spv/rayPick.comp.spv");

  VkPipelineShaderStageCreateInfo shaderStageInfo = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
    .stage = VK_SHADER_STAGE_COMPUTE_BIT,
    .module = computeModule,
    .pName = "main",
  };

  if (vkCreatePipelineLayout(m_device, &pipelineLayoutInfo
        , nullptr, &m_pipelineLayout) != VK_SUCCESS)
    throw PhosHelper::FatalError("failed to create RayPicker pipeline layout!");

  VkComputePipelineCreateInfo pipelineInfo = {
    .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
    .layout = m_pipelineLayout,
  };

  if (vkCreateComputePipelines(m_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_pipeline) != VK_SUCCESS)
    throw PhosHelper::FatalError("failed to create RayPicker pipeline!");
}

void  RayPicker::buildDescSet() {
  m_descSetLayoutBinds[0] = {
    .binding = BindingsPicker::ePickerTlas,
    .descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,
    .descriptorCount = 1,
    .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
  };
  m_descSetLayoutBinds[1] = {
    .binding = BindingsPicker::ePickerCamera,
    .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
    .descriptorCount = 1,
    .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
  };
  m_descSetLayoutBinds[2] = {
    .binding = BindingsPicker::ePickerResult,
    .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
    .descriptorCount = 1,
    .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
  };
  VkDescriptorPoolSize        descPoolSize[3] = {
    (VkDescriptorPoolSize){
      .type = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,
      .descriptorCount = 1,
    },
    (VkDescriptorPoolSize){
      .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
      .descriptorCount = 1,
    },
    (VkDescriptorPoolSize){
      .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
      .descriptorCount = 1,
    },
  };
  VkDescriptorPoolCreateInfo  descPoolInfo = {
    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
    .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT | VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
    .maxSets = 3,
    .poolSizeCount = 3,
    .pPoolSizes = descPoolSize,
  };
  if (vkCreateDescriptorPool(m_device, &descPoolInfo, nullptr, &m_descPool) != VK_SUCCESS)
    throw PhosHelper::FatalVulkanInitError("Failed to create RayPicker descriptor pool !");

  VkDescriptorSetLayoutCreateInfo descSetLayoutInfo = {
    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
    .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
    .bindingCount = static_cast<uint32_t>(m_descSetLayoutBinds.size()),
    .pBindings = m_descSetLayoutBinds.data(),
  };

  if (vkCreateDescriptorSetLayout(m_device, &descSetLayoutInfo, nullptr, &m_descSetLayout) != VK_SUCCESS)
    throw PhosHelper::FatalVulkanInitError("Failed to create RayPicker descriptor set layout !");

  VkDescriptorSetAllocateInfo descSetAllocInfo = {
    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
    .descriptorPool = m_descPool,
    .descriptorSetCount = 1,
    .pSetLayouts = &m_descSetLayout,
  };

  if (vkAllocateDescriptorSets(m_device, &descSetAllocInfo, &m_descSet) != VK_SUCCESS)
    throw PhosHelper::FatalVulkanInitError("Failed to Allocate RayPicker descriptor set !");
}
