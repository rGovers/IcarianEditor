// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#pragma once

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include "Core/InputBindings.h"

class RuntimeFunction;

class EditorInputManager
{
private:
    IcarianCore::KeyboardState m_curKeyState;
    IcarianCore::KeyboardState m_prevKeyState;

    glm::vec2                  m_curPos;

    uint8_t                    m_mouseButton;

    EditorInputManager();
protected:

public:
    ~EditorInputManager();

    static void SetCursorPos(const glm::vec2& a_pos);
    static glm::vec2 GetCursorPos();

    static void SetMouseButton(e_MouseButton a_button, bool a_state);
    static void SetKeyboardKey(e_KeyCode a_keyCode, bool a_state);

    inline bool IsMouseDown(e_MouseButton a_button) const
    {
        return IISBITSET(m_mouseButton, a_button * 2 + 0);
    }
    inline bool IsMousePressed(e_MouseButton a_button) const
    {
        return IISBITSET(m_mouseButton, a_button * 2 + 0) && !IISBITSET(m_mouseButton, a_button * 2 + 1);
    }
    inline bool IsMouseReleased(e_MouseButton a_button) const
    {
        return !IISBITSET(m_mouseButton, a_button * 2 + 0) && IISBITSET(m_mouseButton, a_button * 2 + 1);
    }

    inline bool IsKeyDown(e_KeyCode a_keyCode) const
    {
        return m_curKeyState.IsKeyDown(a_keyCode);
    }
    inline bool IsKeyPressed(e_KeyCode a_keyCode) const
    {
        return m_curKeyState.IsKeyDown(a_keyCode) && !m_prevKeyState.IsKeyDown(a_keyCode);
    }
    inline bool IsKeyReleased(e_KeyCode a_keyCode) const
    {
        return !m_curKeyState.IsKeyDown(a_keyCode) && m_prevKeyState.IsKeyDown(a_keyCode);
    }

    static void Init();
    static void Destroy();
};

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