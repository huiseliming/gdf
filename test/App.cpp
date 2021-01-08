#include <iostream>
//#include "Graphics/GraphicsApplication.h"

int main(int argc, char **argv)
{
    try {
        //gdf::GraphicsApplication gfxApp;
        //gfxApp.Run();
    } catch (const std::exception &e) {
        std::cout << "Fatal exception: " << e.what() << "\n";
        return EXIT_FAILURE;
    } catch (...) {
        std::cout << "Fatal undefined exception!" << "\n";
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
