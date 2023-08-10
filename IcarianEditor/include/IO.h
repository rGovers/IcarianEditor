#pragma once

#include <filesystem>
#include <string>

class IO
{
private:

protected:

public:
    static std::filesystem::path GetHomePath();
    static bool ValidatePathName(const std::string_view& a_name);

    static void OpenFileExplorer(const std::filesystem::path& a_path);
    // TODO: Need to improve with version that has default application.
    static void OpenFile(const std::filesystem::path& a_path);
};