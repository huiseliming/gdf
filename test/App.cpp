#include <iostream>
#include "Base/Application.h"
#include "Graphics/Graphics.h"
#include "Base/TimeManager.h"
#include "DeveloperTool/DeveloperConsole.h"
#include "Log/StdSink.h"
#include "gdf.h"
using namespace gdf;


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
    Logger::instance().RegisterSink(&coutSink);
    gfx_.Initialize();
    window_.Create("test", 800, 600);
}
void GfxApp::MainLoop()
{
    while (!window_.ShouldClose()) {
        window_.PollEvents();
        timerManager_.Update();
        gfx_.DrawFrame();
        // if (window.resized()) {
        //    gfx.swapchain().RequestRecreate();
        // }
        // gfx.swapchain().DrawFrame();
    }
}
void GfxApp::Cleanup()
{
    gfx_.Cleanup();
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
