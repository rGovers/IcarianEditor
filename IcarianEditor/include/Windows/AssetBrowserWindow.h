#pragma once

#include "Windows/Window.h"

#include <filesystem>
#include <list>
#include <vector>

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
    static constexpr int ItemWidth = 128;

    AssetLibrary*              m_assetLibrary;
    Project*                   m_project;

    std::vector<DirectoryNode> m_fileTree;
    uint32_t                   m_curIndex;

    void MakeDirectoryNode(uint32_t a_parent, const std::filesystem::path& a_path);
    void TraverseFolderTree(uint32_t a_index);

protected:

public:
    AssetBrowserWindow(Project* a_project, AssetLibrary* a_assetLibrary);
    virtual ~AssetBrowserWindow();
    
    virtual void Refresh();
    virtual void Update(double a_delta);
};