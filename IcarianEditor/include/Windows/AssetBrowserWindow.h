// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#pragma once

#include "Windows/Window.h"

#include <filesystem>
#include <list>
#include <vector>

class AppMain;
class AssetLibrary;
class Project;
class RuntimeManager;

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
    RuntimeManager*            m_runtime;

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
    AssetBrowserWindow(AppMain* a_app, Project* a_project, AssetLibrary* a_assetLibrary, RuntimeManager* a_runtime);
    virtual ~AssetBrowserWindow();
    
    virtual void Refresh();
    virtual void Update(double a_delta);
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