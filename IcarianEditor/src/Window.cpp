#include "Windows/Window.h"

#include <cstdint>
#include <imgui.h>

Window::Window(const std::string_view& a_displayName)
{
    static uint64_t ID = 0;

    m_open = true;

    m_displayName = a_displayName;
    m_idStr = m_displayName + "[" + std::to_string(ID++) + "]";
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
            Update(a_delta);
        }

        ImGui::End();
    }

    return m_open;
}