#pragma once

#define GLM_FORCE_SWIZZLE
#include <glm/glm.hpp>

#include <filesystem>
#include <list>

class FileDialog
{
private:

protected:

public:
    static bool GenerateDirs(std::list<std::filesystem::path>* a_dirs, const std::filesystem::path& a_path);
    static bool GenerateFileDirs(std::list<std::filesystem::path>* a_dirs, std::list<std::filesystem::path>* a_files, const std::filesystem::path& a_path);

    static bool DirectoryExplorer(const std::list<std::filesystem::path>& a_dirs, std::filesystem::path* a_path, const glm::vec2& a_size = glm::vec2(-1.0f, -1.0f));
    static bool FileExplorer(const std::list<std::filesystem::path>& a_dirs, const std::list<std::filesystem::path>& a_files, std::filesystem::path* a_path, std::string* a_name, const char* a_filter = nullptr, const glm::vec2& a_size = glm::vec2(-1.0f, -1.0f));
};