#include <string>
#include <iostream>
#include "Base/Window.h"
namespace Runtime
{
    void Initialize()
    {
        std::wcout << L"Runtime Initialize\n";
        Window::glfwInit();
    }

    void Cleanup()
    {
        Window::glfwTerminate();
        std::wcout << L"Runtime Cleanup\n";
    }
}

