#include "Base/Application.h"



namespace gdf{

void Application::Run()
{
    StartUp();
    MainLoop();
    Cleanup();
}

}