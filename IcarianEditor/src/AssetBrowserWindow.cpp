#include "Windows/AssetBrowserWindow.h"

#include <filesystem>
#include <imgui.h>

#include "AppMain.h"
#include "AssetLibrary.h"
#include "Datastore.h"
#include "FileHandler.h"
#include "FlareImGui.h"
#include "Logger.h"
#include "Modals/ConfirmModal.h"
#include "Modals/RenamePathModal.h"
#include "Project.h"

AssetBrowserWindow::AssetBrowserWindow(AppMain* a_app, Project* a_project, AssetLibrary* a_assetLibrary) : Window("Asset Browser")
{
    m_app = a_app;
    m_assetLibrary = a_assetLibrary;
    m_project = a_project;

    m_fileTree.clear();
    m_curIndex = -1;
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
    if (ImGui::BeginMenu("New"))
    {
        if (ImGui::MenuItem("Folder"))
        {
            const std::filesystem::path newPath = a_path / "New Folder";

            std::filesystem::create_directory(newPath);

            m_project->SetRefresh(true);
        }

        ImGui::EndMenu();
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
            ImGui::Text(">");
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

        Texture* folderTex = Datastore::GetTexture("Textures/FileIcons/FileIcon_Folder.png");
        Texture* emptyFolderTex = Datastore::GetTexture("Textures/FileIcons/FileIcon_FolderEmpty.png");

        for (uint32_t index : node.Children)
        {
            const DirectoryNode& cNode = m_fileTree[index];

            const std::filesystem::path p = std::filesystem::path(cNode.Path);

            // Failsafe for when a folder is deleted
            if (!std::filesystem::exists(p))
            {
                continue;
            }

            ImGui::BeginGroup();

            const std::string fileName = p.filename().string();

            Texture* tex = folderTex;

            if (std::filesystem::is_empty(p))
            {
                tex = emptyFolderTex;
            }

            FlareImGui::ImageButton(tex, glm::vec2(ItemWidth), false);
            if (ImGui::IsItemHovered())
            {
                if (ImGui::IsMouseDoubleClicked(0))
                {
                    m_curIndex = index;
                }                
            }

            if (!contextCaptured && ImGui::BeginPopupContextItem()) 
            {
                BaseMenu(cNode.Path, "");

                ImGui::Separator();

                AssetMenu(cNode.Path, "");

                ImGui::EndPopup();

                contextCaptured = true;
            }

            ImGui::Text("%s", fileName.c_str());

            ImGui::EndGroup();

            ImGui::NextColumn();
        }

        const std::filesystem::path workingPath = m_project->GetProjectPath();

        for (const std::filesystem::path& path : node.Files)
        {
            // Failsafe for when a file is deleted
            if (!std::filesystem::exists(path))
            {
                continue;
            }

            ImGui::BeginGroup();

            const std::string fileName = path.stem().string();

            FileHandler::FileCallback* openCallback;
            FileHandler::FileCallback* dragCallback;
            Texture* tex;
            FileHandler::GetFileData(path, &openCallback, &dragCallback, &tex);

            FlareImGui::ImageButton(tex, glm::vec2(ItemWidth), false);

            uint32_t size;
            const char* data;
            m_assetLibrary->GetAsset(workingPath, path, &size, &data);

            const std::filesystem::path rPath = AssetLibrary::GetRelativePath(workingPath, path);

            if (size > 0 && data != nullptr)
            {
                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
                {
                    if (openCallback == nullptr)
                    {
                        Logger::Error("Not Implemented File Open");
                    }
                    else
                    {
                        (*openCallback)(rPath, size, data);
                    }
                }
                
                if (dragCallback != nullptr)
                {
                    if (ImGui::BeginDragDropSource())
                    {
                        (*dragCallback)(rPath, size, data);

                        ImGui::EndDragDropSource();
                    }
                }
            }
            
            ImGui::Text("%s", fileName.c_str());

            ImGui::EndGroup();

            ImGui::NextColumn();
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