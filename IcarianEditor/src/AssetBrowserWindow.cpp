#include "Windows/AssetBrowserWindow.h"

#include <filesystem>
#include <imgui.h>

#include "AppMain.h"
#include "AssetLibrary.h"
#include "Datastore.h"
#include "FileHandler.h"
#include "Flare/IcarianDefer.h"
#include "FlareImGui.h"
#include "IO.h"
#include "Logger.h"
#include "Modals/ConfirmModal.h"
#include "Modals/CreateAssemblyControlModal.h"
#include "Modals/CreateDefTableModal.h"
#include "Modals/CreateFileModal.h"
#include "Modals/CreateRigidBodyScriptModal.h"
#include "Modals/CreateSciptableModal.h"
#include "Modals/CreateTriggerBodyScriptModal.h"
#include "Modals/RenamePathModal.h"
#include "Project.h"
#include "Templates/Scene.h"
#include "Texture.h"

AssetBrowserWindow::AssetBrowserWindow(AppMain* a_app, Project* a_project, AssetLibrary* a_assetLibrary) : Window("Asset Browser")
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
        node.Children.emplace_back(m_fileTree.size());
        MakeDirectoryNode(index, child);
    }

    m_fileTree[index] = node;
}
void AssetBrowserWindow::TraverseFolderTree(uint32_t a_index)
{
    const DirectoryNode& node = m_fileTree[a_index];
    if (a_index != 0)
    {
        ImGui::Text("L");
        ImGui::SameLine();
    }

    const std::string fileName = node.Path.filename().string();

    if (ImGui::Button(fileName.c_str()))
    {
        m_curIndex = a_index;
    }

    ImGui::Indent();

    for (const uint32_t index : node.Children)
    {
        TraverseFolderTree(index);
    }

    ImGui::Unindent();
}

void AssetBrowserWindow::Refresh()
{
    m_fileTree.clear(); 

    m_curIndex = 0;

    MakeDirectoryNode(-1, m_project->GetProjectPath());
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

    virtual void Confirm() override
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
            if (ImGui::MenuItem("Assembly Control"))
            {
                m_app->PushModal(new CreateAssemblyControlModal(m_app, m_project, a_path));
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Scriptable"))
            {
                m_app->PushModal(new CreateScriptableModal(m_app, m_project, a_path));
            }

            if (ImGui::MenuItem("Rigid Body"))
            {
                m_app->PushModal(new CreateRigidBodyScriptModal(m_app, m_project, a_path));
            }

            if (ImGui::MenuItem("Trigger Body"))
            {
                m_app->PushModal(new CreateTriggerBodyScriptModal(m_app, m_project, a_path));
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Def Table"))
            {
                m_app->PushModal(new CreateDefTableModal(m_app, m_project, a_path));   
            }

            ImGui::EndMenu();
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Scene"))
        {
            const uint32_t len = (uint32_t)strlen(SCENETEMPLATE);

            m_app->PushModal(new CreateFileModal(m_app, m_project, a_path, SCENETEMPLATE, len, "New Scene", ".iscene"));
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

    const std::string fileName = path.filename().string();

    Texture* tex;

    if (std::filesystem::is_empty(path))
    {
        tex = Datastore::GetTexture("Textures/FileIcons/FileIcon_FolderEmpty.png");
    }
    else 
    {
        tex = Datastore::GetTexture("Textures/FileIcons/FileIcon_Folder.png");
    }

    FlareImGui::ImageButton(tex, glm::vec2((float)ItemWidth), false);
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

    ImGui::Text("%s", fileName.c_str());

    ImGui::EndGroup();

    ImGui::NextColumn();

    return ret;
}
bool AssetBrowserWindow::ShowAsset(bool a_context, const std::filesystem::path& a_path, const std::filesystem::path& a_workingPath)
{
    bool ret = false;

    ImGui::BeginGroup();

    const std::string fileName = a_path.stem().string();
    const std::filesystem::path rPath = IO::GetRelativePath(a_workingPath, a_path);

    FileHandler::FileCallback* openCallback;
    FileHandler::FileCallback* dragCallback;
    Texture* tex;
    FileHandler::GetFileData(rPath, &openCallback, &dragCallback, &tex);

    FlareImGui::ImageButton(tex, glm::vec2((float)ItemWidth), false);

    uint32_t size;
    const char* data;
    m_assetLibrary->GetAsset(rPath, &size, &data);

    if (size > 0 && data != nullptr)
    {
        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
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
            
    ImGui::Text("%s", fileName.c_str());

    ImGui::EndGroup();

    ImGui::NextColumn();

    return ret;
}

bool AssetBrowserWindow::ShowBaseAssetList(const DirectoryNode& a_node)
{
    bool contextCaptured = false;

    for (uint32_t index : a_node.Children)
    {
        const DirectoryNode& cNode = m_fileTree[index];

        const std::filesystem::path& p = cNode.Path;

        // Failsafe for when a folder is deleted
        if (!std::filesystem::exists(p))
        {
            continue;
        }

        if (ShowFolder(contextCaptured, index))
        {
            contextCaptured = true;
        }
    }

    const std::filesystem::path workingPath = m_project->GetProjectPath();

    for (const std::filesystem::path& path : a_node.Files)
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

    const ImVec2 contentRegion = ImGui::GetContentRegionAvail();

    if (ImGui::BeginChild("##FolderView", { 200.0f, contentRegion.y }))
    {
        TraverseFolderTree(0);
    }

    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginGroup();

    const Texture* resetTex = Datastore::GetTexture("Textures/Icons/Icon_Reset.png");
    if (ImGui::ImageButton((ImTextureID)resetTex->GetHandle(), { 16.0f, 16.0f }))
    {
        m_project->SetRefresh(true);
    }

    ImGui::SameLine();

    ImGui::SetNextItemWidth(128.0f);
    ImGui::InputText("Search", m_searchBuffer, SearchBufferSize);

    ImGui::SameLine();

    for (const uint32_t index : breadcrumbs)
    {
        const DirectoryNode& node = m_fileTree[index];

        const std::string fileName = node.Path.filename().string();

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

        ImGui::Columns(glm::max(1, (int)(width / (ItemWidth + 8.0f))));

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