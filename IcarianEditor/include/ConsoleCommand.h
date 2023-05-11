#pragma once

#include <cstdint>
#include <string>

class ConsoleCommand
{
private:
	static constexpr uint32_t BufferSize = 2048;

	std::string m_command;

protected:

public:
	ConsoleCommand(const std::string_view& a_command);
	~ConsoleCommand();

	std::string Run(const std::string* a_args, uint32_t a_argCount);
};