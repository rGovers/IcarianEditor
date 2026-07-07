// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#include "EditorConfig.h"

#include <filesystem>
#include <tinyxml2.h>

#include "Core/StringUtils.h"
#include "Runtime/RuntimeManager.h"

static EditorConfig* Instance = nullptr;

#define EDITORCONFIG_BINDING_FUNCTION_TABLE(F) \
    F(uint32_t, IcarianEditor, EditorConfig, GetUseDegrees, { return (uint32_t)EditorConfig::GetUseDegrees(); }) \

EDITORCONFIG_BINDING_FUNCTION_TABLE(RUNTIME_FUNCTION_DEFINITION);

#define I_INTER_KEYBINDTARGET_SETDEFAULT(val, def) m_keyBinds[KeyBindTarget_##val] = def;

#define I_INTER_CODEEDITOR_LOAD(val) case StringHash(#val): { Instance->m_codeEditor = CodeEditor_##val; break; }
#define I_INTER_CODEEDITOR_SAVE(val) case CodeEditor_##val: { codeEditor->SetText(#val); break; }

#define I_INTER_DEFEDITOR_LOAD(val) case StringHash(#val): { Instance->m_defEditor = DefEditor_##val; break; }
#define I_INTER_DEFEDITOR_SAVE(val) case DefEditor_##val: { defEditor->SetText(#val); break; }

EditorConfig::EditorConfig()
{
    I_INTER_KEYBINDTARGETTABLE(I_INTER_KEYBINDTARGET_SETDEFAULT);
}
EditorConfig::~EditorConfig()
{
    Serialize();
}

void EditorConfig::Deserialize()
{
    if (!std::filesystem::exists(ConfigFile))
    {
        return;
    }

    tinyxml2::XMLDocument doc;
    if (doc.LoadFile(ConfigFile) != tinyxml2::XML_SUCCESS)
    {
        return;
    }

    tinyxml2::XMLElement* root = doc.FirstChildElement("Config");
    if (root == nullptr)
    {
        return;
    }

    for (const tinyxml2::XMLElement* element = root->FirstChildElement(); element != nullptr; element = element->NextSiblingElement())
    {
        const char* name = element->Name();

        switch (StringHash(name))
        {
        case StringHash("EditorUnfocusedFPS"):
        {
            Instance->m_editorUnfocusedFPS = (uint32_t)element->UnsignedText();

            break;
        }
        case StringHash("UseDegrees"):
        {
            Instance->m_useDegrees = element->BoolText();

            break;
        }
        case StringHash("EngineShutdownTimeout"):
        {
            Instance->m_engineShutdownTimeout = element->FloatText();

            break;
        }
        case StringHash("EnginePipeTimeout"):
        {
            Instance->m_enginePipeTimeout = element->FloatText();

            break;
        }
        case StringHash("BackgroundColor"):
        {
            for (const tinyxml2::XMLElement* colorElement = element->FirstChildElement(); colorElement != nullptr; colorElement = colorElement->NextSiblingElement())
            {
                const char* colorName = colorElement->Name();

                switch (StringHash<uint32_t>(colorName))
                {
                case StringHash<uint32_t>("R"):
                {
                    Instance->m_backgroundColor.r = colorElement->FloatText();

                    break;
                }
                case StringHash<uint32_t>("G"):
                {
                    Instance->m_backgroundColor.g = colorElement->FloatText();

                    break;
                }
                case StringHash<uint32_t>("B"):
                {
                    Instance->m_backgroundColor.b = colorElement->FloatText();

                    break;
                }
                case StringHash<uint32_t>("A"):
                {
                    Instance->m_backgroundColor.a = colorElement->FloatText();

                    break;
                }
                }
            }

            break;
        }
        case StringHash("EditorMouseSensitivity"):
        {
            Instance->m_editorMouseSensitivity = element->FloatText();

            break;
        }
        case StringHash("CodeEditor"):
        {
            const char* codeEditor = element->GetText();

            switch (StringHash(codeEditor))
            {
            I_INTER_CODEEDITORTABLE(I_INTER_CODEEDITOR_LOAD)
            }

            break;
        }
        case StringHash("DefEditor"):
        {
            const char* defEditor = element->GetText();

            switch (StringHash(defEditor))
            {
            I_INTER_DEFEDITORTABLE(I_INTER_DEFEDITOR_LOAD)
            }

            break;
        }
        default:
        {
            for (uint32_t i = KeyBindTarget_Start; i < KeyBindTarget_End; ++i)
            {
                const char* keyBindStr = GetKeyBindName((e_KeyBindTarget)i);

                const std::string keyBindName = std::string(keyBindStr) + "Key";
                if (keyBindName == name)
                {
                    Instance->m_keyBinds[i] = (ImGuiKey)element->IntText();

                    break;
                }
            }

            break;
        }
        }
    }
}
void EditorConfig::Serialize()
{
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLDeclaration* decl = doc.NewDeclaration();
    doc.InsertFirstChild(decl);

    tinyxml2::XMLElement* root = doc.NewElement("Config");
    doc.InsertEndChild(root);

    tinyxml2::XMLElement* editorUnfocusedFPS = doc.NewElement("EditorUnfocusedFPS");
    editorUnfocusedFPS->SetText(Instance->m_editorUnfocusedFPS);
    root->InsertEndChild(editorUnfocusedFPS);

    tinyxml2::XMLElement* useDegrees = doc.NewElement("UseDegrees");
    useDegrees->SetText(Instance->m_useDegrees);
    root->InsertEndChild(useDegrees);

    tinyxml2::XMLElement* engineShutdownTimeout = doc.NewElement("EngineShutdownTimeout");
    engineShutdownTimeout->SetText(Instance->m_engineShutdownTimeout);
    root->InsertEndChild(engineShutdownTimeout);

    tinyxml2::XMLElement* pipeTimeout = doc.NewElement("EnginePipeTimeout");
    pipeTimeout->SetText(Instance->m_enginePipeTimeout);
    root->InsertEndChild(pipeTimeout);

    tinyxml2::XMLElement* backgroundColor = doc.NewElement("BackgroundColor");
    root->InsertEndChild(backgroundColor);
    {
        tinyxml2::XMLElement* r = doc.NewElement("R");
        r->SetText(Instance->m_backgroundColor.r);
        backgroundColor->InsertEndChild(r);

        tinyxml2::XMLElement* g = doc.NewElement("G");
        g->SetText(Instance->m_backgroundColor.g);
        backgroundColor->InsertEndChild(g);

        tinyxml2::XMLElement* b = doc.NewElement("B");
        b->SetText(Instance->m_backgroundColor.b);
        backgroundColor->InsertEndChild(b);

        tinyxml2::XMLElement* a = doc.NewElement("A");
        a->SetText(Instance->m_backgroundColor.a);
        backgroundColor->InsertEndChild(a);
    }

    tinyxml2::XMLElement* editorMouseSensitivity = doc.NewElement("EditorMouseSensitivity");
    editorMouseSensitivity->SetText(Instance->m_editorMouseSensitivity);
    root->InsertEndChild(editorMouseSensitivity);

    tinyxml2::XMLElement* codeEditor = doc.NewElement("CodeEditor");
    switch (Instance->m_codeEditor)
    {
    I_INTER_CODEEDITORTABLE(I_INTER_CODEEDITOR_SAVE)
    default:
    {
        codeEditor->SetText("Default");

        break;
    }
    }
    root->InsertEndChild(codeEditor);

    tinyxml2::XMLElement* defEditor = doc.NewElement("DefEditor");
    switch (Instance->m_defEditor)
    {
    I_INTER_DEFEDITORTABLE(I_INTER_DEFEDITOR_SAVE)
    default:
    {
        defEditor->SetText("Editor");

        break;
    }
    }
    root->InsertEndChild(defEditor);

    for (uint32_t i = KeyBindTarget_Start; i < KeyBindTarget_End; ++i)
    {
        const char* keyBindStr = GetKeyBindName((e_KeyBindTarget)i);

        const std::string keyBindName = std::string(keyBindStr) + "Key";

        tinyxml2::XMLElement* keyBind = doc.NewElement(keyBindName.c_str());
        keyBind->SetText((int)Instance->m_keyBinds[i]);
        root->InsertEndChild(keyBind);
    }

    doc.SaveFile(ConfigFile);
}

