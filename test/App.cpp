#include <iostream>
#include "Base/Application.h"
#include "Graphics/Graphics.h"
#include "Base/TimeManager.h"
#include "DeveloperTool/DeveloperConsole.h"
#include "Log/StdSink.h"
#include "Base/Clock.h"

#include "gdf.h"
using namespace gdf;

GDF_DECLARE_LOG_CATEGORY(GfxAppLog, LogLevel::All, LogLevel::Info)
GDF_DEFINE_LOG_CATEGORY(GfxAppLog)

class GfxApp : public Application
{
public:
    GfxApp() = default;
    virtual void StartUp() override;
    virtual void MainLoop() override;
    virtual void Cleanup() override;

private:
    Window window_;
    Graphics gfx_;
    TimeManager timerManager_;
    DeveloperConsole developerConsole;
};

void GfxApp::StartUp()
{
    gdf::Initialize();
    timerManager_.Reset();
    window_.Create("test", 800, 600);
    gfx_.Initialize(&window_);
    GDF_LOG(GfxAppLog, LogLevel::Info, "Entering MainLoop at Time: {}", ProgramClock::CurrentTime());
}
void GfxApp::MainLoop()
{
    while (!window_.ShouldClose()) {
        window_.PollEvents();
        if (window_.framebufferResized()) {
            gfx_.RequireRecreateSwapchain(true);
        }
        timerManager_.Update();
        gfx_.FrameBegin();
        gfx_.DrawFrame();
        gfx_.FrameEnd();
        // if (window.resized()) {
        //    gfx.swapchain().RequestRecreate();
        // }
        // gfx.swapchain().DrawFrame();
    }
}
void GfxApp::Cleanup()
{
    GDF_LOG(GfxAppLog, LogLevel::Info, "Exiting MainLoop at Time: {}", ProgramClock::CurrentTime());
    gfx_.Cleanup();
    window_.Destroy();
    gdf::Cleanup();
}

//App *pApp;

int main(int argc, char **argv)
{
    try {
        GfxApp app;
        //pApp = &app;
        app.Run();
    } catch (const std::exception &e) {
        std::cout << "Fatal exception: " << e.what() << "\n";
        return EXIT_FAILURE;
    } catch (...) {
        std::cout << "Fatal undefined exception!" << "\n";
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
