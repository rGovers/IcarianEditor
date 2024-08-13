// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#pragma once

#include <cstdint>
#include <functional>
#include <string_view>

class ConsoleWindow;

enum e_LoggerMessageType : uint32_t
{
    LoggerMessageType_Message,
    LoggerMessageType_Warning,
    LoggerMessageType_Error
};

class Logger
{
public:

private:
    static std::vector<ConsoleWindow*> Windows;
protected:

public:
    static void AddConsoleWindow(ConsoleWindow* a_window);
    static void RemoveConsoleWindow(ConsoleWindow* a_window);

    static void Message(const std::string_view& a_string, bool a_editor = true, bool a_print = true);
    static void Warning(const std::string_view& a_string, bool a_editor = true, bool a_print = true);
    static void Error(const std::string_view& a_string, bool a_editor = true, bool a_print = true);
};

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