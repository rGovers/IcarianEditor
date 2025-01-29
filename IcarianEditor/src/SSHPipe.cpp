// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#include "SSHPipe.h"

#include <cassert>
#include <cstring>
#include <thread>

#ifndef WIN32
#include <csignal>
#include <sys/poll.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

#include "Core/IcarianError.h"
#include "Logger.h"

SSHPipe::SSHPipe()
{
    m_process = -1;

    m_hostOS = SSHHostOS_Unknown;
    m_hostArchitecture = SSHHostArchitecture_Unknown;

    m_readPipe = -1;
    m_errorPipe = -1;
    m_writePipe = -1;
}
SSHPipe::~SSHPipe()
{
    if (IsAlive())
    {
        Send("exit");
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    if (IsAlive())
    {
        kill(m_process, SIGTERM);
    }

    if (m_readPipe >= 0)
    {
        close(m_readPipe);
    }
    if (m_errorPipe >= 0)
    {
        close(m_errorPipe);
    }
    if (m_writePipe >= 0)
    {
        close(m_writePipe);
    }
}

void SSHPipe::FindHostOS()
{
    Flush();

    {
        Send("cmd /c ver");

        const std::vector<std::string> readLines = Read(500);
        for (const std::string& s : readLines)
        {
            if (s.find("not found") != std::string::npos)
            {
                break;
            }

            if (s.find("Windows") != std::string::npos)
            {
                // I cannot find any rhime or reason behind getting dropped in a CMD or PowerShell session and it is getting annoying
                m_hostOS = SSHHostOS_WindowsPowerCMD;

                Flush();

                // Magic string to detect cmd or powershell
                Send("(dir 2>&1 *`|echo CMD);&<# rem #>echo PowerShell");

                const std::vector<std::string> cmdLines = Read(500);
                // Windows has a habbit of printing the command so skip the 1st line as it will contain PowerShell
                for (uint32_t i = 1; i < cmdLines.size(); ++i)
                {
                    const std::string& s = cmdLines[i];

                    if (s.find("PowerShell") != std::string::npos)
                    {
                        m_hostOS = SSHHostOS_WindowsPowershell;

                        break;
                    }
                }

                return;
            }
        }
    }

    Flush();

    {
        Send("uname");

        const std::vector<std::string> readLines = Read(500);
        for (const std::string& s : readLines)
        {
            if (s.find("not found") != std::string::npos)
            {
                break;
            }

            if (s.find("Linux") != std::string::npos)
            {
                m_hostOS = SSHHostOS_Linux;

                return;
            }
        }
    }
}
void SSHPipe::FindHostArchitecture()
{
    Flush();

    switch (m_hostOS) 
    {
    case SSHHostOS_Linux:
    {
        Send("uname -m");

        const std::vector<std::string> readLines = Read(500);
        for (const std::string& s : readLines)
        {
            if (s.find("x86_64") != std::string::npos)
            {
                m_hostArchitecture = SSHHostArchitecture_AMD64;

                return;
            }
        }

        break;
    }
    case SSHHostOS_WindowsPowerCMD:
    case SSHHostOS_WindowsPowershell:
    {
        // In a pseudo terminal so only have 24 lines so need to trim it on the other side hence findstr
        // If we do not the head of the command will get cuttoff
        Send("systeminfo | findstr /C:\"System Type:\"");

        // This command can take a while but cannot seem to find a better way as others seem to rely upon .NET(Fuck you PowerShell)
        // Fuck you Windows and a fucking 5 second timeout WTF
        const std::vector<std::string> readLines = Read(5000);
        for (const std::string& s : readLines)
        {
            if (s.find("System Type:") != std::string::npos)
            {
                if (s.find("x64") != std::string::npos)
                {
                    m_hostArchitecture = SSHHostArchitecture_AMD64;

                    return;
                }
            }
        }

        break;
    }
    default:
    {
        break;
    }
    }
}

int SSHPipe::ExitCode() const
{
#ifndef WIN32
    int status;
    waitpid(m_process, &status, WNOHANG);

    return WEXITSTATUS(status);
#endif

    return 0;
}
bool SSHPipe::IsAlive() const
{
#ifndef WIN32
    if (m_process <= 0)
    {
        return false;
    }

    int status;
    if (waitpid(m_process, &status, WNOHANG) == 0)
    {
        return true;
    }

    return !WIFEXITED(status);
#endif

    return false;
}

void SSHPipe::Flush()
{
#ifndef WIN32
    struct pollfd pollFd
    {
        .fd = m_readPipe,
        .events = POLLIN
    };

    while (true)
    {
        if (poll(&pollFd, 1, 0) <= 0)
        {
            break;
        }

        if ((pollFd.revents & POLLIN) == 0)
        {
            break;
        }

        char buffer[1024];
        const int ret = read(m_readPipe, buffer, sizeof(buffer) - 1);
        if (ret <= 0)
        {
            break;
        }
    }

    while (true)
    {
        if (poll(&pollFd, 1, 0) <= 0)
        {
            break;
        }

        if ((pollFd.revents & POLLIN) == 0)
        {
            break;
        }

        char buffer[1024];
        const int ret = read(m_errorPipe, buffer, sizeof(buffer) - 1);
        if (ret <= 0)
        {
            break;
        }
    }
#endif

    m_readBuffer.clear();
    m_errorBuffer.clear();
}
std::vector<std::string> SSHPipe::Read(uint32_t a_timeout)
{
    const std::chrono::time_point startTime = std::chrono::high_resolution_clock::now();

#ifndef WIN32
    struct pollfd pollFd
    {
        .fd = m_readPipe,
        .events = POLLIN
    };

    while (true)
    {
        if (poll(&pollFd, 1, a_timeout) <= 0)
        {
            break;
        }

        if ((pollFd.revents & POLLIN) == 0)
        {
            break;
        }

        char buffer[1024];
        const int ret = read(m_readPipe, buffer, sizeof(buffer) - 1);
        if (ret > 0)
        {
            m_readBuffer += std::string(buffer, ret);
        }
        else
        {
            break;
        }
    }
#endif

    std::vector<std::string> lines;

    while (true)
    {
        const size_t pos = m_readBuffer.find("\n");
        if (pos == std::string::npos)
        {
            break;
        }

        IDEFER(m_readBuffer.erase(0, pos + 1));

        const std::string s = m_readBuffer.substr(0, pos + 1);
        if (s != "\n" && s != "\r\n")
        {
            lines.push_back(s);
        }
    }

    return lines;
}
std::vector<std::string> SSHPipe::ReadError(uint32_t a_timeout)
{
#ifndef WIN32
    struct pollfd pollFd
    {
        .fd = m_errorPipe,
        .events = POLLIN
    };

    while (true)
    {
        if (poll(&pollFd, 1, (int)a_timeout) <= 0)
        {
            break;
        }

        if ((pollFd.revents & POLLIN) == 0)
        {
            break;
        }

        char buffer[1024];
        const int ret = read(m_errorPipe, buffer, sizeof(buffer) - 1);
        if (ret > 0)
        {
            m_errorBuffer += std::string(buffer, ret);
        }
        else
        {
            break;
        }
    }
#endif

    std::vector<std::string> lines;

    while (true)
    {
        const size_t pos = m_errorBuffer.find("\n");
        if (pos == std::string::npos)
        {
            break;
        }

        const std::string s = m_errorBuffer.substr(0, pos + 1);

        if (s != "\n" && s != "\r\n")
        {
            lines.push_back(s);
        }

        m_errorBuffer.erase(0, pos + 1);
    }

    return lines;
}
bool SSHPipe::Send(const char* a_data)
{
    const char* slider = a_data;
    while (*slider != 0)
    {
        ++slider;
    }

    const uint32_t size = (uint64_t)(slider - a_data);
    if (size > 0)
    {
        uint32_t writtenBytes = 0;

        do
        {
            const int writeB = write(m_writePipe, a_data + writtenBytes, size - writtenBytes);
            if (writeB < 0)
            {
                return false;
            }

            writtenBytes += (uint32_t)writeB;
        }
        while (writtenBytes < size);
    }

    // Have no idea if the SSH client is looking for the carriage return or new line as we just requested any so just send both
    constexpr char TerminateStr[] = "\r\n";
    constexpr uint32_t TerminateStrLen = sizeof(TerminateStr) - 1;

    return write(m_writePipe, TerminateStr, TerminateStrLen) != TerminateStrLen;
}

SSHPipe* SSHPipe::ConnectPassword(const std::string_view& a_connectionAddr, uint16_t a_port, uint16_t a_scpPort)
{
    IERRBLOCK;

#ifndef WIN32
    int readPipes[2];
    if (pipe(readPipes) != 0)
    {
        Logger::Error("Failed to create SSH read pipes");

        return nullptr;
    }

    int writePipes[2];
    if (pipe(writePipes) != 0)
    {
        Logger::Error("Failed to create SSH write pipes");

        return nullptr;
    }

    int errPipes[2];
    if (pipe(errPipes) != 0)
    {
        Logger::Error("Failed to create SSH error pipes");

        return nullptr;
    }

    const pid_t process = fork();
    if (process < 0)
    {
        // This should not happen but if it does something very bad happen
        for (int i = 0; i < 2; ++i)
        {
            close(readPipes[i]);
            close(writePipes[i]);
            close(errPipes[i]);
        }

        // Probably the best thing to do?
        exit(1);

        return nullptr;
    }
    else if (process == 0)
    {
        close(writePipes[1]);
        close(readPipes[0]);
        close(errPipes[0]);

        dup2(writePipes[0], STDIN_FILENO);
        dup2(readPipes[1], STDOUT_FILENO);
        dup2(errPipes[1], STDERR_FILENO);

        // Technically gets leaked as the process is about to be changed but OS should handle it
        // Today I found out argument order does matter and you can get stuck connecting when they are not urgh...
        const std::string connectionStr = "ssh " + 
            std::string(a_connectionAddr) + 
            " -tt" 
            " -v"
            " -o ControlMaster=auto"
            " -o ControlPath=~/.ssh/control:%h:%p:%r"
            " -o NumberOfPasswordPrompts=1"
            " -o PubkeyAuthentication=no"
            " -o PreferredAuthentications=password" + 
            " -p " + std::to_string(a_port) +
            " -R " + std::to_string(a_scpPort) + ":localhost:" + std::to_string(a_port);

        // In a weird state because of fork so do stuff C style
        if (execl("/bin/sh", "/bin/sh", "-c", connectionStr.c_str(), NULL) < 0)
        {
            printf("Failed to start ssh process \n");
            perror("execl");
            assert(0);
        }
    }
    else
    {
        close(writePipes[0]);
        close(readPipes[1]);
        close(errPipes[1]);

        SSHPipe* pipe = new SSHPipe();
        IERRDEFER(delete pipe);

        pipe->m_process = process;
        pipe->m_addr = std::string(a_connectionAddr);
        pipe->m_scpPort = a_scpPort;
        pipe->m_readPipe = readPipes[0];
        pipe->m_errorPipe = errPipes[0];
        pipe->m_writePipe = writePipes[1];

        e_SSHHostOS host = SSHHostOS_Unknown;
        bool auth = false;
        while (pipe->IsAlive() && !auth)
        {
            const std::vector<std::string> errLines = pipe->ReadError(500);
            for (const std::string& e : errLines)
            {
                IERRCHECKRET(e.find("permission denied") == std::string::npos, nullptr);

                if (e.find("Authenticated to") != std::string::npos)
                {
                    auth = true;
                }

                printf("%s \n", e.c_str());
            }

            const std::vector<std::string> readLines = pipe->Read(500);
            for (const std::string& s : readLines)
            {   
                // If we find any of the following we can assume we are authenticated
                // Thank you Ubuntu for screaming what you are the moment you login actually helpful
                // If we find stuff now except for Windows we can skip detecting OS and architecture
                if (s.find("Ubuntu") != std::string::npos)
                {
                    pipe->m_hostOS = SSHHostOS_Linux;

                    auth = true;
                }

                if (s.find("x86_64") != std::string::npos)
                {
                    pipe->m_hostArchitecture = SSHHostArchitecture_AMD64;

                    auth = true;
                }
            }
        }

        IERRCHECKRET(auth, nullptr);
        IERRCHECKRET(pipe->IsAlive(), nullptr);

        if (pipe->m_hostOS == SSHHostOS_Unknown)
        {
            pipe->FindHostOS();
        }

        IERRCHECKRET(pipe->m_hostOS != SSHHostOS_Unknown, nullptr);

        if (pipe->m_hostArchitecture == SSHHostArchitecture_Unknown)
        {
            pipe->FindHostArchitecture();
        }

        IERRCHECKRET(pipe->m_hostArchitecture != SSHHostArchitecture_Unknown, nullptr);

        return pipe;
    }
#endif

    return nullptr;
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