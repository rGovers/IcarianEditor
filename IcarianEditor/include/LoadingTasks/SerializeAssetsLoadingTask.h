#pragma once

#include "LoadingTasks/LoadingTask.h"

#include <filesystem>

class AssetLibrary;
class Project;

class SerializeAssetsLoadingTask : public LoadingTask
{
private:
    AssetLibrary*         m_library;
    Project*              m_project;
    
    std::filesystem::path m_path;

protected:

public:
    SerializeAssetsLoadingTask(const std::filesystem::path& a_path, Project* a_project, AssetLibrary* a_library);
    ~SerializeAssetsLoadingTask();

    virtual void Run();
};