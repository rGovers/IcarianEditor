#pragma once

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include <filesystem>
#include <functional>
#include <list>
#include <stack>

enum e_FileDialogStatus
{
    FileDialogStatus_None,
    FileDialogStatus_Ok,
    FileDialogStatus_Cancel,
    FileDialogStatus_Error
};

class FileDialogBlock
{
public:
    using BlockCallback = std::function<void()>;

private:
    static constexpr uint32_t BufferSize = 4096;
    static constexpr float DirectoryExplorerWidth = 150.0f;

    uint32_t                          m_filterIndex;  
    uint32_t                          m_filterCount;
    char**                            m_filters;
 
    bool                              m_directoryExplorer;
    glm::vec2                         m_size;
 
    std::filesystem::path             m_path;
    std::stack<std::filesystem::path> m_prevPaths;
    std::string                       m_name;

    std::list<std::filesystem::path>  m_dirs;
    std::list<std::filesystem::path>  m_files;

    void Refresh();
    void SetPath(const std::filesystem::path& a_path);

protected:

public:
    FileDialogBlock(const glm::vec2& a_size = glm::vec2(-1.0f), bool a_directoryExplorer = false, uint32_t a_filterCount = 0, const char* const* a_filters = nullptr);
    ~FileDialogBlock();

    e_FileDialogStatus ShowFileDialog(std::filesystem::path* a_outPath, std::string* a_outString, BlockCallback a_blockCallback = nullptr);
};