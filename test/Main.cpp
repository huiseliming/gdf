#include "Base/CommandRunner.h"
#include "Base/Exception.h"
#include "Base/StringTool.h"
#include "Base/Window.h"
#include "Log/Logger.h"
#include "Renderer/Graphics.h"
#include "gdf.h"
#include <fmt/core.h>
#include <iostream>
#include <ostream>
#include <stdlib.h>
#include <string>
#include <string_view>
using namespace gdf;

class CerrSink : public LogSink
{
public:
    virtual void Log(const LogCategory &category, const LogLevel level, std::string_view message)
    {
        std::cerr << fmt::format(
            "[{:s}][{:s}] {:s}\n", category.displayName_, std::to_string(level), message);
    }

    virtual void Exception()
    {
        std::cerr << std::flush;
    }

    static CerrSink &Instance(){
        static CerrSink cerrSink;
        return cerrSink;
    }
    static CerrSink *pInstance(){
        return &Instance();
    }

};

DECLARE_LOG_CATEGORY(General, LogLevel::All, LogLevel::Info)
DEFINE_LOG_CATEGORY(General)
int main(int argc, char **argv)
{
    CommandRunner commandRunner;
    //Logger::instance().RegisterSink(CerrSink::pInstance());
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
