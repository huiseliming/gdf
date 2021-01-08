#pragma once
#include "Graphics/VulkanApi.h"
#include <vector>

struct GraphicsRenderPass {


	void Create()
    {
        VkRenderPassCreateInfo renderPassCI{
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,

        };
    }

    VkRenderPassCreateFlags flags{};
    std::vector<VkAttachmentDescription> attachments;
    std::vector<SubpassDescriptionHelper> subpasseDescriptionHelpers;
    std::vector<VkSubpassDescription> subpasses;
    std::vector<VkSubpassDependency> dependencies;
    VkRenderPass renderPass_{VK_NULL_HANDLE};
};
//VkStructureType sType;
//const void *pNext;
//VkRenderPassCreateFlags flags;
//uint32_t attachmentCount;
//const VkAttachmentDescription *pAttachments;
//uint32_t subpassCount;
//const VkSubpassDescription *pSubpasses;
//uint32_t dependencyCount;
//const VkSubpassDependency *pDependencies;



