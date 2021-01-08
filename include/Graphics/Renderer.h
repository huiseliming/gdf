#pragma once
#include "Graphics/VulkanApi.h"
#include "Base/Window.h"
#include "Log/Logger.h"
#include <mutex>

#ifdef GDF_DEBUG
#define GDF_ENABLE_VALIDATION_LAYER true
#else
#define GDF_ENABLE_VALIDATION_LAYER false
#endif // GDF_DEBUG

namespace gdf
{

GDF_DECLARE_EXPORT_LOG_CATEGORY(GraphicsLog, LogLevel::Info, LogLevel::All);

struct GDF_EXPORT Graphics : public NonCopyable
{
    void Initialize(bool enableValidationLayer = GDF_ENABLE_VALIDATION_LAYER);
    
    void DrawFrame();

    void Cleanup();

    // Initialize Funtion
    void CreateInstance();
    void CreateDevice();
    void CreateDebugReporter();

    // Cleanup Funtion
    void DestroyDebugReporter();
    void DestroyDevice();
    void DestroyInstance();

    // Tool Funtion
    bool IsPhysicalDeviceSuitable(const VkPhysicalDevice physicalDevice);

    //helpful function
    VkShaderModule CreateShaderModule(const std::vector<char> &code);

    bool GetSupportPresentQueue(VkSurfaceKHR surface, VkQueue &queue);

    void DeviceWaitIdle();
    static VkBool32 DebugReportCallbackEXT(VkDebugReportFlagsEXT flags,
                                           VkDebugReportObjectTypeEXT objectType,
                                           uint64_t object,
                                           size_t location,
                                           int32_t messageCode,
                                           const char *pLayerPrefix,
                                           const char *pMessage,
                                           void *pUserData);

    friend class Swapchain;

    VkInstance instance_{VK_NULL_HANDLE};
    VkDevice device_{VK_NULL_HANDLE};



    VkDebugReportCallbackEXT fpDebugReportCallbackEXT_ = VK_NULL_HANDLE;
    bool enableValidationLayer_;

private:
public:
private:
};
} // namespace gdf



