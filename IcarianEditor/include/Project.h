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