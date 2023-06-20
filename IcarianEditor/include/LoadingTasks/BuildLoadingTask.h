#pragma once

#include "LoadingTask.h"

#include <filesystem>

class Project;

class BuildLoadingTask : public LoadingTask
{
private:
    Project*              m_project;

    std::string           m_platform;
    std::filesystem::path m_path;

protected:

public:
    BuildLoadingTask(const std::filesystem::path& a_path, const std::string_view& a_platform, Project* a_project);
    virtual ~BuildLoadingTask();

    virtual void Run();
};