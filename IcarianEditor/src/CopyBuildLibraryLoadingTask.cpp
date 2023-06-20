#include "LoadingTasks/CopyBuildLibraryLoadingTask.h"

CopyBuildLibraryLoadingTask::CopyBuildLibraryLoadingTask(const std::filesystem::path& a_path, const std::string_view& a_name, const std::string_view& a_platform)
{
    m_platform = std::string(a_platform);
    m_name = std::string(a_name);

    m_path = a_path;
}
CopyBuildLibraryLoadingTask::~CopyBuildLibraryLoadingTask()
{

}

void CopyBuildLibraryLoadingTask::Run()
{
    const std::filesystem::path rootPath = std::filesystem::current_path() / "BuildFiles" / m_platform / "bin";

    for (const auto& iter : std::filesystem::directory_iterator(rootPath, std::filesystem::directory_options::skip_permission_denied))
    {
        if (iter.is_directory())
        {
            std::filesystem::copy(iter.path(), m_path / iter.path().filename(), std::filesystem::copy_options::recursive | std::filesystem::copy_options::skip_symlinks | std::filesystem::copy_options::overwrite_existing);
        }
        else if (iter.is_regular_file())
        {
            const std::filesystem::path filename = iter.path().filename();

            if (filename == "renameexe")
            {
                if (m_platform == "Windows")
                {
                    std::filesystem::copy(iter.path(), m_path / (m_name + ".exe"), std::filesystem::copy_options::overwrite_existing);
                }
                else
                {
                    std::filesystem::copy(iter.path(), m_path / m_name, std::filesystem::copy_options::overwrite_existing);
                }
            }
            else 
            {
                std::filesystem::copy(iter.path(), m_path / filename, std::filesystem::copy_options::overwrite_existing);
            }
        }
    }
}