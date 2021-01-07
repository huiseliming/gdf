#include "Graphics/GraphicsApplication.h"

#include "Log/StdSink.h"
namespace gdf{

GDF_DEFINE_LOG_CATEGORY(General)

GraphicsApplication *pGraphicsApplication;

void GraphicsApplication::StartUp()
{
    pTimerManager_ = std::make_unique<TimeManager>();
    pTimerManager_->Reset();
    pDeveloperConsole_ = std::make_unique<DeveloperConsole>();
    Logger::instance().RegisterSink(&coutSink);
    pWindow_ = std::make_unique<Window>();
    pGraphics_ = std::make_unique<Graphics>();
    pWindow_->Create("test", 800, 600);
    if (!pGraphics_->Initialize())
        THROW_EXCEPT("Graphics initialize failed!");

    // device extensions
    std::vector<const char *> deviceExtensions;
    pGraphics_->CreateSwapchain(*pWindow_);

    pGraphics_->device().CreateLogicalDevice(VkPhysicalDeviceFeatures{}, deviceExtensions, nullptr, pGraphics_->swapchain().surface());

    // Get a graphics queue from the device
    //VkQueue queue;
    //vkGetDeviceQueue(*pDevice_, pDevice_->queueFamilyIndices.graphics, 0, &queue);
    //pGraphicsQueue_ = std::make_unique<GraphicsQueue>(queue);

    pGraphicsApplication = this;
    GDF_LOG(General, LogLevel::Info, "Entering MainLoop at ProgramTime: {}", pTimerManager_->RealCurrentTime());
}

void GraphicsApplication::MainLoop()
{
    while (!pWindow_->ShouldClose()) {
        pWindow_->PollEvents();
        pTimerManager_->Update();
        // if (window.resized()) {
        //    gfx.swapchain().RequestRecreate();
        //}
        // gfx.swapchain().DrawFrame();
    }
}

void GraphicsApplication::Cleanup()
{
    GDF_LOG(General, LogLevel::Info, "Exiting main loop at ProgramTime: {}", pTimerManager_->RealCurrentTime());
    pGraphics_->Cleanup();
    pWindow_->Destroy();
    Logger::instance().DeregisterSink(&coutSink);
}

}



