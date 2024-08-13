// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#pragma once

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include <cstdint>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>
#include <string_view>

#include "EngineInputInteropStructures.h"

enum e_Cursors
{
    Cursor_Arrow,
    Cursor_Hand,
    Cursor_HResize,
    Cursor_VResize,
    Cursor_Move,
    Cursor_Last
};

class Application
{
private:
    bool        m_maximized;

    int         m_xPosState;
    int         m_yPosState;

    GLFWwindow* m_window;

    GLFWcursor* m_cursors[Cursor_Last];

protected:

public:
    Application(uint32_t a_width, uint32_t a_height, const std::string_view& a_title);
    virtual ~Application();

    void Run();

    bool IsFocused() const;

    bool IsMaximized() const;
    void Maximize(bool a_state);

    void Minimize() const;

    glm::vec2 GetWindowSize() const;
    void SetWindowSize(const glm::vec2& a_size);

    void SetCursorState(e_CursorState a_state);
    void SetCursor(e_Cursors a_cursor);

    glm::vec2 GetCursorPos() const;
    glm::vec2 GetMousePos() const;

    float GetDPI() const;

    glm::vec2 GetWindowPos() const;
    void SetWindowPos(const glm::vec2& a_pos);

    virtual void Update(double a_delta, double a_time) = 0;

    inline GLFWwindow* GetWindow() const
    {
        return m_window;
    }

    void SetTitle(const std::string_view& a_title) const;

    void Close() const;
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