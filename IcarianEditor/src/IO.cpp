// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#include "IO.h"

#include <cstdint>
#include <cstdlib>

#include "Logger.h"

#ifdef WIN32
#include "Core/WindowsHeaders.h"

#include <shlobj.h>
#endif

std::filesystem::path IO::GetHomePath()
{
#if WIN32
    const std::string homePath = std::getenv("HOMEPATH");
    const std::string homeDrive = std::getenv("HOMEDRIVE");

    return homeDrive + homePath;
#else
    // Yes I am aware of root home but you should not be running this as root in the first place.
    return std::getenv("HOME");
#endif
}
bool IO::ValidatePathName(const std::string_view& a_name)
{
    const uint32_t nameLen = (uint32_t)a_name.length();
    if (nameLen == 0)
    {
        return false;
    }

#if WIN32
    // Windows does not allow the following characters in file names:
    // https://docs.microsoft.com/en-us/windows/win32/fileio/naming-a-file
    for (uint32_t i = 0; i < nameLen; ++i)
    {
        const char c = a_name[i];
        switch (c)
        {
        case '<':
        case '>':
        case ':':
        case '"':
        case '/':
        case '\\':
        case '|':
        case '?':
        case '*':
        // Works but file explorer does not like it.
        case '[':
        case ']':
        {
            return false;
        }
        }
    }
#else
    // Linux does not allow the following characters in file names:
    for (uint32_t i = 0; i < nameLen; ++i)
    {
        const char c = a_name[i];
        switch (c)
        {
        case '/':
        // Should not occur but just in case.
        case 0:
        {
            return false;
        }
        }
    }
#endif 

    return true;
}

std::filesystem::path IO::GetRelativePath(const std::filesystem::path& a_relative, const std::filesystem::path& a_path)
{
    std::filesystem::path tempPath = a_path;
    std::filesystem::path path;

    while (tempPath != a_relative)
    {
        if (!tempPath.has_filename())
        {
            return std::filesystem::path();
        }

        if (path.empty())
        {
            path = tempPath.stem();
            path.replace_extension(tempPath.extension());
        }
        else
        {
            path = tempPath.stem() / path;
        }
        
        tempPath = tempPath.parent_path();
    }

    return path;
}

std::filesystem::path IO::GetCSCPath()
{
#ifdef WIN32
    return std::filesystem::current_path() / "bin" / "csc.bat";
#else
    return std::filesystem::current_path() / "bin" / "csc";
#endif
}

