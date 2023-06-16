#include "LoadingTasks/BuildLoadingTask.h"

#include <vector>

#include "ConsoleCommand.h"
#include "MonoProjectGenerator.h"
#include "Project.h"

BuildLoadingTask::BuildLoadingTask(const std::filesystem::path& a_path, Project* a_project)
{
    m_project = a_project;

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

    std::filesystem::create_directories(buildPath);

    const std::filesystem::path projectFile = cachePath / (projectName + "ReleaseBuild.csproj");

    std::vector<std::filesystem::path> projectScripts;
    MonoProjectGenerator::GetScripts(&projectScripts, projectFilesPath, projectFilesPath);

    const std::string projectDependencies[] =
    {
        "System",
        "System.Xml",
        "IcarianEngine"
    };

    // TODO: Need to change this down the line to use the pre built engine assemblies as dependencies but for now this will do
    const MonoProjectGenerator project = MonoProjectGenerator(projectScripts.data(), (uint32_t)projectScripts.size(), projectDependencies, sizeof(projectDependencies) / sizeof(*projectDependencies));
    project.Serialize(projectName, projectFile, std::filesystem::path("build") / "Assemblies");

    ConsoleCommand cmd = ConsoleCommand("xbuild");

    const std::string cmdArgs[] =
    {
        projectFile.string(),
        "/p:Configuration=Release",
    };

    cmd.Run(cmdArgs, sizeof(cmdArgs) / sizeof(*cmdArgs));

    const std::filesystem::path finalPath = m_path / "Core" / "Assemblies";
    const std::filesystem::path outPath = buildPath / "Assemblies";

    std::filesystem::create_directories(finalPath);

    for (const auto& iter : std::filesystem::directory_iterator(outPath))
    {
        const std::filesystem::path filename = iter.path().filename();

        if (filename == "IcarianCS.dll" || filename == "IcarianEditorCS.dll")
        {
            continue;
        }

        std::filesystem::copy_file(iter, finalPath / filename, std::filesystem::copy_options::overwrite_existing);
    }
}