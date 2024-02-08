#pragma once

#include <filesystem>
#include <string>
#include <vector>

class IO
{
private:

protected:

public:
    static std::filesystem::path GetHomePath();
    static bool ValidatePathName(const std::string_view& a_name);

    static std::filesystem::path GetRelativePath(const std::filesystem::path& a_relative, const std::filesystem::path& a_path);

    static std::filesystem::path GetCSCPath();

    static std::vector<std::filesystem::path> GetDrives();
    static std::vector<std::filesystem::path> GetUserDirectories();

    static void OpenFileExplorer(const std::filesystem::path& a_path);
    static void OpenFile(const std::filesystem::path& a_path);
    static void OpenFile(const std::string_view& a_application, const std::filesystem::path& a_path);
    static void StartOpenFile(const std::string_view& a_application, const std::filesystem::path& a_path);
};