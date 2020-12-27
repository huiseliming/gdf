#include "Base/Clock.h"
#include "Base/Exception.h"
#include "Base/StringTool.h"
#include "Base/TimeManager.h"
#include "Base/Window.h"
#include "DeveloperTool/DeveloperConsole.h"
#include "Log/Logger.h"
#include "Log/StdSink.h"
#include "Renderer/Graphics.h"
#include "Renderer/Swapchain.h"
#include "gdf.h"
#include <chrono>
#include <fmt/core.h>
#include <iostream>
#include <ostream>
#include <ratio>
#include <stdlib.h>
#include <string>
#include <string_view>
#include <vulkan/vulkan_core.h>
using namespace gdf;

GDF_DECLARE_LOG_CATEGORY(General, LogLevel::All, LogLevel::Info)
GDF_DEFINE_LOG_CATEGORY(General)

int main(int argc, char **argv)
{
    try {
        gdf::Initialize();
        try {
            DeveloperConsole developerConsole;
            Logger::instance().RegisterSink(&coutSink);
            TimeManager tm;
            Window window;
            Graphics gfx;
            developerConsole.RegisterCommand("PrintTest", [](std::string_view) { GDF_LOG(General, LogLevel::Info, "Test"); });
            window.Create("test", 800, 600);
            gfx.Initialize();
            gfx.SetSwapchain(std::make_unique<Swapchain>(window, gfx));
            tm.Reset();
            tm.dilation(0.01);
            GDF_LOG(General, LogLevel::Info, "Entering main loop at ProgramTime: {}", ProgramClock::now().time_since_epoch().count());
            while (!window.ShouldClose()) {
                window.PollEvents();
                tm.Update();
                if (window.resized()) {
                    gfx.pWindowData()->FramebufferResize();
                }
                // GDF_LOG(General, LogLevel::Info, "RealCurrentTime: {}", tm.RealCurrentTime());
                // GDF_LOG(General, LogLevel::Info, "CurrentTime: {}", tm.CurrentTime());
                // GDF_LOG(General, LogLevel::Info, "TimeOffset: {}", (double(tm.RealCurrentTime()) / double(tm.CurrentTime())));
            }
            GDF_LOG(General, LogLevel::Info, "Exiting main loop at ProgramTime: {}", ProgramClock::now().time_since_epoch().count());
            gfx.Cleanup();
            window.Destroy();
            Logger::instance().DeregisterSink(&coutSink);
        } catch (const std::exception &e) {
            GDF_LOG(General, LogLevel::Fatal, "Fatal exception: ", e.what());
        } catch (...) {
            GDF_LOG(General, LogLevel::Fatal, "Fatal undefined exception!");
        }
        gdf::Cleanup();
    } catch (...) {
        std::cout << "gdf Initialize/Cleanup Exception!\n";
    }
    return EXIT_SUCCESS;
}