// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#include "LoadingTasks/SyncRemoteBuildLoadingTask.h"

#include <thread>

#include "Core/IcarianAssert.h"
#include "Project.h"
#include "SCPPipe.h"
#include "SSHPipe.h"

SyncRemoteBuildLoadingTask::SyncRemoteBuildLoadingTask(SSHPipe* a_sshPipe, Project* a_project)
{
    m_scpPipe = nullptr;

    if (a_sshPipe == nullptr)
    {
        return;
    }

    if (!a_sshPipe->IsAlive())
    {
        return;
    }

    const std::string user = a_sshPipe->GetUser();
    const std::string addr = a_sshPipe->GetAddr();
    const uint16_t port = a_sshPipe->GetSSHPort();
    const bool compress = a_sshPipe->IsCompressed();

    const std::filesystem::path tmpPath = a_sshPipe->GetTempDirectory();
    const std::filesystem::path remotePath = tmpPath / "IcarianRemote";
    const std::filesystem::path remoteCore = remotePath / "Core";

    const e_SSHHostOS hostOS = a_sshPipe->GetHostOS();
    switch (hostOS) 
    {
    case SSHHostOS_WindowsPowerCMD:
    {
        const std::string cmd = "rd /s /q \"" + remoteCore.generic_string() + "\"";

        a_sshPipe->Send(cmd.c_str());

        break;
    }
    case SSHHostOS_WindowsPowershell:
    {
        const std::string cmd = "rd -r \"" + remoteCore.generic_string() + "\"";

        a_sshPipe->Send(cmd.c_str()); 

        break;
    }
    case SSHHostOS_Linux:
    {
        const std::string cmd = "rm -rf \"" + remoteCore.generic_string() + "\"";

        a_sshPipe->Send(cmd.c_str());

        break;
    }
    default:
    {
        ICARIAN_ASSERT(0);

        break;
    }
    }

    const std::filesystem::path cachePath = a_project->GetCachePath();
    const std::filesystem::path corePath = cachePath / "RemoteCore" / ".";

    m_scpPipe = SCPPipe::Create(user, addr, corePath, remotePath, port, compress);
}
SyncRemoteBuildLoadingTask::~SyncRemoteBuildLoadingTask()
{
    if (m_scpPipe != nullptr)
    {
        delete m_scpPipe;
    }
}

void SyncRemoteBuildLoadingTask::Run()
{
    while (m_scpPipe != nullptr && m_scpPipe->IsAlive())
    {
        std::this_thread::yield();
    }
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