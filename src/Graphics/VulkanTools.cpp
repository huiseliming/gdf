#include "Graphics/VulkanTools.h"

namespace gdf
{
namespace GraphicsTools
{
// helpful function
std::string_view PhysicalDeviceTypeString(VkPhysicalDeviceType type)
{
    switch (type) {
#define STR(r)                                                                                                                 \
    case VK_PHYSICAL_DEVICE_TYPE_##r:                                                                                          \
        return #r
        STR(OTHER);
        STR(INTEGRATED_GPU);
        STR(DISCRETE_GPU);
        STR(VIRTUAL_GPU);
#undef STR
    default:
        return "UNKNOWN_DEVICE_TYPE";
    }
}


std::string_view VkResultString(VkResult errorCode)
{
    switch (errorCode) {
#define STR(r)                                                                                                                 \
    case VK_##r:                                                                                                               \
        return #r
        STR(SUCCESS);
        STR(NOT_READY);
        STR(TIMEOUT);
        STR(EVENT_SET);
        STR(EVENT_RESET);
        STR(INCOMPLETE);
        STR(ERROR_OUT_OF_HOST_MEMORY);
        STR(ERROR_OUT_OF_DEVICE_MEMORY);
        STR(ERROR_INITIALIZATION_FAILED);
        STR(ERROR_DEVICE_LOST);
        STR(ERROR_MEMORY_MAP_FAILED);
        STR(ERROR_LAYER_NOT_PRESENT);
        STR(ERROR_EXTENSION_NOT_PRESENT);
        STR(ERROR_FEATURE_NOT_PRESENT);
        STR(ERROR_INCOMPATIBLE_DRIVER);
        STR(ERROR_TOO_MANY_OBJECTS);
        STR(ERROR_FORMAT_NOT_SUPPORTED);
        STR(ERROR_FRAGMENTED_POOL);
        STR(ERROR_UNKNOWN);
        STR(ERROR_OUT_OF_POOL_MEMORY);
        STR(ERROR_INVALID_EXTERNAL_HANDLE);
        STR(ERROR_FRAGMENTATION);
        STR(ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS);
        STR(ERROR_SURFACE_LOST_KHR);
        STR(ERROR_NATIVE_WINDOW_IN_USE_KHR);
        STR(SUBOPTIMAL_KHR);
        STR(ERROR_OUT_OF_DATE_KHR);
        STR(ERROR_INCOMPATIBLE_DISPLAY_KHR);
        STR(ERROR_VALIDATION_FAILED_EXT);
        STR(ERROR_INVALID_SHADER_NV);
        STR(ERROR_INCOMPATIBLE_VERSION_KHR);
        STR(ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT);
        STR(ERROR_NOT_PERMITTED_EXT);
        STR(ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT);
        STR(THREAD_IDLE_KHR);
        STR(THREAD_DONE_KHR);
        STR(OPERATION_DEFERRED_KHR);
        STR(OPERATION_NOT_DEFERRED_KHR);
        STR(PIPELINE_COMPILE_REQUIRED_EXT);
#undef STR
    default:
        return "UNKNOWN_ERROR";
    }
}

uint32_t GetQueueFamilyIndex(const std::vector<VkQueueFamilyProperties> &queueFamilyProperties,
                             const VkQueueFlagBits queueFlags)
{
    // Dedicated queue for compute
    // Try to find a queue family index that supports compute but not graphics
    if (queueFlags & VK_QUEUE_COMPUTE_BIT) {
        for (uint32_t i = 0; i < static_cast<uint32_t>(queueFamilyProperties.size()); i++) {
            if ((queueFamilyProperties[i].queueFlags & queueFlags) &&
                ((queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0)) {
                return i;
            }
        }
    }

    // Dedicated queue for transfer
    // Try to find a queue family index that supports transfer but not graphics and compute
    if (queueFlags & VK_QUEUE_TRANSFER_BIT) {
        for (uint32_t i = 0; i < static_cast<uint32_t>(queueFamilyProperties.size()); i++) {
            if ((queueFamilyProperties[i].queueFlags & queueFlags) &&
                ((queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0) &&
                ((queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) == 0)) {
                return i;
            }
        }
    }

    // For other queue types or if no separate compute queue is present, return the first one to support the requested flags
    for (uint32_t i = 0; i < static_cast<uint32_t>(queueFamilyProperties.size()); i++) {
        if (queueFamilyProperties[i].queueFlags & queueFlags) {
            return i;
        }
    }
    return UINT32_MAX;
}

VkAttachmentReference MakeAttachmentReference(uint32_t attachment, VkImageLayout layout)
{
    return VkAttachmentReference{.attachment = attachment, .layout = layout};
}

VkSubpassDescription MakeSubpassDescription(uint32_t colorAttachmentsCount,
                                            VkAttachmentReference *pColorAttachments,
                                            VkAttachmentReference *pDepthStencilAttachment,
                                            uint32_t inputAttachmentCount,
                                            VkAttachmentReference *pInputAttachments,
                                            VkAttachmentReference *pResolveAttachments,
                                            uint32_t preserveAttachmentCount,
                                            uint32_t *pPreserveAttachments,
                                            VkPipelineBindPoint pipelineBindPoint,
                                            VkSubpassDescriptionFlags flags)
{
    return VkSubpassDescription{
        .flags = flags,
        .pipelineBindPoint = pipelineBindPoint,
        .inputAttachmentCount = inputAttachmentCount,
        .pInputAttachments = pInputAttachments,
        .colorAttachmentCount = colorAttachmentsCount,
        .pColorAttachments = pColorAttachments,
        .pResolveAttachments = pResolveAttachments,
        .pDepthStencilAttachment = pDepthStencilAttachment,
        .preserveAttachmentCount = preserveAttachmentCount,
        .pPreserveAttachments = pPreserveAttachments,
    };
}

VkSubpassDependency MakeSubpassDependency(uint32_t srcSubpass, uint32_t dstSubpass, VkPipelineStageFlags srcStageMask,
                                          VkPipelineStageFlags dstStageMask,
                                          VkAccessFlags srcAccessMask,
                                          VkAccessFlags dstAccessMask,
                                          VkDependencyFlags dependencyFlags)
{
    return VkSubpassDependency
    {
        .srcSubpass = srcSubpass, 
        .dstSubpass = dstSubpass, 
        .srcStageMask = srcStageMask, 
        .dstStageMask = dstStageMask,
        .srcAccessMask = srcAccessMask, 
        .dstAccessMask = dstAccessMask, 
        .dependencyFlags = dependencyFlags,
    };
}

VkPipelineShaderStageCreateInfo MakePipelineShaderStageCreateInfo(VkShaderModule module,
                                                                  VkShaderStageFlagBits stage,
                                                                  const char *pName,
                                                                  const VkSpecializationInfo *pSpecializationInfo,
                                                                  VkPipelineShaderStageCreateFlags flags)
{
    return VkPipelineShaderStageCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .flags = flags,
        .stage = stage,
        .module = module,
        .pName = pName,
        .pSpecializationInfo = pSpecializationInfo,
    };
}

VkPipelineVertexInputStateCreateInfo MakePipelineVertexInputStateCreateInfo(
    uint32_t vertexBindingDescriptionCount,
    const VkVertexInputBindingDescription *pVertexBindingDescriptions,
    uint32_t vertexAttributeDescriptionCount,
    const VkVertexInputAttributeDescription *pVertexAttributeDescriptions)
{
    return VkPipelineVertexInputStateCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = vertexBindingDescriptionCount,
        .pVertexBindingDescriptions = pVertexBindingDescriptions,
        .vertexAttributeDescriptionCount = vertexAttributeDescriptionCount,
        .pVertexAttributeDescriptions = pVertexAttributeDescriptions,
    };
}

VkPipelineInputAssemblyStateCreateInfo MakePipelineInputAssemblyStateCreateInfo(VkPrimitiveTopology topology,
                                                                                VkBool32 primitiveRestartEnable)
{
    return VkPipelineInputAssemblyStateCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE,
    };
}

VkPipelineTessellationStateCreateInfo MakePipelineTessellationStateCreateInfo(uint32_t patchControlPoints)
{
    return VkPipelineTessellationStateCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO,
        .patchControlPoints = patchControlPoints,
    };
}

