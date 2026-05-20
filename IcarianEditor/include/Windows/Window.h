// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#pragma once

#include <cstdint>
#include <string>

class Window
{
private:
    static constexpr uint32_t CloseBit = 0;
    static constexpr uint32_t MenuBarBit = 1;

    static constexpr const char* WindowPadding = "   ";

    std::string m_texturePath;
    std::string m_idStr;
    std::string m_displayName;

    uint8_t     m_flags;

protected:

public:
    Window(const std::string_view& a_displayName, const std::string_view& a_texturePath = "", bool a_menubar = false);
    virtual ~Window();

    virtual bool RequiresProject() const
    {
        return true;
    }

    void CloseWindow();

    bool Display(double a_delta);

    virtual void Refresh() { }
    virtual void InternalUpdate(double a_delta) = 0;
    virtual void DisplayUpdate(double a_delta) = 0;
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
