#include "LoadingTasks/SerializeAssetsLoadingTask.h"

#include "AssetLibrary.h"

SerializeAssetsLoadingTask::SerializeAssetsLoadingTask(const std::filesystem::path& a_path, AssetLibrary* a_library)
{
    m_library = a_library;

    m_path = a_path;
}
SerializeAssetsLoadingTask::~SerializeAssetsLoadingTask()
{

}

void SerializeAssetsLoadingTask::Run()
{
    m_library->BuildDirectory(m_path);
}