#include <stdlib.h>

#include <iostream>

#include "Base/Exception.h"
#include "Base/Window.h"
#include "Runtime.h"

int main(int argc, char **argv)
{
    try {
        Runtime::Initialize();
        Window window("test", 800, 600);
        while (!window.ShouldClose()) {
            window.PollEvents();
        }
        if (11) {
            int e23e;
        } else {
            int a;
        }
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
