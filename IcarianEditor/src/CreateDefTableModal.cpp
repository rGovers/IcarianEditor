#include "Modals/CreateDefTableModal.h"

#include <fstream>
#include <imgui.h>

#include "AppMain.h"
#include "Flare/IcarianDefer.h"
#include "Modals/ErrorModal.h"
#include "Project.h"
#include "TemplateBuilder.h"
#include "Templates/DefTableCS.h"

CreateDefTableModal::CreateDefTableModal(AppMain* a_app, Project* a_project, const std::filesystem::path& a_path) : Modal("Create Def Table")
{
    m_app     = a_app;
    m_project = a_project;

    m_path    = a_path;

    memset(m_name, 0, BufferSize);
}
CreateDefTableModal::~CreateDefTableModal()
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

bool CreateDefTableModal::Update()
{
    ImGui::InputText("Name", m_name, BufferSize);

    if (ImGui::Button("Create"))
    {
        if (m_name[0] == 0)
        {
            m_app->PushModal(new ErrorModal("Def Table name cannot be empty!"));

            return true;
        }

        if (!IsAlphaNumeric(m_name))
        {
            m_app->PushModal(new ErrorModal("Def Table name must be alphanumeric!"));

            return true;
        }

        std::filesystem::path path = m_path / m_name;

        const std::filesystem::path ext = path.extension();
        if (ext != ".cs")
        {
            path += ".cs";
        }

        const std::string projectName = m_project->GetName();

        const std::string str = TemplateBuilder::GenerateFromTemplate(DEFTABLETEMPLATECS, projectName, m_name);

        std::ofstream file = std::ofstream(path, std::ios::binary);
        if (file.good() && file.is_open())
        {
            ICARIAN_DEFER_closeOFile(file);
            file.write(str.c_str(), str.length()); 

            m_project->SetRefresh(true);

            return false;
        }
        else 
        {
            m_app->PushModal(new ErrorModal("Failed to create Def Table!"));
        }
    }

    ImGui::SameLine();

    if (ImGui::Button("Cancel"))
    {
        return false;
    }

    return true;
}