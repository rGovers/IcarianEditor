#pragma once

#include "Windows/Window.h"

#include <filesystem>
#include <list>
#include <vector>

class AppMain;
class AssetLibrary;
class Project;

struct DirectoryNode
{
    uint32_t Parent;
    std::filesystem::path Path;
    std::list<std::filesystem::path> Files;
    std::list<uint32_t> Children;
};

class AssetBrowserWindow : public Window
{
private:
    static constexpr uint32_t ItemWidth = 128;
    static constexpr uint32_t SearchBufferSize = 2048;

    char                       m_searchBuffer[SearchBufferSize];
    AppMain*                   m_app;
    AssetLibrary*              m_assetLibrary;
    Project*                   m_project;

    std::vector<DirectoryNode> m_fileTree;
    uint32_t                   m_curIndex;

    bool ShowFolder(bool a_context, uint32_t a_index);
    bool ShowAsset(bool a_context, const std::filesystem::path& a_path, const std::filesystem::path& a_workingPath);

    void MakeDirectoryNode(uint32_t a_parent, const std::filesystem::path& a_path);
    void TraverseFolderTree(uint32_t a_index);

    bool ShowBaseAssetList(const DirectoryNode& a_node);
    bool ShowSearchAssetList(const DirectoryNode& a_node, const std::string_view& a_filter);

    void BaseMenu(const std::filesystem::path& a_path, const std::filesystem::path& a_assetPath);
    void AssetMenu(const std::filesystem::path& a_path, const std::filesystem::path& a_assetPath);

protected:

public:
    AssetBrowserWindow(AppMain* a_app, Project* a_project, AssetLibrary* a_assetLibrary);
    virtual ~AssetBrowserWindow();
    
    virtual void Refresh();
    virtual void Update(double a_delta);
};