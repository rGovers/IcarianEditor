// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#include "Modals/RenamePathModal.h"

#include <imgui.h>

#include "AppMain.h"
#include "IO.h"
#include "Modals/ErrorModal.h"

RenamePathModal::RenamePathModal(AppMain* a_app, Project* a_project, const std::filesystem::path& a_path) : Modal("Rename Path")
{
    m_app = a_app;
    m_project = a_project;

    m_path = a_path;

    memset(m_name, 0, BufferSize);

    const std::string name = m_path.filename().string();
    uint32_t length = (uint32_t)name.length();

    if (length >= BufferSize)
    {
        length = BufferSize - 1;
    }

    for (uint32_t i = 0; i < length; ++i)
    {
        m_name[i] = name[i];
    }
}
RenamePathModal::~RenamePathModal()
{

}

bool RenamePathModal::Update()
{
    ImGui::Text("Rename %s", m_path.string().c_str());

    ImGui::InputText("Name", m_name, BufferSize);

    if (ImGui::Button("Rename"))
    {
        if (m_name[0] == 0)
        {
            m_app->PushModal(new ErrorModal("Name cannot be empty!"));

            return true;
        }

        if (!IO::ValidatePathName(m_name))
        {
            m_app->PushModal(new ErrorModal("Invalid name!"));

            return true;
        }

        std::filesystem::path newPath = m_path.parent_path() / m_name;
        if (m_path.has_extension())
        {
            newPath += m_path.extension();
        }

        std::filesystem::rename(m_path, newPath);

        // TODO: Implement partial refresh
        m_project->SetRefresh(true);

        return false;
    }

    ImGui::SameLine();

    if (ImGui::Button("Cancel"))
    {
        return false;
    }

    return true;
}

// MIT License
// 
// Copyright (c) 2024 River Govers
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.