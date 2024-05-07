#include "FileDialogBlock.h"

#include <cstring>
#include <imgui.h>
#include <imgui_internal.h>

#include "Core/IcarianDefer.h"
#include "Datastore.h"
#include "FileDialog.h"
#include "FlareImGui.h"
#include "IO.h"
#include "Texture.h"

FileDialogBlock::FileDialogBlock(const glm::vec2& a_size, bool a_directoryExplorer, uint32_t a_filterCount, const char* const* a_filters)
{
    m_size = a_size;
    m_directoryExplorer = a_directoryExplorer;

    m_filterCount = a_filterCount;
    m_filterIndex = 0;
    m_filters = nullptr;

    m_path = IO::GetHomePath();
    m_name = "";

    if (m_directoryExplorer)
    {
        FileDialog::GenerateDirs(&m_dirs, m_path);
    }
    else
    {
        FileDialog::GenerateFileDirs(&m_dirs, &m_files, m_path);
    }
    
    if (m_filterCount > 0)
    {
        m_filters = new char*[m_filterCount];

        for (uint32_t i = 0; i < m_filterCount; ++i)
        {
            const uint32_t filterLen = (uint32_t)strlen(a_filters[i]);
            m_filters[i] = new char[filterLen + 1];

            for (uint32_t j = 0; j < filterLen; ++j)
            {
                m_filters[i][j] = a_filters[i][j];
            }

            m_filters[i][filterLen] = 0;
        }
    }
}
FileDialogBlock::~FileDialogBlock()
{
    if (m_filters != nullptr)
    {
        for (uint32_t i = 0; i < m_filterCount; ++i)
        {
            delete[] m_filters[i];
        }

        delete[] m_filters;
    }
}

void FileDialogBlock::Refresh()
{
    m_dirs.clear();

    if (m_directoryExplorer) 
    {
        FileDialog::GenerateDirs(&m_dirs, m_path);
    } 
    else 
    {
        m_files.clear();
        
        FileDialog::GenerateFileDirs(&m_dirs, &m_files, m_path);
    }
}
void FileDialogBlock::SetPath(const std::filesystem::path& a_path)
{
    if (std::filesystem::exists(m_path))
    {
        if (m_prevPaths.empty())
        {
            m_prevPaths.emplace(m_path);
        }
        else if (m_prevPaths.top() != m_path)
        {
            m_prevPaths.emplace(m_path);
        }
    }

    m_path = a_path;
}

static bool IsRootPath(const std::filesystem::path& a_path)
{
    return !a_path.has_parent_path() || a_path == "/";
}

