// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#include "Windows/ConsoleWindow.h"

#include <imgui.h>

#include "Core/Bitfield.h"
#include "Core/IcarianDefer.h"
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
    const bool full = m_messages.size() > MaxMessages;
    if (full)
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

    const ConsoleMessage msg =
    {
        .Message = std::string(a_message),
        .Count = 1,
        .Type = a_type,
        .Editor = a_editor
    };

    m_messages.emplace_back(msg);
}

struct ConsoleDrawList
{
    const Texture* Tex;
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

    bool displayMessage = IISBITSET(m_flags, DisplayMessageBit);
    bool displayWarning = IISBITSET(m_flags, DisplayWarningBit);
    bool displayError = IISBITSET(m_flags, DisplayErrorBit);
    bool displayEditor = IISBITSET(m_flags, DisplayEditorBit);
    bool collapse = IISBITSET(m_flags, CollapseBit);

    if (ImGui::Checkbox("Display Editor", &displayEditor))
    {
        ITOGGLEBIT(displayEditor, m_flags, DisplayEditorBit);
    }
    ImGui::SameLine();
    if (ImGui::Checkbox("Display Message", &displayMessage))
    {
        ITOGGLEBIT(displayMessage, m_flags, DisplayMessageBit);
    }
    ImGui::SameLine();
    if (ImGui::Checkbox("Display Warning", &displayWarning))
    {
        ITOGGLEBIT(displayWarning, m_flags, DisplayWarningBit);
    }
    ImGui::SameLine();
    if (ImGui::Checkbox("Display Error", &displayError))
    {
        ITOGGLEBIT(displayError, m_flags, DisplayErrorBit);
    }
    ImGui::SameLine();
    if (ImGui::Checkbox("Collapse", &collapse))
    {
        ITOGGLEBIT(collapse, m_flags, CollapseBit);
    }

    const Texture* infoTex = Datastore::GetTexture("Textures/Icons/Console_Info.png");
    const Texture* warningTex = Datastore::GetTexture("Textures/Icons/Console_Warning.png");
    const Texture* errorTex = Datastore::GetTexture("Textures/Icons/Console_Error.png");

    std::vector<ConsoleDrawList> drawLists;

    ImGui::BeginChild("##Messages");
    IDEFER(ImGui::EndChild());
    for (const ConsoleMessage& msg : m_messages)
    {
        if (!displayEditor && msg.Editor)
        {
            continue;
        }

        ImVec4 color;
        bool display = false;    
        const Texture* tex = nullptr;    

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

                const bool empty = count <= 0;
                if (empty)
                {
                    const ConsoleDrawList draw =
                    {
                        .Tex = tex,
                        .Color = color,
                        .Count = msg.Count,
                        .Message = msg.Message
                    };

                    drawLists.emplace_back(draw);

                    continue;
                }

                ConsoleDrawList& draw = drawLists[count - 1];
                if (draw.Tex == tex && draw.Message == msg.Message)
                {
                    draw.Count += msg.Count;

                    continue;
                }

                const ConsoleDrawList newDraw =
                {
                    .Tex = tex,
                    .Color = color,
                    .Count = msg.Count,
                    .Message = msg.Message
                };

                drawLists.emplace_back(newDraw);
            }
            else
            {
                for (uint32_t i = 0; i < msg.Count; ++i)
                {
                    const ConsoleDrawList draw =
                    {
                        .Tex = tex,
                        .Color = color,
                        .Count = 1,
                        .Message = msg.Message
                    };

                    drawLists.emplace_back(draw);
                }
            }
        }
    }

    const uint32_t count = (uint32_t)drawLists.size();
    const bool empty = count <= 0;
    if (!empty)
    {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        const ImGuiStyle& style = ImGui::GetStyle();
        const ImVec2 curPos = ImGui::GetCursorScreenPos();
        const float spacing = style.ItemSpacing.y;
        const float lineHeight = 16.0f + spacing;
        const float windowWidth = ImGui::GetWindowWidth();

        const float startYPos = curPos.y - spacing * 0.5f;

        const ImU32 oddColor = ImGui::GetColorU32(ImVec4(0.25f, 0.25f, 0.25f, 0.25f));
        const ImU32 evenColor = ImGui::GetColorU32(ImVec4(0.0f, 0.0f, 0.0f, 0.1f));

        ImGuiListClipper clipper;
        clipper.Begin((int)count, lineHeight);
        while (clipper.Step())
        {
            for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; ++row)
            {
                ImU32 col;
                const bool isOdd = row & 0b1;
                if (isOdd)
                {
                    col = oddColor;
                }
                else
                {
                    col = evenColor;
                }

                const float startY = startYPos + (row * lineHeight);
                const float endY = startY + lineHeight;

                drawList->AddRectFilled(ImVec2(curPos.x, startY), ImVec2(curPos.x + windowWidth, endY), col, 0);

                const ConsoleDrawList& draw = drawLists[row];

                const bool multiItem = draw.Count > 1;
                if (multiItem)
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
    }
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