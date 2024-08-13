// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#pragma once

#include <cstdint>
#include <filesystem>
#include <string>

class AppMain;
class AssetLibrary;
class RuntimeManager;
class Workspace;

#include "Core/Bitfield.h"

class Project
{
private:
    constexpr static uint32_t ConvertKTXBit = 0;

    AppMain*              m_app;
    Workspace*            m_workspace;
    AssetLibrary*         m_assetLibrary;

    std::filesystem::path m_path;
    std::string           m_name;

    uint32_t              m_projectFlags;

    bool                  m_shouldRefresh;

    void SaveProjectFile() const;

    void NewCallback(const std::filesystem::path& a_path, const std::string_view& a_name);
    void OpenCallback(const std::filesystem::path& a_path, const std::string_view& a_name);

protected:

public:
    Project(AppMain* a_app, AssetLibrary* a_assetLibrary, Workspace* a_workspace);
    ~Project();

    void ReloadProjectFile();

    inline std::filesystem::path GetPath() const
    {
        return m_path;
    }
    inline std::string GetName() const
    {
        return m_name;
    }

    inline std::filesystem::path GetProjectFilePath() const
    {
        return m_path / (m_name + ".icproj");
    }
    inline std::filesystem::path GetCachePath() const
    {
        return m_path / ".cache";
    }
    inline std::filesystem::path GetProjectPath() const
    {
        return m_path / "Project";
    }

    inline bool IsValidProject() const
    {
        return !m_path.empty() && !m_name.empty();
    }

    inline bool ShouldRefresh()
    {
        return m_shouldRefresh;
    }
    inline void SetRefresh(bool a_shouldRefresh)
    {
        m_shouldRefresh = a_shouldRefresh;
    }

    inline bool ConvertKTX() const
    {
        return IISBITSET(m_projectFlags, ConvertKTXBit);
    }
    void SetConvertKTX(bool a_state) 
    {
        ITOGGLEBIT(a_state, m_projectFlags, ConvertKTXBit);
    }

    void New();
    void Open();
    void Save() const;

    void Build();
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