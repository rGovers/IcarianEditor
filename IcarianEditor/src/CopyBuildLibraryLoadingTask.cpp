// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

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