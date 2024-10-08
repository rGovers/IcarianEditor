// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#include "FileDialog.h"

#include <filesystem>
#include <imgui.h>

#include "Datastore.h"
#include "FlareImGui.h"
#include "Texture.h"

static bool IDirectoryExplorer(const std::list<std::filesystem::path>& a_dirs, std::filesystem::path* a_path)
{
    const Texture* folderTex = Datastore::GetTexture("Textures/WindowIcons/WindowIcon_AssetBrowser.png");

    for (const std::filesystem::path& dir : a_dirs)
    {
        if (folderTex != nullptr)
        {
            ImGui::Image(TexToImHandle(folderTex), ImVec2(16.0f, 16.0f));

            ImGui::SameLine();
        }

        // C string seems to be a bit wonky so using another var seems to fix crash
        const std::string name = dir.filename().string();
        if (ImGui::Selectable(name.c_str()))
        {
            *a_path /= dir.filename();

            return false;
        }

        ImGui::NextColumn();
    }

    return true;
}

bool FileDialog::GenerateDirs(std::list<std::filesystem::path>* a_dirs, const std::filesystem::path& a_path)
{
    if (std::filesystem::exists(a_path))
    {   
        auto fsIter = std::filesystem::directory_iterator(a_path, std::filesystem::directory_options::skip_permission_denied);

        for (const auto& iter : fsIter)
        {
            if (iter.is_directory())
            {
                a_dirs->emplace_back(iter.path());
            }
        }

        a_dirs->sort();

        return true;
    }

    return false;
}
bool FileDialog::GenerateFileDirs(std::list<std::filesystem::path>* a_dirs, std::list<std::filesystem::path>* a_files, const std::filesystem::path& a_path)
{
    if (std::filesystem::exists(a_path))
    {   
        auto fsIter = std::filesystem::directory_iterator(a_path, std::filesystem::directory_options::skip_permission_denied);

        for (const auto& iter : fsIter)
        {
            if (iter.is_directory())
            {
                a_dirs->emplace_back(iter.path());
            }
            else if (iter.is_regular_file())
            {
                a_files->emplace_back(iter.path());
            }
        }

        a_dirs->sort();
        a_files->sort();

        return true;
    }

    return false;
}

bool FileDialog::DirectoryExplorer(const std::list<std::filesystem::path>& a_dirs, std::filesystem::path* a_path, const glm::vec2& a_size)
{
    const ImGuiStyle& style = ImGui::GetStyle();
    const ImVec2 region = ImGui::GetContentRegionAvail();

    ImVec2 size = ImVec2(a_size.x, a_size.y);
    if (size.x < 0.0f)
    {
        size.x = region.x - style.FramePadding.x;
    }
    if (size.y < 0.0f)
    {
        size.y = region.y - ImGui::GetFrameHeightWithSpacing() * 2;
    }

    bool ret = true;

    if (ImGui::BeginChild("Dir", size))
    {
        if (std::filesystem::exists(*a_path))
        {
            ImGui::Columns(glm::max(1, (int)(size.x / 150.0f)));
            ImGui::BeginGroup();

            ret = IDirectoryExplorer(a_dirs, a_path);

            ImGui::EndGroup();
            ImGui::Columns();
        }

        ImGui::EndChild();
    }

    return ret;
}
bool FileDialog::FileExplorer(const std::list<std::filesystem::path>& a_dirs, const std::list<std::filesystem::path>& a_files, std::filesystem::path* a_path, std::string* a_name, const char* a_filter, const glm::vec2& a_size)
{
    const ImGuiStyle& style = ImGui::GetStyle();
    const ImVec2 region = ImGui::GetContentRegionAvail();

    ImVec2 size = ImVec2(a_size.x, a_size.y);
    if (size.x < 0.0f)
    {
        size.x = region.x - style.FramePadding.x;
    }
    if (size.y < 0.0f)
    {
        size.y = region.y - ImGui::GetFrameHeightWithSpacing() * 2;
    }

    bool ret = true;

    if (ImGui::BeginChild("Dir", size))
    {
        if (std::filesystem::exists(*a_path))
        {
            const int columns = glm::max(1, (int)(size.x / 150.0f));

            ImGui::Columns(columns);
            ImGui::BeginGroup();

            ret = IDirectoryExplorer(a_dirs, a_path);

            ImGui::EndGroup();
            ImGui::Columns();

            ImGui::Separator();

            ImGui::Columns(columns);
            ImGui::BeginGroup();

            for (const std::filesystem::path& path : a_files)
            {
                if (a_filter != nullptr)
                {
                    if (path.extension() != a_filter)
                    {
                        continue;
                    }
                }

                const std::string name = path.filename().string();
                if (ImGui::Selectable(name.c_str()))
                {
                    *a_name = name;
                }

                ImGui::NextColumn();
            }

            ImGui::EndGroup();
            ImGui::Columns();
        }

        ImGui::EndChild();
    }

    return ret;
}

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