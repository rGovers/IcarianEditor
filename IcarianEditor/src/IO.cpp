#include "IO.h"

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