VkPipelineViewportStateCreateInfo MakePipelineViewportStateCreateInfo(uint32_t viewportCount,
                                                                      const VkViewport *pViewports,
                                                                      uint32_t scissorCount,
                                                                      const VkRect2D *pScissors)
{
    return VkPipelineViewportStateCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = viewportCount,
        .pViewports = pViewports,
        .scissorCount = scissorCount,
        .pScissors = pScissors,
    };
}

VkPipelineRasterizationStateCreateInfo MakePipelineRasterizationStateCreateInfo(VkPolygonMode polygonMode,
                                                                                VkCullModeFlags cullMode,
                                                                                VkFrontFace frontFace,
                                                                                VkBool32 depthClampEnable,
                                                                                VkBool32 rasterizerDiscardEnable,
                                                                                float lineWidth,
                                                                                VkBool32 depthBiasEnable,
                                                                                float depthBiasConstantFactor,
                                                                                float depthBiasClamp,
                                                                                float depthBiasSlopeFactor)
{
    return VkPipelineRasterizationStateCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = depthClampEnable,
        .rasterizerDiscardEnable = rasterizerDiscardEnable,
        .polygonMode = polygonMode,
        .cullMode = cullMode,
        .frontFace = frontFace,
        .depthBiasEnable = depthBiasEnable,
        .depthBiasConstantFactor = depthBiasConstantFactor,
        .depthBiasClamp = depthBiasClamp,
        .depthBiasSlopeFactor = depthBiasSlopeFactor,
        .lineWidth = lineWidth,
    };
}

