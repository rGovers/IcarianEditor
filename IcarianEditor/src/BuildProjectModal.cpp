#include "Modals/BuildProjectModal.h"

#include <imgui.h>

#include "AppMain.h"
#include "FileDialog.h"
#include "IO.h"
#include "LoadingTasks/BuildLoadingTask.h"
#include "LoadingTasks/CopyBuildLibraryLoadingTask.h"
#include "LoadingTasks/GenerateConfigLoadingTask.h"
#include "LoadingTasks/SerializeAssetsLoadingTask.h"
#include "Modals/ErrorModal.h"
#include "Modals/LoadingModal.h"

BuildProjectModal::BuildProjectModal(AppMain* a_app, AssetLibrary* a_library, Project* a_project, const std::vector<std::string>& a_exportOptions) : Modal("Build Project", glm::vec2(640, 480))
{
    m_app = a_app;
    m_library = a_library;
    m_project = a_project;

    m_exportOptions = a_exportOptions;

    m_path = IO::GetHomePath();
    m_name = "";
    m_selectedExport = 0;

    FileDialog::GenerateDirs(&m_dirs, m_path);
}
BuildProjectModal::~BuildProjectModal()
{

}

void BuildProjectModal::OptionsDisplay()
{
    ImGui::BeginGroup();

    ImGui::Text("%s", "Build Options");

    ImGui::Separator();

    const std::string defaultOption = m_exportOptions[m_selectedExport];

    ImGui::Text("%s", "Platform");
    if (ImGui::BeginCombo("##Platform", defaultOption.c_str()))
    {
        for (uint32_t i = 0; i < m_exportOptions.size(); ++i)
        {
            if (ImGui::Selectable(m_exportOptions[i].c_str()))
            {
                m_selectedExport = i;
            }

            if (i == m_selectedExport)
            {
                ImGui::SetItemDefaultFocus();
            }
        }

        ImGui::EndCombo();
    }

    ImGui::EndGroup();
}

bool BuildProjectModal::Update()
{
    if (m_exportOptions.empty())
    {
        m_app->PushModal(new ErrorModal("No export options available"));

        return false;
    }

    char buffer[BufferSize];

    std::string pathStr = m_path.string();
    uint32_t pathLen = (uint32_t)pathStr.length();
    if (pathLen > BufferSize)
    {
        m_app->PushModal(new ErrorModal("Path exceeds buffer size"));

        m_path = IO::GetHomePath();
        pathStr = m_path.string();

        pathLen = (uint32_t)pathStr.length();
    }

    for (uint32_t i = 0; i < pathLen; ++i)
    {
        buffer[i] = pathStr[i];
    }
    buffer[pathLen] = 0;

    if (ImGui::InputText("Path", buffer, BufferSize))
    {
        m_path = buffer;

        m_dirs.clear();

        FileDialog::GenerateDirs(&m_dirs, m_path);
    }

    if (!FileDialog::DirectoryExplorer(m_dirs, &m_path, glm::vec2(400.0f, -1.0f)))
    {
        m_dirs.clear();

        FileDialog::GenerateDirs(&m_dirs, m_path);
    }

    ImGui::SameLine();

    OptionsDisplay();

    uint32_t nameLen = (uint32_t)m_name.length();
    if (nameLen > BufferSize)
    {
        m_app->PushModal(new ErrorModal("Name exceeds buffer size"));

        m_name.clear();

        nameLen = 0;
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

    if (ImGui::Button("Build"))
    {
        if (!std::filesystem::exists(m_path))
        {
            m_app->PushModal(new ErrorModal("Path does not exist"));

            return true;
        }

        if (m_name.empty())
        {
            m_app->PushModal(new ErrorModal("Name is empty"));

            return true;
        }

        LoadingTask* tasks[] = 
        {
            new GenerateConfigLoadingTask(m_path, m_name, "Vulkan"),
            new BuildLoadingTask(m_path, m_exportOptions[m_selectedExport], m_project),
            new SerializeAssetsLoadingTask(m_path, m_library),
            new CopyBuildLibraryLoadingTask(m_path, m_name, m_exportOptions[m_selectedExport])
        };

        m_app->PushModal(new LoadingModal(tasks, sizeof(tasks) / sizeof(*tasks)));

        return false;
    }

    ImGui::SameLine();

    if (ImGui::Button("Cancel"))
    {
        return false;
    }

    return true;
}