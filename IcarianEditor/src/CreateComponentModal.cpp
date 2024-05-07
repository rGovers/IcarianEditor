#include "Modals/CreateComponentModal.h"

#include <fstream>
#include <imgui.h>

#include "AppMain.h"
#include "Modals/ErrorModal.h"
#include "Project.h"
#include "TemplateBuilder.h"
#include "Templates.h"

CreateComponentModal::CreateComponentModal(AppMain* a_app, Project* a_project, const std::filesystem::path& a_path) : Modal("Create Component")
{
    m_app     = a_app;
    m_project = a_project;

    m_path    = a_path;

    m_name[0] = 0;
}
CreateComponentModal::~CreateComponentModal()
{

}

static bool IsAlphaNumeric(const std::string_view& a_str)
{
    for (const char c : a_str)
    {
        if (!isalnum(c))
        {
            return false;
        }
    }

    return true;
}

bool CreateComponentModal::Update()
{
    ImGui::InputText("Name", m_name, BufferSize);

    if (ImGui::Button("Create"))
    {
        if (m_name[0] == 0)
        {
            m_app->PushModal(new ErrorModal("Name cannot be empty!"));

            return true;
        }

        if (!IsAlphaNumeric(m_name))
        {
            m_app->PushModal(new ErrorModal("Name must be alphanumeric!"));

            return true;
        }

        std::filesystem::path path = m_path / m_name;

        const std::filesystem::path ext = path.extension();
        if (ext != ".cs")
        {
            path += ".cs";
        }

        const std::string projectName = m_project->GetName();

        const std::string str = TemplateBuilder::GenerateFromTemplate(ComponentTemplate, projectName, m_name);

        std::ofstream file = std::ofstream(path, std::ios::binary);
        if (file.good() && file.is_open())
        {
            file.write(str.c_str(), str.length());

            m_project->SetRefresh(true);

            return false;
        }
        else
        {
            m_app->PushModal(new ErrorModal("Failed to create component!"));
        }
    }   

    ImGui::SameLine();

    if (ImGui::Button("Cancel"))
    {
        return false;
    }

    return true;
}
