#include "TemplateBuilder.h"

#include <algorithm>

std::string TemplateBuilder::GenerateFromTemplate(const std::string_view& a_str, const std::string_view& a_projectName, const std::string_view& a_fileName)
{
    std::string str = std::string(a_str);

    constexpr std::string_view ProjectNamePlaceHolder = "@ProjectName@";
    constexpr size_t ProjectNamePlaceHolderLen = ProjectNamePlaceHolder.length();

    const size_t projectNameLen = a_projectName.length();

    size_t pos = str.find(ProjectNamePlaceHolder);
    while (pos != std::string::npos)
    {
        str.replace(pos, ProjectNamePlaceHolderLen, a_projectName);
        pos += projectNameLen;
        pos = str.find(ProjectNamePlaceHolder, pos);
    }

    constexpr std::string_view ScriptNamePlaceHolder = "@ScriptName@";
    constexpr size_t ScriptNamePlaceHolderLen = ScriptNamePlaceHolder.length();

    const size_t scriptNameLen = a_fileName.length();

    pos = str.find(ScriptNamePlaceHolder);
    while (pos != std::string::npos)
    {
        str.replace(pos, ScriptNamePlaceHolderLen, a_fileName);
        pos += ScriptNamePlaceHolderLen;
        pos = str.find(ScriptNamePlaceHolder, pos);
    }

    return str;
}