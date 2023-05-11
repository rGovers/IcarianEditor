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

void Logger::Message(const std::string_view& a_string, bool a_print)
{
    if (a_print)
    {
        std::cout << "FL: " << a_string << "\n";
    }

    for (ConsoleWindow* console : Windows)
    {
        console->AddMessage(a_string, LoggerMessageType_Message);
    }
}
void Logger::Warning(const std::string_view& a_string, bool a_print)
{
    if (a_print)
    {
        std::cout << "FL: " << a_string << "\n";
    }

    for (ConsoleWindow* console : Windows)
    {
        console->AddMessage(a_string, LoggerMessageType_Warning);
    }
}
void Logger::Error(const std::string_view& a_string, bool a_print)
{
    if (a_print)
    {
        std::cout << "FL: " << a_string << "\n";
    }

    for (ConsoleWindow* console : Windows)
    {
        console->AddMessage(a_string, LoggerMessageType_Error);
    }
}