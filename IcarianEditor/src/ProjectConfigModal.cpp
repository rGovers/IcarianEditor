#include "Modals/ProjectConfigModal.h"

#include <imgui.h>

#include "Core/IcarianDefer.h"
#include "Project.h"

ProjectConfigModal::ProjectConfigModal(Project* a_project) : Modal("Project Config", glm::vec2(450.0f, 300.0f))
{
    m_project = a_project;
}
ProjectConfigModal::~ProjectConfigModal()
{

}

bool ProjectConfigModal::Update()
{
    if (ImGui::BeginChild("##Tabs", ImVec2(100.0f, 230.0f)))
    {
        IDEFER(ImGui::EndChild());

        // TODO: Implement this when we have more tabs
        if (ImGui::Selectable("Flags"))
        {

        }
    }

    {
        ImGui::SameLine();

        ImGui::BeginGroup();
        IDEFER(ImGui::EndGroup());

        // TODO: Move this when we have more tabs
        bool ktxConvert = m_project->ConvertKTX();
        if (ImGui::Checkbox("Convert Images to KTX", &ktxConvert))
        {
            m_project->SetConvertKTX(ktxConvert);
        }
    }

    if (ImGui::Button("Apply"))
    {
        return false;
    }

    ImGui::SameLine();

    if (ImGui::Button("Cancel"))
    {
        m_project->ReloadProjectFile();

        return false;
    }

    return true;
}