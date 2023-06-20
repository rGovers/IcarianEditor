#pragma once

#include "LoadingTask.h"

#include <filesystem>

class CopyBuildLibraryLoadingTask : public LoadingTask
{
private:
    std::filesystem::path m_path;
    std::string           m_name;

    std::string           m_platform;

protected:

public:
    CopyBuildLibraryLoadingTask(const std::filesystem::path& a_path, const std::string_view& a_name, const std::string_view& a_platform);
    virtual ~CopyBuildLibraryLoadingTask();

    virtual void Run();
};