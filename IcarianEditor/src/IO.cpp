#include "IO.h"

#include <cstdint>
#include <cstdlib>

#include "Logger.h"

std::filesystem::path IO::GetHomePath()
{
#if WIN32
    const std::string homePath = std::getenv("HOMEPATH");
    const std::string homeDrive = std::getenv("HOMEDRIVE");

    return homeDrive + homePath;
#else
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