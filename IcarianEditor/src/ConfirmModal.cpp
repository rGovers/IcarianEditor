// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#include "Modals/ConfirmModal.h"

#include <imgui.h>

ConfirmModal::ConfirmModal(const std::string_view& a_text, ConfirmModalData* a_data) : Modal("Confirm")
{
    m_text = std::string(a_text);

    m_data = a_data;
}
ConfirmModal::~ConfirmModal()
{
    if (m_data != nullptr)
    {
        delete m_data;
    }
}

void ConfirmModal::Submit()
{
    if (m_data != nullptr)
    {
        m_data->Confirm();
    }
}
void ConfirmModal::Cancel()
{
    if (m_data != nullptr)
    {
        m_data->Cancel();
    }
}

bool ConfirmModal::Update()
{
    ImGui::Text("%s", m_text.c_str());

    if (ImGui::Button("Confirm"))
    {  
        Submit();

        return false;
    }
    
    ImGui::SameLine();

    if (ImGui::Button("Cancel"))
    {
        Cancel();

        return false;
    }

    if (ImGui::IsKeyPressed(ImGuiKey_Enter))
    {
        Submit();

        return false;
    }

    if (ImGui::IsKeyPressed(ImGuiKey_Escape))
    {
        Cancel();

        return false;
    }

    return true;
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