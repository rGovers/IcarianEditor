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