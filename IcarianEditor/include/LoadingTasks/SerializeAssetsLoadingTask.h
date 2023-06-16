#pragma once

#include "LoadingTasks/LoadingTask.h"

#include <filesystem>

class AssetLibrary;

class SerializeAssetsLoadingTask : public LoadingTask
{
private:
    AssetLibrary*         m_library;

    std::filesystem::path m_path;

protected:

public:
    SerializeAssetsLoadingTask(const std::filesystem::path& a_path, AssetLibrary* a_library);
    ~SerializeAssetsLoadingTask();

    virtual void Run();
};