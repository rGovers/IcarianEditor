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
    ImGui::Text(m_error.c_str());

    if (ImGui::Button("OK"))
    {
        return false;
    }

    return true;
}