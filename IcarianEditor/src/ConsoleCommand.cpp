#include "ConsoleCommand.h"

#include "Logger.h"

ConsoleCommand::ConsoleCommand(const std::string_view& a_command)
{
	m_command = std::string(a_command);
}
ConsoleCommand::~ConsoleCommand()
{

}

std::string ConsoleCommand::Run(const std::string* a_args, uint32_t a_argCount)
{
#if WIN32
#define popen _popen
#define pclose _pclose
#endif

	std::string cmdStr = m_command;
	for (uint32_t i = 0; i < a_argCount; ++i)
	{
		cmdStr += " " + a_args[i];
	}

	FILE* pipe = popen(cmdStr.c_str(), "r");
	std::string ret;

	char buffer[BufferSize];

	try
	{
		while (fgets(buffer, BufferSize, pipe) != NULL)
		{
			ret += buffer;
		}
	}
	catch (std::exception e)
	{
		Logger::Error(std::string("Error running command: ") + e.what());
	}

	pclose(pipe);

	return ret;
}