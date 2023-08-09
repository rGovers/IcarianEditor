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

bool ConfirmModal::Update()
{
    ImGui::Text("%s", m_text.c_str());

    if (ImGui::Button("Confirm"))
    {
        if (m_data != nullptr)
        {
            m_data->Confirm();
        }

        return false;
    }
    
    ImGui::SameLine();

    if (ImGui::Button("Cancel"))
    {
        if (m_data != nullptr)
        {
            m_data->Cancel();
        }

        return false;
    }

    return true;
}