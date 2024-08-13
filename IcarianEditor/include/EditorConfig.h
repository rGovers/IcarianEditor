// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#pragma once

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include <cstdint>
#include <imgui.h>

class RuntimeManager;

enum e_CodeEditor : uint32_t
{
    CodeEditor_Default,
    CodeEditor_VisualStudio,
    CodeEditor_VisualStudioCode,
    CodeEditor_Kate,
    CodeEditor_End
};

enum e_DefEditor : uint32_t
{
    DefEditor_Editor,
    DefEditor_VisualStudioCode,
    DefEditor_Kate,
    DefEditor_End
};

enum e_KeyBindTarget : uint32_t
{
    KeyBindTarget_Null,
    KeyBindTarget_Translate,
    KeyBindTarget_Rotate,
    KeyBindTarget_Scale,

    KeyBindTarget_MoveUp,
    KeyBindTarget_MoveDown,
    KeyBindTarget_CameraModifier,

    KeyBindTarget_End,
    KeyBindTarget_Start = KeyBindTarget_Translate
};

class EditorConfig
{
private:
    static constexpr char ConfigFile[] = "editorConfig.xml";

    bool         m_useDegrees = false;

    glm::vec4    m_backgroundColor = glm::vec4(0.1f, 0.1f, 0.1f, 1.0f);
    float        m_editorMouseSensitivity = 0.01f;

    e_CodeEditor m_codeEditor = CodeEditor_Default;
    e_DefEditor  m_defEditor = DefEditor_Editor;

    ImGuiKey     m_keyBinds[KeyBindTarget_End];

    EditorConfig();

protected:

public:
    ~EditorConfig();

    static void Init(RuntimeManager* a_runtime);
    static void Destroy();

    static bool GetUseDegrees();
    static void SetUseDegrees(bool a_useDegrees);

    static glm::vec4 GetBackgroundColor();
    static void SetBackgroundColor(const glm::vec4& a_backgroundColor);

    static float GetEditorMouseSensitivity();
    static void SetEditorMouseSensitivity(float a_editorMouseSensitivity);

    static e_CodeEditor GetCodeEditor();
    static void SetCodeEditor(e_CodeEditor a_codeEditor);

    static e_DefEditor GetDefEditor();
    static void SetDefEditor(e_DefEditor a_defEditor);

    static ImGuiKey GetKeyBind(e_KeyBindTarget a_keyBind);
    static const char* GetKeyBindName(e_KeyBindTarget a_keyBind);
    static void SetKeyBind(e_KeyBindTarget a_keyBind, ImGuiKey a_key);

    static void Deserialize();
    static void Serialize();
};

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