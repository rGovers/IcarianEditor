#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <string>

#include "AppMain.h"
#include "IcarianEditorConfig.h"
#include "Logger.h"

int main(int a_argc, char* a_argv[])
{
    Logger::Message("IcarianEditor " + std::to_string(ICARIANEDITOR_VERSION_MAJOR) + "." + std::to_string(ICARIANEDITOR_VERSION_MINOR));

    Application* app = new AppMain();
    app->Run();

    delete app;

    return 0;
}