// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#include "Modals/EditorConfigModal.h"

#include <imgui.h>

#include "Core/IcarianDefer.h"
#include "EditorConfig.h"
#include "FlareImGui.h"
#include "Logger.h"

static constexpr const char* EditorConfigTabNames[] =
{
    "General",
    "Key Bindings",
    "External Tools",
    "Engine"
};

#ifdef WIN32
constexpr static bool CodeEditorEnabled[] =
{
    true,   // Default
    true,   // Visual Studio
    true,   // VSCode
    true,   // VSCodium
    false,  // Kate - Unable to validate it works need someone to confirm before enabling
};

constexpr static bool DefEditorEnabled[] =
{
    true,   // Editor
    true,   // VSCode
    true,   // VSCodium
    false,  // Kate - Unable to validate it works need someone to confirm before enabling
};
#else
constexpr static bool CodeEditorEnabled[] =
{
    true,   // Default
    false,  // Visual Studio
    true,   // VSCode
    true,   // VSCodium
    true,   // Kate
};

constexpr static bool DefEditorEnabled[] =
{
    true,   // Editor
    true,   // VSCode
    true,   // VSCodium
    true,   // Kate
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
    FlareImGui::Label("Editor Unfocused FPS", LabelRatio);
    int editorUnfocusedFPS = (int)EditorConfig::GetEditorUnfocusedFPS();
    if (ImGui::DragInt("##EditorUnfocusedFPS", &editorUnfocusedFPS, 1.0f, 0, 500))
    {
        EditorConfig::SetEditorUnfocusedFPS((uint32_t)editorUnfocusedFPS);
    }

    FlareImGui::Label("Use Degrees", LabelRatio);
    bool useDegrees = EditorConfig::GetUseDegrees();
    if (ImGui::Checkbox("##UseDegrees", &useDegrees))
    {
        EditorConfig::SetUseDegrees(useDegrees);
    }

    FlareImGui::Label("Editor Mouse Sensitivity", LabelRatio);
    float editorMouseSensitivity = EditorConfig::GetEditorMouseSensitivity();
    if (ImGui::DragFloat("##EditorMouseSensitivity", &editorMouseSensitivity, 0.01f, 0.0f, 1.0f, "%.5f"))
    {
        EditorConfig::SetEditorMouseSensitivity(editorMouseSensitivity);
    }

    FlareImGui::Label("Background Color", LabelRatio);
    glm::vec4 backgroundColor = EditorConfig::GetBackgroundColor();
    if (ImGui::ColorEdit4("##BackgroundColor", (float*)&backgroundColor))
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

        FlareImGui::Label(keyBindName, LabelRatio);

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

    FlareImGui::Label("Code Editor", LabelRatio);

    const char* codeEditorStr = EditorConfig::GetCodeEditorName(codeEditor);
    if (ImGui::BeginCombo("##CodeEditor", codeEditorStr))
    {
        IDEFER(ImGui::EndCombo());

        for (uint32_t i = 0; i < CodeEditor_End; ++i)
        {
            const bool enabled = CodeEditorEnabled[i];
            if (!enabled)
            {
                continue;
            }

            const e_CodeEditor editor = (e_CodeEditor)i;

            const bool selected = codeEditor == i;
            const char* curEditorStr = EditorConfig::GetCodeEditorName(editor);
            if (ImGui::Selectable(curEditorStr, selected))
            {
                EditorConfig::SetCodeEditor(editor);
            }

            if (selected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
    }

    const e_DefEditor defEditor = EditorConfig::GetDefEditor();

    FlareImGui::Label("Def Editor", LabelRatio);

    const char* defEditorStr = EditorConfig::GetDefEditorName(defEditor);
    if (ImGui::BeginCombo("##DefEditor", defEditorStr))
    {
        IDEFER(ImGui::EndCombo());

        for (uint32_t i = 0; i < DefEditor_End; ++i)
        {
            const bool enabled = DefEditorEnabled[i];
            if (!enabled)
            {
                continue;
            }

            const e_DefEditor editor = (e_DefEditor)i;

            const bool selected = defEditor == i;
            const char* curEditorStr = EditorConfig::GetDefEditorName(editor);
            if (ImGui::Selectable(curEditorStr, selected))
            {
                EditorConfig::SetDefEditor(editor);
            }

            if (selected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
    }
}

void EditorConfigModal::EngineTab()
{
    FlareImGui::Label("Engine Shutdown Timeout", LabelRatio);
    float engineShutdownTimeout = EditorConfig::GetEngineShutdownTimeout();
    if (ImGui::DragFloat("##EngineShutdownTimeout", &engineShutdownTimeout, 0.1f, 1.0f))
    {
        EditorConfig::SetEngineShutdownTimeout(engineShutdownTimeout);
    }

    FlareImGui::Label("Engine Pipe Timeout", LabelRatio);
    float enginePipeTimeout = EditorConfig::GetEnginePipeTimeout();
    if (ImGui::DragFloat("##EnginePipeTimeout", &enginePipeTimeout, 0.01f, 1.0f))
    {
        EditorConfig::SetEnginePipeTimeout(enginePipeTimeout);
    }
}

bool EditorConfigModal::Update()
{
    if (ImGui::BeginChild("##Tabs", ImVec2(110.0f, 230.0f)))
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

        ImGui::BeginChild("##Settings", ImVec2(0.0f, 230.0f));
        IDEFER(ImGui::EndChild());

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
        case EditorConfigTab_Engine:
        {
            EngineTab();

            break;
        }
        default:
        {
            Logger::Error("Invalid EditorConfigTab: " + std::to_string(m_currentTab));

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