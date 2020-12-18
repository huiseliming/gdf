#include "Runtime.h"
#include "Base/Window.h"
#include "Base/Exception.h"

#include <iostream>

int main()
{
    try
    {
        Runtime::Initialize();
        Window window("test", 800, 600);
        while (!window.ShouldClose())
        {
            window.PollEvents();
        }
        
        Runtime::Cleanup();
    }
    catch (const Exception& e)
    {
        std::cout << e.what();
    }
    catch (const std::exception& e)
    {
        std::cout << e.what();
    }
    catch (...)
    {
        std::cout << "Catch Undefined Exception!\n";
    }
}

