#include "Base/Clock.h"
#include "Base/Exception.h"
#include "Base/StringTool.h"
#include "Base/TimeManager.h"
#include "Base/Window.h"
#include "DeveloperTool/DeveloperConsole.h"
#include "Log/Logger.h"
#include "Log/StdSink.h"
#include "Renderer/Graphics.h"
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

DECLARE_LOG_CATEGORY(General, LogLevel::All, LogLevel::Info)
DEFINE_LOG_CATEGORY(General)

int main(int argc, char **argv)
{
    std::chrono::seconds;
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
            tm.Reset();
            tm.dilation(0.01);
            GDF_LOG(General, LogLevel::Info, "Entering main loop at ProgramTime: {}");
            while (!window.ShouldClose()) {
                window.PollEvents();
                // developerConsole.RunCommand("Prinest");
                tm.Update();
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