VkPipelineMultisampleStateCreateInfo MakePipelineMultisampleStateCreateInfo(VkSampleCountFlagBits rasterizationSamples,
                                                                            VkBool32 sampleShadingEnable,
                                                                            float minSampleShading,
                                                                            const VkSampleMask *pSampleMask,
                                                                            VkBool32 alphaToCoverageEnable,
                                                                            VkBool32 alphaToOneEnable)
{
    return VkPipelineMultisampleStateCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = rasterizationSamples,
        .sampleShadingEnable = sampleShadingEnable,
        .minSampleShading = minSampleShading,
        .pSampleMask = pSampleMask,
        .alphaToCoverageEnable = alphaToCoverageEnable,
        .alphaToOneEnable = alphaToOneEnable,
    };
}

VkPipelineDepthStencilStateCreateInfo MakePipelineDepthStencilStateCreateInfo(VkBool32 depthTestEnable,
                                                                              VkBool32 depthWriteEnable,
                                                                              VkCompareOp depthCompareOp,
                                                                              VkBool32 depthBoundsTestEnable,
                                                                              VkBool32 stencilTestEnable,
                                                                              VkStencilOpState front,
                                                                              VkStencilOpState back,
                                                                              float minDepthBounds,
                                                                              float maxDepthBounds)
{
    return VkPipelineDepthStencilStateCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .depthTestEnable = depthTestEnable,
        .depthWriteEnable = depthWriteEnable,
        .depthCompareOp = depthCompareOp,
        .depthBoundsTestEnable = depthBoundsTestEnable,
        .stencilTestEnable = stencilTestEnable,
        .front = front,
        .back = back,
        .minDepthBounds = minDepthBounds,
        .maxDepthBounds = maxDepthBounds,
    };
}

