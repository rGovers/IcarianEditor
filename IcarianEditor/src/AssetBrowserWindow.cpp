// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#include "Windows/AssetBrowserWindow.h"

#include <filesystem>
#include <imgui.h>

#include "AppMain.h"
#include "AssetLibrary.h"
#include "Core/IcarianAssert.h"
#include "Core/IcarianDefer.h"
#include "Datastore.h"
#include "FileHandler.h"
#include "FlareImGui.h"
#include "IO.h"
#include "Modals/ConfirmModal.h"
#include "Modals/CreateAssemblyControlModal.h"
#include "Modals/CreateComponentModal.h"
#include "Modals/CreateDefTableModal.h"
#include "Modals/CreateEmptyScriptModal.h"
#include "Modals/CreateFileModal.h"
#include "Modals/CreateSciptableModal.h"
#include "Modals/RenamePathModal.h"
#include "Project.h"
#include "Runtime/RuntimeManager.h"
#include "Templates.h"
#include "Texture.h"

AssetBrowserWindow::AssetBrowserWindow(AppMain* a_app, Project* a_project, AssetLibrary* a_assetLibrary) : Window("Asset Browser", "Textures/WindowIcons/WindowIcon_AssetBrowser.png")
{
    m_app = a_app;
    m_assetLibrary = a_assetLibrary;
    m_project = a_project;

    m_fileTree.clear();
    m_curIndex = -1;

    m_searchBuffer[0] = 0;
}
AssetBrowserWindow::~AssetBrowserWindow()
{

}

void AssetBrowserWindow::MakeDirectoryNode(uint32_t a_parent, const std::filesystem::path& a_path)
{
    DirectoryNode node;
    node.Parent = a_parent;
    node.Path = a_path;

    std::list<std::filesystem::path> childPaths;

    for (const auto iter : std::filesystem::directory_iterator(a_path, std::filesystem::directory_options::skip_permission_denied))
    {
        if (iter.is_regular_file())
        {
            node.Files.emplace_back(iter.path());
        }
        else if (iter.is_directory())
        {
            childPaths.emplace_back(iter.path());   
        }
    }

    const uint32_t index = (uint32_t)m_fileTree.size();
    m_fileTree.emplace_back(node);

    for (const std::filesystem::path& child : childPaths)
    {
        node.Children.emplace_back((uint32_t)m_fileTree.size());

        MakeDirectoryNode(index, child);
    }

    m_fileTree[index] = node;
}
void AssetBrowserWindow::TraverseFolderTree(uint32_t a_index)
{
    const DirectoryNode& node = m_fileTree[a_index];

    const uint32_t count = (uint32_t)node.Children.size();

    const std::string strID = "##" + std::to_string(a_index);
    const std::string buttonID = strID + "Button";
    const std::string fileName = node.Path.filename().string();

    bool open = false;
    if (count > 0)
    {
        open = ImGui::TreeNode(strID.c_str());

        ImGui::SameLine();
    }
    else
    {
        ImGui::Indent(ImGui::GetTreeNodeToLabelSpacing());
    }
    IDEFER(
    {
        if (open)
        {
            ImGui::TreePop();
        }
        else if (count == 0)
        {
            ImGui::Unindent(ImGui::GetTreeNodeToLabelSpacing());
        } 
    });

    const Texture* folderTex = Datastore::GetTexture("Textures/WindowIcons/WindowIcon_AssetBrowser.png");
    if (folderTex != nullptr)
    {
        ImGui::Image(TexToImHandle(folderTex), ImVec2(16.0f, 16.0f));

        ImGui::SameLine();
    }

    ImGui::PushID(buttonID.c_str());
    IDEFER(ImGui::PopID());

    if (ImGui::Selectable(fileName.c_str()))
    {
        m_curIndex = a_index;
    }

    if (open)
    {
        for (const uint32_t index : node.Children)
        {
            TraverseFolderTree(index);
        }
    }
}

