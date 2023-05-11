#pragma once

#include <filesystem>
#include <list>

class FileDialog
{
private:

protected:

public:
    static bool GenerateDirs(std::list<std::filesystem::path>* a_dirs, const std::filesystem::path& a_path);
    static bool GenerateFileDirs(std::list<std::filesystem::path>* a_dirs, std::list<std::filesystem::path>* a_files, const std::filesystem::path& a_path);

    static bool DirectoryExplorer(const std::list<std::filesystem::path>& a_dirs, std::filesystem::path* a_path);
    static bool FileExplorer(const std::list<std::filesystem::path>& a_dirs, const std::list<std::filesystem::path>& a_files, std::filesystem::path* a_path, std::string* a_name);
};