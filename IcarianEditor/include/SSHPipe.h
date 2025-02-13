// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>
#include <sys/types.h>
#include <vector>

enum e_SSHHostOS : uint8_t
{
    SSHHostOS_Unknown,
    SSHHostOS_Linux,
    SSHHostOS_WindowsPowerCMD,
    SSHHostOS_WindowsPowershell
};
enum e_SSHHostArchitecture : uint8_t
{
    SSHHostArchitecture_Unknown,
    SSHHostArchitecture_AMD64
};

class SSHPipe
{
private:
    std::string           m_user;
    std::string           m_addr;

    std::string           m_readBuffer;
    std::string           m_errorBuffer;

    std::filesystem::path m_tempPath;

#ifndef WIN32
    pid_t                 m_process;
    
    int                   m_readPipe;
    int                   m_errorPipe;
    int                   m_writePipe;
#endif

    e_SSHHostOS           m_hostOS;
    e_SSHHostArchitecture m_hostArchitecture;

    uint16_t              m_sshPort;

    bool                  m_compressed;

    void FindHostOS();
    void FindHostArchitecture();

    int ExitCode() const;

    SSHPipe();
    
protected:

public:
    ~SSHPipe();

    inline e_SSHHostOS GetHostOS() const
    {
        return m_hostOS;
    }
    inline e_SSHHostArchitecture GetHostArchitecture() const
    {
        return m_hostArchitecture;
    }

    inline uint16_t GetSSHPort() const
    {
        return m_sshPort;
    }

    inline bool IsCompressed() const
    {
        return m_compressed;
    }

    inline std::string GetUser() const
    {
        return m_user;
    }
    inline std::string GetAddr() const
    {
        return m_addr;
    }

    inline std::filesystem::path GetTempDirectory() const
    {
        return m_tempPath;
    }

    bool IsAlive() const;

    void Flush();
    std::vector<std::string> Read(uint32_t a_timeout = 0);
    std::vector<std::string> ReadError(uint32_t a_timeout = 0);
    bool Send(const char* a_data);

    // Cannot make guarantees about std::string_view so pointer it is for the password
    // NOTE: This is fucking terrible I am a bumbling buffon do not do this
    static SSHPipe* ConnectPassword(const std::string_view& a_user, const std::string_view& a_addr, uint16_t a_port, bool a_compress);
};

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