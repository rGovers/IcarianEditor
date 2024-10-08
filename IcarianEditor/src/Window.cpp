// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#include "Windows/Window.h"

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include <cstdint>
#include <imgui.h>
#include <imgui_internal.h>

#include "Core/IcarianDefer.h"
#include "Datastore.h"
#include "FlareImGui.h"
#include "Texture.h"

Window::Window(const std::string_view& a_displayName, const std::string_view& a_texturePath)
{
    static uint64_t ID = 0;

    m_open = true;

    m_displayName = std::string(a_displayName);
    m_texturePath = std::string(a_texturePath);
    // The spaces are required to make the window title bar look nice.
    // Trying other methods was breaking stuff in ImGui.
    m_idStr = "   " + m_displayName + "##[" + std::to_string(ID++) + "]";
}
Window::~Window()
{

}

bool Window::Display(double a_delta)
{
    if (m_open)
    {
        if (ImGui::Begin(m_idStr.c_str(), &m_open))
        {
            // That is a hack and a half to get the icon to display in the title bar.
            if (!m_texturePath.empty())
            {
                const Texture* texture = Datastore::GetTexture(m_texturePath);

                if (texture != nullptr)
                {
                    const ImGuiWindow* window = ImGui::GetCurrentWindow();
                    const ImGuiDockNode* dockNode = window->DockNode;
                    ImDrawList* drawList = ImGui::GetWindowDrawList();
                    const ImGuiStyle& style = ImGui::GetStyle();

                    const glm::vec2 windowPos = glm::vec2(window->Pos.x, window->Pos.y);
                    const glm::vec2 offset = glm::vec2(style.FramePadding.x, style.FramePadding.y);

                    const ImRect titleBarRect = window->TitleBarRect();
                    const float titleBarHeight = titleBarRect.Max.y - titleBarRect.Min.y;

                    const float size = titleBarHeight - (offset.y * 2.0f);

                    ImGui::PushClipRect(titleBarRect.Min, titleBarRect.Max, false);
                    IDEFER(ImGui::PopClipRect());
                    
                    if (dockNode != nullptr)
                    {
                        const ImRect r = window->DockTabItemRect;

                        const glm::vec2 basePos = glm::vec2(r.Min.x - titleBarRect.Min.x, 0.0f);

                        const glm::vec2 startRect = windowPos + basePos + offset;
                        const glm::vec2 endRect = startRect + glm::vec2(size);

                        drawList->AddImage(TexToImHandle(texture), ImVec2(startRect.x, startRect.y), ImVec2(endRect.x, endRect.y));
                    }
                    else
                    {
                        const glm::vec2 startRect = windowPos + offset;
                        const glm::vec2 endRect = startRect + glm::vec2(size);

                        drawList->AddImage(TexToImHandle(texture), ImVec2(startRect.x, startRect.y), ImVec2(endRect.x, endRect.y));
                    }
                }                
            }
            
            Update(a_delta);
        }

        ImGui::End();
    }

    return m_open;
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