void EditorConfig::Init()
{
    if (Instance == nullptr)
    {
        Instance = new EditorConfig();
        Deserialize();

        EDITORCONFIG_BINDING_FUNCTION_TABLE(RUNTIME_FUNCTION_ATTACH);
    }
}
void EditorConfig::Destroy()
{
    if (Instance != nullptr)
    {
        delete Instance;
        Instance = nullptr;
    }
}

uint32_t EditorConfig::GetEditorUnfocusedFPS()
{
    return Instance->m_editorUnfocusedFPS;
}
void EditorConfig::SetEditorUnfocusedFPS(uint32_t a_fps)
{
    Instance->m_editorUnfocusedFPS = a_fps;
}

bool EditorConfig::GetUseDegrees()
{
    return Instance->m_useDegrees;
}
void EditorConfig::SetUseDegrees(bool a_useDegrees)
{
    Instance->m_useDegrees = a_useDegrees;
}

float EditorConfig::GetEditorMouseSensitivity()
{
    return Instance->m_editorMouseSensitivity;
}
void EditorConfig::SetEditorMouseSensitivity(float a_editorMouseSensitivity)
{
    Instance->m_editorMouseSensitivity = a_editorMouseSensitivity;
}

glm::vec4 EditorConfig::GetBackgroundColor()
{
    return Instance->m_backgroundColor;
}
void EditorConfig::SetBackgroundColor(const glm::vec4& a_backgroundColor)
{
    Instance->m_backgroundColor = a_backgroundColor;
}

e_CodeEditor EditorConfig::GetCodeEditor()
{
    return Instance->m_codeEditor;
}
void EditorConfig::SetCodeEditor(e_CodeEditor a_codeEditor)
{
    Instance->m_codeEditor = a_codeEditor;
}

e_DefEditor EditorConfig::GetDefEditor()
{
    return Instance->m_defEditor;
}
void EditorConfig::SetDefEditor(e_DefEditor a_defEditor)
{
    Instance->m_defEditor = a_defEditor;
}

ImGuiKey EditorConfig::GetKeyBind(e_KeyBindTarget a_keyBind)
{
    return Instance->m_keyBinds[a_keyBind];
}
void EditorConfig::SetKeyBind(e_KeyBindTarget a_keyBind, ImGuiKey a_key)
{
    Instance->m_keyBinds[a_keyBind] = a_key;
}

float EditorConfig::GetEngineShutdownTimeout()
{
    return Instance->m_engineShutdownTimeout;
}
void EditorConfig::SetEngineShutdownTimeout(float a_timeout)
{
    Instance->m_engineShutdownTimeout = a_timeout;
}

float EditorConfig::GetEnginePipeTimeout()
{
    return Instance->m_enginePipeTimeout;
}
void EditorConfig::SetEnginePipeTimeout(float a_timeout)
{
    Instance->m_enginePipeTimeout = a_timeout;
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