e_FileDialogStatus FileDialogBlock::ShowFileDialog(std::filesystem::path* a_outPath, std::string* a_outString, BlockCallback a_blockCallback)
{
    const ImGuiStyle& style = ImGui::GetStyle();

    char buffer[BufferSize];

    const std::string pathStr = m_path.string();
    const uint32_t pathLen = (uint32_t)pathStr.size();
    const bool tooLong = pathLen >= BufferSize;
    if (tooLong)
    {
        SetPath(IO::GetHomePath());

        return FileDialogStatus_Error;
    }

    for (uint32_t i = 0; i < pathLen; ++i)
    {
        buffer[i] = pathStr[i];
    }
    buffer[pathLen] = 0;    

    {
        const bool disableBack = m_prevPaths.empty();

        if (disableBack)
        {
            ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, style.Alpha * 0.5f);
        }

        IDEFER(
        if (disableBack)
        {
            ImGui::PopItemFlag();
            ImGui::PopStyleVar();
        });

        if (FlareImGui::ImageButton("Back", "Textures/Icons/Icon_Back.png", { 16.0f, 16.0f }))
        {
            const std::filesystem::path p = m_prevPaths.top();
            m_prevPaths.pop();

            m_path = p;

            Refresh();
        }

        if (ImGui::IsItemHovered())
        {
            ImGui::BeginTooltip();
            IDEFER(ImGui::EndTooltip());

            ImGui::Text("%s", "Back");
        }

        ImGui::SameLine();
    }

    {
        const bool disableUp = IsRootPath(m_path);

        if (disableUp)
        {
            ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, style.Alpha * 0.5f);
        }

        IDEFER(
        if (disableUp)
        {
            ImGui::PopItemFlag();
            ImGui::PopStyleVar();
        });

        if (FlareImGui::ImageButton("Up", "Textures/Icons/Icon_Up.png", { 16.0f, 16.0f }))
        {
            SetPath(m_path.parent_path());

            Refresh();
        }

        if (ImGui::IsItemHovered())
        {
            ImGui::BeginTooltip();
            IDEFER(ImGui::EndTooltip());

            ImGui::Text("%s", "Up Level");
        }

        ImGui::SameLine();
    }
    
    if (FlareImGui::ImageButton("Refresh", "Textures/Icons/Icon_Reset.png", { 16.0f, 16.0f }))
    {
        Refresh();   
    }

    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        IDEFER(ImGui::EndTooltip());
        
        ImGui::Text("%s", "Refresh");
    }

    ImGui::SameLine();

    if (ImGui::InputText("Path", buffer, BufferSize, ImGuiInputTextFlags_EnterReturnsTrue))
    {
        SetPath(buffer);

        Refresh();
    }

    glm::vec2 size = m_size;
    const ImVec2 region = ImGui::GetContentRegionAvail();
    if (size.x < 0.0f)
    {
        size.x = region.x - style.FramePadding.x * 2.0f;
    }
    if (size.y < 0.0f)
    {
        size.y = region.y - ImGui::GetFrameHeightWithSpacing() * 1.5f;
    }

    {
        ImGui::BeginGroup();
        IDEFER(ImGui::EndGroup());

        const float upperHeight = size.y * 0.25f - style.FramePadding.y;
        const float lowerHeight = size.y * 0.75f;

        if (ImGui::BeginChild("Drives", { DirectoryExplorerWidth, upperHeight }))
        {  
            IDEFER(ImGui::EndChild());

            const std::vector<std::filesystem::path> drives = IO::GetDrives();

            for (const std::filesystem::path& drive : drives)
            {
                const std::string str = drive.string();

                const bool selected = m_path == drive;
                if (ImGui::Selectable(str.c_str(), selected))
                {
                    SetPath(drive);

                    Refresh();
                }
            }        
        }

        if (ImGui::BeginChild("Special", { DirectoryExplorerWidth, lowerHeight }))
        {
            IDEFER(ImGui::EndChild());

            const Texture* folderTex = Datastore::GetTexture("Textures/WindowIcons/WindowIcon_AssetBrowser.png");

            const std::vector<std::filesystem::path> specialDirectories = IO::GetUserDirectories();

            for (const std::filesystem::path& specialDirectory : specialDirectories)
            {
                const std::string filename = specialDirectory.filename().string();

                if (folderTex != nullptr)
                {
                    ImGui::Image((ImTextureID)(uintptr_t)folderTex->GetHandle(), ImVec2(16.0f, 16.0f));

                    ImGui::SameLine();
                }

                const bool selected = m_path == specialDirectory;
                if (ImGui::Selectable(filename.c_str(), selected))
                {
                    SetPath(specialDirectory);

                    Refresh();
                }
            }
        }
    }
    
    ImGui::SameLine();

    size.x -= DirectoryExplorerWidth + style.FramePadding.x;

    const std::filesystem::path oldPath = m_path;
    if (m_directoryExplorer)
    {
        if (!FileDialog::DirectoryExplorer(m_dirs, &m_path, size))
        {
            m_prevPaths.emplace(oldPath);

            Refresh();
        }
    }
    else 
    {
        const char* filter = nullptr;
        if (m_filterCount > 0)
        {
            if (strcmp(m_filters[m_filterIndex], "*") != 0)
            {
                filter = m_filters[m_filterIndex];
            }
        }

        if (!FileDialog::FileExplorer(m_dirs, m_files, &m_path, &m_name, filter, size))
        {
            m_prevPaths.emplace(oldPath);

            Refresh();
        }
    }

    if (a_blockCallback)
    {
        ImGui::SameLine();

        ImGui::BeginGroup();
        IDEFER(ImGui::EndGroup());

        a_blockCallback();
    }

    const uint32_t nameLen = (uint32_t)m_name.length();
    if (nameLen > BufferSize)
    {
        m_name.clear();

        return FileDialogStatus_Error;
    }

    for (uint32_t i = 0; i < nameLen; ++i)
    {
        buffer[i] = m_name[i];
    }
    buffer[nameLen] = 0;

    if (ImGui::InputText("Name", buffer, BufferSize))
    {
        m_name = buffer;
    }

    ImGui::SameLine();

    if (m_filterCount > 0)
    {
        ImGui::PushItemWidth(64.0f);
        IDEFER(ImGui::PopItemWidth());

        if (ImGui::BeginCombo("##Filter", m_filters[m_filterIndex]))
        {
            IDEFER(ImGui::EndCombo());

            for (uint32_t i = 0; i < m_filterCount; ++i)
            {
                if (ImGui::Selectable(m_filters[i], m_filterIndex == i))
                {
                    m_filterIndex = i;
                }

                if (m_filterIndex == i)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }
        }

        ImGui::SameLine();
    }

    if (ImGui::Button("OK"))
    {
        if (m_name.empty() || !m_path.has_filename())
        {
            return FileDialogStatus_Error;
        }

        *a_outPath = m_path;
        *a_outString = m_name;

        return FileDialogStatus_Ok;
    }

    ImGui::SameLine();

    if (ImGui::Button("Cancel"))
    {
        return FileDialogStatus_Cancel;
    }

    return FileDialogStatus_None;
}