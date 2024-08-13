// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

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