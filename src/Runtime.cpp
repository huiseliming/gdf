#include "Base/Window.h"
#include "Log/Logger.h"
#include <iostream>
#include <string>

namespace Runtime
{
void Initialize()
{
    std::wcout << L"Runtime Initialize\n";
    ::glfwInit();
    Logger::Create();
}

void Cleanup()
{
    Logger::Destroy();
    ::glfwTerminate();
    std::wcout << L"Runtime Cleanup\n";
}
} // namespace Runtime
