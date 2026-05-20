// Icarian Editor - Editor for the Icarian Game Engine
//
// License at end of file.

#include "Windows/WelcomeWindow.h"

#include <imgui.h>

#include "Core/IcarianDefer.h"
#include "Core/StringUtils.h"
#include "EditorData.h"
#include "FlareImGui.h"
#include "Project.h"

#define ICARIANEDITOR_VERSION_STRX(x) #x
#define ICARIANEDITOR_VERSION_STRI(x) ICARIANEDITOR_VERSION_STRX(x)
#define ICARIANEDITOR_VERSION_TAGSTR ICARIANEDITOR_VERSION_STRI(ICARIANEDITOR_VERSION_TAG)
#define ICARIANEDITOR_COMMIT_HASHSTR ICARIANEDITOR_VERSION_STRI(ICARIANEDITOR_COMMIT_HASH)

WelcomeWindow::WelcomeWindow(Project* a_project) : Window("Welcome")
{
    m_project = a_project;
}
WelcomeWindow::~WelcomeWindow()
{

}

static void Header()
{
    ImGui::BeginGroup();
    IDEFER(ImGui::EndGroup());

    FlareImGui::Image("./Textures/Icons/Logo_White_Large.png", glm::vec2(128.0f));

    ImGui::SameLine();

    {
        ImGui::BeginGroup();
        IDEFER(ImGui::EndGroup());

        // Shift the text down a bit
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 16);

        FlareImGui::Header("Icarian Editor");

        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::ColorConvertFloat4ToU32(ImVec4(0.5f, 0.5f, 0.5f, 1.0f)));

        constexpr uintmax_t TagHash = StringHash(ICARIANEDITOR_VERSION_TAGSTR);

        // Keep it simple for release versions
        if constexpr (TagHash == StringHash("REL"))
        {
            ImGui::Text("v%d.%d.%d", ICARIANEDITOR_VERSION_MAJOR, ICARIANEDITOR_VERSION_MINOR, ICARIANEDITOR_VERSION_PATCH);
        }
        else
        {
            // Compiler is being weird about else if with constexpr so just if else chaining

            // For non dev versions that are not release include the verion tag
            if constexpr (TagHash != StringHash("DEV"))
            {
                ImGui::Text
                (
                    "v%d.%d.%d %s",
                    ICARIANEDITOR_VERSION_MAJOR,
                    ICARIANEDITOR_VERSION_MINOR,
                    ICARIANEDITOR_VERSION_PATCH,
                    ICARIANEDITOR_VERSION_TAGSTR
                );
            }
            // Include everything for dev versions of the editor/engine
            else
            {
                ImGui::Text
                (
                    "v%d.%d.%d:%s %s",
                    ICARIANEDITOR_VERSION_MAJOR,
                    ICARIANEDITOR_VERSION_MINOR,
                    ICARIANEDITOR_VERSION_PATCH,
                    ICARIANEDITOR_COMMIT_HASHSTR,
                    ICARIANEDITOR_VERSION_TAGSTR
                );
            }
        }

        ImGui::PopStyleColor();
    }
}

void WelcomeWindow::Actions()
{
    ImGui::BeginGroup();
    IDEFER(ImGui::EndGroup());

    ImGui::Text("Actions");

    if (ImGui::Button("New Project"))
    {
        m_project->New();

        CloseWindow();
    }
    if (ImGui::Button("Open Project"))
    {
        m_project->Open();

        CloseWindow();
    }
}
void WelcomeWindow::RecentProjects()
{
    ImGui::BeginGroup();
    IDEFER(ImGui::EndGroup());

    ImGui::Text("Recent Projects");

    {
        ImGui::BeginChild("##RecentProjects");
        IDEFER(ImGui::EndChild());

        const uint32_t projectCount = EditorData::GetLastProjectCount();
        const std::string* projects = EditorData::GetLastProjectPaths();

        for (uint32_t i = 0; i < projectCount; ++i)
        {
            const std::string& str = projects[i];

            if (!std::filesystem::exists(str))
            {
                continue;
            }

            ImGui::Selectable(str.c_str());

            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
            {
                m_project->OpenProjectFolder(str);

                CloseWindow();
            }
        }
    }
}

void WelcomeWindow::DisplayUpdate(double a_delta)
{
    Header();

    Actions();

    ImGui::SameLine();

    RecentProjects();
}

// MIT License
//
// Copyright (c) 2026 River Govers
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
