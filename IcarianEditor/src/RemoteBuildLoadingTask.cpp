// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#include "LoadingTasks/RemoteBuildLoadingTask.h"

#include <filesystem>

#include "CUBE/CUBE.h"
#include "Core/IcarianDefer.h"
#include "IO.h"
#include "Logger.h"
#include "MonoProjectGenerator.h"
#include "ProcessManager.h"
#include "Project.h"
#include "SSHPipe.h"

RemoteBuildLoadingTask::RemoteBuildLoadingTask(ProcessManager* a_process, Project* a_project) : LoadingTask()
{
    m_process = a_process;
    m_project = a_project;
}
RemoteBuildLoadingTask::~RemoteBuildLoadingTask()
{

}

void RemoteBuildLoadingTask::Run()
{
    const std::filesystem::path cwd = std::filesystem::current_path();

    const SSHPipe* pipe = m_process->GetRemotePipe();

    if (pipe == nullptr || !pipe->IsAlive())
    {
        return;
    }

    std::filesystem::path icarianCSPath;
    switch (pipe->GetHostOS()) 
    {
    case SSHHostOS_WindowsPowerCMD:
    case SSHHostOS_WindowsPowershell:
    {
        icarianCSPath = cwd / "RemoteFiles" / "Windows" / "lib" / "IcarianCS.dll";

        break;
    }
    case SSHHostOS_Linux:
    {
        icarianCSPath = cwd / "RemoteFiles" / "Linux" / "lib" / "IcarianCS.dll";

        break;
    }
    default:
    {
        assert(0);

        break;
    }
    }

    const std::string name = m_project->GetName();
    const std::filesystem::path path = m_project->GetPath();

    const std::string icarianCSPathStr = icarianCSPath.string();

    const std::filesystem::path cscPath = IO::GetCSCPath();
    const std::string cscPathStr = cscPath.string();

    const std::filesystem::path cachePath = path / ".cache";
    const std::string cachePathStr = cachePath.string();
    const std::filesystem::path projectPath = path / "Project";

    CUBE_CSProject project = 
    { 
        .Name = CUBE_StackString_CreateC(name.data()),
        .Target = CUBE_CSProjectTarget_Library,
        .OutputPath = CUBE_Path_CreateC("RemoteCore/Core/Assemblies"),
    };
    IDEFER(CUBE_CSProject_Destroy(&project));

    CUBE_CSProject_AppendReference(&project, icarianCSPathStr.c_str());

    std::vector<std::filesystem::path> projectScripts;
    MonoProjectGenerator::GetScripts(&projectScripts, projectPath, projectPath);

    for (const std::filesystem::path& p : projectScripts)
    {
        const std::filesystem::path absPath = projectPath / p;

        const std::string absStr = absPath.string();
        CUBE_CSProject_AppendSource(&project, absStr.c_str());
    }

    CUBE_String* lines = CBNULL;
    CBUINT32 lineCount = 0;

    const bool error = !CUBE_CSProject_Compile(&project, cachePathStr.c_str(), cscPathStr.c_str(), &lines, &lineCount);

    IDEFER(
    {
        if (lines != CBNULL)
        {
            for (CBUINT32 i = 0; i < lineCount; ++i)
            {
                CUBE_String_Destroy(&lines[i]);
            }

            free(lines);
        }

        lineCount = 0;
    });

    // TODO: Early exit from error states
    if (error)
    {
        for (CBUINT32 i = 0; i < lineCount; ++i)
        {
            Logger::Error(lines[i].Data);
        }
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