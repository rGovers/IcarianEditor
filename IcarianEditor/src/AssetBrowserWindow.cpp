#include "Windows/AssetBrowserWindow.h"

#include <filesystem>
#include <imgui.h>

#include "AssetLibrary.h"
#include "Datastore.h"
#include "FileHandler.h"
#include "FlareImGui.h"
#include "Logger.h"
#include "Project.h"

AssetBrowserWindow::AssetBrowserWindow(Project* a_project, AssetLibrary* a_assetLibrary) : Window("Asset Browser")
{
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
            ImGui::Text(fileName.c_str());
        }
    }

    if (ImGui::BeginChild("##Explorer"))
    {
        const float width = ImGui::GetWindowWidth();

        ImGui::Columns(glm::max(1, (int)(width / (ItemWidth + 8.0f))));

        Texture* folderTex = Datastore::GetTexture("Textures/FileIcons/FileIcon_Folder.png");
        Texture* emptyFolderTex = Datastore::GetTexture("Textures/FileIcons/FileIcon_FolderEmpty.png");

        for (uint32_t index : node.Children)
        {
            const DirectoryNode& cNode = m_fileTree[index];
            ImGui::BeginGroup();
            
            const std::filesystem::path p = std::filesystem::path(cNode.Path);
            
            const std::string fileName = p.filename().string();

            Texture* tex = folderTex;

            if (std::filesystem::is_empty(p))
            {
                tex = emptyFolderTex;
            }

            FlareImGui::ImageButton(tex, glm::vec2(ItemWidth), false);
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
            {
                m_curIndex = index;
            }

            ImGui::Text(fileName.c_str());

            ImGui::EndGroup();

            ImGui::NextColumn();
        }

        const std::filesystem::path workingPath = m_project->GetProjectPath();

        for (const std::filesystem::path& path : node.Files)
        {
            ImGui::BeginGroup();

            const std::string fileName = path.stem().string();

            FileHandler::FileCallback* openCallback;
            FileHandler::FileCallback* dragCallback;
            Texture* tex;
            FileHandler::GetFileData(path, &openCallback, &dragCallback, &tex);

            FlareImGui::ImageButton(tex, glm::vec2(ItemWidth), false);

            uint32_t size;
            const char *data;
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
            
            ImGui::Text(fileName.c_str());

            ImGui::EndGroup();

            ImGui::NextColumn();
        }

        ImGui::Columns();
    }

    ImGui::EndChild();

    ImGui::EndGroup();
}