void AssetBrowserWindow::Refresh()
{
    std::filesystem::path curPath;
    if (m_curIndex != -1)
    {
        ICARIAN_ASSERT(m_curIndex < m_fileTree.size());
        
        curPath = m_fileTree[m_curIndex].Path;
    }

    const std::filesystem::path projectPath = m_project->GetProjectPath();

    m_fileTree.clear(); 

    m_curIndex = 0;

    MakeDirectoryNode(-1, projectPath);

    const uint32_t count = (uint32_t)m_fileTree.size();
    for (uint32_t i = 0; i < count; ++i)
    {
        const DirectoryNode& node = m_fileTree[i];

        if (node.Path == curPath)
        {
            m_curIndex = i;

            break;
        }
    }
}

class DeleteAssetData : public ConfirmModalData
{
private:
    Project*              m_project;
    
    std::filesystem::path m_path;

protected:

public:
    DeleteAssetData(Project* a_project, const std::filesystem::path& a_path) 
    {
        m_project = a_project;

        m_path = a_path;
    }
    virtual ~DeleteAssetData() { }

    virtual void Confirm()
    {
        // TODO: Should allow for partial update of tree instead of full refresh
        // Full refresh of project is expensive as it requires a full reload of
        // all assets and recompilation of code
        // Would require knowing what assets are affected by the delete plus
        // would need to walk the whole tree because of directory linking
        // Hypothetically possible to delete system files if not careful but
        // would require user to go out of their way 
        // Because of the above naive implementation is used for now
        std::filesystem::remove_all(m_path);

        m_project->SetRefresh(true);
    }
};

