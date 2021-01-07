#pragma once 
#include "Base/Application.h"
#include "Base/TimeManager.h"
#include "DeveloperTool/DeveloperConsole.h"
#include "Graphics/Graphics.h"
#include "Base/Window.h"

namespace gdf{

GDF_DECLARE_LOG_CATEGORY(General, LogLevel::All, LogLevel::Info)
class GDF_EXPORT GraphicsApplication : public Application
{
public:
	virtual void StartUp() override;
	virtual void MainLoop() override;
	virtual void Cleanup() override;

    DeveloperConsole &developerConsole()
    {
        return *pDeveloperConsole_;
    }
    TimeManager &timerManager()
    {
        return *pTimerManager_;
    }
    Window &window()
    {
        return *pWindow_;
    }
    Graphics &graphics()
    {
        return *pGraphics_;
    }

private:
    std::unique_ptr<DeveloperConsole> pDeveloperConsole_;
    std::unique_ptr<TimeManager> pTimerManager_;
    std::unique_ptr<Window> pWindow_;
    std::unique_ptr<Graphics> pGraphics_;
};

extern GDF_EXPORT GraphicsApplication *pGraphicsApplication;
/* This global variable is use in MainLoop */
DeveloperConsole &FastGetDeveloperConsole()
{
    return pGraphicsApplication->developerConsole();
}

TimeManager &FastGetTimeManager()
{
    return pGraphicsApplication->timerManager();
}

Window &FastGetWindow()
{
    return pGraphicsApplication->window();
}

Graphics &FastGetGraphics()
{
    return pGraphicsApplication->graphics();
}

/* This global variable is use in MainLoop */ 

}
