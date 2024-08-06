#include "Modals/CreateEmptyScriptModal.h"

#include <fstream>
#include <imgui.h>

#include "AppMain.h"
#include "Core/IcarianDefer.h"
#include "Core/StringUtils.h"
#include "Modals/ErrorModal.h"
#include "Project.h"
#include "TemplateBuilder.h"
#include "Templates.h"

CreateEmptyScriptModal::CreateEmptyScriptModal(AppMain* a_app, Project* a_project, const std::filesystem::path& a_path) : Modal("Create Empty Script")
{
    m_app     = a_app;
    m_project = a_project;

    m_path    = a_path;

    m_name[0] = 0;
}
CreateEmptyScriptModal::~CreateEmptyScriptModal()
{

}

bool CreateEmptyScriptModal::Update()
{
    ImGui::InputText("Name", m_name, BufferSize);

    if (ImGui::Button("Create"))
    {
        if (m_name[0] == 0)
        {
            m_app->PushModal(new ErrorModal("Empty Script name cannot be empty!"));

            return true;
        }

        if (!IsAlphaNumeric(m_name))
        {
            m_app->PushModal(new ErrorModal("Empty Script name must be alphanumeric!"));

            return true;
        }

        std::filesystem::path path = m_path / m_name;

        const std::filesystem::path ext = path.extension();
        if (ext != ".cs")
        {
            path += ".cs";
        }

        const std::string projectName = m_project->GetName();

        const std::string str = TemplateBuilder::GenerateFromTemplate(EmptyScriptTemplate, projectName, m_name);

        std::ofstream file = std::ofstream(path, std::ios::binary);
        if (file.good() && file.is_open())
        {
            IDEFER(file.close());
            file.write(str.c_str(), str.length()); 

            m_project->SetRefresh(true);

            return false;
        }
        else 
        {
            m_app->PushModal(new ErrorModal("Failed to create Empty Script!"));
        }
    }

    ImGui::SameLine();

    if (ImGui::Button("Cancel"))
    {
        return false;
    }

    return true;
}