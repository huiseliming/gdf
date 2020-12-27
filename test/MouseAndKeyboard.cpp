#include "Base/Exception.h"
#include "Base/Window.h"
#include "gdf.h"
#include <iostream>
#include <stdlib.h>

using namespace gdf;

int main(int argc, char **argv)
{
    try {
        gdf::Initialize();
        Window window;
        window.Create("test", 800, 600);
        window.mouse().mouseEntredCallbacks_.push_back(
            [](const Window &window, const Mouse &mouse) { std::cout << "MouseEntred\n"; });
        window.mouse().mouseExitedCallbacks_.push_back(
            [](const Window &window, const Mouse &mouse) { std::cout << "MouseExited\n"; });
        while (!window.ShouldClose()) {
            window.PollEvents();
            // std::cout << "MousePostion:" << window.mouse().position.x << "," <<
            // window.mouse().position.y << "\n";
            if (window.mouse().offset.x != 0 || window.mouse().offset.y != 0) {
                std::cout << "MouseOffset:" << window.mouse().offset.x << "," << window.mouse().offset.y << "\n";
            }

            for (auto pressedButton : window.mouse().pressedButtons)
                std::cout << "MousePressedButton:" << pressedButton << "\n";
            for (auto releasedButton : window.mouse().releasedButtons)
                std::cout << "MouseReleasedButton:" << releasedButton << "\n";
            for (auto heldButton : window.mouse().heldButtons)
                std::cout << "MouseHeldButton:" << heldButton << "\n";

            if (window.keyboard().unicodePoint != 0)
                std::cout << window.keyboard().unicodePoint << "\n";
            if (window.keyboard().repeatKey != 0)
                std::cout << "MouseRepeatKey:" << window.keyboard().repeatKey << "\n";

            for (auto pressedKey : window.keyboard().pressedKeys)
                std::cout << "MousePressedKey:" << pressedKey << "\n";
            for (auto releasedKey : window.keyboard().releasedKeys)
                std::cout << "MouseReleasedKey:" << releasedKey << "\n";
            for (auto heldKey : window.keyboard().heldKeys)
                std::cout << "MouseHeldKey:" << heldKey << "\n";
        }
        gdf::Cleanup();
    } catch (const Exception &e) {
        std::cout << e.what();
    } catch (const std::exception &e) {
        std::cout << e.what();
    } catch (...) {
        std::cout << "Catch Undefined Exception!\n";
    }
    return EXIT_SUCCESS;
}
