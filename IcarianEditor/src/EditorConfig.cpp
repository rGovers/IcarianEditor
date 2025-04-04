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

static constexpr const char* KeyBindNames[] =
{
    "Null",
    "Translate",
    "Rotate",
    "Scale",
    "MoveUp",
    "MoveDown",
    "CameraModifier"
};

EditorConfig::EditorConfig()
{
    m_keyBinds[KeyBindTarget_Translate] = ImGuiKey_Q;
    m_keyBinds[KeyBindTarget_Rotate] = ImGuiKey_W;
    m_keyBinds[KeyBindTarget_Scale] = ImGuiKey_E;

    m_keyBinds[KeyBindTarget_MoveUp] = ImGuiKey_Space;
    m_keyBinds[KeyBindTarget_MoveDown] = ImGuiKey_LeftShift;
    m_keyBinds[KeyBindTarget_CameraModifier] = ImGuiKey_LeftCtrl;
}
EditorConfig::~EditorConfig()
{
    Serialize();
}

void EditorConfig::Deserialize()
{
    if (std::filesystem::exists(ConfigFile))
    {
        tinyxml2::XMLDocument doc;
        if (doc.LoadFile(ConfigFile) == tinyxml2::XML_SUCCESS)
        {
            tinyxml2::XMLElement* root = doc.FirstChildElement("Config");
            if (root != nullptr)
            {
                for (const tinyxml2::XMLElement* element = root->FirstChildElement(); element != nullptr; element = element->NextSiblingElement())
                {
                    const char* name = element->Name();

                    switch (StringHash(name))
                    {
                    case StringHash("UseDegrees"):
                    {
                        Instance->m_useDegrees = element->BoolText();

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
                        case StringHash("Default"):
                        {
                            Instance->m_codeEditor = CodeEditor_Default;

                            break;
                        }
                        case StringHash("VisualStudioCode"):
                        {
                            Instance->m_codeEditor = CodeEditor_VisualStudioCode;

                            break;
                        }
                        case StringHash("VisualStudio"):
                        {
                            Instance->m_codeEditor = CodeEditor_VisualStudio;

                            break;
                        }
                        case StringHash("Kate"):
                        {
                            Instance->m_codeEditor = CodeEditor_Kate;

                            break;
                        }
                        }

                        break;
                    }
                    case StringHash("DefEditor"):
                    {
                        const char* defEditor = element->GetText();
                        if (strcmp(defEditor, "Editor") == 0)
                        {
                            Instance->m_defEditor = DefEditor_Editor;
                        }
                        else if (strcmp(defEditor, "VisualStudioCode") == 0)
                        {
                            Instance->m_defEditor = DefEditor_VisualStudioCode;
                        }

                        break;
                    }
                    default:
                    {
                        for (uint32_t i = KeyBindTarget_Start; i < KeyBindTarget_End; ++i)
                        {
                            const std::string keyBindName = std::string(KeyBindNames[i]) + "Key";

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

    tinyxml2::XMLElement* useDegrees = doc.NewElement("UseDegrees");
    useDegrees->SetText(Instance->m_useDegrees);
    root->InsertEndChild(useDegrees);

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
    case CodeEditor_VisualStudio:
    {
        codeEditor->SetText("VisualStudio");

        break;
    }
    case CodeEditor_VisualStudioCode:
    {
        codeEditor->SetText("VisualStudioCode");

        break;
    }
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
    case DefEditor_VisualStudioCode:
    {
        defEditor->SetText("VisualStudioCode");

        break;
    }
    default:
    {
        defEditor->SetText("Editor");

        break;
    }
    }
    root->InsertEndChild(defEditor);

    for (uint32_t i = KeyBindTarget_Start; i < KeyBindTarget_End; ++i)
    {
        const std::string keyBindName = std::string(KeyBindNames[i]) + "Key";

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
const char* EditorConfig::GetKeyBindName(e_KeyBindTarget a_keyBind)
{
    return KeyBindNames[a_keyBind];
}
void EditorConfig::SetKeyBind(e_KeyBindTarget a_keyBind, ImGuiKey a_key)
{
    Instance->m_keyBinds[a_keyBind] = a_key;
}

// MIT License
// 
// Copyright (c) 2025 River Govers
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