std::vector<std::filesystem::path> IO::GetDrives()
{
    std::vector<std::filesystem::path> drives;

#ifdef WIN32
    const DWORD drivesMask = GetLogicalDrives();
    for (uint32_t i = 0; i < 26; ++i)
    {
        if (drivesMask & (0b1 << i))
        {
            const std::string drive = std::string(1, 'A' + i) + ":\\";

            drives.push_back(drive);
        }
    }
#else
    // Bit tricky to get drives on Linux.
    // I believe I want to read /proc/mounts but I am not sure if that is across all systems cause linux is a pain like that.
    // Cannot be bothered to do it right now as everything mounts under root directory so everything should be accessible.

    drives.push_back("/");
#endif

    return drives;
}
std::vector<std::filesystem::path> IO::GetUserDirectories()
{
    std::vector<std::filesystem::path> specialDirectories;

#ifdef WIN32
    CHAR path[MAX_PATH];

    HRESULT result = SHGetFolderPathA(NULL, CSIDL_DESKTOPDIRECTORY, NULL, 0, path);
    if (result == S_OK)
    {
        specialDirectories.push_back(path);
    }

    result = SHGetFolderPathA(NULL, CSIDL_MYDOCUMENTS, NULL, 0, path);
    if (result == S_OK)
    {
        specialDirectories.push_back(path);
    }

    result = SHGetFolderPathA(NULL, CSIDL_MYPICTURES, NULL, 0, path);
    if (result == S_OK)
    {
        specialDirectories.push_back(path);
    }

    result = SHGetFolderPathA(NULL, CSIDL_MYVIDEO, NULL, 0, path);
    if (result == S_OK)
    {
        specialDirectories.push_back(path);
    }

    result = SHGetFolderPathA(NULL, CSIDL_MYMUSIC, NULL, 0, path);
    if (result == S_OK)
    {
        specialDirectories.push_back(path);
    } 
#else
    // To my knowledge there is no standard way to get special directories on Linux.
    // There are ways that are basically standard but nothing is guaranteed.
    // Some systems do not even have these directories.
    const std::filesystem::path homePath = GetHomePath();

    specialDirectories.push_back(homePath);

    if (std::filesystem::exists(homePath / "Desktop"))
    {
        specialDirectories.push_back(homePath / "Desktop");
    }

    if (std::filesystem::exists(homePath / "Documents"))
    {
        specialDirectories.push_back(homePath / "Documents");
    }

    if (std::filesystem::exists(homePath / "Pictures"))
    {
        specialDirectories.push_back(homePath / "Pictures");
    }

    if (std::filesystem::exists(homePath / "Videos"))
    {
        specialDirectories.push_back(homePath / "Videos");
    }

    if (std::filesystem::exists(homePath / "Music"))
    {
        specialDirectories.push_back(homePath / "Music");
    }

    if (std::filesystem::exists(homePath / "Downloads"))
    {
        specialDirectories.push_back(homePath / "Downloads");
    }

    if (std::filesystem::exists(homePath / "Templates"))
    {
        specialDirectories.push_back(homePath / "Templates");
    }
#endif

    return specialDirectories;
}

void IO::OpenFileExplorer(const std::filesystem::path& a_path)
{
#if WIN32
    // I know I should be using the Win32 API but this is easier.
    // I like when I dont have to write 100 lines of code to do something simple.
    // And to those that say but C++ and Vulkan I say hush.
    const std::string path = a_path.string();

    const std::string command = "explorer.exe " + path;

    system(command.c_str());
#else
    const std::string path = a_path.string();

    // TODO: Need to improve as only works with Linux systems that have xdg
    const std::string command = "xdg-open \"" + path + "\"";

    const int ret = system(command.c_str());
    if (ret != 0)
    {
        Logger::Error("Failed to open file explorer!");
    }
#endif
}
void IO::OpenFile(const std::filesystem::path& a_path)
{
#if WIN32
    const std::string path = a_path.string();

    const std::string command = "start " + path;

    const int ret = system(command.c_str());
    if (ret != 0)
    {
        Logger::Error("Failed to open file!");
    }
#else
    const std::string path = a_path.string();

    const std::string command = "xdg-open \"" + path + "\"";

    const int ret = system(command.c_str());
    if (ret != 0)
    {
        Logger::Error("Failed to open file!");
    }
#endif
}
void IO::OpenFile(const std::string_view& a_application, const std::filesystem::path& a_path)
{
#if WIN32
    const std::string path = a_path.string();

    const std::string command = a_application.data() + std::string(" ") + path;

    const int ret = system(command.c_str());
    if (ret != 0)
    {
        Logger::Error("Failed to open file!");
    }
#else
    const std::string path = a_path.string();

    const std::string command = a_application.data() + std::string(" \"") + path + "\"";

    const int ret = system(command.c_str());
    if (ret != 0)
    {
        Logger::Error("Failed to open file!");
    }
#endif
}
void IO::StartOpenFile(const std::string_view& a_application, const std::filesystem::path& a_path)
{
#if WIN32
    const std::string path = a_path.string();

    const std::string command = std::string("start ") + a_application.data() + " " + path;

    const int ret = system(command.c_str());
    if (ret != 0)
    {
        Logger::Error("Failed to open file!");
    }
#else
    const std::string path = a_path.string();

    const std::string command = a_application.data() + std::string(" \"") + path + "\"";

    const int ret = system(command.c_str());
    if (ret != 0)
    {
        Logger::Error("Failed to open file!");
    }
#endif
}

// MIT License
// 
// Copyright (c) 2024 River Govers
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