VkPipelineColorBlendAttachmentState MakePipelineColorBlendAttachmentState(VkColorComponentFlags colorWriteMask,
                                                                          VkBool32 blendEnable,
                                                                          VkBlendFactor srcColorBlendFactor,
                                                                          VkBlendFactor dstColorBlendFactor,
                                                                          VkBlendOp colorBlendOp,
                                                                          VkBlendFactor srcAlphaBlendFactor,
                                                                          VkBlendFactor dstAlphaBlendFactor,
                                                                          VkBlendOp alphaBlendOp)
{
    return VkPipelineColorBlendAttachmentState{
        .blendEnable = blendEnable,
        .srcColorBlendFactor = srcColorBlendFactor,
        .dstColorBlendFactor = dstColorBlendFactor,
        .colorBlendOp = colorBlendOp,
        .srcAlphaBlendFactor = srcAlphaBlendFactor,
        .dstAlphaBlendFactor = dstAlphaBlendFactor,
        .alphaBlendOp = alphaBlendOp,
        .colorWriteMask = colorWriteMask,
    };
}
//constexpr float blendConstantsDefault[4] = {0.f,0.f,0.f,0.f};
VkPipelineColorBlendStateCreateInfo MakePipelineColorBlendStateCreateInfo(
    VkBool32 logicOpEnable,
    VkLogicOp logicOp,
    uint32_t attachmentCount,
    const VkPipelineColorBlendAttachmentState *pAttachments,
    float blendConstants[4])
{
    return VkPipelineColorBlendStateCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = logicOpEnable,
        .logicOp = logicOp,
        .attachmentCount = attachmentCount,
        .pAttachments = pAttachments,
        .blendConstants = {blendConstants[0], blendConstants[1], blendConstants[2], blendConstants[3]},
    };
}

VkPipelineDynamicStateCreateInfo MakePipelineDynamicStateCreateInfo(uint32_t dynamicStatesCount,
                                                                    const VkDynamicState *pDynamicStates)
{
    return VkPipelineDynamicStateCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = dynamicStatesCount,
        .pDynamicStates = pDynamicStates,
    };
}


VkPipelineLayoutCreateInfo MakePipelineLayoutCreateInfo(uint32_t setLayoutCount,
                                                        const VkDescriptorSetLayout *pSetLayouts,
                                                        uint32_t pushConstantRangeCount,
                                                        const VkPushConstantRange *pPushConstantRanges)
{
    return VkPipelineLayoutCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = setLayoutCount,
        .pSetLayouts = pSetLayouts,
        .pushConstantRangeCount = pushConstantRangeCount,
        .pPushConstantRanges = pPushConstantRanges,
    };
}

VkSubmitInfo MakeSubmitInfo(uint32_t waitSemaphoreCount,
                            const VkSemaphore *pWaitSemaphores,
                            const VkPipelineStageFlags *pWaitDstStageMask,
                            uint32_t commandBufferCount,
                            const VkCommandBuffer *pCommandBuffers,
                            uint32_t signalSemaphoreCount,
                            const VkSemaphore *pSignalSemaphores)
{
    return VkSubmitInfo{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = waitSemaphoreCount,
        .pWaitSemaphores = pWaitSemaphores,
        .pWaitDstStageMask = pWaitDstStageMask,
        .commandBufferCount = commandBufferCount,
        .pCommandBuffers = pCommandBuffers,
        .signalSemaphoreCount = signalSemaphoreCount,
        .pSignalSemaphores = pSignalSemaphores,
    };
}

VkPresentInfoKHR MakePresentInfoKHR(uint32_t waitSemaphoreCount,
                                    const VkSemaphore *pWaitSemaphores,
                                    const VkSwapchainKHR *pSwapchains,
                                    const uint32_t *pImageIndices,
                                    uint32_t swapchainCount,
                                    VkResult *pResults)
{
    return VkPresentInfoKHR{
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = waitSemaphoreCount,
        .pWaitSemaphores = pWaitSemaphores,
        .swapchainCount = swapchainCount,
        .pSwapchains = pSwapchains,
        .pImageIndices = pImageIndices,
        .pResults = pResults,
    };
}


} // namespace VulkanTools
} // namespace gdf
