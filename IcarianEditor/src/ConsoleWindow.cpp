// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#include "Windows/ConsoleWindow.h"

#include <imgui.h>

#include "Core/Bitfield.h"
#include "Core/IcarianDefer.h"
#include "Core/IcarianLambda.h"
#include "Datastore.h"
#include "FlareImGui.h"
#include "Logger.h"
#include "Texture.h"

ConsoleWindow::ConsoleWindow() : Window("Console", "Textures/WindowIcons/WindowIcon_Console.png")
{
    Logger::AddConsoleWindow(this);

    m_messages = new ConsoleMessage[MaxMessages];

    m_messageCount = 0;
    m_messageHead = 0;
    m_selectedMessage = -1;

    m_flags = 0;
    ISETBIT(m_flags, DisplayMessageBit);
    ISETBIT(m_flags, DisplayWarningBit);
    ISETBIT(m_flags, DisplayErrorBit);
    ISETBIT(m_flags, CollapseBit);
}
ConsoleWindow::~ConsoleWindow()
{
    delete[] m_messages;

    Logger::RemoveConsoleWindow(this);
}

void ConsoleWindow::AddMessage(e_LoggerMessageType a_type, const LoggerMessageData& a_msg)
{
    if (m_messageCount > 0)
    {
        const uint32_t prevIndex = (m_messageHead + (m_messageCount - 1)) % MaxMessages;
        ConsoleMessage& msg = m_messages[prevIndex];

        if (msg.Type != a_type)
        {
            goto EndLookup;
        }

        const uint32_t stacktraceSize = (uint32_t)a_msg.Stacktrace.size();
        const uint32_t msgStacktraceSize = (uint32_t)msg.Stacktrace.size();
        if (msgStacktraceSize != stacktraceSize)
        {
            goto EndLookup;
        }

        if (msg.Message != a_msg.Message)
        {
            goto EndLookup;
        }

        for (uint32_t i = 0; i < stacktraceSize; ++i)
        {
            if (msg.Stacktrace[i] != a_msg.Stacktrace[i])
            {
                goto EndLookup;
            }
        }

        ++msg.Count;

        return;
    }

EndLookup:;

    const uint32_t currentIndex = (m_messageHead + m_messageCount) % MaxMessages;

    const ConsoleMessage msg =
    {
        .Message = a_msg.Message,
        .Stacktrace = a_msg.Stacktrace,
        .Count = 1,
        .Type = a_type,
        .IsEditor = a_msg.IsEditor,
    };

    m_messages[currentIndex] = msg;

    if (m_messageCount >= MaxMessages)
    {
        if (m_messageHead == m_selectedMessage)
        {
            m_selectedMessage = uint32_t(-1);
        }

        ++m_messageHead;
    }
    else 
    {
        ++m_messageCount;
    }
}

void ConsoleWindow::Clear()
{
    m_messageCount = 0;
    m_messageHead = 0;
    m_selectedMessage = -1;
}

struct ConsoleDrawList
{
    std::string Message;
    const Texture* Tex;
    ImVec4 Color;
    uint32_t Count;
    uint32_t Index;
};

