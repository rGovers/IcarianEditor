#include "SCPConnection.h"

#ifndef WIN32
#include <unistd.h>
#endif

SCPConnection::SCPConnection()
{

}
SCPConnection::~SCPConnection()
{

}

SCPConnection* SCPConnection::Create(const std::filesystem::path& a_srcPath, const std::filesystem::path& a_dstPath, uint16_t a_port)
{
#ifndef WIN32
    const pid_t process = fork();
#endif

    return nullptr;
}