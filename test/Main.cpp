#include "Base/Exception.h"
#include "Base/StringTool.h"
#include "Base/Window.h"
#include "Log/Logger.h"
#include "Renderer/Graphics.h"
#include "gdf.h"
#include <iostream>
#include <stdlib.h>

using namespace gdf;

DECLARE_LOG_CATEGORY(General, LogLevel::All, LogLevel::Info)
DEFINE_LOG_CATEGORY(General)

int main(int argc, char **argv)
{
    try {
        gdf::Initialize();
        try {
            Window window;
            Graphics gfx;
            window.Create("test", 800, 600);
            gfx.Initialize();
            LOG(General, LogLevel::Info, "Entering main loop");
            while (!window.ShouldClose()) {
                window.PollEvents();
            }
            LOG(General, LogLevel::Info, "Exiting main loop");
            gfx.Cleanup();
        } catch (const std::exception &e) {
            LOG(General, LogLevel::Fatal, "Fatal exception: ", e.what());
        } catch (...) {
            LOG(General, LogLevel::Fatal, "Fatal undefined exception!");
        }
        gdf::Cleanup();
    } catch (...) {
        std::cout << "gdf Initialize/Cleanup Exception!\n";
    }
    return EXIT_SUCCESS;
}
