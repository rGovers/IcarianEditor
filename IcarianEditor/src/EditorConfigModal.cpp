#include "Modals/EditorConfigModal.h"

#include <imgui.h>

#include "Core/IcarianDefer.h"
#include "EditorConfig.h"
#include "Logger.h"

static constexpr const char* EditorConfigTabNames[] =
{
    "General",
    "Key Bindings",
    "External Tools"
};
static constexpr const char* CodeEditorNames[] =
{
    "Default",
    "Visual Studio",
    "Visual Studio Code"
};

static constexpr const char* DefEditorNames[] =
{
    "Editor",
    "Visual Studio Code"
};

#if WIN32
constexpr static bool CodeEditorEnabled[] =
{
    true,
    true,
    true
};
#else
constexpr static bool CodeEditorEnabled[] =
{
    true,
    false,
    true
};
#endif

EditorConfigModal::EditorConfigModal() : Modal("Config", glm::vec2(450.0f, 300.0f))
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

    ImGui::SetNextItemWidth(ItemWidth);
    float editorMouseSensitivity = EditorConfig::GetEditorMouseSensitivity();
    if (ImGui::DragFloat("Editor Mouse Sensitivity", &editorMouseSensitivity, 0.01f, 0.0f, 1.0f, "%.5f"))
    {
        EditorConfig::SetEditorMouseSensitivity(editorMouseSensitivity);
    }
    
    ImGui::SetNextItemWidth(ItemWidth);
    glm::vec4 backgroundColor = EditorConfig::GetBackgroundColor();
    if (ImGui::ColorEdit4("Background Color", (float*)&backgroundColor))
    {
        EditorConfig::SetBackgroundColor(backgroundColor);
    }
}

void EditorConfigModal::KeyBindingsTab()
{
    for (uint32_t i = KeyBindTarget_Start; i < KeyBindTarget_End; ++i)
    {
        const e_KeyBindTarget keyBindTarget = (e_KeyBindTarget)i;

        const ImGuiKey keyBind = EditorConfig::GetKeyBind(keyBindTarget);
        const char* keyBindName = EditorConfig::GetKeyBindName(keyBindTarget);

        ImGui::PushID(keyBindName);
        IDEFER(ImGui::PopID());

        ImGui::Text("%s", keyBindName);

        ImGui::SameLine();

        if (m_keyBindTarget == keyBindTarget)
        {
            if (ImGui::Button("Press Any Key"))
            {
                m_keyBindTarget = KeyBindTarget_Null;
            }

            for (int j = ImGuiKey_NamedKey_BEGIN; j < ImGuiKey_NamedKey_END; ++j)
            {
                const ImGuiKey key = (ImGuiKey)j;
                if (ImGui::IsKeyPressed(key))
                {
                    EditorConfig::SetKeyBind(keyBindTarget, key);

                    m_keyBindTarget = KeyBindTarget_Null;

                    break;
                }
            }
        }
        else
        {
            const char* keyName = ImGui::GetKeyName(keyBind);

            if (ImGui::Button(keyName))
            {
                m_keyBindTarget = keyBindTarget;
            }
        }
    }
}

void EditorConfigModal::ExternalToolsTab()
{
    const e_CodeEditor codeEditor = EditorConfig::GetCodeEditor();

    ImGui::SetNextItemWidth(ItemWidth);
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

    const e_DefEditor defEditor = EditorConfig::GetDefEditor();

    ImGui::SetNextItemWidth(ItemWidth);
    if (ImGui::BeginCombo("Def Editor", DefEditorNames[defEditor]))
    {
        IDEFER(ImGui::EndCombo());

        for (uint32_t i = 0; i < DefEditor_End; ++i)
        {
            const bool selected = defEditor == i;

            if (ImGui::Selectable(DefEditorNames[i], selected))
            {
                EditorConfig::SetDefEditor((e_DefEditor)i);
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
        case EditorConfigTab_KeyBindings:
        {
            KeyBindingsTab();

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