#include "Base/Exception.h"
#include "Base/Window.h"
#include "Renderer/Graphics.h"
#include "Runtime.h"
#include <iostream>
#include <stdlib.h>

int main(int argc, char **argv)
{
    try {
        Runtime::Initialize();
        Window window;
        Graphics gfx;
        window.Create("test", 800, 600);
        gfx.Initialize();
        while (!window.ShouldClose()) {
            window.PollEvents();
        }
        gfx.Cleanup();
        Runtime::Cleanup();
    } catch (const Exception &e) {
        std::cout << e.what();
    } catch (const std::exception &e) {
        std::cout << e.what();
    } catch (...) {
        std::cout << "Catch Undefined Exception!\n";
    }
    return EXIT_SUCCESS;
}
