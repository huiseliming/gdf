#include "Base/Exception.h"
#include "Base/StringTool.h"
#include "Base/Window.h"
#include "Log/Logger.h"
#include "Renderer/Graphics.h"
#include "Runtime.h"
#include <iostream>
#include <stdlib.h>

using namespace gdf;

LOG_DEFINE_CATEGORY(General, LogLevel::All, LogLevel::Info)

int main(int argc, char **argv)
{
    try {
        gdf::Initialize();
        {
            LOG(General,
                LogLevel::Info,
                "Version   : {}",
                String::ConvertString(gdf::GitVerison()));
            LOG(General,
                LogLevel::Info,
                "HeadSHA1  : {}",
                String::ConvertString(gdf::GitHeadSHA1()));
            LOG(General,
                LogLevel::Info,
                "CommitDate: {}",
                String::ConvertString(gdf::GitCommitDate()));
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
        }
        gdf::Cleanup();
    } catch (const Exception &e) {
        std::cout << e.what();
    } catch (const std::exception &e) {
        std::cout << e.what();
    } catch (...) {
        std::cout << "Catch Undefined Exception!\n";
    }
    return EXIT_SUCCESS;
}
