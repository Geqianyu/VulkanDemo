#include "Application.h"

int main()
{
    try
    {
        Application app(800, 600, "Vulkan Demo");
        app.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}