// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#pragma once

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include <cstdint>
#include <imgui.h>

class RuntimeManager;

#define I_INTER_CODEEDITORTABLE(F) \
    F(Default) \
    F(VisualStudio) \
    F(VisualStudioCode) \
    F(VSCodium) \
    F(Kate) \

#define I_INTER_CODEEDITOR_ENUM(val) CodeEditor_##val,
#define I_INTER_CODEEDITOR_STR(val) case CodeEditor_##val: { return #val; }

enum e_CodeEditor : uint32_t
{
    I_INTER_CODEEDITORTABLE(I_INTER_CODEEDITOR_ENUM)

    CodeEditor_End
};

#define I_INTER_DEFEDITORTABLE(F) \
    F(Editor) \
    F(VisualStudioCode) \
    F(VSCodium) \
    F(Kate) \

#define I_INTER_DEFEDITOR_ENUM(val) DefEditor_##val,
#define I_INTER_DEFEDITOR_STR(val) case DefEditor_##val: { return #val; }

enum e_DefEditor : uint32_t
{
    I_INTER_DEFEDITORTABLE(I_INTER_DEFEDITOR_ENUM)

    DefEditor_End
};

#define I_INTER_KEYBINDTARGETTABLE(F) \
    F(Null, ImGuiKey_None) \
    F(Translate, ImGuiKey_Q) \
    F(Rotate, ImGuiKey_W) \
    F(Scale, ImGuiKey_E) \
    F(AmbientLightMode, ImGuiKey_I) \
    F(ViewportLightMode, ImGuiKey_O) \
    F(SceneLightMode, ImGuiKey_P) \
    F(MoveForward, ImGuiKey_W) \
    F(MoveBackward, ImGuiKey_S) \
    F(MoveLeft, ImGuiKey_A) \
    F(MoveRight, ImGuiKey_D) \
    F(MoveUp, ImGuiKey_Space) \
    F(MoveDown, ImGuiKey_LeftShift) \
    F(CameraModifier, ImGuiKey_LeftCtrl) \

#define I_INTER_KEYBINDTARGET_ENUM(val, def) KeyBindTarget_##val,
#define I_INTER_KEYBINDTARGET_STR(val, def) case KeyBindTarget_##val: { return #val; }

enum e_KeyBindTarget : uint32_t
{
    I_INTER_KEYBINDTARGETTABLE(I_INTER_KEYBINDTARGET_ENUM)

    KeyBindTarget_End,
    KeyBindTarget_Start = KeyBindTarget_Translate
};

class EditorConfig
{
private:
    static constexpr char ConfigFile[] = "editorConfig.xml";

    bool         m_useDegrees = false;

    float        m_engineShutdownTimeout = 5.0f;
    float        m_enginePipeTimeout = 5.0f;

    uint32_t     m_editorUnfocusedFPS = 10;

    glm::vec4    m_backgroundColor = glm::vec4(0.1f, 0.1f, 0.1f, 1.0f);
    float        m_editorMouseSensitivity = 0.01f;

    e_CodeEditor m_codeEditor = CodeEditor_Default;
    e_DefEditor  m_defEditor = DefEditor_Editor;

    ImGuiKey     m_keyBinds[KeyBindTarget_End];

    EditorConfig();

protected:

public:
    ~EditorConfig();

    static void Init();
    static void Destroy();

    static uint32_t GetEditorUnfocusedFPS();
    static void SetEditorUnfocusedFPS(uint32_t a_fps);

    static bool GetUseDegrees();
    static void SetUseDegrees(bool a_useDegrees);

    static glm::vec4 GetBackgroundColor();
    static void SetBackgroundColor(const glm::vec4& a_backgroundColor);

    static float GetEditorMouseSensitivity();
    static void SetEditorMouseSensitivity(float a_editorMouseSensitivity);

    static e_CodeEditor GetCodeEditor();
    constexpr static const char* GetCodeEditorName(e_CodeEditor a_codeEditor)
    {
        switch (a_codeEditor)
        {
        I_INTER_CODEEDITORTABLE(I_INTER_CODEEDITOR_STR)
        default:
        {
            break;
        }
        }

        return "Unknown";
    }
    static void SetCodeEditor(e_CodeEditor a_codeEditor);

    static e_DefEditor GetDefEditor();
    constexpr static const char* GetDefEditorName(e_DefEditor a_defEditor)
    {
        switch (a_defEditor)
        {
        I_INTER_DEFEDITORTABLE(I_INTER_DEFEDITOR_STR)
        default:
        {
            break;
        }
        }

        return "Unknown";
    }
    static void SetDefEditor(e_DefEditor a_defEditor);

    static ImGuiKey GetKeyBind(e_KeyBindTarget a_keyBind);
    constexpr static const char* GetKeyBindName(e_KeyBindTarget a_keyBind)
    {
        switch (a_keyBind)
        {
        I_INTER_KEYBINDTARGETTABLE(I_INTER_KEYBINDTARGET_STR)
        default:
        {
            break;
        }
        }

        return "Unknown";
    }
    static void SetKeyBind(e_KeyBindTarget a_keyBind, ImGuiKey a_key);

    static float GetEngineShutdownTimeout();
    static void SetEngineShutdownTimeout(float a_timeoutSec);

    static float GetEnginePipeTimeout();
    static void SetEnginePipeTimeout(float a_timeoutSec);

    static void Deserialize();
    static void Serialize();
};

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