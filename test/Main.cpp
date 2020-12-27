#include "Base/Clock.h"
#include "Base/Exception.h"
#include "Base/StringTool.h"
#include "Base/Timer.h"
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
    try {
        gdf::Initialize();
        try {
            DeveloperConsole developerConsole;
            Logger::instance().RegisterSink(&cerrSink);
            Logger::instance().RegisterSink(&coutSink);
            developerConsole.RegisterCommand(
                "PrintTest", [](std::string_view) { GDF_LOG(General, LogLevel::Info, "Test"); });
            Window window;
            Graphics gfx;
            window.Create("test", 800, 600);
            gfx.Initialize();
            GDF_LOG(General, LogLevel::Info, "Entering main loop");
            Timer pc;
            pc.Reset();
            while (!window.ShouldClose()) {
                window.PollEvents();
                // developerConsole.RunCommand("Prinest");
                pc.Update();
            }
            GDF_LOG(General, LogLevel::Info, "Last tick elapsed: {}", pc.Elapsed());
            GDF_LOG(General, LogLevel::Info, "CurrentTime: {}", pc.CurrentTime());
            GDF_LOG(General, LogLevel::Info, "Exiting main loop");
            gfx.Cleanup();
            GDF_LOG(General,
                    LogLevel::Info,
                    "Exiting main loop in {}",
                    ProgramClock::now().time_since_epoch().count());
            Logger::instance().DeregisterSink(&cerrSink);
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