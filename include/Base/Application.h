#pragma once 
#include "Base/Common.h"

namespace gdf
{

class GDF_EXPORT Application
{
public:
    Application() = default;
    virtual ~Application() = default;
private:
    Application(const Application&) =delete;
    Application & operator=(const Application&) =delete;
    Application(Application&&) =delete;
    Application &operator=(Application&&) =delete;
public:
    virtual void Run();
    virtual void StartUp() = 0;
    virtual void MainLoop() = 0;
    virtual void Cleanup() = 0;
};

}
