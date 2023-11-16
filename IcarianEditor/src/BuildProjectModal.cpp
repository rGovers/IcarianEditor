#include "Modals/BuildProjectModal.h"

#include <imgui.h>

#include "AppMain.h"
#include "FileDialogBlock.h"
#include "Flare//IcarianDefer.h"
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

    m_selectedExport = 0;

    m_fileDialogBlock = new FileDialogBlock(glm::vec2(440.0f, -1.0f), true);
}
BuildProjectModal::~BuildProjectModal()
{
    delete m_fileDialogBlock;
}

void BuildProjectModal::OptionsDisplay()
{
    ImGui::Text("%s", "Build Options");

    ImGui::Separator();

    const std::string defaultOption = m_exportOptions[m_selectedExport];

    ImGui::Text("%s", "Platform");
    
    ImGui::PushItemWidth(128.0f);
    IDEFER(ImGui::PopItemWidth());

    if (ImGui::BeginCombo("##Platform", defaultOption.c_str()))
    {
        IDEFER(ImGui::EndCombo());

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
    }
}

bool BuildProjectModal::Update()
{
    if (m_exportOptions.empty())
    {
        m_app->PushModal(new ErrorModal("No export options available"));

        return false;
    }

    std::filesystem::path path;
    std::string name;

    const e_FileDialogStatus status = m_fileDialogBlock->ShowFileDialog(&path, &name, std::bind(&BuildProjectModal::OptionsDisplay, this));

    switch (status)
    {
    case FileDialogStatus_Ok:
    {
        if (!std::filesystem::exists(path))
        {
            m_app->PushModal(new ErrorModal("Directory does not exist"));

            return true;
        }

        if (name.empty())
        {
            m_app->PushModal(new ErrorModal("Invalid Name"));

            return true;
        }

        LoadingTask* tasks[] = 
        {
            new GenerateConfigLoadingTask(path, name, "Vulkan"),
            new BuildLoadingTask(path, m_exportOptions[m_selectedExport], m_project),
            new SerializeAssetsLoadingTask(path, m_library),
            new CopyBuildLibraryLoadingTask(path, name, m_exportOptions[m_selectedExport])
        };

        m_app->PushModal(new LoadingModal(tasks, sizeof(tasks) / sizeof(*tasks)));

        return false;
    }
    case FileDialogStatus_Error:
    {
        m_app->PushModal(new ErrorModal("File dialog error"));

        break;
    }
    case FileDialogStatus_Cancel:
    {
        return false;
    }
    default:
    {
        break;
    }
    }

    return true;
}