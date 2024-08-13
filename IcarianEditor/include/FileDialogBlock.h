// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

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

// MIT License
// 
// Copyright (c) 2024 River Govers
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.