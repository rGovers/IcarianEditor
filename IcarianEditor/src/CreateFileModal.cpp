#include "Modals/CreateFileModal.h"

#include <fstream>
#include <imgui.h>

#include "AppMain.h"
#include "Core/IcarianDefer.h"
#include "IO.h"
#include "Modals/ErrorModal.h"
#include "Project.h"

CreateFileModal::CreateFileModal(AppMain* a_app, Project* a_project, const std::filesystem::path& a_path, const char* a_data, uint32_t a_size, const std::string_view& a_name, const std::string_view& a_extension) : Modal("Create File")
{
    m_app = a_app;
    m_project = a_project;

    m_extension = std::string(a_extension);

    m_size = a_size;

    m_path = a_path;

    uint32_t length = (uint32_t)a_name.length();
    if (length >= BufferSize)
    {
        length = BufferSize - 1;
    }

    memset(m_name, 0, BufferSize);
    for (uint32_t i = 0; i < length; ++i)
    {
        m_name[i] = a_name[i];
    }

    m_data = new char[m_size];
    for (uint32_t i = 0; i < m_size; ++i)
    {
        m_data[i] = a_data[i];
    }
}
CreateFileModal::~CreateFileModal()
{
    if (m_data != nullptr)
    {
        delete[] m_data;
    }
}

bool CreateFileModal::Update()
{
    ImGui::InputText("Name", m_name, BufferSize);

    if (ImGui::Button("Create"))
    {
        if (m_name[0] == 0)
        {
            m_app->PushModal(new ErrorModal("File name cannot be empty!"));

            return true;
        }

        if (!IO::ValidatePathName(m_name))
        {
            m_app->PushModal(new ErrorModal("Invalid file name!"));

            return true;
        }

        std::filesystem::path path = m_path / m_name;

        const std::filesystem::path ext = path.extension();
        if (ext.empty() || ext != m_extension)
        {
            path += m_extension;
        }

        std::ofstream file = std::ofstream(path, std::ios::binary);
        if (file.good() && file.is_open())
        {
            IDEFER(file.close());
            // ICARIAN_DEFER_closeOFile(file);
            file.write(m_data, m_size);

            m_project->SetRefresh(true);

            return false;
        }
        else 
        {
            m_app->PushModal(new ErrorModal("Failed to create file!"));
        }
    }

    ImGui::SameLine();

    if (ImGui::Button("Cancel"))
    {
        return false;
    }

    return true;
}