// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

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

// MIT License
// 
// Copyright (c) 2024 River Govers
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.