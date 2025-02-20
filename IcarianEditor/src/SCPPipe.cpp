// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#include "SCPPipe.h"

#include <cassert>

#ifndef WIN32
#include <sys/wait.h>
#include <unistd.h>
#endif

SCPPipe::SCPPipe()
{

}
SCPPipe::~SCPPipe()
{
#ifndef WIN32
    if (IsAlive())
    {
        kill(m_process, SIGTERM);
    }
#endif
}

bool SCPPipe::IsAlive() const
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

SCPPipe* SCPPipe::Create(const std::string_view& a_user, const std::string_view& a_addr, const std::filesystem::path& a_srcPath, const std::filesystem::path& a_dstPath, uint16_t a_port, bool a_compress)
{
    const std::string userAddr = std::string(a_user) + "@" + std::string(a_addr);

#ifndef WIN32
    const pid_t process = fork();

    if (process < 0)
    {
        // Something bad has happen likely cannot recover so just terminate
        exit(1);
    }
    else if (process == 0)
    {
        const std::string src = a_srcPath.generic_string();
        const std::string dst = a_dstPath.generic_string();

        std::string scpStr = "scp" 
            // " -q"
            " -o ControlPath=~/.ssh/IcarianSSHControl:%h:%p:%r"
            " -P " + std::to_string(a_port);

        if (a_compress)
        {
            scpStr += " -C";
        }

        if (std::filesystem::is_directory(a_srcPath))
        {
            scpStr += " -r";
        }

        scpStr += " \"" + src + "\""
            " " + userAddr + ":\"" + dst + "\"";

        execl("/bin/sh", "/bin/sh", "-c", scpStr.c_str(), NULL);

        // Realised that I am over complicating it and can infer that if it returns execl has failed
        printf("Failed to start scp process \n");
        perror("execl");
        assert(0);
    }
    else
    {
        SCPPipe* pipe = new SCPPipe();
        pipe->m_process = process;

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