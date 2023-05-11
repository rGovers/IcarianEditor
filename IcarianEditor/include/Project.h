#pragma once

#include <filesystem>
#include <string>

class AppMain;
class AssetLibrary;
class RuntimeManager;
class Workspace;

class Project
{
private:
    AppMain*              m_app;
    Workspace*            m_workspace;
    AssetLibrary*         m_assetLibrary;

    std::filesystem::path m_path;
    std::string           m_name;

    void NewCallback(const std::filesystem::path& a_path, const std::string_view& a_name);
    void OpenCallback(const std::filesystem::path& a_path, const std::string_view& a_name);

protected:

public:
    Project(AppMain* a_app, AssetLibrary* a_assetLibrary, Workspace* a_workspace);
    ~Project();

    inline std::filesystem::path GetPath() const
    {
        return m_path;
    }
    inline std::string_view GetName() const
    {
        return m_name;
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

    void New();
    void Open();
    void Save() const;
};