#include "Modals/OpenProjectModal.h"

#include <imgui.h>

#include "AppMain.h"
#include "FileDialog.h"
#include "IO.h"
#include "Modals/ErrorModal.h"

OpenProjectModal::OpenProjectModal(AppMain* a_app, Callback a_callback) : Modal("Open Project", glm::vec2(640, 480))
{
    m_app = a_app;

    m_callback = a_callback;

    m_path = IO::GetHomePath();
    m_name = "";

    FileDialog::GenerateFileDirs(&m_dirs, &m_files, m_path);
}
OpenProjectModal::~OpenProjectModal()
{

}

bool OpenProjectModal::Update()
{
    char buffer[BufferSize];

    const std::string pathStr = m_path.string();
    const uint32_t pathLen = (uint32_t)pathStr.length();
    if (pathLen > BufferSize)
    {
        m_app->PushModal(new ErrorModal("Path exceeds buffer size"));

        m_path = IO::GetHomePath();
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
        m_files.clear();

        FileDialog::GenerateFileDirs(&m_dirs, &m_files, m_path);
    }

    if (!FileDialog::FileExplorer(m_dirs, m_files, &m_path, &m_name))
    {
        m_dirs.clear();
        m_files.clear();

        FileDialog::GenerateFileDirs(&m_dirs, &m_files, m_path);
    }

    const uint32_t nameLen = (uint32_t)m_name.length();
    if (nameLen > BufferSize)
    {
        m_app->PushModal(new ErrorModal("Name exceeds buffer size"));

        m_name.clear();
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
    
    if (ImGui::Button("Open"))
    {
        if (!std::filesystem::exists(m_path))
        {
            m_app->PushModal(new ErrorModal("Directory does not exist"));

            return true;
        }

        if (m_name.empty())
        {
            m_app->PushModal(new ErrorModal("Invalid Name"));

            return true;
        }

        if (!std::filesystem::exists(m_path / m_name))
        {
            if (!std::filesystem::exists(m_path / (m_name + ".flareproj")))
            {
                m_app->PushModal(new ErrorModal("File does not exist"));

                return true;
            }
        }

        m_callback(m_path, m_name);

        return false;
    }

    ImGui::SameLine();

    if (ImGui::Button("Cancel"))
    {
        return false;
    }

    return true;
}