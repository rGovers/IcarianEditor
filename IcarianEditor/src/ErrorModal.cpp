// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#include "Modals/ErrorModal.h"

#include <imgui.h>

#include "Logger.h"

ErrorModal::ErrorModal(const std::string_view& a_error) : Modal("Error", glm::vec2(ImGui::CalcTextSize(a_error.data()).x + 20.0f, 80.0f))
{
    m_error = std::string(a_error);

    Logger::Error(m_error);
}
ErrorModal::~ErrorModal()
{

}

bool ErrorModal::Update()
{
    ImGui::Text("%s", m_error.c_str());

    if (ImGui::Button("OK"))
    {
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