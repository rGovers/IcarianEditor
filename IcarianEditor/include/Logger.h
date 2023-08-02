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