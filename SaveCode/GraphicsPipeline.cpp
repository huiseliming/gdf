#include "Graphics/GraphicsPipeline.h"

namespace gdf
{

void GraphicsPipeline::Create()
{
    VkPipelineVertexInputStateCreateInfo vertexInputStateCI{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = static_cast<uint32_t>(vertexInputBindingDescriptions.size()),
        .pVertexBindingDescriptions = vertexInputBindingDescriptions.data(),
        .vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexInputAttributeDescriptions.size()),
        .pVertexAttributeDescriptions = vertexInputAttributeDescriptions.data(),
    };
    VkPipelineViewportStateCreateInfo viewportStateCI{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = static_cast<uint32_t>(viewports.size()),
        .pViewports = viewports.data(),
        .scissorCount = static_cast<uint32_t>(scissors.size()),
        .pScissors = scissors.data(),
    };
    colorBlendStateCI.attachmentCount = static_cast<uint32_t>(colorBlendAttachmentStates.size());
    colorBlendStateCI.pAttachments = colorBlendAttachmentStates.data();
    VkPipelineDynamicStateCreateInfo dynamicStateCI{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
        .pDynamicStates = dynamicStates.data(),
    };
    VkPipelineLayoutCreateInfo graphicsPipelineLayoutCI{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size()),
        .pSetLayouts = descriptorSetLayouts.data(),
        .pushConstantRangeCount = static_cast<uint32_t>(pushConstantRanges.size()),
        .pPushConstantRanges = pushConstantRanges.data(),
    };
    VK_ASSERT_SUCCESSED(vkCreatePipelineLayout(device_, &graphicsPipelineLayoutCI, nullptr, &pipelineLayout_));
    VkGraphicsPipelineCreateInfo GraphicsPipelineCI{
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = static_cast<uint32_t>(shaderStageCI.size()),
        .pStages = shaderStageCI.data(),
        .pVertexInputState = &vertexInputStateCI,
        .pInputAssemblyState = &inputAssemblyStateCI,
        .pTessellationState = tessellationStateCI.sType ? &tessellationStateCI : nullptr,
        .pViewportState = &viewportStateCI,
        .pRasterizationState = &rasterizationStateCI,
        .pMultisampleState = &multisampleStateCI,
        .pDepthStencilState = depthStencilStateCI.sType ? &depthStencilStateCI : nullptr,
        .pColorBlendState = &colorBlendStateCI,
        .pDynamicState = &dynamicStateCI,
        .layout = pipelineLayout_,
        .renderPass = renderPass_,
        .subpass = subpass,
        .basePipelineHandle = basePipelineHandle,
        .basePipelineIndex = basePipelineIndex,
    };
    VK_ASSERT_SUCCESSED(
        vkCreateGraphicsPipelines(device_, VK_NULL_HANDLE, 1, &GraphicsPipelineCI, nullptr, &graphicsPipeline_));
}

} // namespace gdf