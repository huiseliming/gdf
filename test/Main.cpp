#include "Base/CommandRunner.h"
#include "Base/Exception.h"
#include "Base/StringTool.h"
#include "Base/Window.h"
#include "Log/Logger.h"
#include "Log/StdSink.h"
#include "Renderer/Graphics.h"
#include "gdf.h"
#include <fmt/core.h>
#include <iostream>
#include <ostream>
#include <stdlib.h>
#include <string>
#include <string_view>
using namespace gdf;

DECLARE_LOG_CATEGORY(General, LogLevel::All, LogLevel::Info)
DEFINE_LOG_CATEGORY(General)

int main(int argc, char **argv)
{
    try {
        gdf::Initialize();
        try {
            CommandRunner commandRunner;
            Logger::instance().RegisterSink(&cerrSink);
            Logger::instance().RegisterSink(&coutSink);
            Window window;
            Graphics gfx;
            window.Create("test", 800, 600);
            gfx.Initialize();
            GDF_LOG(General, LogLevel::Info, "Entering main loop");
            while (!window.ShouldClose()) {
                window.PollEvents();
                GDF_LOG(General, LogLevel::Info, "test");
            }
            GDF_LOG(General, LogLevel::Info, "Exiting main loop");
            gfx.Cleanup();
        } catch (const std::exception &e) {
            GDF_LOG(General, LogLevel::Fatal, "Fatal exception: ", e.what());
        } catch (...) {
            GDF_LOG(General, LogLevel::Fatal, "Fatal undefined exception!");
        }
        Logger::instance().DeregisterSink(&cerrSink);
        Logger::instance().DeregisterSink(&coutSink);
        gdf::Cleanup();
    } catch (...) {
        std::cout << "gdf Initialize/Cleanup Exception!\n";
    }
    return EXIT_SUCCESS;
}
