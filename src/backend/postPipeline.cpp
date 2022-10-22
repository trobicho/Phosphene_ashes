#include "vkImpl.hpp"

void  VkImpl::createPostDescriptorSet() {
  m_postDescSetLayoutBinds[0] = {
    .binding = 0,
    .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
    .descriptorCount = 1,
    .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
  };

  VkDescriptorPoolSize        descPoolSize[1] = {
    (VkDescriptorPoolSize){
      .type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
      .descriptorCount = 1,
    },
  };
  VkDescriptorPoolCreateInfo  descPoolInfo = {
    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
    .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT | VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
    .maxSets = 1,
    .poolSizeCount = 1,
    .pPoolSizes = descPoolSize,
  };
  if (vkCreateDescriptorPool(m_device, &descPoolInfo, nullptr, &m_postDescPool) != VK_SUCCESS)
    throw PhosHelper::FatalVulkanInitError("Failed to create POST descriptor pool !");

  VkDescriptorSetLayoutCreateInfo descSetLayoutInfo = {
    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
    .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
    .bindingCount = 1,
    .pBindings = m_postDescSetLayoutBinds.data(),
  };

  if (vkCreateDescriptorSetLayout(m_device, &descSetLayoutInfo, nullptr, &m_postDescSetLayout) != VK_SUCCESS)
    throw PhosHelper::FatalVulkanInitError("Failed to create POST descriptor set layout !");

  VkDescriptorSetAllocateInfo descSetAllocInfo = {
    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
    .descriptorPool = m_postDescPool,
    .descriptorSetCount = 1,
    .pSetLayouts = &m_postDescSetLayout,
  };

  if (vkAllocateDescriptorSets(m_device, &descSetAllocInfo, &m_postDescSet) != VK_SUCCESS)
    throw PhosHelper::FatalVulkanInitError("Failed to Allocate POST descriptor set !");
}

