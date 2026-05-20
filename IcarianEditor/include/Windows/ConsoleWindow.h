// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#pragma once

#include "Window.h"

#include "Logger.h"

struct ConsoleMessage
{
    std::string Message;
    std::vector<std::string> Stacktrace;
    uint32_t Count;
    e_LoggerMessageType Type;
    bool IsEditor;
};

class ConsoleWindow : public Window
{
private:
    // Consider the Logger holding the messages rather then forwarding the messages to the Console window
    // Would be more memory efficient that way as the console window would pull from a global list and only show what is relevant to its context
    // This is better then each console window keeping its own copy of the message history
    // I have not done this yet as with ~1-2 console windows it is more memory efficient for each to hold the message history
    // Yee 'Ol high fixed memory usage with the logger vs low increasing memory usage with each console so need to see use cases
    constexpr static uint32_t MaxMessages = 2048;

    // TODO: Should probably implement a clear on play at some point
    constexpr static uint32_t DisplayMessageBit = 0;
    constexpr static uint32_t DisplayWarningBit = 1;
    constexpr static uint32_t DisplayErrorBit = 2;
    constexpr static uint32_t DisplayEditorBit = 3;
    constexpr static uint32_t CollapseBit = 4;
    constexpr static uint32_t ClearOnReloadBit = 5;

    ConsoleMessage* m_messages;
    uint32_t        m_messageCount;
    uint32_t        m_messageHead;

    uint32_t        m_selectedMessage;

    uint8_t         m_flags;

    void Clear();

    void DrawMessages();
    void DrawStacktrace();

protected:

public:
    ConsoleWindow();
    virtual ~ConsoleWindow();

    void AddMessage(e_LoggerMessageType a_type, const LoggerMessageData& a_msg);

    virtual void Refresh();
    virtual void InternalUpdate(double a_delta) { }
    virtual void DisplayUpdate(double a_delta);
};

// MIT License
// 
// Copyright (c) 2026 River Govers
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