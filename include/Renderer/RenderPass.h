#pragma once
#include "Renderer/RenderPass.h"
#include "VulkanObject.h"
#include <vector>

namespace gdf
{

struct SubpassDescriptionHelper {
    VkSubpassDescriptionFlags flags{};
    VkPipelineBindPoint pipelineBindPoint{};
    std::vector<VkAttachmentReference> inputAttachments;
    std::vector<VkAttachmentReference> colorAttachments;
    VkAttachmentReference resolveAttachments{UINT32_MAX, VK_IMAGE_LAYOUT_UNDEFINED};
    VkAttachmentReference depthStencilAttachment{UINT32_MAX, VK_IMAGE_LAYOUT_UNDEFINED};
    std::vector<uint32_t> preserveAttachments;

    VkSubpassDescription Get()
    {
        return VkSubpassDescription{
            .flags = flags,
            .pipelineBindPoint = pipelineBindPoint,
            .inputAttachmentCount = static_cast<uint32_t>(inputAttachments.size()),
            .pInputAttachments = inputAttachments.data(),
            .colorAttachmentCount = static_cast<uint32_t>(colorAttachments.size()),
            .pColorAttachments = colorAttachments.data(),
            .pResolveAttachments = (resolveAttachments.attachment != UINT32_MAX ? &resolveAttachments : nullptr),
            .pDepthStencilAttachment = (depthStencilAttachment.attachment != UINT32_MAX ? &depthStencilAttachment : nullptr),
            .preserveAttachmentCount = static_cast<uint32_t>(preserveAttachments.size()),
            .pPreserveAttachments = preserveAttachments.data(),
        };
    }
};

class RenderPass : public VulkanObject
{
public:
    RenderPass(VkDevice device) : VulkanObject(device)
    {
    }

    RenderPass(RenderPass &&rhs) : VulkanObject(std::forward<RenderPass>(rhs))
    {
        flags = rhs.flags;
        rhs.flags = 0;
        attachments = std::move(rhs.attachments);
        subpasses = std::move(rhs.subpasses);
        dependencies = std::move(rhs.dependencies);
        renderPass_ = rhs.renderPass_;
        rhs.renderPass_ = VK_NULL_HANDLE;
    }
    RenderPass &operator=(RenderPass &&rhs)
    {
        if (std::addressof(rhs) != this)
            return *this;
        flags = rhs.flags;
        rhs.flags = 0;
        attachments = std::move(rhs.attachments);
        subpasses = std::move(rhs.subpasses);
        dependencies = std::move(rhs.dependencies);
        renderPass_ = rhs.renderPass_;
        rhs.renderPass_ = VK_NULL_HANDLE;
        VulkanObject::operator=(std::forward<RenderPass>(rhs));
        return *this;
    }

    ~RenderPass()
    {
        if (renderPass_ != VK_NULL_HANDLE) {
            vkDestroyRenderPass(device_, renderPass_, nullptr);
            renderPass_ = VK_NULL_HANDLE;
        }
    }
    void Reset()
    {
        flags = {};
        attachments.clear();
        subpasseDescriptionHelpers.clear();
        subpasses.clear();
        dependencies.clear();
        Desstroy();
    }

    void Create()
    {
        subpasses.clear();
        for (auto &subpasseDescriptionHelper : subpasseDescriptionHelpers)
            subpasses.push_back(subpasseDescriptionHelper.Get());
        VkRenderPassCreateInfo renderPassCI_{
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            .attachmentCount = static_cast<uint32_t>(attachments.size()),
            .pAttachments = attachments.data(),
            .subpassCount = static_cast<uint32_t>(subpasses.size()),
            .pSubpasses = subpasses.data(),
            .dependencyCount = static_cast<uint32_t>(dependencies.size()),
            .pDependencies = dependencies.data(),
        };
        VK_ASSERT_SUCCESSED(vkCreateRenderPass(device_, &renderPassCI_, nullptr, &renderPass_));
    }

    void Desstroy()
    {
        if (renderPass_ != VK_NULL_HANDLE) {
            vkDestroyRenderPass(device_, renderPass_, nullptr);
            renderPass_ = VK_NULL_HANDLE;
        }
    }

    VkRenderPass Get()
    {
        if (renderPass_ == VK_NULL_HANDLE) {
            Create();
        }
        return renderPass_;
    }

    void SetRenderPassCreateFlags(VkRenderPassCreateFlags flags)
    {
        this->flags = flags;
    }

    void AddAttachmentDescription(VkAttachmentDescription attachmentDescription)
    {
        attachments.push_back(attachmentDescription);
    }

    void AddSubpassDescriptionHelper(SubpassDescriptionHelper subpassDescriptionHelper)
    {
        subpasseDescriptionHelpers.emplace_back(subpassDescriptionHelper);
    }

    void AddSubpassDependency(VkSubpassDependency subpassDependency)
    {
        dependencies.push_back(subpassDependency);
    }

private:
    VkRenderPassCreateFlags flags{};
    std::vector<VkAttachmentDescription> attachments;
    std::vector<SubpassDescriptionHelper> subpasseDescriptionHelpers;
    std::vector<VkSubpassDescription> subpasses;
    std::vector<VkSubpassDependency> dependencies;
    VkRenderPass renderPass_{VK_NULL_HANDLE};
};

} // namespace gdf
