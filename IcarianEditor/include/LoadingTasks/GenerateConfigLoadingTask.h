#pragma once

#include "LoadingTask.h"

#include <filesystem>

class GenerateConfigLoadingTask : public LoadingTask
{
private:
    std::filesystem::path m_path;
    std::string           m_name;
    std::string           m_renderBackend;

protected:

public:
    GenerateConfigLoadingTask(const std::filesystem::path& a_path, const std::string_view& a_name, const std::string_view& a_renderBackend);
    virtual ~GenerateConfigLoadingTask();

    virtual void Run();
};