#pragma once

#include <string>

class IO
{
private:

protected:

public:
    static std::string GetHomePath();
    static bool ValidatePathName(const std::string_view& a_name);
};