void ConsoleWindow::DrawMessages()
{
    const ImVec2 size = ILAMBDA(
    {
        if (m_selectedMessage != uint32_t(-1))
        {
            const ImVec2 winSize = ImGui::GetContentRegionAvail();

            ILRETURN ImVec2(0.0f, glm::max(winSize.y * 0.5f, winSize.y - 128.0f));
        }

        ILRETURN ImVec2(0.0f, 0.0f);
    });

    ImGui::BeginChild("##Messages", size);
    IDEFER(ImGui::EndChild());

    const Texture* infoTex = Datastore::GetTexture("Textures/Icons/Console_Info.png");
    const Texture* warningTex = Datastore::GetTexture("Textures/Icons/Console_Warning.png");
    const Texture* errorTex = Datastore::GetTexture("Textures/Icons/Console_Error.png");

    std::vector<ConsoleDrawList> drawLists;

    for (uint32_t i = 0; i < m_messageCount; ++i)
    {
        const uint32_t currentIndex = (m_messageHead + i) % MaxMessages;
        const ConsoleMessage& msg = m_messages[currentIndex];

        if (!IISBITSET(m_flags, DisplayEditorBit) && msg.IsEditor)
        {
            continue;
        }

        const bool display = ILAMBDA(
        {
            switch (msg.Type) 
            {
            case LoggerMessageType_Message:
            {
                ILRETURN IISBITSET(m_flags, DisplayMessageBit);
            }
            case LoggerMessageType_Warning:
            {
                ILRETURN IISBITSET(m_flags, DisplayWarningBit);
            }
            case LoggerMessageType_Error:
            {
                ILRETURN IISBITSET(m_flags, DisplayErrorBit);
            }
            }

            ILRETURN false;
        });

        if (!display)
        {
            continue;
        }

        const ImVec4 color = ILAMBDA(
        {
            switch (msg.Type) 
            {
            case LoggerMessageType_Message:
            {
                ILRETURN ImVec4(1.0, 1.0f, 1.0f, 1.0f);
            }
            case LoggerMessageType_Warning:
            {
                ILRETURN ImVec4(1.0, 1.0f, 0.0f, 1.0f);
            }
            case LoggerMessageType_Error:
            {
                ILRETURN ImVec4(1.0, 0.0f, 0.0f, 1.0f);
            }
            }

            ILRETURN ImVec4();
        });

        const Texture* tex = ILAMBDA(
        {
            switch (msg.Type) 
            {
            case LoggerMessageType_Message:
            {
                ILRETURN infoTex;
            }
            case LoggerMessageType_Warning:
            {
                ILRETURN warningTex;
            }
            case LoggerMessageType_Error:
            {
                ILRETURN errorTex;
            }
            }

            ILRETURN (const Texture*)nullptr;
        });

        if (IISBITSET(m_flags, CollapseBit))
        {
            const uint32_t count = (uint32_t)drawLists.size();

            const bool empty = count <= 0;
            if (empty)
            {
                const ConsoleDrawList draw =
                {
                    .Message = msg.Message,
                    .Tex = tex,
                    .Color = color,
                    .Count = msg.Count,
                    .Index = currentIndex,
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
                .Message = msg.Message,
                .Tex = tex,
                .Color = color,
                .Count = msg.Count,
                .Index = currentIndex,
            };

            drawLists.emplace_back(newDraw);
        }
        else
        {
            for (uint32_t i = 0; i < msg.Count; ++i)
            {
                const ConsoleDrawList draw =
                {
                    .Message = msg.Message,
                    .Tex = tex,
                    .Color = color,
                    .Count = 1,
                    .Index = currentIndex,
                };

                drawLists.emplace_back(draw);
            }
        }
    }

    const uint32_t count = (uint32_t)drawLists.size();
    const bool empty = count <= 0;
    if (empty)
    {
        return;
    }

    ImDrawList* drawList = ImGui::GetWindowDrawList();

    const ImGuiStyle& style = ImGui::GetStyle();

    const ImVec2 curPos = ImGui::GetCursorScreenPos();
    const float spacing = style.ItemSpacing.y;
    const float lineHeight = 16.0f + spacing;

    const float startYPos = curPos.y - spacing * 0.5f;

    const ImU32 oddColor = ImGui::GetColorU32(ImVec4(0.25f, 0.25f, 0.25f, 0.25f));
    const ImU32 evenColor = ImGui::GetColorU32(ImVec4(0.0f, 0.0f, 0.0f, 0.1f));

    ImGuiListClipper clipper;
    clipper.Begin((int)count, lineHeight);
    while (clipper.Step())
    {
        for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; ++row)
        {
            const ImU32 col = ILAMBDA(
            {
                const bool isOdd = row & 0b1;
                if (isOdd)
                {
                    ILRETURN oddColor;
                }

                ILRETURN evenColor;
            });

            const float startY = startYPos + (row * lineHeight);
            const float endY = startY + lineHeight;

            // Should probably do better but using a stupid number so that it does not get cutoff in theory
            drawList->AddRectFilled(ImVec2(curPos.x, startY), ImVec2(curPos.x + 10000000.0f, endY), col, 0);

            const ConsoleDrawList& draw = drawLists[row];

            ImGui::PushID((int)draw.Index);
            IDEFER(ImGui::PopID());

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

            ImGui::PushStyleColor(ImGuiCol_Text, draw.Color);
            IDEFER(ImGui::PopStyleColor());

            std::string str;
            str.reserve(draw.Message.size());

            // Can probably do better then this but turn it into a single line string
            // We currently do not handle multiline strings very well
            for (const char c : draw.Message)
            {
                switch (c) 
                {
                case '\n':
                {
                    str += ' ';

                    break;
                }
                default:
                {
                    str += c;

                    break;
                }
                }
            }

            const char* cStr = str.c_str();
            const ImVec2 strSize = ImGui::CalcTextSize(cStr);

            const bool selected = m_selectedMessage == draw.Index;
            if (ImGui::Selectable(cStr, selected, 0, ImVec2(strSize.x, 0.0f)))
            {
                m_selectedMessage = draw.Index;
            }
        }
    }
}
void ConsoleWindow::DrawStacktrace()
{
    if (m_selectedMessage == uint32_t(-1))
    {
        return;
    }

    ImGui::BeginChild("##Stacktrace");
    IDEFER(ImGui::EndChild());

    const ConsoleMessage& msg = m_messages[m_selectedMessage];

    const uint32_t stackTraceSize = (uint32_t)msg.Stacktrace.size();
    if (stackTraceSize <= 0)
    {
        ImGui::Text("No stacktrace available");

        return;
    }

    for (uint32_t i = 0; i < stackTraceSize; ++i)
    {
        ImGui::Text("[%d] %s", i, msg.Stacktrace[i].c_str());
    }
}

