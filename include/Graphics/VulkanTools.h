#pragma once
#include "VulkanApi.h"
#include <vector>
#include <string>
namespace gdf
{
namespace GraphicsTools
{
std::string_view PhysicalDeviceTypeString(VkPhysicalDeviceType type);
std::string_view VkResultString(VkResult errorCode);
uint32_t GetQueueFamilyIndex(const std::vector<VkQueueFamilyProperties> &queueFamilyProperties,
                             const VkQueueFlagBits queueFlags);

VkAttachmentReference MakeAttachmentReference(uint32_t attachment, VkImageLayout layout);

// RenderPass 
VkSubpassDescription MakeSubpassDescription(uint32_t colorAttachmentsCount = 0,
                                            VkAttachmentReference *pColorAttachments = nullptr,
                                            VkAttachmentReference *pDepthStencilAttachment = nullptr,
                                            uint32_t inputAttachmentCount = 0,
                                            VkAttachmentReference *pInputAttachments = nullptr,
                                            VkAttachmentReference *pResolveAttachments = nullptr,
                                            uint32_t preserveAttachmentCount = 0,
                                            uint32_t *pPreserveAttachments = nullptr,
                                            VkPipelineBindPoint pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
                                            VkSubpassDescriptionFlags flags = 0);

VkSubpassDependency MakeSubpassDependency(uint32_t srcSubpass = VK_SUBPASS_EXTERNAL,
                                          uint32_t dstSubpass = VK_SUBPASS_EXTERNAL,
                                          VkPipelineStageFlags srcStageMask = 0,
                                          VkPipelineStageFlags dstStageMask = 0,
                                          VkAccessFlags srcAccessMask = 0,
                                          VkAccessFlags dstAccessMask = 0,
                                          VkDependencyFlags dependencyFlags = 0);
// PipelineShaderStage

VkPipelineShaderStageCreateInfo MakePipelineShaderStageCreateInfo(VkShaderModule module,
                                                                  VkShaderStageFlagBits stage,
                                                                  const char *pName = "main",
                                                                  const VkSpecializationInfo *pSpecializationInfo = nullptr,
                                                                  VkPipelineShaderStageCreateFlags flags = 0);

// GraohicsPipeline
VkPipelineVertexInputStateCreateInfo MakePipelineVertexInputStateCreateInfo(
    uint32_t vertexBindingDescriptionCount = 0,
    const VkVertexInputBindingDescription *pVertexBindingDescriptions = nullptr,
    uint32_t vertexAttributeDescriptionCount = 0,
    const VkVertexInputAttributeDescription *pVertexAttributeDescriptions = nullptr);

VkPipelineInputAssemblyStateCreateInfo MakePipelineInputAssemblyStateCreateInfo(VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VkBool32 primitiveRestartEnable = VK_FALSE);

VkPipelineTessellationStateCreateInfo MakePipelineTessellationStateCreateInfo(uint32_t patchControlPoints = 0);

VkPipelineViewportStateCreateInfo MakePipelineViewportStateCreateInfo(uint32_t viewportCount = 0,
                                                                      const VkViewport *pViewports = nullptr,
                                                                      uint32_t scissorCount = 0,
                                                                      const VkRect2D *pScissors = nullptr);

VkPipelineRasterizationStateCreateInfo MakePipelineRasterizationStateCreateInfo(VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL,
                                                                                VkCullModeFlags cullMode = VK_CULL_MODE_BACK_BIT,
                                                                                VkFrontFace frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
                                                                                VkBool32 depthClampEnable = VK_FALSE,                                    
                                                                                VkBool32 rasterizerDiscardEnable = VK_FALSE,
                                                                                float lineWidth = 1.0f,
                                                                                VkBool32 depthBiasEnable = VK_FALSE,
                                                                                float depthBiasConstantFactor = 0,
                                                                                float depthBiasClamp = 0,
                                                                                float depthBiasSlopeFactor = 0);


VkPipelineMultisampleStateCreateInfo MakePipelineMultisampleStateCreateInfo(VkSampleCountFlagBits rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
                                                                            VkBool32 sampleShadingEnable = VK_FALSE,
                                                                            float minSampleShading = 0.f,
                                                                            const VkSampleMask *pSampleMask = nullptr,
                                                                            VkBool32 alphaToCoverageEnable = VK_FALSE,
    VkBool32 alphaToOneEnable = VK_FALSE);
VkPipelineDepthStencilStateCreateInfo MakePipelineDepthStencilStateCreateInfo(VkBool32 depthTestEnable = VK_TRUE,
                                                                                VkBool32 depthWriteEnable = VK_TRUE, 
                                                                                VkCompareOp depthCompareOp = VK_COMPARE_OP_LESS,
                                                                                VkBool32 depthBoundsTestEnable =VK_FALSE,  
                                                                                VkBool32 stencilTestEnable = VK_FALSE,
                                                                                VkStencilOpState front = {},
                                                                                VkStencilOpState back = {},
                                                                                float minDepthBounds = 0.f,
                                                                              float maxDepthBounds = 0.f);

VkPipelineColorBlendAttachmentState MakePipelineColorBlendAttachmentState(
    VkColorComponentFlags colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
    VkBool32 blendEnable = VK_FALSE,
    VkBlendFactor srcColorBlendFactor = VK_BLEND_FACTOR_ZERO,
    VkBlendFactor dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
    VkBlendOp colorBlendOp = VK_BLEND_OP_ADD,
    VkBlendFactor srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
    VkBlendFactor dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
    VkBlendOp alphaBlendOp = VK_BLEND_OP_ADD);

static float blendConstantsDefault[4] = {0.f,0.f,0.f,0.f};
VkPipelineColorBlendStateCreateInfo MakePipelineColorBlendStateCreateInfo(
    VkBool32 logicOpEnable = VK_FALSE,
    VkLogicOp logicOp = VK_LOGIC_OP_COPY,
    uint32_t attachmentCount = 0,
    const VkPipelineColorBlendAttachmentState *pAttachments = nullptr,
    float blendConstants[4] = blendConstantsDefault);


VkPipelineDynamicStateCreateInfo MakePipelineDynamicStateCreateInfo(uint32_t dynamicStatesCount = 0,
                                                                    const VkDynamicState *pDynamicStates = nullptr);


VkPipelineLayoutCreateInfo MakePipelineLayoutCreateInfo(uint32_t setLayoutCount = 0,
                                                        const VkDescriptorSetLayout *pSetLayouts = nullptr,
                                                        uint32_t pushConstantRangeCount = 0,
                                                        const VkPushConstantRange *pPushConstantRanges = nullptr );


VkSubmitInfo MakeSubmitInfo(uint32_t waitSemaphoreCount,
                            const VkSemaphore *pWaitSemaphores,
                            const VkPipelineStageFlags *pWaitDstStageMask,
                            uint32_t commandBufferCount,
                            const VkCommandBuffer *pCommandBuffers,
                            uint32_t signalSemaphoreCount,
                            const VkSemaphore *pSignalSemaphores);

VkPresentInfoKHR MakePresentInfoKHR(uint32_t waitSemaphoreCount,
                                    const VkSemaphore *pWaitSemaphores,
                                    const VkSwapchainKHR *pSwapchains,
                                    const uint32_t *pImageIndices,
                                    uint32_t swapchainCount = 1, 
                                    VkResult *pResults = nullptr);

}; // namespace VulkanTools
} // namespace gdf
