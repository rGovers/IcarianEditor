// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#include "Modals/Modal.h"

#include <imgui.h>

Modal::Modal(const std::string_view& a_displayName, const glm::vec2& a_size)
{
    static uint64_t ID = 0;

    m_size = a_size;

    m_displayName = a_displayName;

    m_idStr = m_displayName + "##[" + std::to_string(ID++) + "]";
}
Modal::~Modal()
{
    
}

bool Modal::Display()
{
    if (m_size.x > 0 && m_size.y > 0)
    {
        ImGui::SetNextWindowSize({ m_size.x, m_size.y });
    }

    const char* name = m_idStr.c_str();
    bool ret = true;

    if (ImGui::BeginPopupModal(name, nullptr, ImGuiWindowFlags_NoResize))
    {
        ret = Update();

        ImGui::EndPopup();
    }

    ImGui::OpenPopup(name);

    if (!ret)
    {
        ImGui::CloseCurrentPopup();
    }

    return ret;
}

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