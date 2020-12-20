#include "Base/Window.h"
#include <iostream>
#include <string>

namespace Runtime
{
void Initialize()
{
    std::wcout << L"Runtime Initialize\n";
    ::glfwInit();
}

void Cleanup()
{
    ::glfwTerminate();
    std::wcout << L"Runtime Cleanup\n";
}
} // namespace Runtime
