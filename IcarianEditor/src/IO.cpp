#include "IO.h"

#include <cstdint>
#include <cstdlib>

std::string IO::GetHomePath()
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