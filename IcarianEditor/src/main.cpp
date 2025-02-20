// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

// Windows headers need to be included first otherwise stuff breaks
#include "Core/WindowsHeaders.h"

#include <ctime>
#include <string>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_GIF
#define STBI_NO_PSD
#define STBI_NO_PIC
#define STBI_NO_PNM
#include <stb_image.h>

#define CUBE_IMPLEMENTATION
#ifndef NDEBUG
#define CUBE_PRINT_COMMANDS
#endif
#include "CUBE/CUBE.h"

#include "AppMain.h"
#include "Core/IcarianDefer.h"
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

#ifdef WIN32
    // Whatever enet needs we will do ourselves
    // We need a newer version and enet does not allow overriding
    WSADATA wsaData;
    if (WSAStartup (MAKEWORD(2, 0), &wsaData))
    {
        return 1;
    }
    timeBeginPeriod(1);

    IDEFER(
    {
        timeEndPeriod(1);
        WSACleanup();
    });
#endif
    
    srand(time(NULL));

    AppMain app;
    app.Run();

    return 0;
}

// MIT License
// 
// Copyright (c) 2025 River Govers
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.