void AssetBrowserWindow::BaseMenu(const std::filesystem::path& a_path, const std::filesystem::path& a_assetPath)
{
    const bool isFolder = a_assetPath.empty();

    if (ImGui::BeginMenu("New"))
    {
        if (ImGui::MenuItem("Folder"))
        {
            const std::filesystem::path newPath = a_path / "New Folder";

            std::filesystem::create_directory(newPath);

            m_project->SetRefresh(true);
        }

        ImGui::Separator();

        if (ImGui::BeginMenu("Script"))
        {
            IDEFER(ImGui::EndMenu());

            if (ImGui::MenuItem("Empty"))
            {
                m_app->PushModal(new CreateEmptyScriptModal(m_app, m_project, a_path));
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Assembly Control"))
            {
                m_app->PushModal(new CreateAssemblyControlModal(m_app, m_project, a_path));
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Component"))
            {
                m_app->PushModal(new CreateComponentModal(m_app, m_project, a_path));
            }

            if (ImGui::MenuItem("Scriptable"))
            {
                m_app->PushModal(new CreateScriptableModal(m_app, m_project, a_path));
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Def Table"))
            {
                m_app->PushModal(new CreateDefTableModal(m_app, m_project, a_path));   
            }
        }

        if (ImGui::BeginMenu("Shader"))
        {
            IDEFER(ImGui::EndMenu());

            if (ImGui::MenuItem("Vertex Shader"))
            {
                constexpr uint32_t Length = sizeof(VertexShaderTemplate) / sizeof(*VertexShaderTemplate);

                m_app->PushModal(new CreateFileModal(m_app, m_project, a_path, VertexShaderTemplate, Length - 1, "New Vertex Shader", ".fvert"));
            }

            if (ImGui::MenuItem("Skinned Vertex Shader"))
            {
                constexpr uint32_t Length = sizeof(SkinnedVertexShaderTemplate) / sizeof(*SkinnedVertexShaderTemplate);

                m_app->PushModal(new CreateFileModal(m_app, m_project, a_path, SkinnedVertexShaderTemplate, Length - 1, "New Skinned Vertex Shader", ".fvert"));
            }

            if (ImGui::MenuItem("Shadow Vertex Shader"))
            {
                constexpr uint32_t Length = sizeof(ShadowVertexShaderTemplate) / sizeof(*ShadowVertexShaderTemplate);

                m_app->PushModal(new CreateFileModal(m_app, m_project, a_path, ShadowVertexShaderTemplate, Length - 1, "New Shadow Vertex Shader", ".fvert"));
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Pixel Shader"))
            {
                constexpr uint32_t Length = sizeof(PixelShaderTemplate) / sizeof(*PixelShaderTemplate);

                m_app->PushModal(new CreateFileModal(m_app, m_project, a_path, PixelShaderTemplate, Length - 1, "New Pixel Shader", ".fpix"));
            }
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Def"))
        {
            const std::string pathStr = a_path.string();

            MonoString* str = mono_string_new(RuntimeManager::GetEditorDomain(), pathStr.c_str());

            void* args[] =
            {
                str
            };

            RuntimeManager::ExecFunction("IcarianEditor.Modals", "CreateDefModal", ":Create(string)", args);
        }

        if (ImGui::MenuItem("Scene"))
        {
            constexpr uint32_t Length = sizeof(SceneTemplate) / sizeof(*SceneTemplate) - 1;

            m_app->PushModal(new CreateFileModal(m_app, m_project, a_path, SceneTemplate, Length, "New Scene", ".iscene"));
        }

        if (ImGui::MenuItem("Canvas"))
        {
            constexpr uint32_t Length = sizeof(CanvasTemplate) / sizeof(*CanvasTemplate) - 1;

            m_app->PushModal(new CreateFileModal(m_app, m_project, a_path, CanvasTemplate, Length, "New Canvas", ".ui"));
        }

        ImGui::EndMenu();
    }

    ImGui::Separator();

    if (ImGui::MenuItem("Show in file explorer"))
    {
        if (isFolder)
        {
            IO::OpenFileExplorer(a_path);
        }
        else
        {
            const std::filesystem::path assetPath = a_assetPath.parent_path();

            IO::OpenFileExplorer(assetPath);
        }
    }
}
void AssetBrowserWindow::AssetMenu(const std::filesystem::path& a_path, const std::filesystem::path& a_assetPath)
{
    const bool isFolder = a_assetPath.empty();

    if (ImGui::Selectable("Rename"))
    {
        if (isFolder)
        {
            m_app->PushModal(new RenamePathModal(m_app, m_project, a_path));
        }
        else
        {
            m_app->PushModal(new RenamePathModal(m_app, m_project, a_assetPath));
        }
    }

    if (ImGui::Selectable("Delete")) 
    {
        if (isFolder)
        {
            m_app->PushModal(new ConfirmModal("Are you sure you want to delete this folder?", new DeleteAssetData(m_project, a_path)));   
        }
        else 
        {
            m_app->PushModal(new ConfirmModal("Are you sure you want to delete this asset?", new DeleteAssetData(m_project, a_assetPath)));
        }
    }
}

bool AssetBrowserWindow::ShowFolder(bool a_context, uint32_t a_index)
{
    bool ret = false;

    const DirectoryNode& cNode = m_fileTree[a_index];

    const std::filesystem::path& path = cNode.Path;

    ImGui::BeginGroup();

    const std::filesystem::path filenamePath = path.filename();
    const std::string filename = filenamePath.string();

    Texture* tex;

    if (std::filesystem::is_empty(path))
    {
        tex = Datastore::GetTexture("Textures/FileIcons/FileIcon_FolderEmpty.png");
    }
    else 
    {
        tex = Datastore::GetTexture("Textures/FileIcons/FileIcon_Folder.png");
    }

    FlareImGui::ImageButton(filename.c_str(), tex, glm::vec2((float)ItemWidth), false);
    if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
    {
        m_curIndex = a_index;
    }

    if (!a_context && ImGui::BeginPopupContextItem()) 
    {
        IDEFER(ImGui::EndPopup());

        BaseMenu(path, "");

        ImGui::Separator();

        AssetMenu(path, "");

        ret = true;
    }

    ImGui::Text("%s", filename.c_str());

    ImGui::EndGroup();

    ImGui::NextColumn();

    return ret;
}
bool AssetBrowserWindow::ShowAsset(bool a_context, const std::filesystem::path& a_path, const std::filesystem::path& a_workingPath)
{
    bool ret = false;

    const ImGuiStyle& style = ImGui::GetStyle();
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    const ImVec2 curPos = ImGui::GetCursorScreenPos();
    const ImVec2 spacing = style.ItemSpacing;

    const float startXPos = curPos.x;
    const float startYPos = curPos.y;

    const float width = ImGui::GetColumnWidth() - spacing.x;
    const float height = 170.0f + spacing.y;

    IDEFER(ImGui::NextColumn());

    ImGui::BeginGroup();

    const ImU32 rectColor = ImGui::GetColorU32(ImVec4(0.25f, 0.25f, 0.25f, 0.25f));

    drawList->AddRectFilled(ImVec2(startXPos, startYPos), ImVec2(startXPos + width, startYPos + height), rectColor, 2.0f);

    const std::filesystem::path filenamePath = a_path.stem();
    const std::string filename = filenamePath.string();
    const std::filesystem::path rPath = IO::GetRelativePath(a_workingPath, a_path);
    const std::string pathStr = a_path.string();

    FileHandler::FileCallback* openCallback;
    FileHandler::FileCallback* dragCallback;
    GLuint tex;
    FileHandler::GetFileData(rPath, &openCallback, &dragCallback, &tex);

    FlareImGui::ImageButton(pathStr.c_str(), tex, glm::vec2((float)ItemWidth), false);

    uint32_t size;
    const uint8_t* data;
    e_AssetType type;
    m_assetLibrary->GetAsset(rPath, &size, &data, &type);

    if (size > 0 && data != nullptr)
    {
        const bool hovered = ImGui::IsItemHovered();
        if (hovered && ImGui::IsMouseDoubleClicked(0))
        {
            if (openCallback == nullptr)
            {
                IO::OpenFile(a_path);
            }
            else
            {
                (*openCallback)(a_path, rPath, size, data);
            }
        }
                
        if (dragCallback != nullptr)
        {
            if (ImGui::BeginDragDropSource())
            {
                IDEFER(ImGui::EndDragDropSource());

                (*dragCallback)(a_path, rPath, size, data);
            }
        }

        if (!a_context && ImGui::BeginPopupContextItem())
        {
            IDEFER(ImGui::EndPopup());

            const std::filesystem::path dirPath = a_path.parent_path();

            BaseMenu(dirPath, a_path);

            ImGui::Separator();

            AssetMenu(dirPath, a_path);

            ret = true;
        }
    }
            
    ImGui::Text("%s", filename.c_str());

    if (type != AssetType_Null)
    {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.75f, 0.75f, 0.75f, 0.75f));
        IDEFER(ImGui::PopStyleColor());
        
        ImGui::Text("%s", AssetTypeStrings[type]);
    }

    ImGui::EndGroup();

    return ret;
}

bool AssetBrowserWindow::ShowBaseAssetList(const DirectoryNode& a_node)
{
    bool contextCaptured = false;

    std::list<uint32_t> folders;

    // Sort folders by name
    for (const uint32_t index : a_node.Children)
    {
        const DirectoryNode& cNode = m_fileTree[index];

        if (!std::filesystem::exists(cNode.Path))
        {
            continue;
        }

        for (auto iter = folders.begin(); iter != folders.end(); ++iter)
        {
            const DirectoryNode& pNode = m_fileTree[*iter];

            if (pNode.Path > cNode.Path)
            {
                folders.insert(iter, index);

                goto NextFolder;
            }
        }

        folders.emplace_back(index);

        NextFolder:;
    }

    for (const uint32_t index : folders)
    {
        const DirectoryNode& cNode = m_fileTree[index];

        if (!std::filesystem::exists(cNode.Path))
        {
            continue;
        }

        if (ShowFolder(contextCaptured, index))
        {
            contextCaptured = true;
        }
    }

    std::list<std::filesystem::path> files;

    // Sort files by name
    for (const std::filesystem::path& path : a_node.Files)
    {
        if (!std::filesystem::exists(path))
        {
            continue;
        }

        for (auto iter = files.begin(); iter != files.end(); ++iter)
        {
            if (*iter > path)
            {
                files.insert(iter, path);

                goto NextFile;
            }
        }

        files.emplace_back(path);

        NextFile:;
    }

    const std::filesystem::path workingPath = m_project->GetProjectPath();

    for (const std::filesystem::path& path : files)
    {
        // Failsafe for when a file is deleted
        if (!std::filesystem::exists(path))
        {
            continue;
        }

        if (ShowAsset(contextCaptured, path, workingPath))
        {
            contextCaptured = true;
        }
    }

    return contextCaptured;
}
bool AssetBrowserWindow::ShowSearchAssetList(const DirectoryNode& a_node, const std::string_view& a_filter)
{
    bool contextCaptured = false;

    Texture* folderTex = Datastore::GetTexture("Textures/FileIcons/FileIcon_Folder.png");
    Texture* emptyFolderTex = Datastore::GetTexture("Textures/FileIcons/FileIcon_FolderEmpty.png");

    for (uint32_t index : a_node.Children)
    {
        const DirectoryNode& cNode = m_fileTree[index];

        const std::filesystem::path& p = cNode.Path;

        // Failsafe for when a folder is deleted
        if (!std::filesystem::exists(p))
        {
            continue;
        }

        if (ShowSearchAssetList(cNode, a_filter))
        {
            contextCaptured = true;
        }

        const std::string pString = p.string();
        if (pString.find(a_filter) == std::string::npos)
        {
            continue;
        }

        if (ShowFolder(contextCaptured, index))
        {
            contextCaptured = true;
        }
    }

    for (const std::filesystem::path& path : a_node.Files)
    {
        // Failsafe for when a file is deleted
        if (!std::filesystem::exists(path))
        {
            continue;
        }

        const std::string pString = path.string();
        if (pString.find(a_filter) == std::string::npos)
        {
            continue;
        }

        if (ShowAsset(contextCaptured, path, m_project->GetProjectPath()))
        {
            contextCaptured = true;
        }
    }

    return contextCaptured;
}

void AssetBrowserWindow::Update(double a_delta)
{
    if (m_curIndex == -1)
    {
        return;
    }

    const DirectoryNode& node = m_fileTree[m_curIndex];
    std::list<uint32_t> breadcrumbs; 
    uint32_t pIndex = node.Parent;
    breadcrumbs.emplace_front(m_curIndex);
    while (pIndex != -1)
    {
        breadcrumbs.emplace_front(pIndex);

        pIndex = m_fileTree[pIndex].Parent;
    }

    if (ImGui::BeginChild("##FolderView", { 200.0f, 0.0f }))
    {
        TraverseFolderTree(0);
    }

    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginGroup();

    const Texture* resetTex = Datastore::GetTexture("Textures/Icons/Icon_Reset.png");
    if (ImGui::ImageButton(TexToImHandle(resetTex), { 16.0f, 16.0f }))
    {
        m_project->SetRefresh(true);
    }
    if (ImGui::IsItemHovered() && ImGui::BeginTooltip())
    {
        IDEFER(ImGui::EndTooltip());
        
        ImGui::Text("Reload Project");
    }

    ImGui::SameLine();

    ImGui::AlignTextToFramePadding();
    ImGui::Text("%s", "Search");

    ImGui::SameLine();

    ImGui::SetNextItemWidth(128.0f);
    ImGui::InputText("##Search", m_searchBuffer, SearchBufferSize);

    ImGui::SameLine();

    for (const uint32_t index : breadcrumbs)
    {
        const DirectoryNode& node = m_fileTree[index];

        const std::string fileName = node.Path.filename().string();

        if (index == 0)
        {
            if (FlareImGui::ImageButton("Project", "Textures/Icons/Icon_Home.png", { 16.0f, 16.0f }))
            {
                m_curIndex = index;
            }

            if (m_curIndex != index)
            {
                ImGui::SameLine();
                ImGui::Text("/");
                ImGui::SameLine();
            }

            continue;
        }

        if (m_curIndex != index)
        {
            if (ImGui::Button(fileName.c_str()))
            {
                m_curIndex = index;
            }

            ImGui::SameLine();
            ImGui::Text("/");
            ImGui::SameLine();
        }
        else
        {
            ImGui::Text("%s", fileName.c_str());
        }
    }

    bool contextCaptured = false;
    if (ImGui::BeginChild("##Explorer"))
    {
        const float width = ImGui::GetWindowWidth();

        ImGui::Columns(glm::max(1, (int)(width / (ItemWidth + 20.0f))));

        if (m_searchBuffer[0] == 0)
        {
            contextCaptured = ShowBaseAssetList(node);
        }
        else 
        {
            contextCaptured = ShowSearchAssetList(node, m_searchBuffer);
        }     

        ImGui::Columns();
    }

    if (!contextCaptured && ImGui::BeginPopupContextWindow())
    {
        BaseMenu(node.Path, "");

        ImGui::EndPopup();
    }

    ImGui::EndChild();

    ImGui::EndGroup();
}

// MIT License
// 
// Copyright (c) 2025 River Govers
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