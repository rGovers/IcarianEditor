#include "Windows/Window.h"

#include <cstdint>
#include <imgui.h>
#include <imgui_internal.h>

#include "Datastore.h"
#include "Flare/IcarianDefer.h"
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
                    const ImGuiStyle& style = ImGui::GetStyle();

                    const ImVec2 offset = style.FramePadding;

                    const ImRect titleBarRect = window->TitleBarRect();

                    const float titleBarHeight = titleBarRect.Max.y - titleBarRect.Min.y;

                    const float size = titleBarHeight - (offset.y * 2.0f);

                    ImGui::PushClipRect(titleBarRect.Min, titleBarRect.Max, false);
                    IDEFER(ImGui::PopClipRect());
                    if (dockNode != nullptr)
                    {
                        const ImRect r = window->DockTabItemRect;

                        const ImVec2 basePos = ImVec2(r.Min.x - titleBarRect.Min.x, 0.0f);

                        ImGui::SetCursorPos(ImVec2(basePos.x + offset.x, basePos.y + offset.y));
                    }
                    else
                    {
                        ImGui::SetCursorPos(offset);
                    }

                    ImGui::Image((ImTextureID)texture->GetHandle(), ImVec2(size, size));
                }                
            }
            
            Update(a_delta);
        }

        ImGui::End();
    }

    return m_open;
}