void ConsoleWindow::Refresh()
{
    if (!IISBITSET(m_flags, ClearOnReloadBit))
    {
        return;
    }

    Clear();
}

void ConsoleWindow::Update(double a_delta)
{
    if (ImGui::Button("Clear"))
    {
        Clear();
    }

    ImGui::SameLine();

    bool displayMessage = IISBITSET(m_flags, DisplayMessageBit);
    bool displayWarning = IISBITSET(m_flags, DisplayWarningBit);
    bool displayError = IISBITSET(m_flags, DisplayErrorBit);
    bool displayEditor = IISBITSET(m_flags, DisplayEditorBit);
    bool collapse = IISBITSET(m_flags, CollapseBit);
    bool clearOnRefresh = IISBITSET(m_flags, ClearOnReloadBit);

    ImGui::Text("Display Editor");
    ImGui::SameLine();
    if (ImGui::Checkbox("##DisplayEditor", &displayEditor))
    {
        ITOGGLEBIT(displayEditor, m_flags, DisplayEditorBit);
    }

    ImGui::SameLine();

    ImGui::Text("Display Message");
    ImGui::SameLine();
    if (ImGui::Checkbox("##DisplayMessage", &displayMessage))
    {
        ITOGGLEBIT(displayMessage, m_flags, DisplayMessageBit);
    }

    ImGui::SameLine();

    ImGui::Text("Display Warning");
    ImGui::SameLine();
    if (ImGui::Checkbox("##DisplayWarning", &displayWarning))
    {
        ITOGGLEBIT(displayWarning, m_flags, DisplayWarningBit);
    }

    ImGui::SameLine();

    ImGui::Text("Display Error");
    ImGui::SameLine();
    if (ImGui::Checkbox("##DisplayError", &displayError))
    {
        ITOGGLEBIT(displayError, m_flags, DisplayErrorBit);
    }

    ImGui::SameLine();

    ImGui::Text("Collapse");
    ImGui::SameLine();
    if (ImGui::Checkbox("##Collapse", &collapse))
    {
        ITOGGLEBIT(collapse, m_flags, CollapseBit);
    }

    ImGui::SameLine();

    ImGui::Text("Clear On Reload");
    ImGui::SameLine();
    if (ImGui::Checkbox("##ClearOnReload", &clearOnRefresh))
    {
        ITOGGLEBIT(clearOnRefresh, m_flags, ClearOnReloadBit);
    }

    DrawMessages();
    DrawStacktrace();
}

// MIT License
// 
// Copyright (c) 2025 River Govers
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