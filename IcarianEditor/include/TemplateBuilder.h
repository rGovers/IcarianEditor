#pragma once

#include <string>

class TemplateBuilder
{
private:

protected:

public:
    TemplateBuilder() = delete;

    static std::string GenerateFromTemplate(const std::string_view& a_str, const std::string_view& a_projectName, const std::string_view& a_fileName);
};