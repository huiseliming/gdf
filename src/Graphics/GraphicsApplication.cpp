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



