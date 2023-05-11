#include "Windows/ConsoleWindow.h"

#include <imgui.h>

#include "Datastore.h"
#include "Logger.h"
#include "Texture.h"

ConsoleWindow::ConsoleWindow() : Window("Console")
{
    Logger::AddConsoleWindow(this);

    m_flags = 0;
    m_flags |= 0b1 << DisplayWarningBit;
    m_flags |= 0b1 << DisplayErrorBit;
    m_flags |= 0b1 << CollapseBit;
}   
ConsoleWindow::~ConsoleWindow()
{
    Logger::RemoveConsoleWindow(this);
}

void ConsoleWindow::AddMessage(const std::string_view& a_message, e_LoggerMessageType a_type)
{
    if (m_messages.size() > MaxMessages)
    {
        m_messages.erase(m_messages.begin());
    }

    if (!m_messages.empty())
    {
        ConsoleMessage& msg = *--m_messages.end();   

        if (msg.Type == a_type && msg.Message == a_message)
        {
            ++msg.Count;

            return;
        }
    }

    ConsoleMessage msg;
    msg.Type = a_type;
    msg.Count = 1;
    msg.Message = a_message;

    m_messages.emplace_back(msg);
}

void ConsoleWindow::Update(double a_delta)
{
    if (ImGui::Button("Clear"))
    {
        m_messages.clear();
    }

    ImGui::SameLine();

    bool displayMessage = (m_flags & 0b1 << DisplayMessageBit) != 0;
    bool displayWarning = (m_flags & 0b1 << DisplayWarningBit) != 0;
    bool displayError = (m_flags & 0b1 << DisplayErrorBit) != 0;
    bool collapse = (m_flags & 0b1 << CollapseBit) != 0;

    if (ImGui::Checkbox("Display Message", &displayMessage))
    {
        if (displayMessage)
        {
            m_flags |= 0b1 << DisplayMessageBit;
        }
        else
        {
            m_flags &= ~(0b1 << DisplayMessageBit);
        }
    }
    ImGui::SameLine();
    if (ImGui::Checkbox("Display Warning", &displayWarning))
    {
        if (displayWarning)
        {
            m_flags |= 0b1 << DisplayWarningBit;
        }
        else
        {
            m_flags &= ~(0b1 << DisplayWarningBit);
        }
    }
    ImGui::SameLine();
    if (ImGui::Checkbox("Display Error", &displayError))
    {
        if (displayError)
        {
            m_flags |= 0b1 << DisplayErrorBit;
        }
        else
        {
            m_flags &= ~(0b1 << DisplayErrorBit);
        }
    }
    ImGui::SameLine();
    if (ImGui::Checkbox("Collapse", &collapse))
    {
        if (collapse)
        {
            m_flags |= 0b1 << CollapseBit;
        }
        else
        {
            m_flags &= ~(0b1 << CollapseBit);
        }
    }

    const Texture* infoTex = Datastore::GetTexture("Textures/Icons/Console_Info.png");
    const Texture* warningTex = Datastore::GetTexture("Textures/Icons/Console_Warning.png");
    const Texture* errorTex = Datastore::GetTexture("Textures/Icons/Console_Error.png");

    ImGui::BeginChild("##Messages");
    for (const ConsoleMessage msg : m_messages)
    {
        ImVec4 color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
        bool display = displayMessage;

        const Texture* tex = infoTex;

        switch (msg.Type)
        {
        case LoggerMessageType_Error:
        {
            color = ImVec4(1.0, 0.0f, 0.0f, 1.0f);
            display = displayError;

            tex = errorTex;

            break;
        }
        case LoggerMessageType_Warning:
        {   
            color = ImVec4(1.0, 1.0f, 0.0f, 1.0f);
            display = displayWarning;

            tex = warningTex;

            break;
        }
        }

        if (display)
        {
            if (collapse)
            {
                if (msg.Count > 1)
                {
                    const std::string c = "[" + std::to_string(msg.Count)  + "]";

                    ImGui::Text(c.c_str());

                    ImGui::SameLine();
                }

                if (tex != nullptr)
                {
                    ImGui::Image((ImTextureID)tex->GetHandle(), { 16, 16 });
                    ImGui::SameLine();
                }

                ImGui::TextColored(color, msg.Message.c_str());
            }
            else
            {
                for (uint32_t i = 0; i < msg.Count; ++i)
                {
                    if (tex != nullptr)
                    {
                        ImGui::Image((ImTextureID)tex->GetHandle(), { 16, 16 });
                        ImGui::SameLine();
                    }

                    ImGui::TextColored(color, msg.Message.c_str());
                }
            }
        }
    }
    ImGui::EndChild();
}