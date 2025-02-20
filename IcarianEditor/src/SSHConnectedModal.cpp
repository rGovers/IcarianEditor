// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#include "Modals/SSHConnectedModal.h"

#include <filesystem>
#include <imgui.h>

#include "Core/IcarianAssert.h"
#include "Core/IcarianError.h"
#include "SCPPipe.h"
#include "SSHPipe.h"

static std::string FormatWindowsStr(const std::string_view& a_str)
{
    const size_t endPos = a_str.find_last_of('/');

    if (endPos == std::string::npos)
    {
        return std::string();
    }

    // Get and reformat the string to a valid path format because Windows is weird
    std::string str = std::string(a_str.substr(0, endPos));
    while (true) 
    {
        constexpr char SlashStr[] = "\\";
        constexpr uint32_t SlashStrLen = sizeof(SlashStr) - 1;

        const size_t pos = str.find(SlashStr);
        if (pos == std::string::npos)
        {
            break;
        }

        str.replace(pos, SlashStrLen, "/");
    }

    return str;
}

SSHConnectedModal::SSHConnectedModal(SSHPipe* a_pipe) : Modal("Syncing SSH")
{
    IERRBLOCK;

    m_scpPipe = nullptr;

    IERRDEFER(
    if (m_scpPipe != nullptr)
    {
        delete m_scpPipe;
        m_scpPipe = nullptr;
    });

    IERRCHECK(a_pipe != nullptr);
    IERRCHECK(a_pipe->IsAlive());

    const bool compress = a_pipe->IsCompressed();
    const uint16_t port = a_pipe->GetSSHPort();
    const std::string user = a_pipe->GetUser();
    const std::string addr = a_pipe->GetAddr();

    const std::filesystem::path tmpPath = a_pipe->GetTempDirectory();
    const std::filesystem::path remotePath = tmpPath / "IcarianRemote";

    const std::filesystem::path cwd = std::filesystem::current_path();

    std::filesystem::path srcPath;

    switch (a_pipe->GetHostOS()) 
    {
    case SSHHostOS_WindowsPowerCMD:
    case SSHHostOS_WindowsPowershell:
    {
        srcPath = cwd / "RemoteFiles" / "Windows" / "bin" / ".";

        break;
    }
    case SSHHostOS_Linux:
    {
        srcPath = cwd / "RemoteFiles" / "Linux" / "bin" / ".";

        break;
    }
    default:
    {
        ICARIAN_ASSERT(0);

        break;
    }
    }

    m_scpPipe = SCPPipe::Create(user, addr, srcPath, remotePath, port, compress);
    IERRCHECK(m_scpPipe != nullptr);
    IERRCHECK(m_scpPipe->IsAlive());
}
SSHConnectedModal::~SSHConnectedModal()
{
    if (m_scpPipe != nullptr)
    {
        delete m_scpPipe;
    }   
}

bool SSHConnectedModal::Update()
{
    if (m_scpPipe == nullptr || !m_scpPipe->IsAlive())
    {
        return false;
    }

    ImGui::Text("Syncing SSH Files");

    return true;
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