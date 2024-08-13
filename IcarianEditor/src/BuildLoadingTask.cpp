// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#include "LoadingTasks/BuildLoadingTask.h"

#include <vector>

#include "CUBE/CUBE.h"
#include "Core/IcarianDefer.h"
#include "IO.h"
#include "Logger.h"
#include "MonoProjectGenerator.h"
#include "Project.h"

BuildLoadingTask::BuildLoadingTask(const std::filesystem::path& a_path, const std::string_view& a_platform, Project* a_project)
{
    m_project = a_project;

    m_platform = a_platform;
    m_path = a_path;
}
BuildLoadingTask::~BuildLoadingTask()
{

}

void BuildLoadingTask::Run()
{
    const std::filesystem::path projectPath = m_project->GetPath();
    const std::string projectName = m_project->GetName();

    const std::filesystem::path projectFilesPath = projectPath / "Project";
    const std::filesystem::path cachePath = projectPath / ".cache";
    const std::filesystem::path buildPath = cachePath / "build";

    const std::filesystem::path cwd = std::filesystem::current_path();
    const std::filesystem::path buildFilesPath = cwd / "BuildFiles" / m_platform;

    const std::filesystem::path cscPath = IO::GetCSCPath();

    const std::filesystem::path icarianCSPath = buildFilesPath / "lib" / "IcarianCS.dll";

    std::filesystem::create_directories(buildPath);

    std::vector<std::filesystem::path> projectScripts;
    MonoProjectGenerator::GetScripts(&projectScripts, projectFilesPath, projectFilesPath);

    CUBE_CSProject project = { 0 };
    IDEFER(CUBE_CSProject_Destroy(&project));

    project.Name = CUBE_StackString_CreateC(projectName.c_str());
    project.Target = CUBE_CSProjectTarget_Library;
    project.OutputPath = CUBE_Path_CreateC("build");
    project.Optimise = CBTRUE;

    for (const std::filesystem::path& p : projectScripts)
    {
        const std::filesystem::path absPath = projectFilesPath / p;

        CUBE_CSProject_AppendSource(&project, absPath.string().c_str());
    }

    CUBE_CSProject_AppendReference(&project, icarianCSPath.string().c_str());

    CBUINT32 lineCount = 0;
    CUBE_String* lines = CBNULL;
    
    const bool error = !CUBE_CSProject_Compile(&project, cachePath.string().c_str(), cscPath.string().c_str(), &lines, &lineCount);
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

    // TODO: Down the line add error states for loading tasks to allow for error early outs
    if (error)
    {
        for (CBUINT32 i = 0; i < lineCount; ++i)
        {
            Logger::Error(lines[i].Data);
        }
    }

    const std::filesystem::path finalPath = m_path / "Core" / "Assemblies";

    std::filesystem::create_directories(finalPath);

    for (const auto& iter : std::filesystem::directory_iterator(buildPath))
    {
        const std::filesystem::path filename = iter.path().filename();

        // Do not need the engine dlls as they will be already loaded
        if (filename == "IcarianCS.dll" || filename == "IcarianEditorCS.dll")
        {
            continue;
        }

        std::filesystem::copy_file(iter, finalPath / filename, std::filesystem::copy_options::overwrite_existing);
    }
}

// MIT License
// 
// Copyright (c) 2024 River Govers
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