void  VkImpl::createPostPipeline() {
  VkPipelineVertexInputStateCreateInfo  vertexInputInfo = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
    //.vertexBindingDescriptionCount = 1,
    //.pVertexBindingDescriptions = &bindingDescription, // Optional
    //.vertexAttributeDescriptionCount = 1,
    //.pVertexAttributeDescriptions = &attributeDescription, // Optional
  };

  VkPipelineInputAssemblyStateCreateInfo  inputAssemblyInfo = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
    .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
    .primitiveRestartEnable = VK_FALSE,
  };

  VkPipelineRasterizationStateCreateInfo rasterizationInfo = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
    .depthClampEnable = VK_FALSE,
    .rasterizerDiscardEnable = VK_FALSE,
    .polygonMode = VK_POLYGON_MODE_FILL,
    .cullMode = VK_CULL_MODE_BACK_BIT,
    .frontFace = VK_FRONT_FACE_CLOCKWISE,
    .depthBiasEnable = VK_FALSE,
    .depthBiasConstantFactor = 0.0f, // Optional
    .depthBiasClamp = 0.0f, // Optional
    .depthBiasSlopeFactor = 0.0f, // Optional
    .lineWidth = 1.0f,
  };

  VkPipelineViewportStateCreateInfo   viewportInfo = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
    .viewportCount = 1,
    .scissorCount = 1,
  };

  VkPipelineMultisampleStateCreateInfo multisampleStateInfo = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
    .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
    .sampleShadingEnable = VK_FALSE,
    .minSampleShading = 1.0f, // Optional
    .pSampleMask = nullptr, // Optional
    .alphaToCoverageEnable = VK_FALSE, // Optional
    .alphaToOneEnable = VK_FALSE, // Optional
  };

  VkPipelineColorBlendAttachmentState colorBlendAttachment = {
    .blendEnable = VK_TRUE,
    .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
    .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
    .colorBlendOp = VK_BLEND_OP_ADD,
    .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
    .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
    .alphaBlendOp = VK_BLEND_OP_ADD,
    .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
  };

  VkPipelineColorBlendStateCreateInfo colorBlendInfo = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
    .logicOpEnable = VK_FALSE,
    .attachmentCount = 1,
    .pAttachments = &colorBlendAttachment,
  };

  /*
  VkPipelineColorBlendAttachmentState colorBlendAttachment = {
    .blendEnable = VK_FALSE,
    .srcColorBlendFactor = VK_BLEND_FACTOR_ONE, //Optional
    .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO, //Optional
    .colorBlendOp = VK_BLEND_OP_ADD, // Optional
    .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE, //Optional
    .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO, //Optional
    .alphaBlendOp = VK_BLEND_OP_ADD, // Optional
    .colorWriteMask = VK_COLOR_COMPONENT_R_BIT
      | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
  };

  VkPipelineColorBlendStateCreateInfo colorBlendInfo = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
    .logicOpEnable = VK_FALSE,
    .logicOp = VK_LOGIC_OP_COPY, // Optional
    .attachmentCount = 1,
    .pAttachments = &colorBlendAttachment,
  };
  */
  colorBlendInfo.blendConstants[0] = 0.0f; // Optional
  colorBlendInfo.blendConstants[1] = 0.0f; // Optional
  colorBlendInfo.blendConstants[2] = 0.0f; // Optional
  colorBlendInfo.blendConstants[3] = 0.0f; // Optional

  VkShaderModule    vertShaderModule =
    PhosHelper::createShaderModuleFromFile(m_device, "./spv/post.vert.spv");
  VkShaderModule    fragShaderModule =
    PhosHelper::createShaderModuleFromFile(m_device, "./spv/post.frag.spv");

  VkDynamicState dynamicStates[] = {
    VK_DYNAMIC_STATE_VIEWPORT,
    VK_DYNAMIC_STATE_SCISSOR,
  };

  VkPipelineDynamicStateCreateInfo  dynamicStateInfo = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
    .dynamicStateCount = 2,
    .pDynamicStates = dynamicStates,
  };

  VkPipelineShaderStageCreateInfo   vertShaderStageInfo = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
    .stage = VK_SHADER_STAGE_VERTEX_BIT,
    .module = vertShaderModule,
    .pName = "main", //TODO: entrypoint
    .pSpecializationInfo = nullptr,
  };
  VkPipelineShaderStageCreateInfo   fragShaderStageInfo = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
    .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
    .module = fragShaderModule,
    .pName = "main", //TODO: entrypoint
    .pSpecializationInfo = nullptr,
  };
  VkPipelineShaderStageCreateInfo   shaderStages[] = {
    vertShaderStageInfo,
    fragShaderStageInfo,
  };

  std::vector<VkDescriptorSetLayout> postDescSetLayout = {m_postDescSetLayout};

  VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
    .setLayoutCount = static_cast<uint32_t>(postDescSetLayout.size()),
    .pSetLayouts = postDescSetLayout.data(),
    .pushConstantRangeCount = 0, // Optional
    .pPushConstantRanges = nullptr, // Optional
  };

  if (vkCreatePipelineLayout(m_device, &pipelineLayoutInfo
        , nullptr, &m_postPipelineLayout) != VK_SUCCESS)
    throw PhosHelper::FatalError("failed to create post pipeline layout!");

  VkGraphicsPipelineCreateInfo  pipelineInfo = {
    .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
    .stageCount = 2,
    .pStages = shaderStages,
    .pVertexInputState = &vertexInputInfo,
    .pInputAssemblyState = &inputAssemblyInfo,
    .pViewportState = &viewportInfo,
    .pRasterizationState = &rasterizationInfo,
    .pMultisampleState = &multisampleStateInfo,
    .pDepthStencilState = nullptr, // Optional
    .pColorBlendState = &colorBlendInfo,
    .pDynamicState = &dynamicStateInfo,
    .layout = m_postPipelineLayout,
    .renderPass = m_renderPass,
  };

  if (vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1,
        &pipelineInfo, nullptr, &m_postPipeline) != VK_SUCCESS)
    throw PhosHelper::FatalError("failed to create graphics pipeline!");

  vkDestroyShaderModule(m_device, vertShaderModule, nullptr);
  vkDestroyShaderModule(m_device, fragShaderModule, nullptr);
}
