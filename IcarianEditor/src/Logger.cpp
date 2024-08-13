// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#include "Logger.h"
 
#include <iostream>

#include "Windows/ConsoleWindow.h"

std::vector<ConsoleWindow*> Logger::Windows = std::vector<ConsoleWindow*>();

void Logger::AddConsoleWindow(ConsoleWindow* a_window)
{
    Windows.emplace_back(a_window);
}
void Logger::RemoveConsoleWindow(ConsoleWindow* a_window)
{
    for (auto iter = Windows.begin(); iter != Windows.end(); ++iter)
    {
        if (*iter == a_window)
        {
            Windows.erase(iter);

            return;
        }
    }
}

void Logger::Message(const std::string_view& a_string, bool a_editor, bool a_print)
{
    if (a_print)
    {
        std::cout << "ILM: " << a_string << "\n";
    }

    for (ConsoleWindow* console : Windows)
    {
        console->AddMessage(a_string, a_editor, LoggerMessageType_Message);
    }
}
void Logger::Warning(const std::string_view& a_string, bool a_editor, bool a_print)
{
    if (a_print)
    {
        std::cout << "ILW: " << a_string << "\n";
    }

    for (ConsoleWindow* console : Windows)
    {
        console->AddMessage(a_string, a_editor, LoggerMessageType_Warning);
    }
}
void Logger::Error(const std::string_view& a_string, bool a_editor, bool a_print)
{
    if (a_print)
    {
        std::cout << "ILE: " << a_string << "\n";
    }

    for (ConsoleWindow* console : Windows)
    {
        console->AddMessage(a_string, a_editor, LoggerMessageType_Error);
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