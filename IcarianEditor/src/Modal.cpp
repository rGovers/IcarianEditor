#include "Modals/Modal.h"

#include <imgui.h>

Modal::Modal(const std::string_view& a_displayName, const glm::vec2& a_size)
{
    static uint64_t ID = 0;

    m_size = a_size;

    m_displayName = a_displayName;

    m_idStr = m_displayName + "[" + std::to_string(ID++) + "]";
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