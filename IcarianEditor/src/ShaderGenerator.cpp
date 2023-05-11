#include "ShaderGenerator.h"

#include <vector>

#include "ShaderBuffers.h"

static std::vector<std::string> SplitArgs(const std::string_view& a_string)
{
	std::vector<std::string> args;

	std::size_t pos = 0;

	while (true)
	{
		while (a_string[pos] == ' ')
		{
			++pos;
		}

		const std::size_t sPos = a_string.find(',', pos);
		if (sPos == std::string_view::npos)
		{
			args.emplace_back(a_string.substr(pos));

			break;
		}

		args.emplace_back(a_string.substr(pos, sPos - pos));
		pos = sPos + 1;
	}
	
	return args;
}

std::string GLSL_FromFShader(const std::string_view& a_str)
{
    std::string shader = std::string(a_str);

    std::size_t pos = 0;
    while (true)
    {
        const std::size_t sPos = shader.find("#!", pos);
		if (sPos == std::string::npos)
		{
			break;
		}

        const std::size_t sAPos = shader.find("(", sPos + 1);
		const std::size_t eAPos = shader.find(')', sPos + 1);

        const std::string defName = shader.substr(sPos + 2, sAPos - sPos - 2);
		std::vector<std::string> args = SplitArgs(shader.substr(sAPos + 1, eAPos - sAPos - 1));

        std::string rStr;
		if (defName == "structure")
		{
			if (args[0] == "CameraBuffer")
			{
				rStr = GLSL_UNIFORM_STRING(args[1], args[3], GLSL_CAMERA_SHADER_STRUCTURE);
			}
			else if (args[0] == "TimeBuffer")
			{
				rStr = GLSL_UNIFORM_STRING(args[1], args[3], GLSL_TIME_SHADER_STRUCTURE);
			}
		}
		else if (defName == "pushbuffer")
		{
			if (args[0] == "ModelBuffer")
			{
				rStr = GLSL_PUSHBUFFER_STRING(args[1], GLSL_MODEL_SHADER_STRUCTURE);
			}
		}

        std::size_t next = 1;
		if (!rStr.empty())
		{
			next = rStr.size();
		}

        shader.replace(sPos, eAPos - sPos + 1, rStr);

		pos = sPos + next;
    }

    return shader;
}