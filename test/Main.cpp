#include "Runtime.h"
#include "Base/Window.h"


int main()
{
    Runtime::Initialize();
    Window window("test", 800, 600);
    while (!window.ShouldClose())
    {
        window.PollEvents();
    }
    
    Runtime::Cleanup();
}

