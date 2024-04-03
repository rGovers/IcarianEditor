#include "Windows/ConsoleWindow.h"

#include <imgui.h>

#include "Datastore.h"
#include "FlareImGui.h"
#include "Logger.h"
#include "Texture.h"

ConsoleWindow::ConsoleWindow() : Window("Console", "Textures/WindowIcons/WindowIcon_Console.png")
{
    Logger::AddConsoleWindow(this);

    m_flags = 0;
    m_flags |= 0b1 << DisplayWarningBit;
    m_flags |= 0b1 << DisplayErrorBit;
    m_flags |= 0b1 << DisplayEditorBit;
    m_flags |= 0b1 << CollapseBit;
}   
ConsoleWindow::~ConsoleWindow()
{
    Logger::RemoveConsoleWindow(this);
}

void ConsoleWindow::AddMessage(const std::string_view& a_message, bool a_editor, e_LoggerMessageType a_type)
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
    msg.Editor = a_editor;
    msg.Type = a_type;
    msg.Count = 1;
    msg.Message = a_message;

    m_messages.emplace_back(msg);
}

struct ConsoleDrawList
{
    Texture* Tex;
    ImVec4 Color;
    uint32_t Count;
    std::string Message;
};

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
    bool displayEditor = (m_flags & 0b1 << DisplayEditorBit) != 0;
    bool collapse = (m_flags & 0b1 << CollapseBit) != 0;

    if (ImGui::Checkbox("Display Editor", &displayEditor))
    {
        if (displayEditor)
        {
            m_flags |= 0b1 << DisplayEditorBit;
        }
        else 
        {
            m_flags &= ~(0b1 << DisplayEditorBit);
        }
    }
    ImGui::SameLine();
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

    Texture* infoTex = Datastore::GetTexture("Textures/Icons/Console_Info.png");
    Texture* warningTex = Datastore::GetTexture("Textures/Icons/Console_Warning.png");
    Texture* errorTex = Datastore::GetTexture("Textures/Icons/Console_Error.png");

    std::vector<ConsoleDrawList> drawLists;

    ImGui::BeginChild("##Messages");
    for (const ConsoleMessage msg : m_messages)
    {
        if (!displayEditor && msg.Editor)
        {
            continue;
        }

        ImVec4 color;
        bool display = false;    
        Texture* tex = nullptr;    

        switch (msg.Type) 
        {
        case LoggerMessageType_Error:
        {
            display = displayError;

            color = ImVec4(1.0, 0.0f, 0.0f, 1.0f);
            tex = errorTex;

            break;
        }
        case LoggerMessageType_Warning:
        {
            display = displayWarning;

            color = ImVec4(1.0, 1.0f, 0.0f, 1.0f);
            tex = warningTex;

            break;
        }
        case LoggerMessageType_Message:
        {
            display = displayMessage;

            color = ImVec4(1.0, 1.0f, 1.0f, 1.0f);
            tex = infoTex;

            break;
        }
        }

        if (display)
        {
            if (collapse)
            {
                const uint32_t count = (uint32_t)drawLists.size();

                if (count <= 0)
                {
                    ConsoleDrawList draw;
                    draw.Tex = tex;
                    draw.Count = msg.Count;
                    draw.Message = msg.Message;
                    draw.Color = color;

                    drawLists.emplace_back(draw);

                    continue;
                }

                ConsoleDrawList& draw = drawLists[count - 1];
                if (draw.Tex == tex && draw.Message == msg.Message)
                {
                    draw.Count += msg.Count;

                    continue;
                }

                ConsoleDrawList newDraw;
                newDraw.Tex = tex;
                newDraw.Count = msg.Count;
                newDraw.Message = msg.Message;
                newDraw.Color = color;

                drawLists.emplace_back(newDraw);
            }
            else
            {
                for (uint32_t i = 0; i < msg.Count; ++i)
                {
                    ConsoleDrawList draw;
                    draw.Tex = tex;
                    draw.Count = 1;
                    draw.Message = msg.Message;
                    draw.Color = color;

                    drawLists.emplace_back(draw);
                }
            }
        }
    }

    const uint32_t count = (uint32_t)drawLists.size();
    if (count > 0)
    {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        const ImVec2 curPos = ImGui::GetCursorScreenPos();
        const float spacing = ImGui::GetStyle().ItemSpacing.y;
        const float lineHeight = 16.0f + spacing;

        const float startYPos = curPos.y - spacing * 0.5f;

        const ImU32 oddColor = ImGui::GetColorU32(ImVec4(0.25f, 0.25f, 0.25f, 0.25f));
        const ImU32 evenColor = ImGui::GetColorU32(ImVec4(0.0f, 0.0f, 0.0f, 0.1f));

        int rowDisplayStart;
        int rowDisplayEnd;
        ImGui::CalcListClipping(count, lineHeight, &rowDisplayStart, &rowDisplayEnd);
        for (int row = rowDisplayStart; row < rowDisplayEnd; ++row)
        {
            ImU32 col;
            if (row & 0b1)
            {
                col = oddColor;
            }
            else
            {
                col = evenColor;
            }

            const float startY = startYPos + (row * lineHeight);
            const float endY = startY + lineHeight;

            drawList->AddRectFilled(ImVec2(curPos.x, startY), ImVec2(curPos.x + ImGui::GetWindowWidth(), endY), col);
        }

        for (uint32_t i = 0; i < count; ++i)
        {
            const ConsoleDrawList& draw = drawLists[i];

            if (draw.Count > 1)
            {
                ImGui::Text("[%d]", draw.Count);
                ImGui::SameLine();
            }
            
            if (draw.Tex != nullptr)
            {
                ImGui::Image(TexToImHandle(draw.Tex), ImVec2(16.0f, 16.0f));
                ImGui::SameLine();
            }

            ImGui::TextColored(draw.Color, "%s", draw.Message.c_str());
        }
    }

    ImGui::EndChild();
}