#pragma once

#include "LoadingTask.h"

#include <filesystem>

class Project;

class BuildLoadingTask : public LoadingTask
{
private:
    Project*              m_project;

    std::filesystem::path m_path;

protected:

public:
    BuildLoadingTask(const std::filesystem::path& a_path, Project* a_project);
    virtual ~BuildLoadingTask();

    virtual void Run();
};