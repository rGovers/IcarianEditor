#include "Modals/EditorConfigModal.h"

#include <imgui.h>

#include "EditorConfig.h"
#include "Flare/IcarianDefer.h"
#include "Logger.h"

static constexpr char* EditorConfigTabNames[] =
{
    "General",
    "External Tools"
};
static constexpr char* CodeEditorNames[] =
{
    "Default",
    "Visual Studio",
    "Visual Studio Code"
};

#if WIN32
static contexpr bool CodeEditorEnabled[] =
{
    true,
    true,
    true
};
#else
static constexpr bool CodeEditorEnabled[] =
{
    true,
    false,
    true
};
#endif

EditorConfigModal::EditorConfigModal() : Modal("Config", glm::vec2(400.0f, 300.0f))
{
    m_currentTab = EditorConfigTab_General;
}
EditorConfigModal::~EditorConfigModal()
{

}

void EditorConfigModal::GeneralTab()
{
    bool useDegrees = EditorConfig::GetUseDegrees();

    if (ImGui::Checkbox("Use Degrees", &useDegrees))
    {
        EditorConfig::SetUseDegrees(useDegrees);
    }
}
void EditorConfigModal::ExternalToolsTab()
{
    const e_CodeEditor codeEditor = EditorConfig::GetCodeEditor();

    ImGui::SetNextItemWidth(200.0f);
    if (ImGui::BeginCombo("Code Editor", CodeEditorNames[codeEditor]))
    {
        IDEFER(ImGui::EndCombo());

        for (uint32_t i = 0; i < CodeEditor_End; ++i)
        {
            const bool enabled = CodeEditorEnabled[i];
            if (!enabled)
            {
                continue;
            }

            const bool selected = codeEditor == i;

            if (ImGui::Selectable(CodeEditorNames[i], selected))
            {
                EditorConfig::SetCodeEditor((e_CodeEditor)i);
            }

            if (selected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
    }
}

bool EditorConfigModal::Update()
{
    if (ImGui::BeginChild("##Tabs", ImVec2(100.0f, 240.0f), true))
    {
        IDEFER(ImGui::EndChild());

        for (uint32_t i = 0; i < EditorConfigTab_End; ++i)
        {
            if (ImGui::Selectable(EditorConfigTabNames[i], m_currentTab == i))
            {
                m_currentTab = (e_EditorConfigTab)i;
            }
        }
    }

    {
        ImGui::SameLine();

        ImGui::BeginGroup();
        IDEFER(ImGui::EndGroup());

        switch (m_currentTab) 
        {
        case EditorConfigTab_General:
        {
            GeneralTab();   

            break;
        }
        case EditorConfigTab_ExternalTools:
        {
            ExternalToolsTab();

            break;
        }
        default:
        {
            Logger::Error("Invalid EditorConfigTab: %d", m_currentTab);

            break;
        }
        }
    }
    

    if (ImGui::Button("Apply"))
    {
        EditorConfig::Serialize();

        return false;
    }

    ImGui::SameLine();

    if (ImGui::Button("Cancel"))
    {
        EditorConfig::Deserialize();

        return false;
    }

    return true;
}