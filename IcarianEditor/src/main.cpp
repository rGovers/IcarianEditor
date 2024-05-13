#include <ctime>
#include <string>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define CUBE_IMPLEMENTATION
#ifndef NDEBUG
#define CUBE_PRINT_COMMANDS
#endif
#include "CUBE/CUBE.h"

#include "AppMain.h"
#include "Logger.h"

#define ICARIANEDITOR_VERSION_STRX(x) #x
#define ICARIANEDITOR_VERSION_STRI(x) ICARIANEDITOR_VERSION_STRX(x)
#define ICARIANEDITOR_VERSION_TAGSTR ICARIANEDITOR_VERSION_STRI(ICARIANEDITOR_VERSION_TAG)
#define ICARIANEDITOR_COMMIT_HASHSTR ICARIANEDITOR_VERSION_STRI(ICARIANEDITOR_COMMIT_HASH)

void PrintVersion()
{
    Logger::Message("IcarianEditor " + std::to_string(ICARIANEDITOR_VERSION_MAJOR) + "." + std::to_string(ICARIANEDITOR_VERSION_MINOR) + "." + std::to_string(ICARIANEDITOR_VERSION_PATCH) + "." + ICARIANEDITOR_COMMIT_HASHSTR + " " + ICARIANEDITOR_VERSION_TAGSTR);
}

int main(int a_argc, char* a_argv[])
{
    PrintVersion();

    srand(time(NULL));

    AppMain app;
    app.Run();

    return 0;
}